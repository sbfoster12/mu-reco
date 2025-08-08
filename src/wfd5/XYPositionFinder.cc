
#include "reco/wfd5/XYPositionFinder.hh"
#include <iostream>

using namespace reco;

void XYPositionFinder::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "");
    inputFitResultsLabel_ = config.value("inputFitResultsLabel", "");
    outputPositionLabel_ = config.value("outputPositionLabel", "");
}

void XYPositionFinder::Process(EventStore& store, const ServiceManager& serviceManager) {
    // // std::cout << "XYPositionFinder with name '" << GetLabel() << "' is processing...\n";
    // try {
    //     // // Get original waveforms as const shared_ptr collection (safe because get is const)
    //     // auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

    //     // // //Make a collection of fit results
    //     // // std::vector<std::shared_ptr<dataProducts::WFD5Waveform>> newWaveforms;
    //     // // newWaveforms.reserve(waveforms.size());

    //     // for (const auto& wf : waveforms) {
    //     //     // // Make a fit result and add it to the collection here
    //     //     // auto newWaveform = std::make_shared<dataProducts::WFD5Waveform>(*wf);
    //     //     // newWaveforms.push_back(std::move(newWaveform));
    //     // }

    //     // // // Store corrected fit results under a new key
    //     // // store.put(this->GetRecoLabel(), outputPeaksLabel_, std::move(newWaveforms));

    //     // // std::cout << "XYPositionFinder: corrected " << waveforms.size() << " waveforms.\n";

    // } catch (const std::exception& e) {
    //    throw std::runtime_error(std::string("XYPositionFinder error: ") + e.what());
    // }
}