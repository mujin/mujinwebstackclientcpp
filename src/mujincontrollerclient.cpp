void ControllerClientInfo::Reset()
{
    host.clear();
    httpPort = 0;
    username.clear();
    password.clear();
    additionalHeaders.clear();
    unixEndpoint.clear();
}

void ControllerClientInfo::LoadFromJson(const rapidjson::Value& rClientInfo)
{
    mujinjson::LoadJsonValueByKey(rClientInfo, "host", host);
    mujinjson::LoadJsonValueByKey(rClientInfo, "httpPort", httpPort);
    mujinjson::LoadJsonValueByKey(rClientInfo, "username", username);
    mujinjson::LoadJsonValueByKey(rClientInfo, "password", password);
    mujinjson::LoadJsonValueByKey(rClientInfo, "additionalHeaders", additionalHeaders);
    mujinjson::LoadJsonValueByKey(rClientInfo, "unixEndpoint", unixEndpoint);
}

void ControllerClientInfo::SaveToJson(rapidjson::Value& rClientInfo, rapidjson::Document::AllocatorType& alloc) const
{
    rClientInfo.SetObject();
    if( !host.empty() ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "host", host, alloc);
    }
    if( httpPort != 0 ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "httpPort", httpPort, alloc);
    }
    if( !username.empty() ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "username", username, alloc);
    }
    if( !password.empty() ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "password", password, alloc);
    }
    if( !additionalHeaders.empty() ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "additionalHeaders", additionalHeaders, alloc);
    }
    if( !unixEndpoint.empty() ) {
        mujinjson::SetJsonValueByKey(rClientInfo, "unixEndpoint", unixEndpoint, alloc);
    }
}

void ControllerClientInfo::SaveToJson(rapidjson::Document& rClientInfo) const
{
    SaveToJson(rClientInfo, rClientInfo.GetAllocator());
}

bool ControllerClientInfo::operator==(const ControllerClientInfo &rhs) const
{
    return host == rhs.host &&
           httpPort == rhs.httpPort &&
           username == rhs.username &&
           password == rhs.password &&
           additionalHeaders == rhs.additionalHeaders &&
           unixEndpoint == rhs.unixEndpoint;
}

std::string ControllerClientInfo::GetURL(bool bIncludeNamePassword) const
{
    std::string url;
    if( host.empty() ) {
        return url;
    }
    url += "http://";
    if( bIncludeNamePassword ) {
        url += username;
        url += ":";
        url += password;
        url += "@";
    }

    url += host;
    if( httpPort != 0 ) {
        url += ":";
        url += std::to_string(httpPort);
    }
    return url;
}
