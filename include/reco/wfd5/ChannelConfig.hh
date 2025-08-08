#ifndef CHANNEL_CONFIG_HH
#define CHANNEL_CONFIG_HH

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

class ChannelConfig {
public:
    ChannelConfig() = default;
    ChannelConfig(const nlohmann::json& json);

    const std::string& GetDetectorSystem() const;
    const std::string& GetSubdetector() const;
    int GetCrateNum() const;
    int GetAmcSlotNum() const;
    int GetChannelNum() const;

    void SetDetectorSystem(const std::string& system);
    void SetSubdetector(const std::string& sub);
    void SetCrateNum(int crateNum);
    void SetAmcSlotNum(int amcSlotNum);
    void SetChannelNum(int channelNum);

    void Print() const;
    
private:
    int crateNum_;
    int amcSlotNum_;
    int channelNum_;
    std::string detectorSystem_;
    std::string subdetector_;
};

#endif // CHANNEL_CONFIG_HH
