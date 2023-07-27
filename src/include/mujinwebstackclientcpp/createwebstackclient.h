// -*- coding: utf-8 -*-
// Copyright (C) MUJIN Inc.
// Do not distribute this file outside of Mujin

#ifndef __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
#define __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__

#include <mujinwebstackclientcpp/config.h>
#include <mujinwebstackclientcpp/webstackclient.h>

namespace mujinwebstackclient {

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
MUJINWEBSTACKCLIENT_API bool IsHostnameLocal(const char* hostname, ssize_t len = -1);

/// \brief determine if the url is pointing to local webstack
MUJINWEBSTACKCLIENT_API bool IsWebstackLocal(const char* url);

/// \brief determine the unix endpoint if webstack is local
MUJINWEBSTACKCLIENT_API const char* GetUnixEndpointForLocalWebstack(const char* url);

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
MUJINWEBSTACKCLIENT_API WebstackClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport = std::string(),
    const std::string& proxyuserpw = std::string(),
    int options = 0,
    double timeout = 3.0);

} // namespace mujinwebstackclient

#endif // __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
