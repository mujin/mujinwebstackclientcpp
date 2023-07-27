// -*- coding: utf-8 -*-
// Copyright (C) 2012-2013 MUJIN Inc. <rosen.diankov@mujin.co.jp>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <mujinwebstackclientcpp/webstackclient.h>

#include <boost/algorithm/string.hpp>
#include <boost/scope_exit.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/make_shared.hpp>
#include <strstream>

#include "common.h"
#include "logging.h"

#define SKIP_PEER_VERIFICATION // temporary
//#define SKIP_HOSTNAME_VERIFICATION

MUJIN_LOGGER("mujin.webstackcpp");

#define CURL_OPTION_SAVER(curl, curlopt, curvalue) boost::shared_ptr<void> __curloptionsaver ## curlopt((void*)0, boost::bind(boost::function<CURLcode(CURL*, CURLoption, decltype(curvalue))>(curl_easy_setopt), curl, curlopt, curvalue))
#define CURL_OPTION_SETTER(curl, curlopt, newvalue) CHECKCURLCODE(curl_easy_setopt(curl, curlopt, newvalue), "curl_easy_setopt " # curlopt)
#define CURL_OPTION_SAVE_SETTER(curl, curlopt, curvalue, newvalue) CURL_OPTION_SAVER(curl, curlopt, curvalue); CURL_OPTION_SETTER(curl, curlopt, newvalue)
#define CURL_INFO_GETTER(curl, curlinfo, outvalue) CHECKCURLCODE(curl_easy_getinfo(curl, curlinfo, outvalue), "curl_easy_getinfo " # curlinfo)
#define CURL_PERFORM(curl) CHECKCURLCODE(curl_easy_perform(curl), "curl_easy_perform")
#define CURL_FORM_RELEASER(form) boost::shared_ptr<void> __curlformreleaser ## form((void*)0, boost::bind(boost::function<void(decltype(form))>(curl_formfree), form))

