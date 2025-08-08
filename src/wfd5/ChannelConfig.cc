#include "reco/wfd5/ChannelConfig.hh"

ChannelConfig::ChannelConfig(const nlohmann::json& json) {
    crateNum_= json.value("crateNum", -1);
    amcSlotNum_ = json.value("amcSlotNum", -1);
    channelNum_ = json.value("channelNum", -1);
    detectorSystem_ = json.value("detectorSystem", "");
    subdetector_ = json.value("subdetector", "");
}

const std::string& ChannelConfig::GetDetectorSystem() const {
    return detectorSystem_;
}

const std::string& ChannelConfig::GetSubdetector() const {
    return subdetector_;
}

int ChannelConfig::GetCrateNum() const {
    return crateNum_;
}

int ChannelConfig::GetAmcSlotNum() const {
    return amcSlotNum_;
}

int ChannelConfig::GetChannelNum() const {
    return channelNum_;
}

void ChannelConfig::SetDetectorSystem(const std::string& system) {
    detectorSystem_ = system;
}

void ChannelConfig::SetSubdetector(const std::string& sub) {
    subdetector_ = sub;
}

void ChannelConfig::SetCrateNum(int crateNum) {
    crateNum_ = crateNum;
}

void ChannelConfig::SetAmcSlotNum(int amcSlotNum) {
    amcSlotNum_ = amcSlotNum;
}

void ChannelConfig::SetChannelNum(int channelNum) {
    channelNum_ = channelNum;
}

void ChannelConfig::Print() const {
    std::cout << "Crate: " << crateNum_
                << ", AMC Slot: " << amcSlotNum_
                << ", Channel: " << channelNum_
                << ", Detector System: " << detectorSystem_
                << ", Subdetector: " << subdetector_ << std::endl;
}
