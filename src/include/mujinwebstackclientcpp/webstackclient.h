// -*- coding: utf-8 -*-
//
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
/** \file mujinwebstackclient.h
    \brief  Defines the public headers of the MUJIN Controller Client
 */
#ifndef MUJIN_WEBSTACKCLIENT_H
#define MUJIN_WEBSTACKCLIENT_H

#ifdef _MSC_VER

#pragma warning(disable:4251)// needs to have dll-interface to be used by clients of class
#pragma warning(disable:4190)// C-linkage specified, but returns UDT 'boost::shared_ptr<T>' which is incompatible with C
#pragma warning(disable:4819)//The file contains a character that cannot be represented in the current code page (932). Save the file in Unicode format to prevent data loss using native typeof

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#endif

#else
#endif

#if defined(__GNUC__)
#define MUJINWEBSTACKCLIENT_DEPRECATED __attribute__((deprecated))
#else
#define MUJINWEBSTACKCLIENT_DEPRECATED
#endif

#include <cmath>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <exception>

#include <iomanip>
#include <fstream>
#include <sstream>

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>
#include <curl/curl.h>

#include <mujinwebstackclientcpp/config.h>
#include <mujinwebstackclientcpp/mujinexceptions.h>
#include <mujinwebstackclientcpp/mujinjson.h>
//#include <mujinwebstackclientcpp/mujindefinitions.h>


namespace mujinwebstackclient {

/// \brief (scene) file entry in mujin controller
struct FileEntry
{
    std::string filename;
    double modified; // in epoch seconds
    size_t size; // file size in bytes
};

typedef double Real;

/// \brief connecting to a controller's webstack
class MUJINWEBSTACKCLIENT_API WebstackClientInfo : public mujinjsonwebstack::JsonSerializable
{
public:
    /// \brief given a url "http[s]://[username[:password]@]hostname[:port][/path]", parse WebstackClientInfo
    static WebstackClientInfo FromUrl(const char* url);

    virtual void Reset();

    void LoadFromJson(const rapidjson::Value& rClientInfo) override;
    void SaveToJson(rapidjson::Value& rClientInfo, rapidjson::Document::AllocatorType& alloc) const override;

    bool operator==(const WebstackClientInfo &rhs) const;
    bool operator!=(const WebstackClientInfo &rhs) const {
        return !operator==(rhs);
    }
    std::string GetURL(bool bIncludeNamePassword) const;

    inline bool IsEnabled() const {
        return !host.empty();
    }

    std::string host;
    uint16_t httpPort = 0; ///< Post to communicate with the webstack. If 0, then use the default port
    std::string username;
    std::string password;
    std::vector<std::string> additionalHeaders; ///< expect each value to be in the format of "Header-Name: header-value"
    std::string unixEndpoint; ///< unix socket endpoint for communicating with HTTP server over unix socket
};

/// \brief Creates on MUJIN Controller instance.
///
/// Only one call can be made at a time. In order to make multiple calls simultaneously, create another instance.
class MUJINWEBSTACKCLIENT_API WebstackClient
{
public:
    WebstackClient(const std::string& usernamepassword, const std::string& baseuri, const std::string& proxyserverport, const std::string& proxyuserpw, int options, double timeout);
    virtual ~WebstackClient();