namespace mujinwebstackclient {

using namespace mujinjsonwebstack;

namespace {

template <typename T>
std::wstring ParseWincapsWCNPath(const T& sourcefilename, const boost::function<std::string(const T&)>& ConvertToFileSystemEncoding)
{
    // scenefilenames is the WPJ file, so have to open it up to see what directory it points to
    // note that the encoding is utf-16
    // <clsProject Object="True">
    //   <WCNPath VT="8">.\threegoaltouch\threegoaltouch.WCN;</WCNPath>
    // </clsProject>
    // first have to get the raw utf-16 data
#if defined(_WIN32) || defined(_WIN64)
    std::ifstream wpjfilestream(sourcefilename.c_str(), std::ios::binary|std::ios::in);
#else
    // linux doesn't mix ifstream and wstring
    std::ifstream wpjfilestream(ConvertToFileSystemEncoding(sourcefilename).c_str(), std::ios::binary|std::ios::in);
#endif
    if( !wpjfilestream ) {
        throw MUJIN_EXCEPTION_FORMAT("failed to open file %s", ConvertToFileSystemEncoding(sourcefilename), MEC_InvalidArguments);
    }
    std::wstringstream utf16stream;
    bool readbom = false;
    while(!wpjfilestream.eof() ) {
        unsigned short c;
        wpjfilestream.read(reinterpret_cast<char*>(&c),sizeof(c));
        if( !wpjfilestream ) {
            break;
        }
        // skip the first character (BOM) due to a bug in boost property_tree (should be fixed in 1.49)
        if( readbom || c != 0xfeff ) {
            utf16stream << static_cast<wchar_t>(c);
        }
        else {
            readbom = true;
        }
    }
    boost::property_tree::wptree wpj;
    boost::property_tree::read_xml(utf16stream, wpj);
    boost::property_tree::wptree& clsProject = wpj.get_child(L"clsProject");
    boost::property_tree::wptree& WCNPath = clsProject.get_child(L"WCNPath");
    std::wstring strWCNPath = WCNPath.data();
    if( strWCNPath.size() > 0 ) {
        // post process the string to get the real filesystem directory
        if( strWCNPath.at(strWCNPath.size()-1) == L';') {
            strWCNPath.resize(strWCNPath.size()-1);
        }

        if( strWCNPath.size() >= 2 && (strWCNPath[0] == L'.' && strWCNPath[1] == L'\\') ) {
            // don't need the prefix
            strWCNPath = strWCNPath.substr(2);
        }
    }

    return strWCNPath;
}

}  // end namespace

WebstackClient::WebstackClient(const std::string& usernamepassword, const std::string& baseuri, const std::string& proxyserverport, const std::string& proxyuserpw, int options, double timeout)
{
    BOOST_ASSERT( !baseuri.empty() );
    size_t usernameindex = 0;
    usernameindex = usernamepassword.find_first_of(':');
    BOOST_ASSERT(usernameindex != std::string::npos );
    _username = usernamepassword.substr(0,usernameindex);
    std::string password = usernamepassword.substr(usernameindex+1);

    _httpheadersjson = NULL;
    _httpheadersstl = NULL;
    _httpheadersmultipartformdata = NULL;
    _baseuri = baseuri;
    // ensure trailing slash
    if( _baseuri[_baseuri.size()-1] != '/' ) {
        _baseuri.push_back('/');
    }
    _baseapiuri = _baseuri + std::string("api/v1/");
    // hack for now since webdav server and api server could be running on different ports
    if( boost::algorithm::ends_with(_baseuri, ":8000/") || (options&0x80000000) ) {
        // testing on localhost, however the webdav server is running on port 80...
        _basewebdavuri = str(boost::format("%s/u/%s/")%_baseuri.substr(0,_baseuri.size()-6)%_username);
    }
    else {
        _basewebdavuri = str(boost::format("%su/%s/")%_baseuri%_username);
    }

    //CURLcode code = curl_global_init(CURL_GLOBAL_SSL|CURL_GLOBAL_WIN32);
    _curl = curl_easy_init();
    BOOST_ASSERT(!!_curl);

#ifdef _DEBUG
    // CURL_OPTION_SETTER(_curl, CURLOPT_VERBOSE, 1L);
#endif
    _errormessage.resize(CURL_ERROR_SIZE);
    CURL_OPTION_SETTER(_curl, CURLOPT_ERRORBUFFER, &_errormessage[0]);

#ifdef SKIP_PEER_VERIFICATION
    /*
     * if you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the ca bundle you have, you can skip the
     * verification of the server's certificate. this makes the connection
     * a lot less secure.
     *
     * if you have a ca cert for the server stored someplace else than in the
     * default bundle, then the curlopt_capath option might come handy for
     * you.
     */
    CURL_OPTION_SETTER(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    CURL_OPTION_SETTER(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    if( proxyserverport.size() > 0 ) {
        SetProxy(proxyserverport, proxyuserpw);
    }

    CURL_OPTION_SETTER(_curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    CURL_OPTION_SETTER(_curl, CURLOPT_USERPWD, usernamepassword.c_str());

    // need to set the following?
    //CURLOPT_USERAGENT
    //CURLOPT_TCP_KEEPIDLE
    //CURLOPT_TCP_KEEPALIVE
    //CURLOPT_TCP_KEEPINTVL

    CURL_OPTION_SETTER(_curl, CURLOPT_COOKIEFILE, ""); // just to start the cookie engine

    // save everything to _buffer, neceesary to do it before first POST/GET calls or data will be output to stdout
    // these should be set on individual calls
    // CURL_OPTION_SETTER(_curl, CURLOPT_WRITEFUNCTION, _WriteStringStreamCallback); // just to start the cookie engine
    // CURL_OPTION_SETTER(_curl, CURLOPT_WRITEDATA, &_buffer);

    std::string useragent = std::string("controllerclientcpp/")+MUJINWEBSTACKCLIENT_VERSION_STRING;
    CURL_OPTION_SETTER(_curl, CURLOPT_USERAGENT, useragent.c_str());

    CURL_OPTION_SETTER(_curl, CURLOPT_FOLLOWLOCATION, 1L); // we can always follow redirect now, we don't need to detect login page
    CURL_OPTION_SETTER(_curl, CURLOPT_MAXREDIRS, 10L);
    CURL_OPTION_SETTER(_curl, CURLOPT_NOSIGNAL, 1L);

    CURL_OPTION_SETTER(_curl, CURLOPT_POSTFIELDSIZE, 0L);
    CURL_OPTION_SETTER(_curl, CURLOPT_POSTFIELDS, NULL);

    // csrftoken can be any non-empty string
    _csrfmiddlewaretoken = "csrftoken";
    std::string cookie = "Set-Cookie: csrftoken=" + _csrfmiddlewaretoken;
#if CURL_AT_LEAST_VERSION(7,60,0)
    // with https://github.com/curl/curl/commit/b8d5036ec9b702d6392c97a6fc2e141d6c7cce1f, setting domain param to cookie is required.
    if(_baseuri.find('/') == _baseuri.size()-1) {
        // _baseuri should be hostname with trailing slash
        cookie += "; domain=";
        cookie += _baseuri.substr(0,_baseuri.size()-1);
    } else {
        CURLU *url = curl_url();
        BOOST_SCOPE_EXIT_ALL(&url) {
            curl_url_cleanup(url);
        };
        CHECKCURLUCODE(curl_url_set(url, CURLUPART_URL, _baseuri.c_str(), 0), "cannot parse url");
        char *host = NULL;
        BOOST_SCOPE_EXIT_ALL(&host) {
            if(host) {
                curl_free(host);
            }
        };
        CHECKCURLUCODE(curl_url_get(url, CURLUPART_HOST, &host, 0), "cannot determine hostname from url");
        cookie += "; domain=";
        cookie += host;
    }
#endif
    CURL_OPTION_SETTER(_curl, CURLOPT_COOKIELIST, cookie.c_str());

    _charset = "utf-8";
    _language = "en-us";
#if defined(_WIN32) || defined(_WIN64)
    UINT codepage = GetACP();
    std::map<int, std::string>::const_iterator itcodepage = encoding::GetCodePageMap().find(codepage);
    if( itcodepage != encoding::GetCodePageMap().end() ) {
        _charset = itcodepage->second;
    }
#endif
    MUJIN_LOG_INFO("setting character set to " << _charset);
    _SetupHTTPHeadersJSON();
    _SetupHTTPHeadersSTL();
    _SetupHTTPHeadersMultipartFormData();
}

WebstackClient::~WebstackClient()
{
    if( !!_httpheadersjson ) {
        curl_slist_free_all(_httpheadersjson);
    }
    if( !!_httpheadersstl ) {
        curl_slist_free_all(_httpheadersstl);
    }
    if( !!_httpheadersmultipartformdata ) {
        curl_slist_free_all(_httpheadersmultipartformdata);
    }
    curl_easy_cleanup(_curl);
}

void WebstackClient::SetCharacterEncoding(const std::string& newencoding)
{
    boost::mutex::scoped_lock lock(_mutex);
    _charset = newencoding;
    _SetupHTTPHeadersJSON();
    // the following two format does not need charset
    // _SetupHTTPHeadersSTL();
    // _SetupHTTPHeadersMultipartFormData();
}

void WebstackClient::SetLanguage(const std::string& language)
{
    boost::mutex::scoped_lock lock(_mutex);
    if (language!= "") {
        _language = language;
    }
    _SetupHTTPHeadersJSON();
    // the following two format does not need language
    // _SetupHTTPHeadersSTL();
    // _SetupHTTPHeadersMultipartFormData();
}

void WebstackClient::SetUserAgent(const std::string& userAgent)
{
    CURL_OPTION_SETTER(_curl, CURLOPT_USERAGENT, userAgent.c_str());
}

void WebstackClient::SetAdditionalHeaders(const std::vector<std::string>& additionalHeaders)
{
    boost::mutex::scoped_lock lock(_mutex);
    _additionalHeaders = additionalHeaders;
    _SetupHTTPHeadersJSON();
    _SetupHTTPHeadersSTL();
    _SetupHTTPHeadersMultipartFormData();
}

const std::string& WebstackClient::GetUserName() const
{
    return _username;
}

const std::string& WebstackClient::GetBaseURI() const
{
    return _baseuri;
}

void WebstackClient::SetProxy(const std::string& serverport, const std::string& userpw)
{
    // mutally exclusive with unix endpoint settings
    CURL_OPTION_SETTER(_curl, CURLOPT_UNIX_SOCKET_PATH, NULL);
    CURL_OPTION_SETTER(_curl, CURLOPT_PROXY, serverport.c_str());
    CURL_OPTION_SETTER(_curl, CURLOPT_PROXYUSERPWD, userpw.c_str());
}

void WebstackClient::SetUnixEndpoint(const std::string& unixendpoint)
{
    // mutually exclusive with proxy settings
    CURL_OPTION_SETTER(_curl, CURLOPT_PROXY, NULL);
    CURL_OPTION_SETTER(_curl, CURLOPT_PROXYUSERPWD, NULL);
    CURL_OPTION_SETTER(_curl, CURLOPT_UNIX_SOCKET_PATH, unixendpoint.c_str());
}

void WebstackClient::RestartServer(double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    _uri = _baseuri + std::string("restartserver/");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, _uri.c_str());
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POST, 0L, 1L);
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( http_code != 200 ) {
        throw MUJIN_EXCEPTION_FORMAT0("Failed to restart server, please try again or contact MUJIN support", MEC_HTTPServer);
    }
}

void WebstackClient::ExecuteGraphQuery(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResult, rapidjson::Document::AllocatorType& rAlloc, double timeout)
{
    _ExecuteGraphQuery(operationName, query, rVariables, rResult, rAlloc, timeout, true, false);
}

void WebstackClient::ExecuteGraphQueryRaw(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResult, rapidjson::Document::AllocatorType& rAlloc, double timeout)
{
    _ExecuteGraphQuery(operationName, query, rVariables, rResult, rAlloc, timeout, false, true);
}

void WebstackClient::_ExecuteGraphQuery(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResult, rapidjson::Document::AllocatorType& rAlloc, double timeout, bool checkForErrors, bool returnRawResponse)
{
    rResult.SetNull(); // zero output

    rapidjson::Document rResultDoc(&rAlloc);

    {
        boost::mutex::scoped_lock lock(_mutex);

        rapidjson::StringBuffer& rRequestStringBuffer = _rRequestStringBufferCache;
        rRequestStringBuffer.Clear();

        {
            // use the callers allocator to construct the request body
            rapidjson::Value rRequest, rValue;
            rRequest.SetObject();
            rValue.SetString(operationName, rAlloc);
            rRequest.AddMember(rapidjson::Document::StringRefType("operationName"), rValue, rAlloc);
            rValue.SetString(query, rAlloc);
            rRequest.AddMember(rapidjson::Document::StringRefType("query"), rValue, rAlloc);
            rValue.CopyFrom(rVariables, rAlloc);
            rRequest.AddMember(rapidjson::Document::StringRefType("variables"), rValue, rAlloc);

            rapidjson::Writer<rapidjson::StringBuffer> writer(rRequestStringBuffer);
            rRequest.Accept(writer);
        }

        _uri = _baseuri + "api/v2/graphql";
        _CallPost(_uri, rRequestStringBuffer.GetString(), rResultDoc, 200, timeout);
    }

    // parse response
    if (!rResultDoc.IsObject()) {
        throw MUJIN_EXCEPTION_FORMAT("Execute graph query does not return valid response \"%s\", invalid response: %s", operationName%mujinjsonwebstack::DumpJson(rResultDoc), MEC_HTTPServer);
    }

    if (checkForErrors) {
        // look for errors in response
        const rapidjson::Value::ConstMemberIterator itErrors = rResultDoc.FindMember("errors");
        if (itErrors != rResultDoc.MemberEnd() && itErrors->value.IsArray() && itErrors->value.Size() > 0) {
            MUJIN_LOG_VERBOSE(str(boost::format("graph query has errors \"%s\": %s")%operationName%mujinjsonwebstack::DumpJson(rResultDoc)));
            for (rapidjson::Value::ConstValueIterator itError = itErrors->value.Begin(); itError != itErrors->value.End(); ++itError) {
                const rapidjson::Value& rError = *itError;
                if (rError.IsObject() && rError.HasMember("message") && rError["message"].IsString()) {
                    const char* errorCode = "unknown";
                    const rapidjson::Value::ConstMemberIterator itExtensions = rError.FindMember("extensions");
                    if (itExtensions != rError.MemberEnd() && itExtensions->value.IsObject() && itExtensions->value.HasMember("errorCode") && itExtensions->value["errorCode"].IsString()) {
                        errorCode = itExtensions->value["errorCode"].GetString();
                    }
                    throw mujinwebstackclient::MujinGraphQueryError(boost::str(boost::format("[%s:%d] graph query has errors \"%s\": %s")%(__PRETTY_FUNCTION__)%(__LINE__)%operationName%rError["message"].GetString()), errorCode);
                }
            }
            throw MUJIN_EXCEPTION_FORMAT("graph query has undefined errors \"%s\": %s", operationName%mujinjsonwebstack::DumpJson(rResultDoc), MEC_HTTPServer);
        }
    }

    // should have data member
    if (!rResultDoc.HasMember("data")) {
        throw MUJIN_EXCEPTION_FORMAT("Execute graph query does not have 'data' field in \"%s\", invalid response: %s", operationName%mujinjsonwebstack::DumpJson(rResultDoc), MEC_HTTPServer);
    }

    // set output
    if (returnRawResponse) {
        rResult.Swap(rResultDoc);
    } else {
        rResult = rResultDoc["data"];
    }
}

std::string WebstackClient::GetVersion()
{
    if (!_profile.IsObject()) {
        _profile.SetObject();
        CallGet("profile/", _profile);
    }
    return GetJsonValueByKey<std::string>(_profile, "version");
}

void WebstackClient::CancelAllJobs()
{
    CallDelete("job/?format=json", 204);
}

void WebstackClient::DownloadFileFromControllerIfModifiedSince_UTF8(const std::string& desturi, long localtimeval, long& remotetimeval, std::vector<unsigned char>& vdata, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _DownloadFileFromController(_PrepareDestinationURI_UTF8(desturi, false), localtimeval, remotetimeval, vdata, timeout);
}

void WebstackClient::DownloadFileFromControllerIfModifiedSince_UTF16(const std::wstring& desturi, long localtimeval, long& remotetimeval, std::vector<unsigned char>& vdata, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _DownloadFileFromController(_PrepareDestinationURI_UTF16(desturi, false), localtimeval, remotetimeval, vdata, timeout);
}

void WebstackClient::_DownloadFileFromController(const std::string& desturi, long localtimeval, long &remotetimeval, std::vector<unsigned char>& outputdata, double timeout)
{
    remotetimeval = 0;

    // ask for remote file time
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_FILETIME, 0L, 1L);

    // use if modified since if local file time is provided
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE, localtimeval > 0 ? CURL_TIMECOND_IFMODSINCE : CURL_TIMECOND_NONE);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEVALUE, 0L, localtimeval > 0 ? localtimeval : 0L);

    // do the get call
    long http_code = _CallGet(desturi, outputdata, 0, timeout);
    if ((http_code != 200 && http_code != 304)) {
        if (outputdata.size() > 0) {
            std::stringstream ss;
            ss.write((const char*)&outputdata[0], outputdata.size());
            throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s: %s", desturi%http_code%ss.str(), MEC_HTTPServer);
        }
        throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s", desturi%http_code, MEC_HTTPServer);
    }

    // retrieve remote file time
    if (http_code != 304) {
        // got the entire file so fill in the timestamp of that file
        CURL_INFO_GETTER(_curl, CURLINFO_FILETIME, &remotetimeval);
    }
}

