// -*- coding: utf-8 -*-
// Copyright (C) MUJIN Inc.
// Do not distribute this file outside of Mujin

#pragma once

#include <mujincontrollerclient/mujincontrollerclient.h>

namespace mujincontrollercommon {

static bool IsHostnameLocal(const std::string& hostname)
{
    static char hostnamebuf[256];
    static int status = gethostname(hostnamebuf, sizeof(hostnamebuf));

    return hostname == "localhost" || hostname == "127.0.0.1" || hostname == hostnamebuf;
}

// \brief Transparently diverge to private webstack if url is localhost
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
        start += std::strlen("://");

        const std::size_t end = url.find("/", start); // not find is ok

        std::string host = url.substr(start, end == std::string::npos ? end : end - start);
        const std::size_t port = host.find(":");
        if (port != std::string::npos) {
            if (host.substr(port + 1) != "80") {
                break;
            }
            host.erase(port);
        }

        if (IsHostnameLocal(host)) {
            unixendpoint = "/run/webstack/http.sock";

            const char *const username = std::getenv("MUJIN_WEBSTACK_PRIVATE_USERNAME");
            const char *const password = std::getenv("MUJIN_WEBSTACK_PRIVATE_PASSWORD");
            if (username != nullptr && password != nullptr) {
                usernamepassword = std::string(username) + ":" + password;
            }
        }
        break;
    }

    return mujinclient::CreateControllerClient(usernamepassword, url, proxyserverport, proxyuserpw, options, timeout, unixendpoint);
}

}