    /** \en \brief creates the controller with an account. <b>This function is not thread safe.</b>

        You must not call it when any other thread in the program (i.e. a thread sharing the same memory) is running.
        \param usernamepassword user:password
        \param url the URI-encoded URL of controller server, it needs to have a trailing slash. It can also be in the form of https://username@server/ in order to force login of a particular user.
        \param proxyserverport Specify proxy server to use. To specify port number in this string, append :[port] to the end of the host name. The proxy string may be prefixed with [protocol]:// since any such prefix will be ignored. The proxy's port number may optionally be specified with the separate option. If not specified, will default to using port 1080 for proxies. Setting to empty string will disable the proxy.
        \param proxyuserpw If non-empty, [user name]:[password] to use for the connection to the HTTP proxy.
        \param options extra options for connecting to the controller. If 0x1 is set, the client will optimize usage to only allow GET calls. Set 0x80000000 if using a development server.

        \ja \brief MUJINコントローラのクライアントを作成する。<b>この関数はスレッドセーフではない。</b>

        この関数はスレッドセーフではないため、呼び出す時に他のスレッドが走っていないようにご注意ください。
        \param usernamepassword ユーザ:パスワード
        \param url コントローラにアクセスするためのURLです。スラッシュ「/」で終わる必要があります。強制的にユーザも指定出来ます、例えば<b>https://username@server/</b>。
        \param proxyserverport Specify proxy server to use. To specify port number in this string, append :[port] to the end of the host name. The proxy string may be prefixed with [protocol]:// since any such prefix will be ignored. The proxy's port number may optionally be specified with the separate option. If not specified, will default to using port 1080 for proxies. Setting to empty string will disable the proxy.
        \param proxyuserpw If non-empty, [user name]:[password] to use for the connection to the HTTP proxy.
        \param options １が指定されたら、クライアントがGETのみを呼び出し出来ます。それで初期化がもっと速くなれます。
        \param timeout set timeout in seconds for the initial login requests
    */
    MUJINWEBSTACKCLIENT_API static boost::shared_ptr<WebstackClient> CreateWebstackClient(const std::string& usernamepassword, const std::string& url, const std::string& proxyserverport=std::string(), const std::string& proxyuserpw=std::string(), int options=0, double timeout=3.0);

    /// \brief sets the character encoding for all strings that are being input and output from the resources
    ///
    /// The default character encoding is \b utf-8, can also set it to \b Shift_JIS for windows japanese unicode, \b iso-2022-jp
    /// List of possible sets: http://www.iana.org/assignments/character-sets/character-sets.xml
    void SetCharacterEncoding(const std::string& newencoding);

    /// \brief sets the language code for all output
    ///
    /// Check out http://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
    void SetLanguage(const std::string& language);

    /// \brief sets the user agent to be sent with each http request
    void SetUserAgent(const std::string& userAgent);

    /// \brief sets additional http headers to be included on all requests
    ///
    /// \param additionalHeaders expect each value to be in the format of "Header-Name: header-value"
    void SetAdditionalHeaders(const std::vector<std::string>& additionalHeaders);

    /// \brief returns the username logged into this controller
    const std::string& GetUserName() const;

    /// \brief returns the URI used to setup the connection
    const std::string& GetBaseURI() const;

    const WebstackClientInfo& GetClientInfo() const;

    /// \brief If necessary, changes the proxy to communicate to the controller server. Setting proxy disables previously set unix endpoint.
    ///
    /// \param serverport Specify proxy server to use. To specify port number in this string, append :[port] to the end of the host name. The proxy string may be prefixed with [protocol]:// since any such prefix will be ignored. The proxy's port number may optionally be specified with the separate option. If not specified, will default to using port 1080 for proxies. Setting to empty string will disable the proxy.
    /// \param userpw If non-empty, [user name]:[password] to use for the connection to the HTTP proxy.
    void SetProxy(const std::string& serverport, const std::string& userpw);

    /// \brief If necessary, changes the unix domain socket to be used to communicate to the controller server. Setting unix endpoint disables previously set proxy.
    ///
    /// \param unixendpoint Specify the file path to the unix domain socket to connect to.
    void SetUnixEndpoint(const std::string& unixendpoint);

    /// \brief Restarts the MUJIN Controller Server and destroys any optimizaiton jobs.
    ///
    /// If the server is not responding, call this method to clear the server state and initialize everything.
    /// The method is blocking, when it returns the MUJIN Controller would have been restarted.
    void RestartServer(double timeout = 5.0);