void WebstackClient::ModifySceneAddReferenceObjectPK(const std::string &scenepk, const std::string &referenceobjectpk, double timeout)
{
    rapidjson::Document pt, pt2;
    rapidjson::Value value;

    pt.SetObject();

    value.SetString(scenepk.c_str(), pt.GetAllocator());
    pt.AddMember("scenepk", value, pt.GetAllocator());

    value.SetString(referenceobjectpk.c_str(), pt.GetAllocator());
    pt.AddMember("referenceobjectpk", value, pt.GetAllocator());

    boost::mutex::scoped_lock lock(_mutex);
    _CallPost(_baseuri + "referenceobjectpks/add/", DumpJson(pt), pt2, 200, timeout);
}

void WebstackClient::ModifySceneRemoveReferenceObjectPK(const std::string &scenepk, const std::string &referenceobjectpk, double timeout)
{
    rapidjson::Document pt, pt2;
    rapidjson::Value value;

    pt.SetObject();

    value.SetString(scenepk.c_str(), pt.GetAllocator());
    pt.AddMember("scenepk", value, pt.GetAllocator());

    value.SetString(referenceobjectpk.c_str(), pt.GetAllocator());
    pt.AddMember("referenceobjectpk", value, pt.GetAllocator());

    boost::mutex::scoped_lock lock(_mutex);
    _CallPost(_baseuri + "referenceobjectpks/remove/", DumpJson(pt), pt2, 200, timeout);
}

