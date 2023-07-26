#include <mujinwebstackclientcpp/createwebstackclient.h>
#include <mujinwebstackclientcpp/logging.h>

MUJIN_LOGGER("mujin.mujinwebstackclientcpp.createwebstackclient");

namespace mujinwebstackclient {

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
bool IsHostnameLocal(const char* hostname, ssize_t len)
{
    if (len == -1) {
        len = std::strlen(hostname);
    }
    if (len == sizeof("127.0.0.1") - 1 && strncmp(hostname, "127.0.0.1", len) == 0) {
        return true;/** \en \brief creates the controller with an account. <b>This function is not thread safe.</b>

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
MUJINWEBSTACKCLIENT_API WebstackClientPtr CreateWebstackClient(const std::string& usernamepassword, const std::string& url, const std::string& proxyserverport=std::string(), const std::string& proxyuserpw=std::string(), int options=0, double timeout=3.0);
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
        return true;/** \en \brief creates the controller with an account. <b>This function is not thread safe.</b>

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
MUJINWEBSTACKCLIENT_API WebstackClientPtr CreateWebstackClient(const std::string& usernamepassword, const std::string& url, const std::string& proxyserverport=std::string(), const std::string& proxyuserpw=std::string(), int options=0, double timeout=3.0);
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
mujinwebstackclient::WebstackClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport,
    const std::string& proxyuserpw,
    int options,
    double timeout)
{
    WebstackClientPtr pClient = WebstackClient::CreateWebstackClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout);
    std::string unixendpoint = GetUnixEndpointForLocalWebstack(url.c_str());
    if (!unixendpoint.empty()) {
        pClient->SetUnixEndpoint(unixendpoint);
    }
    return pClient;
}

}  // namespace mujinwebstackclient
