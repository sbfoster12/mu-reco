#include "reco/wfd5/ChannelMapService.hh"

using namespace reco;

 void ChannelMapService::Configure(const nlohmann::json& config, EventStore& eventStore) {

    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();
    
    std::cout << "-> reco::ChannelMapService: Configuring ChannelMapService for run: " << run << ", subrun: " << subrun << std::endl;

    // Check the parameter exists
    if (!config.contains("channel_map_iov")) {
        throw std::runtime_error("ChannelMapService configuration must contain 'channel_map_iov' key");
    }

    // Get the list of channel map configurations per iov
    auto iovConfigListFileName = config.value("channel_map_iov","");
    std::string iovConfigListFilePath = "";
    auto iovConfigJson = jsonParserUtil.GetPathAndParseFile(iovConfigListFileName, iovConfigListFilePath);
    
    // Check that the list exists
    if (!iovConfigJson.contains("channel_map_iov")) {
        throw std::runtime_error("ChannelMapService: File " + iovConfigListFilePath + " must contain 'channel_map_iov' key");
    }
    auto iovConfigList = iovConfigJson["channel_map_iov"];
    if (!iovConfigList.is_array()) {
        throw std::runtime_error("'channel_map_iov' key must contain an array");
    }
    
    // Determine the correct configuration based on run and subrun
    auto iovConfigMatch = jsonParserUtil.GetIOVMatch(iovConfigList, run, subrun);

    // Now get the actual configuration now
    std::string configFileName = iovConfigMatch.value("file","");

    if (configFileName.empty()) {
        throw std::runtime_error("ChannelMapService configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
    }
    // Parse the channel map file
    try {
        std::string configFilePath = "";
        auto channelMapJson = jsonParserUtil.GetPathAndParseFile(configFileName, configFilePath);
        if (channelMapJson.empty()) {
            throw std::runtime_error("ChannelMapService configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
        }
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