const std::string& WebstackClient::GetDefaultTaskType()
{
    return _defaulttasktype;
}

std::string WebstackClient::GetScenePrimaryKeyFromURI_UTF8(const std::string& uri)
{
    size_t index = uri.find(":/");
    if (index == std::string::npos) {
        throw MUJIN_EXCEPTION_FORMAT("bad URI: %s", uri, MEC_InvalidArguments);
    }
    return EscapeString(uri.substr(index+2));
}

std::string WebstackClient::GetScenePrimaryKeyFromURI_UTF16(const std::wstring& uri)
{
    std::string utf8line;
    utf8::utf16to8(uri.begin(), uri.end(), std::back_inserter(utf8line));
    return GetScenePrimaryKeyFromURI_UTF8(utf8line);
}

std::string WebstackClient::GetNameFromPrimaryKey_UTF8(const std::string& pk)
{
    return UnescapeString(pk);
}

std::wstring WebstackClient::GetNameFromPrimaryKey_UTF16(const std::string& pk)
{
    std::string utf8 = GetNameFromPrimaryKey_UTF8(pk);
    std::wstring utf16;
    utf8::utf8to16(utf8.begin(), utf8.end(), std::back_inserter(utf16));
    return utf16;
}

std::string WebstackClient::CreateObjectGeometry(const std::string& objectPk, const std::string& geometryName, const std::string& linkPk, const std::string& geomtype, double timeout)
{
    rapidjson::Document pt(rapidjson::kObjectType);
    const std::string geometryData("{\"name\":\"" + geometryName + "\", \"linkpk\":\"" + linkPk + "\", \"geomtype\": \"" + geomtype + "\"}");
    const std::string uri(str(boost::format("object/%s/geometry/") % objectPk));

    CallPost(uri, geometryData, pt, 201, timeout);
    return GetJsonValueByKey<std::string>(pt, "pk");
}

