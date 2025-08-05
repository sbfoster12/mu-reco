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

    // Get the appropriate configuration file for the given run and subrun
    const auto& configFile = GetFileFromRunSubrun(run, subrun, configFiles);

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

const nlohmann::json& ChannelMapService::GetFileFromRunSubrun(int run, int subrun, const nlohmann::json& configFiles) {
    // This function retrieves the appropriate configuration file based on the run and subrun numbers
    // It assumes that the configFiles is a JSON object with keys as file names and values as IOV ranges
    for (const auto& [key, value] : configFiles.items()) {
        if (value.contains("iov") && value["iov"].is_array() && value["iov"].size() == 2) {
            int startRun = value["iov"][0];
            int endRun = value["iov"][1];
            if (run >= startRun && run <= endRun) {
                std::cout << "-> reco::ChannelMapService: Found configuration file for run: " << run
                          << ", subrun: " << subrun << " -> " << key << " with IOV: [" << startRun << ", " << endRun << "]" << std::endl;
                return configFiles[key];
            }
        }
    }
    throw std::runtime_error("No configuration file found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
}