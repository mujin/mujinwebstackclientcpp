/// \brief connecting to a controller's webstack
class MUJINCLIENT_API ControllerClientInfo : public mujinjson::JsonSerializable
{
public:
    virtual void Reset();

    void LoadFromJson(const rapidjson::Value& rClientInfo) override;
    void SaveToJson(rapidjson::Value& rClientInfo, rapidjson::Document::AllocatorType& alloc) const override;
    void SaveToJson(rapidjson::Document& rClientInfo) const override;

    bool operator==(const ControllerClientInfo &rhs) const;
    bool operator!=(const ControllerClientInfo &rhs) const {
        return !operator==(rhs);
    }
    std::string GetURL(bool bIncludeNamePassword) const;

    inline bool IsEnabled() const {
        return !host.empty();
    }

    std::string host;
    uint16_t httpPort = 0; ///< Post to communicate with the webstack. If 0, then use the default port
    std::string username;
    std::string password;
    std::vector<std::string> additionalHeaders; ///< expect each value to be in the format of "Header-Name: header-value"
    std::string unixEndpoint; ///< unix socket endpoint for communicating with HTTP server over unix socket
};