std::string WebstackClient::SetObjectGeometryMesh(const std::string& objectPk, const std::string& geometryPk, const std::vector<unsigned char>& meshData, const std::string& unit, double timeout)
{
    rapidjson::Document pt(rapidjson::kObjectType);
    const std::string uri(str(boost::format("object/%s/geometry/%s/?unit=%s")%objectPk%geometryPk%unit));
    CallPutSTL(uri, meshData, pt, 202, timeout);
    return GetJsonValueByKey<std::string>(pt, "pk");
}

int WebstackClient::CallGet(const std::string& relativeuri, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _uri = _baseapiuri;
    _uri += relativeuri;
    return _CallGet(_uri, pt, expectedhttpcode, timeout);
}

int WebstackClient::_CallGet(const std::string& desturi, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_INFO(str(boost::format("GET %s")%desturi));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, desturi.c_str());
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPGET, 0L, 1L);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( _buffer.rdbuf()->in_avail() > 0 ) {
       mujinjsonwebstack::ParseJson(pt, _buffer.str());
    }
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        std::string error_message = GetJsonValueByKey<std::string>(pt, "error_message");
        std::string traceback = GetJsonValueByKey<std::string>(pt, "traceback");
        throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s: %s", desturi%http_code%error_message, MEC_HTTPServer);
    }
    return http_code;
}

int WebstackClient::CallGet(const std::string& relativeuri, std::string& outputdata, int expectedhttpcode, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _uri = _baseapiuri;
    _uri += relativeuri;
    return _CallGet(_uri, outputdata, expectedhttpcode, timeout);
}

int WebstackClient::_CallGet(const std::string& desturi, std::string& outputdata, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_VERBOSE(str(boost::format("GET %s")%desturi));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, desturi.c_str());
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPGET, 0L, 1L);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    outputdata = _buffer.str();
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        if( outputdata.size() > 0 ) {
            rapidjson::Document d;
            ParseJson(d, _buffer.str());
            std::string error_message = GetJsonValueByKey<std::string>(d, "error_message");
            std::string traceback = GetJsonValueByKey<std::string>(d, "traceback");
            throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s: %s", desturi%http_code%error_message, MEC_HTTPServer);
        }
        throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s", desturi%http_code, MEC_HTTPServer);
    }
    return http_code;
}

int WebstackClient::CallGet(const std::string& relativeuri, std::ostream& outputStream, int expectedhttpcode, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _uri = _baseapiuri;
    _uri += relativeuri;
    return _CallGet(_uri, outputStream, expectedhttpcode, timeout);
}

int WebstackClient::_CallGet(const std::string& desturi, std::ostream& outputStream, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_VERBOSE(str(boost::format("GET %s")%desturi));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, desturi.c_str());
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteOStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &outputStream);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPGET, 0L, 1L);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        // outputStream is not always seekable; ignore any error message.
        throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s (outputStream might have information)", desturi%http_code, MEC_HTTPServer);
    }
    return http_code;
}

int WebstackClient::CallGet(const std::string& relativeuri, std::vector<unsigned char>& outputdata, int expectedhttpcode, double timeout)
{
    boost::mutex::scoped_lock lock(_mutex);
    _uri = _baseapiuri;
    _uri += relativeuri;
    return _CallGet(_uri, outputdata, expectedhttpcode, timeout);
}