    /// \brief Execute GraphQL query or mutation against Mujin Controller.
    ///
    /// Throws an exception if there are any errors
    /// \param rResultData The "data" field of the result if the query returns without problems
    void ExecuteGraphQuery(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResultData, rapidjson::Document::AllocatorType& rAlloc, double timeout = 60.0);

    /// \brief Execute the GraphQL query or mutation against Mujin Controller and return any output as-is without doing any error processing
    ///
    /// \param rResult The entire result field of the query. Should have keys "data" and "errors". Each error should have keys: "message", "locations", "path", "extensions". And "extensions" has keys "errorCode".
    void ExecuteGraphQueryRaw(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResult, rapidjson::Document::AllocatorType& rAlloc, double timeout = 60.0);

    /// \brief returns the mujin controller version
    std::string GetVersion();

    /// \brief sends the cancel message to all jobs.
    ///
    /// The method is non-blocking
    void CancelAllJobs();

    /// \param localtimeval seconds since epoch, will use input as If-Modified-Since header
    /// \param remotetimeval will output the modified date in response
    /// \param vdata filled with the contents of the file on the controller filesystem
    void DownloadFileFromControllerIfModifiedSince_UTF8(const std::string& desturi, long localtimeval, long &remotetimeval, std::vector<unsigned char>& vdata, double timeout = 5.0);

    /// \param localtimeval seconds since epoch, will use input as If-Modified-Since header
    /// \param remotetimeval will output the modified date in response
    /// \param vdata filled with the contents of the file on the controller filesystem
    void DownloadFileFromControllerIfModifiedSince_UTF16(const std::wstring& desturi, long localtimeval, long &remotetimeval, std::vector<unsigned char>& vdata, double timeout = 5.0);

    void ModifySceneAddReferenceObjectPK(const std::string &scenepk, const std::string &referenceobjectpk, double timeout = 5.0);

    void ModifySceneRemoveReferenceObjectPK(const std::string &scenepk, const std::string &referenceobjectpk, double timeout = 5.0);

    const std::string& GetDefaultTaskType();

    /** \brief Get the url-encoded primary key of a scene from a scene uri (utf-8 encoded)

        For example, the URI

        mujin:/検証動作_121122.mujin.dae

        is represented as:

        "mujin:/\xe6\xa4\x9c\xe8\xa8\xbc\xe5\x8b\x95\xe4\xbd\x9c_121122.mujin.dae"

        Return value will be: "%E6%A4%9C%E8%A8%BC%E5%8B%95%E4%BD%9C_121122"
        \param uri utf-8 encoded URI
     */
    std::string GetScenePrimaryKeyFromURI_UTF8(const std::string& uri);

    /** \brief Get the url-encoded primary key of a scene from a scene uri (utf-16 encoded)

        If input URL is L"mujin:/\u691c\u8a3c\u52d5\u4f5c_121122.mujin.dae"
        Return value will be: "%E6%A4%9C%E8%A8%BC%E5%8B%95%E4%BD%9C_121122"

        \param uri utf-16 encoded URI
     */
    std::string GetScenePrimaryKeyFromURI_UTF16(const std::wstring& uri);

    /// \brief returns the uncoded name from a primary key
    ///
    /// \return utf-8 encoded name
    std::string GetNameFromPrimaryKey_UTF8(const std::string& pk);

    /// \brief returns the uncoded name from a primary key
    ///
    /// \return utf-16 encoded name
    std::wstring GetNameFromPrimaryKey_UTF16(const std::string& pk);

    std::string CreateObjectGeometry(const std::string& objectPk, const std::string& name, const std::string& linkPk, const std::string& geomtype, double timeout = 5);

