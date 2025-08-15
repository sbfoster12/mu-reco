#include "reco/wfd5/ChannelMapService.hh"

using namespace reco;

 void ChannelMapService::Configure(const nlohmann::json& config, EventStore& eventStore) {

    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();
    
    std::cout << "-> reco::ChannelMapService: Configuring ChannelMapService for run: " << run << ", subrun: " << subrun << std::endl;

    // Get the top level json file (it lists all channel map files)
    if (!config.contains("channel_map_files")) {
        throw std::runtime_error("ChannelMapService configuration must contain 'channel_map_files' key");
    }

    // Open the top level file
    auto topFileName = config.value("channel_map_files","");
    std::string configFileName = jsonParserUtil.GetFileFromRunSubrun(run, subrun, topFileName, "channel_map_files");

    if (configFileName.empty()) {
        throw std::runtime_error("ChannelMapService configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
    }
    // Parse the channel map file
    try {
        std::string configFilePath = "";
        auto channelMapJson = jsonParserUtil.GetPathAndParseFile(configFileName, configFilePath);
        std::cout << "-> reco::ChannelMapService: Loading channel map from file: " << configFilePath << std::endl;

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