int WebstackClient::_CallGet(const std::string& desturi, std::vector<unsigned char>& outputdata, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_VERBOSE(str(boost::format("GET %s")%desturi));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, desturi.c_str());

    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteVectorCallback);
    outputdata.resize(0);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &outputdata);

    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPGET, 0L, 1L);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        if( outputdata.size() > 0 ) {
            rapidjson::Document d;
            std::stringstream ss;
            ss.write((const char*)&outputdata[0], outputdata.size());
            ParseJson(d, ss.str());
            std::string error_message = GetJsonValueByKey<std::string>(d, "error_message");
            std::string traceback = GetJsonValueByKey<std::string>(d, "traceback");
            throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s: %s", desturi%http_code%error_message, MEC_HTTPServer);
        }
        throw MUJIN_EXCEPTION_FORMAT("HTTP GET to '%s' returned HTTP status %s", desturi%http_code, MEC_HTTPServer);
    }
    return http_code;
}

int WebstackClient::CallPost(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_DEBUG(str(boost::format("POST %s%s")%_baseapiuri%relativeuri));
    boost::mutex::scoped_lock lock(_mutex);
    _uri = _baseapiuri;
    _uri += relativeuri;
    return _CallPost(_uri, data, pt, expectedhttpcode, timeout);
}

/// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
int WebstackClient::_CallPost(const std::string& desturi, const std::string& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_VERBOSE(str(boost::format("POST %s")%desturi));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, desturi.c_str());
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POST, 0L, 1L);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POSTFIELDSIZE, 0, data.size());
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POSTFIELDS, NULL, data.size() > 0 ? data.c_str() : NULL);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( _buffer.rdbuf()->in_avail() > 0 ) {
        ParseJson(pt, _buffer.str());
    } else {
        pt.SetObject();
    }
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        std::string error_message = GetJsonValueByKey<std::string>(pt, "error_message");
        std::string traceback = GetJsonValueByKey<std::string>(pt, "traceback");
        throw MUJIN_EXCEPTION_FORMAT("HTTP POST to '%s' returned HTTP status %s: %s", desturi%http_code%error_message, MEC_HTTPServer);
    }
    return http_code;
}

int WebstackClient::CallPost_UTF8(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    return CallPost(relativeuri, encoding::ConvertUTF8ToFileSystemEncoding(data), pt, expectedhttpcode, timeout);
}

int WebstackClient::CallPost_UTF16(const std::string& relativeuri, const std::wstring& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    return CallPost(relativeuri, encoding::ConvertUTF16ToFileSystemEncoding(data), pt, expectedhttpcode, timeout);
}

int WebstackClient::CallPutJSON(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    return _CallPut(relativeuri, static_cast<const void*>(&data[0]), data.size(), pt, _httpheadersjson, expectedhttpcode, timeout);
}

int WebstackClient::CallPutSTL(const std::string& relativeuri, const std::vector<unsigned char>& data, rapidjson::Document& pt, int expectedhttpcode, double timeout)
{
    return _CallPut(relativeuri, static_cast<const void*> (&data[0]), data.size(), pt, _httpheadersstl, expectedhttpcode, timeout);
}

int WebstackClient::_CallPut(const std::string& relativeuri, const void* pdata, size_t nDataSize, rapidjson::Document& pt, curl_slist* headers, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_DEBUG(str(boost::format("PUT %s%s")%_baseapiuri%relativeuri));
    boost::mutex::scoped_lock lock(_mutex);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, headers);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    _uri = _baseapiuri;
    _uri += relativeuri;
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, _uri.c_str());
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_CUSTOMREQUEST, NULL, "PUT");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POSTFIELDSIZE, 0, nDataSize);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_POSTFIELDS, NULL, pdata);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( _buffer.rdbuf()->in_avail() > 0 ) {
        ParseJson(pt, _buffer.str());
    } else {
        pt.SetObject();
    }
    if( expectedhttpcode != 0 && http_code != expectedhttpcode ) {
        std::string error_message = GetJsonValueByKey<std::string>(pt, "error_message");
        std::string traceback = GetJsonValueByKey<std::string>(pt, "traceback");
        throw MUJIN_EXCEPTION_FORMAT("HTTP PUT to '%s' returned HTTP status %s: %s", relativeuri%http_code%error_message, MEC_HTTPServer);
    }
    return http_code;
}

void WebstackClient::CallDelete(const std::string& relativeuri, int expectedhttpcode, double timeout)
{
    MUJIN_LOG_DEBUG(str(boost::format("DELETE %s%s")%_baseapiuri%relativeuri));
    boost::mutex::scoped_lock lock(_mutex);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    _uri = _baseapiuri;
    _uri += relativeuri;
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, _uri.c_str());
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_CUSTOMREQUEST, NULL, "DELETE");
    _buffer.clear();
    _buffer.str("");
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_PERFORM(_curl);
    long http_code = 0;
    CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if( http_code != expectedhttpcode ) {
        rapidjson::Document d;
        ParseJson(d, _buffer.str());
        std::string error_message = GetJsonValueByKey<std::string>(d, "error_message");
        std::string traceback = GetJsonValueByKey<std::string>(d, "traceback");
        throw MUJIN_EXCEPTION_FORMAT("HTTP DELETE to '%s' returned HTTP status %s: %s", relativeuri%http_code%error_message, MEC_HTTPServer);
    }
}

std::stringstream& WebstackClient::GetBuffer()
{
    return _buffer;
}

int WebstackClient::_WriteStringStreamCallback(char *data, size_t size, size_t nmemb, std::stringstream *writerData)
{
    if (writerData == NULL) {
        return 0;
    }
    writerData->write(data, size*nmemb);
    return size * nmemb;
}