    /// \brief set geometry mesh to an object
    /// \param objectPk primary key for the object to set mesh data to
    /// \param geometryPk primary key for the geometry
    /// \param data stl format binary mesh data
    /// \param unit length unit of mesh
    /// \param timeout timeout of uploading mesh
    ///
    std::string SetObjectGeometryMesh(const std::string& objectPk, const std::string& geometryPk, const std::vector<unsigned char>& data, const std::string& unit = "mm", double timeout = 5);

    /// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
    int CallGet(const std::string& relativeuri, rapidjson::Document& pt, int expectedhttpcode=200, double timeout = 5.0);

    /// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
    int CallGet(const std::string& relativeuri, std::string& outputdata, int expectedhttpcode=200, double timeout = 5.0);

    /// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
    int CallGet(const std::string& relativeuri, std::ostream& outputStream, int expectedhttpcode=200, double timeout = 5.0);

    /// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
    int CallGet(const std::string& relativeuri, std::vector<unsigned char>& outputdata, int expectedhttpcode=200, double timeout = 5.0);

    /// \brief expectedhttpcode is not 0, then will check with the returned http code and if not equal will throw an exception
    ///
    /// \param relativeuri URL-encoded UTF-8 encoded
    /// \param data encoded depending on the character encoding set on the system
    int CallPost(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode=201, double timeout = 5.0);

    /// \param data utf-8 encoded
    int CallPost_UTF8(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode=201, double timeout = 5.0);
    /// \param data utf-16 encoded
    int CallPost_UTF16(const std::string& relativeuri, const std::wstring& data, rapidjson::Document& pt, int expectedhttpcode=201, double timeout = 5.0);

    /// \brief puts json string
    /// \param relativeuri relative uri to put at
    /// \param data json string to put
    /// \param pt reply is stored
    /// \param expectedhttpcode expected http code
    /// \param timeout timeout of puts
    /// \return http code returned
    int CallPutJSON(const std::string& relativeuri, const std::string& data, rapidjson::Document& pt, int expectedhttpcode=202, double timeout = 5.0);

    /// \brief puts stl data
    /// \param relativeuri relative uri to put at
    /// \param data stl raw data
    /// \param pt reply is stored
    /// \param expectedhttpcode expected http code
    /// \param timeout timeout of puts
    /// \return http code returned
    int CallPutSTL(const std::string& relativeuri, const std::vector<unsigned char>& data, rapidjson::Document& pt, int expectedhttpcode=202, double timeout = 5.0);


    void CallDelete(const std::string& relativeuri, int expectedhttpcode, double timeout = 5.0);

    std::stringstream& GetBuffer();

    /// \brief URL-encode raw string
    inline std::string EscapeString(const std::string& raw) const
    {
        boost::shared_ptr<char> pstr(curl_easy_escape(_curl, raw.c_str(), raw.size()), curl_free);
        return std::string(pstr.get());
    }

    /// \brief decode URL-encoded raw string
    inline std::string UnescapeString(const std::string& raw) const
    {
        int outlength = 0;
        boost::shared_ptr<char> pstr(curl_easy_unescape(_curl, raw.c_str(), raw.size(), &outlength), curl_free);
        return std::string(pstr.get(), outlength);
    }

private:
    int _CallPut(const std::string& relativeuri, const void* pdata, size_t nDataSize, rapidjson::Document& pt, curl_slist* headers, int expectedhttpcode=202, double timeout = 5.0);

    /// For all webdav internal functions: mutex is already locked, desturi directories are already created
    //@{

    /// \param desturi expects the fully resolved URI to pass to curl
    int _CallGet(const std::string& desturi, rapidjson::Document& pt, int expectedhttpcode=200, double timeout = 5.0);
    int _CallGet(const std::string& desturi, std::string& outputdata, int expectedhttpcode=200, double timeout = 5.0);
    int _CallGet(const std::string& desturi, std::vector<unsigned char>& outputdata, int expectedhttpcode=200, double timeout = 5.0);
    int _CallGet(const std::string& desturi, std::ostream& outputStream, int expectedhttpcode=200, double timeout = 5.0);

