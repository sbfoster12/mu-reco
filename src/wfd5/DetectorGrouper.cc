#include "reco/wfd5/DetectorGrouper.hh"
#include <iostream>

using namespace reco;

void DetectorGrouper::Configure(const nlohmann::json& config, const ServiceManager& serviceManager) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsBaseLabel_ = config.value("outputWaveformsBaseLabel", "WFD5WaveformCollection");
    channelMapServiceLabel_ = config.value("channelMapServiceLabel", "channelMap");
}

void DetectorGrouper::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "DetectorGrouper with name '" << GetLabel() << "' is processing...\n";
    try {
        // Get the input waveforms as const shared_ptr collection (safe because get is const)
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Get the channel map service
        auto channelMapService = serviceManager.Get<reco::ChannelMapService>(channelMapServiceLabel_);
        
        if (!channelMapService) {
            throw std::runtime_error("ChannelMapService not found: " + channelMapServiceLabel_);
        }

        // Make map for all the individual detector waveform collections
        std::map<std::string,std::vector<std::shared_ptr<dataProducts::WFD5Waveform>>> detectorWaveformsMap;

        // Print out the channel map for debugging

        for (const auto& wf : waveforms) {
            // Make a copy of the waveforms
            auto thisWaveform = std::make_shared<dataProducts::WFD5Waveform>(*wf);

            // Create the key
            auto key = std::make_tuple(thisWaveform->crateNum, thisWaveform->amcNum, thisWaveform->channelTag);
            
            //Get the channel map from the service
            const auto& channelMap = channelMapService->GetChannelMap();
            
            // Check that the key is in the channel map
            if (channelMap.find(key) != channelMap.end()) {

                // Get the detector name
                std::string detectorName = channelMap.at(key);

                // Have we made this collection yet?
                if (detectorWaveformsMap.find(detectorName) == detectorWaveformsMap.end()) {
                    detectorWaveformsMap[detectorName] = std::vector<std::shared_ptr<dataProducts::WFD5Waveform>>();
                }

                // Add the waveform to the appropriate detector collection
                detectorWaveformsMap[detectorName].push_back(std::move(thisWaveform));
            } else {
                // Not in channel map, so categorize as "OTHER"
                if (detectorWaveformsMap.find("OTHER") == detectorWaveformsMap.end()) {
                    detectorWaveformsMap["OTHER"] = std::vector<std::shared_ptr<dataProducts::WFD5Waveform>>();
                }
                detectorWaveformsMap["OTHER"].push_back(std::move(thisWaveform));
            }
        }

        // Store each detector's collections with a unique label
        for (const auto& [detectorName, waveforms] : detectorWaveformsMap) {
            store.put(this->GetRecoLabel(), outputWaveformsBaseLabel_ + detectorName, std::move(waveforms));
        }

        // std::cout << "DetectorGrouper: corrected " << waveforms.size() << " waveforms.\n";

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DetectorGrouper error: ") + e.what());
    }
}