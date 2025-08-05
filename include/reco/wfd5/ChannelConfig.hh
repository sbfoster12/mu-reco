#ifndef CHANNEL_CONFIG_HH
#define CHANNEL_CONFIG_HH

#include <string>

class ChannelConfig {
public:
    ChannelConfig() = default;
    ChannelConfig(const std::string& system, const std::string& sub);

    const std::string& GetDetectorSystem() const;
    const std::string& GetSubdetector() const;

    void SetDetectorSystem(const std::string& system);
    void SetSubdetector(const std::string& sub);

private:
    std::string detectorSystem_;
    std::string subdetector_;
};

#endif // CHANNEL_CONFIG_HH
