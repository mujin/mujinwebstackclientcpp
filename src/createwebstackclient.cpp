#include <mujincontrollercommon/createwebstackclient.h>
#include <mujincontrollercommon/logging.h>

MUJIN_LOGGER("mujin.controllercommon.createwebstackclient");

namespace mujincontrollercommon {

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
    const char* colonSlashSlash = strstr(url, "://");
    if (colonSlashSlash == nullptr) {
        return false;
    }
    const char* hostname = colonSlashSlash + sizeof("://") - 1;
    const char* at = strstr(hostname, "@"); // not found is ok
    const char* slash = strstr(hostname, "/"); // not found is ok
    if (at != nullptr && (slash == nullptr || at < slash)) {
        hostname = at + 1;
    }
    const char* port = strstr(hostname, ":"); // not found is ok
    ssize_t len = -1;
    if (slash == nullptr) {
        if (port == nullptr) {
            // no port, no slash
            len = -1; // use the null-terminator
        } else {
            // has port, no slash
            if (strcmp(port, ":80") != 0) {
                return false;
            }
            len = port - hostname;
        }
    } else {
        if (port != nullptr && port < slash) {
            // has port before slash
            if (strncmp(port, ":80", slash - port) != 0) {
                return false;
            }
            len = port - hostname;
        } else {
            // no port, but has slash
            len = slash - hostname;
        }
    }
    return IsHostnameLocal(hostname, len);
}

/// \brief determine the unix endpoint if webstack is local
const char* GetUnixEndpointForLocalWebstack(const char* url)
{
    if (IsWebstackLocal(url)) {
        const char* unixendpoint = std::getenv("MUJIN_WEBSTACK_UNIX_ENDPOINT");
        if (unixendpoint != nullptr && unixendpoint[0] != '\0') {
            MUJIN_LOG_DEBUG_FORMAT("forcing webstack client to use unix endpoint \"%s\" since url is \"%s\"", unixendpoint%url);
            return unixendpoint;
        }
    }
    return "";
}

/// \brief Transparently diverge to private webstack if url is localhost
mujinclient::ControllerClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport,
    const std::string& proxyuserpw,
    int options,
    double timeout)
{
    mujinclient::ControllerClientPtr pClient = mujinclient::CreateControllerClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout);
    std::string unixendpoint = GetUnixEndpointForLocalWebstack(url.c_str());
    if (!unixendpoint.empty()) {
        pClient->SetUnixEndpoint(unixendpoint);
    }
    return pClient;
}

}
