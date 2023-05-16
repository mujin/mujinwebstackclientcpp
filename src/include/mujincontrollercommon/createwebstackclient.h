// -*- coding: utf-8 -*-
// Copyright (C) MUJIN Inc.
// Do not distribute this file outside of Mujin

#ifndef __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
#define __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__

#include <mujincontrollercommon/config.h>
#include <mujincontrollerclient/mujincontrollerclient.h>

namespace mujincontrollercommon {

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
bool MUJINCONTROLLERCOMMON_API IsHostnameLocal(const char* hostname, ssize_t len = -1);

/// \brief Transparently diverge to private webstack if url is localhost
mujinclient::ControllerClientPtr MUJINCONTROLLERCOMMON_API CreateWebstackClient(
    const std::string& usernamepassword,
    const std::string& url,
    const std::string& proxyserverport = std::string(),
    const std::string& proxyuserpw = std::string(),
    int options = 0,
    double timeout = 3.0,
    std::string unixendpoint = std::string());

} // namespace mujincontrollercommon

#endif // __MUJIN_CONTROLLERCOMMON_CREATEWEBSTACKCLIENT__
