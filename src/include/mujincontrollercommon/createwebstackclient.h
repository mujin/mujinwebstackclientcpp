// -*- coding: utf-8 -*-
// Copyright (C) MUJIN Inc.
// Do not distribute this file outside of Mujin

#pragma once

#include <mujincontrollerclient/mujincontrollerclient.h>

namespace mujincontrollercommon {

/// \brief determine if hostname is local, if len is not given, then use strlen to determine string length
static bool IsHostnameLocal(const char* hostname, std::ssize_t len = -1)
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
    const std::size_t localHostnameLen = std::strlen(localHostname);
    if (len == localHostnameLen && strncmp(hostname, localHostname, len) == 0) {
        return true;
    }
    return false;
}

/// \brief Transparently diverge to private webstack if url is localhost
static inline mujinclient::ControllerClientPtr CreateWebstackClient(
    std::string usernamepassword,
    const std::string& url,
    const std::string& proxyserverport = std::string(),
    const std::string& proxyuserpw = std::string(),
    int options = 0,
    double timeout = 3.0,
    std::string unixendpoint = std::string())
{
    while (unixendpoint.empty()) {
        std::size_t start = url.find("://");
        if (start == std::string::npos) {
            break;
        }
        start += sizeof("://") - 1;
        const std::size_t port = url.find(":", start); // not found is ok
        const std::size_t slash = url.find("/", start); // not found is ok
        std::ssize_t len = -1;

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
                const char *const username = std::getenv("MUJIN_WEBSTACK_LEGACY_USERNAME");
                const char *const password = std::getenv("MUJIN_WEBSTACK_PRIVATE_PASSWORD");
                if (username != nullptr && password != nullptr) {
                    usernamepassword = std::string(username) + ":" + password;
                }
            }
        }
        break;
    }

    return mujinclient::CreateControllerClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout, unixendpoint);
}

}
