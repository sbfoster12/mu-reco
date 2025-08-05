#include "reco/wfd5/ChannelMapService.hh"

using namespace reco;

 void ChannelMapService::Configure(const nlohmann::json& config) {

    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();
    
    std::cout << "-> reco::ChannelMapService: Configuring ChannelMapService for run: " << run << ", subrun: " << subrun << std::endl;

    // Get the configuration file for the channel map
    if (!config.contains("config_files")) {
        throw std::runtime_error("ChannelMapService configuration must contain 'config_files' key");
    }
    const auto& configFiles = config["config_files"];
    if (configFiles.empty()) {
        throw std::runtime_error("ChannelMapService configuration must contain at least one config file");
    }

    // Add logic here to choose config file from the run number
    if (!configFiles.contains("config_file_0")) {
        throw std::runtime_error("ChannelMapService configuration must contain 'config_file_0' key");
    }
    const auto& configFile = configFiles["config_file_0"];
    if (!configFile.contains("file")) {
        throw std::runtime_error("ChannelMapService configuration file must contain 'file' key");
    }
    // Parse the channel map file
    try {
        std::string configFileName = configFile.value("file","");
        std::string configFilePath = "";
        if (configFileName.find('/') != std::string::npos) {
            // If not a base name, try using this path directly
            configFilePath = configFileName;
        } else {
            // If a base name, prepend the config directory
            configFilePath = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + configFileName;
        }
        std::cout << "-> reco::ChannelMapService: Loading channel map from file: " << configFilePath << std::endl;
        auto channelMapJson = jsonParserUtil.ParseFile(configFilePath);

        if (!channelMapJson.contains("channelMap")) {
            throw std::runtime_error("Channel map JSON must contain 'channelMap' key");
        }
        const auto& channelMapArray = channelMapJson["channelMap"];
        if (!channelMapArray.is_array()) {
            throw std::runtime_error("'channelMap' key must contain an array");
        }
        for (const auto& entry : channelMapArray) {
            if (!entry.contains("crateNum") || !entry.contains("amcSlotNum") || !entry.contains("channelNum") ||
                !entry.contains("detectorSystem") || !entry.contains("subdetector")) {
                throw std::runtime_error("Each channel map entry must contain 'crateNum', 'amcSlotNum', 'channelNum', 'detectorSystem', and 'subdetector' keys");
            }
            int crateNum = entry["crateNum"];
            int amcSlotNum = entry["amcSlotNum"];
            int channelNum = entry["channelNum"];
            std::string detectorSystem = entry["detectorSystem"];
            std::string subdetector = entry["subdetector"];

            channelMap_[std::make_tuple(crateNum, amcSlotNum, channelNum)] = {detectorSystem, subdetector};
        }
        std::cout << "-> reco::ChannelMapService: Successfully loaded channel map with "
                    << channelMap_.size() << " entries." << std::endl;
        // Print the loaded channel map for debugging
        for (const auto& [key, value] : channelMap_) {
            const auto& [crateNum, amcSlotNum, channelNum] = key;
            const auto& [detectorSystem, subdetector] = value;
            std::cout << "-> reco::ChannelMapService: Crate: " << crateNum
                        << ", AMC Slot: " << amcSlotNum
                        << ", Channel: " << channelNum
                        << " -> Detector System: " << detectorSystem
                        << ", Subdetector: " << subdetector << std::endl;
        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse channel map file: " + std::string(e.what()));
    }
}