
#include "reco/wfd5/RFFitter.hh"
#include <iostream>

using namespace reco;

void RFFitter::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "CorrectedWaveforms");
    outputFitResultLabel_ = config.value("outputFitResultLabel", "FitResultXtal");
}

void RFFitter::Process(EventStore& store, const ServiceManager& serviceManager) {
    // // std::cout << "RFFitter with name '" << GetLabel() << "' is processing...\n";
    // try {
    //     // Get original waveforms as const shared_ptr collection (safe because get is const)
    //     auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

    //     //Make a collection of fit results
    //     // std::vector<std::shared_ptr<dataProducts::WFD5Waveform>> correctedWaveforms;
    //     // correctedWaveforms.reserve(waveforms.size());

    //     for (const auto& wf : waveforms) {
    //         // // Make a fit result and add it to the collection here
    //         // auto corrected = std::make_shared<dataProducts::WFD5Waveform>(*wf);
    //         // correctedWaveforms.push_back(std::move(corrected));
    //     }

    //     // Store corrected fit results under a new key
    //     // store.put(this->GetRecoLabel(), outputFitResultLabel_, std::move(fitResults));

    //     // std::cout << "RFFitter: corrected " << waveforms.size() << " waveforms.\n";

    // } catch (const std::exception& e) {
    //    throw std::runtime_error(std::string("RFFitter error: ") + e.what());
    // }
}