int WebstackClient::_WriteOStreamCallback(char *data, size_t size, size_t nmemb, std::ostream *writerData)
{
    if (writerData == NULL) {
        return 0;
    }
    writerData->write(data, size*nmemb);
    return size * nmemb;
}

int WebstackClient::_WriteVectorCallback(char *data, size_t size, size_t nmemb, std::vector<unsigned char> *writerData)
{
    if (writerData == NULL) {
        return 0;
    }
    writerData->insert(writerData->end(), data, data+size*nmemb);
    return size * nmemb;
}

int WebstackClient::_ReadIStreamCallback(char *data, size_t size, size_t nmemb, std::istream *readerData)
{
    if (readerData == NULL) {
        return 0;
    }
    return readerData->read(data, size*nmemb).gcount();
}

void WebstackClient::_SetupHTTPHeadersJSON()
{
    // set the header to only send json
    std::string s = std::string("Content-Type: application/json; charset=") + _charset;
    if( !!_httpheadersjson ) {
        curl_slist_free_all(_httpheadersjson);
    }
    _httpheadersjson = curl_slist_append(NULL, s.c_str());
    s = str(boost::format("Accept-Language: %s,en-us")%_language);
    _httpheadersjson = curl_slist_append(_httpheadersjson, s.c_str()); //,en;q=0.7,ja;q=0.3',")
    s = str(boost::format("Accept-Charset: %s")%_charset);
    _httpheadersjson = curl_slist_append(_httpheadersjson, s.c_str());
    //_httpheadersjson = curl_slist_append(_httpheadersjson, "Accept:"); // necessary?
    s = std::string("X-CSRFToken: ")+_csrfmiddlewaretoken;
    _httpheadersjson = curl_slist_append(_httpheadersjson, s.c_str());
    _httpheadersjson = curl_slist_append(_httpheadersjson, "Connection: Keep-Alive");
    _httpheadersjson = curl_slist_append(_httpheadersjson, "Keep-Alive: 20"); // keep alive for 20s?
    // test on windows first
    //_httpheadersjson = curl_slist_append(_httpheadersjson, "Accept-Encoding: gzip, deflate");
    for (const std::string& additionalHeader : _additionalHeaders) {
        _httpheadersjson = curl_slist_append(_httpheadersjson, additionalHeader.c_str());
    }
}

void WebstackClient::_SetupHTTPHeadersSTL()
{
    // set the header to only send stl
    std::string s = std::string("Content-Type: application/sla");
    if( !!_httpheadersstl ) {
        curl_slist_free_all(_httpheadersstl);
    }
    _httpheadersstl = curl_slist_append(NULL, s.c_str());
    //_httpheadersstl = curl_slist_append(_httpheadersstl, "Accept:"); // necessary?
    s = std::string("X-CSRFToken: ")+_csrfmiddlewaretoken;
    _httpheadersstl = curl_slist_append(_httpheadersstl, s.c_str());
    _httpheadersstl = curl_slist_append(_httpheadersstl, "Connection: Keep-Alive");
    _httpheadersstl = curl_slist_append(_httpheadersstl, "Keep-Alive: 20"); // keep alive for 20s?
    // test on windows first
    //_httpheadersstl = curl_slist_append(_httpheadersstl, "Accept-Encoding: gzip, deflate");
    for (const std::string& additionalHeader : _additionalHeaders) {
        _httpheadersstl = curl_slist_append(_httpheadersstl, additionalHeader.c_str());
    }
}

void WebstackClient::_SetupHTTPHeadersMultipartFormData()
{
    // set the header to only send stl
    std::string s = std::string("Content-Type: multipart/form-data");
    if( !!_httpheadersmultipartformdata ) {
        curl_slist_free_all(_httpheadersmultipartformdata);
    }
    _httpheadersmultipartformdata = curl_slist_append(NULL, s.c_str());
    //_httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, "Accept:"); // necessary?
    s = std::string("X-CSRFToken: ")+_csrfmiddlewaretoken;
    _httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, s.c_str());
    _httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, "Connection: Keep-Alive");
    _httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, "Keep-Alive: 20"); // keep alive for 20s?
    // test on windows first
    //_httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, "Accept-Encoding: gzip, deflate");
    for (const std::string& additionalHeader : _additionalHeaders) {
        _httpheadersmultipartformdata = curl_slist_append(_httpheadersmultipartformdata, additionalHeader.c_str());
    }
}

std::string WebstackClient::_PrepareDestinationURI_UTF8(const std::string& rawuri, bool bEnsurePath, bool bEnsureSlash, bool bIsDirectory)
{
    std::string baseuploaduri;
    if( rawuri.size() >= 7 && rawuri.substr(0,7) == "mujin:/" ) {
        baseuploaduri = _basewebdavuri;
        std::string s = rawuri.substr(7);
        baseuploaduri += _EncodeWithoutSeparator(s);
        if( bEnsurePath ) {
            if( !bIsDirectory ) {
                size_t nBaseFilenameStartIndex = s.find_last_of(s_filesep);
                if( nBaseFilenameStartIndex != std::string::npos ) {
                    s = s.substr(0, nBaseFilenameStartIndex);
                } else {
                    s = "";
                }
            }
            _EnsureWebDAVDirectories(s);
        }
    }
    else {
        if( !bEnsureSlash ) {
            return rawuri;
        }
        baseuploaduri = rawuri;
    }
    if( bEnsureSlash ) {
        // ensure trailing slash
        if( baseuploaduri[baseuploaduri.size()-1] != '/' ) {
            baseuploaduri.push_back('/');
        }
    }
    return baseuploaduri;
}

