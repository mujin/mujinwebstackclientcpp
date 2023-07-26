// -*- coding: utf-8 -*-
// Copyright (C) MUJIN Inc.
// Do not distribute this file outside of Mujin

#ifndef __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
#define __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__

#include <mujincontrollercommon/config.h>
#include <mujinwebstackclientcpp/webstackclient.h>

namespace mujincontrollercommon {

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
MUJINCONTROLLERCOMMON_API bool IsHostnameLocal(const char* hostname, ssize_t len = -1);

/// \brief determine if the url is pointing to local webstack
MUJINCONTROLLERCOMMON_API bool IsWebstackLocal(const char* url);

/// \brief determine the unix endpoint if webstack is local
MUJINCONTROLLERCOMMON_API const char* GetUnixEndpointForLocalWebstack(const char* url);

/// \brief Transparently diverge to private webstack if url is localhost
MUJINCONTROLLERCOMMON_API mujinwebstackclient::WebstackClientPtr CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport = std::string(),
    const std::string& proxyuserpw = std::string(),
    int options = 0,
    double timeout = 3.0);

} // namespace mujincontrollercommon

namespace mujinclient {
// TODO(document/team#86): remove this once all call sites are migrated.
typedef mujinwebstackclient::WebstackClientPtr ControllerClientPtr;
}

#endif // __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
