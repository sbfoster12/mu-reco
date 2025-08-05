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
        // Get original waveforms as const shared_ptr collection (safe because get is const)
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Make new collection for corrected waveforms
        std::vector<std::shared_ptr<dataProducts::WFD5Waveform>> correctedWaveforms;
        correctedWaveforms.reserve(waveforms.size());

        for (const auto& wf : waveforms) {
            // Make a copy for correction
            auto corrected = std::make_shared<dataProducts::WFD5Waveform>(*wf);

            correctedWaveforms.push_back(std::move(corrected));
        }

        // Store corrected waveforms under a new key
        store.put(this->GetRecoLabel(), outputWaveformsLabel_, std::move(correctedWaveforms));

        // std::cout << "DigitizerTimeAligner: corrected " << waveforms.size() << " waveforms.\n";

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("DigitizerTimeAligner error: ") + e.what());
    }
}