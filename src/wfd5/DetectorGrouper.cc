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

            // std::cout << "Processing waveform with key: (" 
            //           << std::get<0>(key) << ", " 
            //           << std::get<1>(key) << ", " 
            //           << std::get<2>(key) << ")\n";
            
            //Get the channel map from the service
            const auto& channelMap = channelMapService->GetChannelMap();
            
            // Check that the key is in the channel map
            if (channelMap.find(key) != channelMap.end()) {

                // Get the detector name
                std::string detectorSystem = channelMap.at(key).first;
                std::string subdetector = channelMap.at(key).second;
                thisWaveform->SetDetectorSystem(detectorSystem);
                thisWaveform->SetSubdetector(subdetector);

                // std::cout << "Waveform (crate " << thisWaveform->crateNum 
                //           << ", amc " << thisWaveform->amcNum 
                //           << ", channel " << thisWaveform->channelTag 
                //           << ") mapped to detector system: " << detectorSystem 
                //           << ", subdetector: " << subdetector << std::endl;

                // Have we made this collection yet?
                if (detectorWaveformsMap.find(detectorSystem) == detectorWaveformsMap.end()) {
                    detectorWaveformsMap[detectorSystem] = std::vector<std::shared_ptr<dataProducts::WFD5Waveform>>();
                }

                // Add the waveform to the appropriate detector collection
                detectorWaveformsMap[detectorSystem].push_back(std::move(thisWaveform));
            } else {
                // Not in channel map, so categorize as "Other"
                if (detectorWaveformsMap.find("Other") == detectorWaveformsMap.end()) {
                    detectorWaveformsMap["Other"] = std::vector<std::shared_ptr<dataProducts::WFD5Waveform>>();
                }
                thisWaveform->SetDetectorSystem("Other");
                thisWaveform->SetSubdetector("Other");
                detectorWaveformsMap["Other"].push_back(std::move(thisWaveform));
            }
        }

        // Store each detector's collections with a unique label
        for (const auto& [detectorName, waveforms] : detectorWaveformsMap) {
            //remove whitespace from detectorName
            std::string cleanDetectorName = detectorName;
            cleanDetectorName.erase(
                std::remove_if(cleanDetectorName.begin(), cleanDetectorName.end(), ::isspace),
                cleanDetectorName.end()
            );
            store.put(this->GetRecoLabel(), outputWaveformsBaseLabel_ + cleanDetectorName, std::move(waveforms));
        }

        // std::cout << "DetectorGrouper: corrected " << waveforms.size() << " waveforms.\n";

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DetectorGrouper error: ") + e.what());
    }
}