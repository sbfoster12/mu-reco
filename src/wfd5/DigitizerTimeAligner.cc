#include "reco/wfd5/DigitizerTimeAligner.hh"
#include <iostream>

using namespace reco;

void DigitizerTimeAligner::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "CorrectedWaveforms");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "AlignedWaveforms");

    inputT0Reco_ = config.value("inputT0Reco", "t0PeakLocator" );
    inputT0Label_ = config.value("inputT0Label", "t0" );
    requireT0Seed_ = config.value("requireT0Seed", false);
    debug_ = config.value("debug", false);

    channelMapServiceLabel_ = config.value("channelMapServiceLabel", "channelMap");
    auto channelMapService = serviceManager.Get<reco::ChannelMapService>(channelMapServiceLabel_);
    if (!channelMapService) {
        throw std::runtime_error("ChannelMapService not found: " + channelMapServiceLabel_);
    }
    if (debug_) std::cout << "Setting up known time offset map:" << std::endl;
    for (auto& map_entry:channelMapService->GetChannelMap())
    {
        if (debug_) std::cout << "   -> found time offset " << map_entry.second.GetTimeOffset() << " for channel ("
            << std::get<0>(map_entry.first) << " / "
            << std::get<1>(map_entry.first) << " / "
            << std::get<2>(map_entry.first) << ")"
            << std::endl;
        knownTimeOffsetMap_[map_entry.first] = map_entry.second.GetTimeOffset();
    }



}

void DigitizerTimeAligner::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "DigitizerTimeAligner with name '" << GetLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);
        auto seeds = store.get<const dataProducts::TimeSeed>(inputT0Reco_, inputT0Label_);

        foundSeed_ = false;
        dataProducts::TimeSeed* seed = static_cast<dataProducts::TimeSeed*>(seeds->ConstructedAt(0));
        dataProducts::WFD5Waveform* seed_wf;
        if (!seed) {
            if (requireT0Seed_) throw std::runtime_error("Failed to retrieve T0 time seed");
            seed = new dataProducts::TimeSeed(); // else construct a default seed object.
        }
        else {
            foundSeed_ = true;
            seed_wf = (dataProducts::WFD5Waveform*) ((seed->inputs[0]).GetObject());
        }


        //Make a collection new waveforms
        auto newWaveforms = store.getOrCreate<dataProducts::WFD5Waveform>(this->GetRecoLabel(), outputWaveformsLabel_);

        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }
            //Make the new waveform
            dataProducts::WFD5Waveform* newWaveform = new ((*newWaveforms)[i]) dataProducts::WFD5Waveform(waveform);
            newWaveforms->Expand(i + 1);

            ApplyTimeAligner(newWaveform, seed, seed_wf);
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DigitizerTimeAligner error: ") + e.what());
    }
}

void DigitizerTimeAligner::ApplyTimeAligner(dataProducts::WFD5Waveform* wf, dataProducts::TimeSeed *seed, dataProducts::WFD5Waveform* seed_wf) {
    if (debug_) std::cout << "Applying time alignment to waveform " << wf << std::endl;
    double known_offset = 0.0;
    if (knownTimeOffsetMap_.count(wf->GetID()))
    {
        known_offset = knownTimeOffsetMap_[wf->GetID()];
    }
    if (foundSeed_)
    {
        if (debug_) std::cout << "   -> Found time seed:" << seed << " with time " << seed->GetTimeSeed() << std::endl;
        if (debug_) std::cout << "   -> known offset for this channel: " << known_offset << std::endl;
        // get clock counter difference between this waveform and the t0 reference waveform
        // auto seed_cc = seed_wf->clockCounter;
        wf->digitizationShift = int(seed_wf->clockCounter) - int(wf->clockCounter);
        if (debug_) std::cout   << "    -> Clock counter shift: " << seed_wf->clockCounter 
                                << " - " << wf->clockCounter << " = " << wf->digitizationShift 
                                << std::endl;
        wf->SetTimeOffset(seed->GetTimeSeed() + known_offset);
        // 
    }
    else
    {   
        if (debug_) std::cout << "   -> WARNING: No seed found... Only setting the known offset." << std::endl;
        wf->SetTimeOffset(known_offset);
    }

    // TODO: add application of custom cable length offsets



}