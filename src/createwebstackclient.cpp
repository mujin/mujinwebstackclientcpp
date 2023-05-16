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

/// \brief Transparently diverge to private webstack if url is localhost
mujinclient::ControllerClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport,
    const std::string& proxyuserpw,
    int options,
    double timeout,
    std::string unixendpoint)
{
    if (unixendpoint.empty()) {
        const size_t colonSlashSlash = url.find("://");
        if (colonSlashSlash != std::string::npos) {
            const size_t start = colonSlashSlash + sizeof("://") - 1;
            const size_t port = url.find(":", start); // not found is ok
            const size_t slash = url.find("/", start); // not found is ok
            ssize_t len = -1;
            if (slash == std::string::npos) {
                if (port == std::string::npos) {
                    // no port, no slash
                    len = -1; // use the null-terminator
                } else {
                    // has port, no slash
                    len = port - start - 1;
                }
            } else {
                if (port != std::string::npos && port < slash) {
                    // has port before slash
                    len = port - start - 1;
                } else {
                    // no port, but has slash
                    len = slash - start - 1;
                }
            }
            if (IsHostnameLocal(url.c_str() + start, len)) {
                const char *const envunixendpoint = std::getenv("MUJIN_WEBSTACK_UNIX_ENDPOINT");
                if (envunixendpoint != nullptr && envunixendpoint[0] != '\0') {
                    unixendpoint = envunixendpoint;
                }
            }
        }
    }

    return mujinclient::CreateControllerClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout, unixendpoint);
}

}
