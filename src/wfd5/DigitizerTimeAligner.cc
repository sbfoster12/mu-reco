#include "reco/wfd5/DigitizerTimeAligner.hh"
#include <iostream>

using namespace reco;

void DigitizerTimeAligner::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "CorrectedWaveforms");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "AlignedWaveforms");
}

void DigitizerTimeAligner::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "DigitizerTimeAligner with name '" << GetLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

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

             ApplyTimeAligner(newWaveform);
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DigitizerTimeAligner error: ") + e.what());
    }
}

void DigitizerTimeAligner::ApplyTimeAligner(dataProducts::WFD5Waveform* wf) {
}