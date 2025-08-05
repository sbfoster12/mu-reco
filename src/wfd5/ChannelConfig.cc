#include "reco/wfd5/ChannelConfig.hh"

ChannelConfig::ChannelConfig(const std::string& system, const std::string& sub)
    : detectorSystem_(system), subdetector_(sub) {}

const std::string& ChannelConfig::GetDetectorSystem() const {
    return detectorSystem_;
}

const std::string& ChannelConfig::GetSubdetector() const {
    return subdetector_;
}

void ChannelConfig::SetDetectorSystem(const std::string& system) {
    detectorSystem_ = system;
}

void ChannelConfig::SetSubdetector(const std::string& sub) {
    subdetector_ = sub;
}
