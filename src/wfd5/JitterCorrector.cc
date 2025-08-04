#include "reco/wfd5/JitterCorrector.hh"
#include <iostream>

using namespace reco;

void JitterCorrector::Configure(const nlohmann::json& config, const ServiceManager& serviceManager) {

    correctionFactor_ = config.value("correctionFactor", 1.0);
    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WFD5WaveformCorrectedCollection");
    templateServiceLabel_ = config.value("templateServiceLabel", "TemplateService");
}

void JitterCorrector::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "JitterCorrector with name '" << GetLabel() << "' is processing...\n";
    try {
        // Get original waveforms as const shared_ptr collection (safe because get is const)
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Make new collection for corrected waveforms
        std::vector<std::shared_ptr<dataProducts::WFD5Waveform>> correctedWaveforms;
        correctedWaveforms.reserve(waveforms.size());

        //Get the template service (example of getting a service)
        auto templateService = serviceManager.Get<TemplateService>(templateServiceLabel_);

        for (const auto& wf : waveforms) {
            // Make a copy for correction
            auto corrected = std::make_shared<dataProducts::WFD5Waveform>(*wf);

            ApplyJitterCorrection(corrected);

            //Fill a histogram for fun
            store.GetHistogram("energy")->Fill(50);

            correctedWaveforms.push_back(std::move(corrected));
        }

        // Store corrected waveforms under a new key
        store.put(this->GetRecoLabel(), outputWaveformsLabel_, std::move(correctedWaveforms));

        // std::cout << "JitterCorrector: corrected " << waveforms.size() << " waveforms.\n";

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("JitterCorrector error: ") + e.what());
    }
}

void JitterCorrector::ApplyJitterCorrection(std::shared_ptr<dataProducts::WFD5Waveform>& wf) {
    // Implement jitter correction here
}