    int _CallPost(const std::string& desturi, const std::string& data, rapidjson::Document& pt, int expectedhttpcode=201, double timeout = 5.0);

    static int _WriteStringStreamCallback(char *data, size_t size, size_t nmemb, std::stringstream *writerData);
    static int _WriteVectorCallback(char *data, size_t size, size_t nmemb, std::vector<unsigned char> *writerData);
    static int _WriteOStreamCallback(char *data, size_t size, size_t nmemb, std::ostream *writerData);
    static int _ReadIStreamCallback(char *data, size_t size, size_t nmemb, std::istream *writerData);

    /// \brief sets up http header for doing http operation with json data
    void _SetupHTTPHeadersJSON();

    /// \brief sets up http header for doing http operation with stl data
    void _SetupHTTPHeadersSTL();

    /// \brief sets up http header for doing http operation with multipart/form-data data
    void _SetupHTTPHeadersMultipartFormData();

    void _DownloadFileFromController(const std::string& desturi, long localtimeval, long &remotetimeval, std::vector<unsigned char>& vdata, double timeout = 5);

    /// \brief given a raw uri with "mujin:/", return the real network uri
    ///
    /// mutex should be locked
    /// \param bEnsurePath if true, will make sure the directories on the server side are created
    /// \param bEnsureSlash if true, will ensure returned uri ends with slash /
    std::string _PrepareDestinationURI_UTF8(const std::string& rawuri, bool bEnsurePath=true, bool bEnsureSlash=false, bool bIsDirectory=false);
    std::string _PrepareDestinationURI_UTF16(const std::wstring& rawuri, bool bEnsurePath=true, bool bEnsureSlash=false, bool bIsDirectory=false);

    // encode a URL without the / separator
    std::string _EncodeWithoutSeparator(const std::string& raw);

    /// \param relativeuri utf-8 encoded directory inside the user webdav folder. has a trailing slash. relative to real uri
    void _EnsureWebDAVDirectories(const std::string& relativeuri, double timeout = 3.0);


    void _ExecuteGraphQuery(const char* operationName, const char* query, const rapidjson::Value& rVariables, rapidjson::Value& rResult, rapidjson::Document::AllocatorType& rAlloc, double timeout, bool checkForErrors, bool returnRawResponse);

    // INSTANCE FIELDS
    int _lastmode;
    CURL *_curl;
    boost::mutex _mutex;
    std::stringstream _buffer;
    std::string _baseuri, _baseapiuri, _basewebdavuri, _uri;

    WebstackClientInfo _clientInfo;

    curl_slist *_httpheadersjson;
    curl_slist *_httpheadersstl;
    curl_slist *_httpheadersmultipartformdata;
    std::string _charset, _language;
    std::string _csrfmiddlewaretoken;

    rapidjson::Document _profile; ///< user profile and versioning
    std::string _errormessage; ///< set when an error occurs in libcurl

    std::string _defaultscenetype, _defaulttasktype;

    rapidjson::StringBuffer _rRequestStringBufferCache; ///< cache for request string, protected by _mutex

};  // End class WebstackClient

typedef boost::shared_ptr<WebstackClient> WebstackClientPtr;
typedef boost::weak_ptr<WebstackClient> WebstackClientWeakPtr;


} // namespace mujinwebstackclient

BOOST_STATIC_ASSERT(MUJINWEBSTACKCLIENT_VERSION_MAJOR>=0&&MUJINWEBSTACKCLIENT_VERSION_MAJOR<=255);
BOOST_STATIC_ASSERT(MUJINWEBSTACKCLIENT_VERSION_MINOR>=0&&MUJINWEBSTACKCLIENT_VERSION_MINOR<=255);
BOOST_STATIC_ASSERT(MUJINWEBSTACKCLIENT_VERSION_PATCH>=0&&MUJINWEBSTACKCLIENT_VERSION_PATCH<=255);

#endif
