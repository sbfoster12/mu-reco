#include "reco/wfd5/WaveformInitializer.hh"
#include <iostream>

using namespace reco;

void WaveformInitializer::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "");
    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);
}

void WaveformInitializer::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "WaveformInitializer with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        // Get the odb
        auto odb = dynamic_cast<dataProducts::WFD5ODB*>(store.GetODB().get());

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

            newWaveform->SetRunSubrun(store.GetRun(), store.GetSubrun());
            newWaveform->SetDigitizationFrequency(odb->GetDigitizationFrequency(waveform->amcNum));

        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("WaveformInitializer error: ") + e.what());
    }
}