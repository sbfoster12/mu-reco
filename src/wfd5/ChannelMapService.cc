#include "reco/wfd5/ChannelMapService.hh"

using namespace reco;

 void ChannelMapService::Configure(const nlohmann::json& config, EventStore& eventStore) {

    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();
    
    std::cout << "-> reco::ChannelMapService: Configuring ChannelMapService for run: " << run << ", subrun: " << subrun << std::endl;

    // Get the top level json file (it lists all channel map files)
    if (!config.contains("channel_map_config_files")) {
        throw std::runtime_error("ChannelMapService configuration must contain 'channel_map_config_files' key");
    }

    // Open the top level file
    auto topFileName = config.value("channel_map_config_files","");
    std::string topFilePath = "";
    if (topFileName.find('/') != std::string::npos) {
        // If not a base name, try using this path directly
        topFilePath = topFileName;
    } else {
        // If a base name, prepend the config directory
        topFilePath = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + topFileName;
    }
    std::cout << "-> reco::ChannelMapService: Loading top level channel map file from " << topFilePath << std::endl;
    auto allChannelMapsJson = jsonParserUtil.ParseFile(topFilePath);

    // Now we want to find the correct configuration file for the given run and subrun
    if (!allChannelMapsJson.contains("channel_map_config_files")) {
        throw std::runtime_error("ChannelMapService configuration file must contain 'channel_map_config_files' key");
    }
    const auto& configFiles = allChannelMapsJson["channel_map_config_files"];

    // Get the appropriate configuration file for the given run and subrun
    std::string configFileName = GetFileFromRunSubrun(run, subrun, configFiles);

    if (configFileName.empty()) {
        throw std::runtime_error("ChannelMapService configuration file must contain 'file' key");
    }
    // Parse the channel map file
    try {
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
            
            channelConfigMap_[std::make_tuple(crateNum, amcSlotNum, channelNum)] = ChannelConfig(entry);
        }
        std::cout << "-> reco::ChannelMapService: Successfully loaded channel map with "
                    << channelConfigMap_.size() << " entries." << std::endl;
        // Print the loaded channel map for debugging
        for (const auto& [key, value] : channelConfigMap_) {
            const auto& [crateNum, amcSlotNum, channelNum] = key;
            std::cout << "-> reco::ChannelMapService: ";
            std::cout << "(crate: " << crateNum
                      << ", amcSlot: " << amcSlotNum
                      << ", channel " << channelNum
                      << ") -> ";
            value.Print();

        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse channel map file: " + std::string(e.what()));
    }
}

std::string ChannelMapService::GetFileFromRunSubrun(int run, int subrun, const nlohmann::json& configFiles) {
    // This function retrieves the appropriate configuration file based on the run and subrun numbers
    // It requires that the configFiles is a JSON array of configFile, which each have an iov and file field
    for (const auto& configFile : configFiles) {
        if (!configFile.contains("iov") || !configFile["iov"].is_array() || configFile["iov"].size() != 2) {
            throw std::runtime_error("Each configuration file must contain 'iov' key with an array of two elements");
            return "";
        }
        int startRun = configFile["iov"][0];
        int endRun = configFile["iov"][1];
        if (run >= startRun && run <= endRun) {
            if (!configFile.contains("file")) {
                throw std::runtime_error("Configuration file must contain 'file' key");
                return "";
            }
            std::cout << "-> reco::ChannelMapService: Found configuration file for run: " << run
                      << ", subrun: " << subrun << " -> " << configFile["file"]
                      << " with IOV: [" << startRun << ", " << endRun << "]" << std::endl;
            return configFile["file"];
        }
    }

    return "";
}