std::string WebstackClient::_PrepareDestinationURI_UTF16(const std::wstring& rawuri_utf16, bool bEnsurePath, bool bEnsureSlash, bool bIsDirectory)
{
    std::string baseuploaduri;
    std::string desturi_utf8;
    utf8::utf16to8(rawuri_utf16.begin(), rawuri_utf16.end(), std::back_inserter(desturi_utf8));

    if( desturi_utf8.size() >= 7 && desturi_utf8.substr(0,7) == "mujin:/" ) {
        baseuploaduri = _basewebdavuri;
        std::string s = desturi_utf8.substr(7);
        baseuploaduri += _EncodeWithoutSeparator(s);
        if( bEnsurePath ) {
            if( !bIsDirectory ) {
                size_t nBaseFilenameStartIndex = s.find_last_of(s_filesep);
                if( nBaseFilenameStartIndex != std::string::npos ) {
                    s = s.substr(0, nBaseFilenameStartIndex);
                } else {
                    s = "";
                }
            }
            _EnsureWebDAVDirectories(s);
        }
    }
    else {
        if( !bEnsureSlash ) {
            return desturi_utf8;
        }
        baseuploaduri = desturi_utf8;
    }
    if( bEnsureSlash ) {
        // ensure trailing slash
        if( baseuploaduri[baseuploaduri.size()-1] != '/' ) {
            baseuploaduri.push_back('/');
        }
    }
    return baseuploaduri;
}

std::string WebstackClient::_EncodeWithoutSeparator(const std::string& raw)
{
    std::string output;
    size_t startindex = 0;
    for(size_t i = 0; i < raw.size(); ++i) {
        if( raw[i] == '/' ) {
            if( startindex != i ) {
                output += EscapeString(raw.substr(startindex, i-startindex));
                startindex = i+1;
            }
            output += '/';
        }
    }
    if( startindex != raw.size() ) {
        output += EscapeString(raw.substr(startindex));
    }
    return output;
}

void WebstackClient::_EnsureWebDAVDirectories(const std::string& relativeuri, double timeout)
{
    if (relativeuri.empty()) {
        return;
    }

    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_TIMEOUT_MS, 0L, (long)(timeout * 1000L));
    std::list<std::string> listCreateDirs;
    std::string output;
    size_t startindex = 0;
    for(size_t i = 0; i < relativeuri.size(); ++i) {
        if( relativeuri[i] == '/' ) {
            if( startindex != i ) {
                listCreateDirs.push_back(EscapeString(relativeuri.substr(startindex, i-startindex)));
                startindex = i+1;
            }
        }
    }
    if( startindex != relativeuri.size() ) {
        listCreateDirs.push_back(EscapeString(relativeuri.substr(startindex)));
    }

    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEFUNCTION, NULL, _WriteStringStreamCallback);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_WRITEDATA, NULL, &_buffer);
    CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_CUSTOMREQUEST, NULL, "MKCOL");

    std::string totaluri = "";
    for(std::list<std::string>::iterator itdir = listCreateDirs.begin(); itdir != listCreateDirs.end(); ++itdir) {
        // first have to create the directory structure up to destinationdir
        if( totaluri.size() > 0 ) {
            totaluri += '/';
        }
        totaluri += *itdir;
        _uri = _basewebdavuri + totaluri;
        CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_HTTPHEADER, NULL, _httpheadersjson);
        CURL_OPTION_SAVE_SETTER(_curl, CURLOPT_URL, NULL, _uri.c_str());
        _buffer.clear();
        _buffer.str("");
        CURL_PERFORM(_curl);
        long http_code = 0;
        CURL_INFO_GETTER(_curl, CURLINFO_RESPONSE_CODE, &http_code);
        /* creating directories

           Responses from a MKCOL request MUST NOT be cached as MKCOL has non-idempotent semantics.

           201 (Created) - The collection or structured resource was created in its entirety.

           403 (Forbidden) - This indicates at least one of two conditions: 1) the server does not allow the creation of collections at the given location in its namespace, or 2) the parent collection of the Request-URI exists but cannot accept members.

           405 (Method Not Allowed) - MKCOL can only be executed on a deleted/non-existent resource.

           409 (Conflict) - A collection cannot be made at the Request-URI until one or more intermediate collections have been created.

           415 (Unsupported Media Type)- The server does not support the request type of the body.

           507 (Insufficient Storage) - The resource does not have sufficient space to record the state of the resource after the execution of this method.

         */
        if( http_code != 201 && http_code != 301 ) {
            throw MUJIN_EXCEPTION_FORMAT("HTTP MKCOL failed with HTTP status %d: %s", http_code%_errormessage, MEC_HTTPServer);
        }
    }
}

// static
MUJINWEBSTACKCLIENT_API WebstackClientPtr WebstackClient::CreateWebstackClient(const std::string& usernamepassword, const std::string& url, const std::string& proxyserverport, const std::string& proxyuserpw, int options, double timeout)
{
    return boost::make_shared<WebstackClient>(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout);
}

} // end namespace mujinwebstackclient
