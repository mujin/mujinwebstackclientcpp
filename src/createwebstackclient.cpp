#include <mujinwebstackclientcpp/createwebstackclient.h>
#include "logging.h"

MUJIN_LOGGER("mujin.mujinwebstackclientcpp.createwebstackclient");

namespace mujinwebstackclient {

namespace {

// Versions of the below implemented in terms of the WebstackClientInfo,
// to avoid re-parsing the URL.

bool IsWebstackLocal(const WebstackClientInfo& clientInfo)
{
    return IsHostnameLocal(clientInfo.host.c_str(), clientInfo.host.length());
}

const char* GetUnixEndpointForLocalWebstack(const WebstackClientInfo& clientInfo)
{
    if (IsWebstackLocal(clientInfo)) {
        const char* unixendpoint = std::getenv("MUJIN_WEBSTACK_UNIX_ENDPOINT");
        if (unixendpoint != nullptr && unixendpoint[0] != '\0') {
            MUJIN_LOG_DEBUG(boost::str(boost::format("forcing webstack client to use unix endpoint \"%s\" since host is \"%s\"")%unixendpoint%clientInfo.host));
            return unixendpoint;
        }
    }
    return "";
}

}

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
bool IsHostnameLocal(const char* hostname, ssize_t len)
{
    if (len == -1) {
        len = std::strlen(hostname);
    }
    if (len == sizeof("127.0.0.1") - 1 && strncmp(hostname, "127.0.0.1", len) == 0) {
        return true;
    }
    if (len == sizeof("localhost") - 1 && strncmp(hostname, "localhost", len) == 0) {
        return true;
    }
    char localHostname[HOST_NAME_MAX + 1] = {};
    if (gethostname(localHostname, sizeof(localHostname)) != 0) {
        return false;
    }
    // If the null-terminated hostname is too large to fit, then the name is truncated, and no error is returned (but see NOTES below).
    // POSIX.1 says that if such truncation occurs, then it is unspecified whether the returned buffer includes a terminating null byte.
    localHostname[HOST_NAME_MAX] = '\0';
    const size_t localHostnameLen = std::strlen(localHostname);
    if ((size_t)len == localHostnameLen && strncmp(hostname, localHostname, len) == 0) {
        return true;
    }
    return false;
}

/// \brief determine if the url is pointing to local webstack
bool IsWebstackLocal(const char* url)
{
    return IsWebstackLocal(WebstackClientInfo::FromUrl(url));
}

/// \brief determine the unix endpoint if webstack is local
const char* GetUnixEndpointForLocalWebstack(const char* url)
{
    return GetUnixEndpointForLocalWebstack(WebstackClientInfo::FromUrl(url));
}

/// \brief Transparently diverge to private webstack if url is localhost
mujinwebstackclient::WebstackClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport,
    const std::string& proxyuserpw,
    int options,
    double timeout)
{
    WebstackClientPtr pClient = WebstackClient::CreateWebstackClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout);
    std::string unixendpoint = GetUnixEndpointForLocalWebstack(pClient->GetClientInfo());
    if (!unixendpoint.empty()) {
        pClient->SetUnixEndpoint(unixendpoint);
    }
    return pClient;
}

}  // namespace mujinwebstackclient
