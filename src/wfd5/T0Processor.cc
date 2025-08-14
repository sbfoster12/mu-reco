#include "reco/wfd5/T0Processor.hh"
#include <iostream>

using namespace reco;

void T0Processor::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputT0TimeRefLabel_ = config.value("outputT0TimeRefLabel", "t0");

    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);
    defaultTime_ = config.value("defaultTime",0.0);

    triggerSearchWindow_ = {
        config.value("triggerWindowLow", 0),
        config.value("triggerWindowHigh", 1000)
    };
    failIfT0OutsideWindow_ = config.value("failIfT0OutsideWindow", false);
    failIfT0NotFound_ = config.value("failIfT0NotFound", false);
    if (config.contains("t0Channel"))
    {
        std::vector<int> id = config["t0Channel"];
        t0Channel_ = dataProducts::ChannelID(id[0], id[1], id[2]);
    }

    channelMapServiceLabel_ = config.value("channelMapServiceLabel", "channelMap");
    auto channelMapService = serviceManager.Get<reco::ChannelMapService>(channelMapServiceLabel_);
    if (!channelMapService) {
        throw std::runtime_error("ChannelMapService not found: " + channelMapServiceLabel_);
    }

    if (debug_) std::cout << "Getting the T0 channel:" << std::endl;
    int nt0 = 0;
    for (auto& map_entry:channelMapService->GetChannelMap())
    {
        if (map_entry.second.GetSubdetector().find("T0") != std::string::npos)
        {
            nt0 += 1;
            t0Channel_ = map_entry.first;
            if (debug_) std::cout << "   -> found t0 channel in config with labels " 
                << map_entry.second.GetDetectorSystem() << " / " << map_entry.second.GetSubdetector() 
                << " -> ("
                << std::get<0>(map_entry.first) << " / "
                << std::get<1>(map_entry.first) << " / "
                << std::get<2>(map_entry.first) << ")"
                << std::endl;
        }
    }

    if (nt0 > 1)
    {
        throw std::runtime_error("Error, during search of the channel map found "+std::to_string(nt0)+" 't0' detectors in config");
    }
    else if (nt0 == 0)
    {
        std::cout << "Warning: could not find t0 in channel map, defaulting to t0Channel from config -> (" 
                << std::get<0>(t0Channel_) << " / "
                << std::get<1>(t0Channel_) << " / "
                << std::get<2>(t0Channel_) << ")"
                << std::endl;
    }

    // Create some histograms
    // auto hist = std::make_shared<TH1D>("energy", "Energy Spectrum", 100, 0, 1000);
    // eventStore.putHistogram("energy", std::move(hist));
}

void T0Processor::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "T0Processor with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);
        int peakIndex;

        //Make a collection new waveforms
        auto output = store.getOrCreate<dataProducts::TimeSeed>(this->GetRecoLabel(), outputT0TimeRefLabel_);
        dataProducts::TimeSeed* seed = new ((*output)[0]) dataProducts::TimeSeed();
        output->Expand(1);
        
        bool t0Found = false;
        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }
            if (waveform->GetID() == t0Channel_)
            {
                t0Found = true;
                peakIndex = waveform->GetPeakIndexInBounds(triggerSearchWindow_.first, triggerSearchWindow_.second);
                if (debug_) std::cout << "Located T0 peak for seed at index " << peakIndex << " -> time = " << waveform->GetTime(peakIndex) << std::endl;
                if (debug_) waveform->Show();

                seed->SetSeed(
                    waveform->GetTime(peakIndex)
                );
                seed->inputs.push_back(waveform);
                seed->seedWindow = triggerSearchWindow_;
                break;
            }
        }
        if (!t0Found)
        {
            if(debug_) std::cout << "Warning: unable to find the t0 waveform to seed" << std::endl;
            if (failOnError_) throw std::runtime_error("Unable to find the T0 waveform and set to fail!");
            if(debug_) std::cout << "    -> defaulting to: " << defaultTime_ << std::endl;
            seed->SetSeed(defaultTime_);
        }
        // else if ()
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("T0Processor error: ") + e.what());
    }
}
