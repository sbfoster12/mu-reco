#include "reco/wfd5/DetectorGrouper.hh"
#include <iostream>

using namespace reco;

void DetectorGrouper::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsBaseLabel_ = config.value("outputWaveformsBaseLabel", "WFD5WaveformCollection");
    channelMapServiceLabel_ = config.value("channelMapServiceLabel", "channelMap");
}

void DetectorGrouper::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "DetectorGrouper with name '" << GetLabel() << "' is processing...\n";
    try {
        // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Get the channel map service
        auto channelMapService = serviceManager.Get<reco::ChannelMapService>(channelMapServiceLabel_);
        
        if (!channelMapService) {
            throw std::runtime_error("ChannelMapService not found: " + channelMapServiceLabel_);
        }

        // Make map for all the individual detector waveform collections
        std::map<std::string,TClonesArray*> detectorWaveformsMap;

        // Loop over the waveforms
        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));

            // Create the key
            auto key = std::make_tuple(waveform->crateNum, waveform->amcNum, waveform->channelTag);

            // std::cout << "Processing waveform with key: (" 
            //           << std::get<0>(key) << ", " 
            //           << std::get<1>(key) << ", " 
            //           << std::get<2>(key) << ")\n";
            
            //Get the channel map from the service
            const auto& channelConfigMap = channelMapService->GetChannelMap();
            
            // Check that the key is in the channel map
            if (channelConfigMap.find(key) != channelConfigMap.end()) {

                // Get the detector name
                std::string detectorSystem = channelConfigMap.at(key).GetDetectorSystem();
                std::string subdetector = channelConfigMap.at(key).GetSubdetector();

                // Make the output name
                std::string cleanDetectorName = detectorSystem;
                cleanDetectorName.erase(
                    std::remove_if(cleanDetectorName.begin(), cleanDetectorName.end(), ::isspace),
                    cleanDetectorName.end()
                );
                std::string outputWaveformsLabel = outputWaveformsBaseLabel_ + cleanDetectorName;

                // Have we retrieved this collection yet?
                if (detectorWaveformsMap.find(detectorSystem) == detectorWaveformsMap.end()) {
                    // Fill the map with the TClonesArray (get or create)
                    detectorWaveformsMap[detectorSystem] = store.getOrCreate<dataProducts::WFD5Waveform>(this->GetRecoLabel(), outputWaveformsLabel);
                }

                // Make the new waveform
                int idx = detectorWaveformsMap[detectorSystem]->GetEntriesFast();
                auto* newWaveform = new ((*detectorWaveformsMap[detectorSystem])[idx]) dataProducts::WFD5Waveform(waveform);
                detectorWaveformsMap[detectorSystem]->Expand(idx + 1);
                newWaveform->SetDetectorSystem(detectorSystem);
                newWaveform->SetSubdetector(subdetector);

                // std::cout << "Waveform (crate " << thisWaveform->crateNum 
                //           << ", amc " << thisWaveform->amcNum 
                //           << ", channel " << thisWaveform->channelTag 
                //           << ") mapped to detector system: " << detectorSystem 
                //           << ", subdetector: " << subdetector << std::endl;

            } else {
                // Not in channel map, so categorize as "Other"
                std::string outputWaveformsLabel = outputWaveformsBaseLabel_ + "Other";
                if (detectorWaveformsMap.find("Other") == detectorWaveformsMap.end()) {
                    // Fill the map with the TClonesArray (get or create)
                    detectorWaveformsMap["Other"] = store.getOrCreate<dataProducts::WFD5Waveform>(this->GetRecoLabel(), outputWaveformsLabel);
                }
                int idx = detectorWaveformsMap["Other"]->GetEntriesFast();
                auto* newWaveform = new ((*detectorWaveformsMap["Other"])[idx]) dataProducts::WFD5Waveform(*waveform);
                detectorWaveformsMap["Other"]->Expand(idx + 1);
                newWaveform->SetDetectorSystem("Other");
                newWaveform->SetSubdetector("Other");
            }
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DetectorGrouper error: ") + e.what());
    }
}