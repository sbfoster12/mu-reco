#include "reco/wfd5/EmptyChannelPruner.hh"
#include <iostream>

using namespace reco;

void EmptyChannelPruner::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {
    
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WaveformsCorreted");
    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);
    minAmplitude_ = config.value("minAmplitude",25.0);

    
    
    if (config.contains("file_name"))
    {
        std::string file_name = config.value("file_name", "minimum_amplitudes.json");  
        std::cout << "-> EmptyChannelPruner::Reading in custom minimum amplitudes from file: " << file_name << std::endl;  
        auto minAmpChannelConfig_ = jsonParserUtil.GetPathAndParseFile(file_name);  
        
        for (const auto& configi : minAmpChannelConfig_["minimumAmplitudes"]) 
        {
            std::vector<int> id = configi["channel"];
            minAmplMap_[std::make_tuple(id[0], id[1], id[2])] = configi["minimumAmplitude"];
            if (debug_) std::cout << "Loading configuration for custom min-amplitude in channel ("
                << id[0]      << " / "
                << id[1]        << " / "
                << id[2]    << ") -> " 
                << configi["minimumAmplitude"]
                << std::endl;
        }
    }

    if (config.contains("keepChannels"))
    {
        for (auto& id: config["keepChannels"])
        {
            if (debug_) std::cout << "Pruner has been set to always keep channel ("
                << id[0]      << " / "
                << id[1]        << " / "
                << id[2]    << ")" 
                << std::endl;
            dataProducts::ChannelID this_id = std::make_tuple(id[0], id[1], id[2]);
            minAmplMap_[id] = -1;

        }

    }
}

void EmptyChannelPruner::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "EmptyChannelPruner with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Make a collection new waveforms
        auto newWaveforms = store.getOrCreate<dataProducts::WFD5Waveform>(this->GetRecoLabel(), outputWaveformsLabel_);

        double thisMinAmplitude = 1e10;
        dataProducts::ChannelID this_id;

        int counter = 0;
        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }

            this_id = waveform->GetID();
            thisMinAmplitude = minAmplMap_.count(this_id) ? minAmplMap_.at(this_id) : minAmplitude_;
            if (debug_) 
            {
                std::cout << "Evaluating channel ("
                    << std::get<0>(this_id) << " / "
                    << std::get<1>(this_id) << " / "
                    << std::get<2>(this_id) << ") "
                    << "with peak to peak amplitude " 
                    << waveform->PeakToPeak() 
                    << " and set minimum amplitude " 
                    << thisMinAmplitude 
                    << std::endl;
            }

            if (waveform->PeakToPeak() >= thisMinAmplitude)
            {
                //Make the new waveform
                if (debug_) std::cout << "    -> Keeping channel!" << std::endl;
                dataProducts::WFD5Waveform* newWaveform = new ((*newWaveforms)[counter]) dataProducts::WFD5Waveform(waveform);
                newWaveforms->Expand(counter + 1);
                counter++;
            }
            else if (debug_)
            {
                std::cout << "    -> Pruned!" << std::endl;
            }
            else 
            {
                // don't expand the collection
            }
        }
        
        if(debug_) std::cout << "Pruned waveforms from " << waveforms->GetEntriesFast() 
            << " -> " << newWaveforms->GetEntriesFast() 
            << std::endl;

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("EmptyChannelPruner error: ") + e.what());
    }
}
