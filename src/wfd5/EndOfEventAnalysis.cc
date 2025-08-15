#include "reco/wfd5/EndOfEventAnalysis.hh"
#include <iostream>

using namespace reco;

void EndOfEventAnalysis::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    lysoRecoLabel_ = config.value("lysoRecoLabel", "");
    lysoWaveformsLabel_ = config.value("lysoWaveformsLabel", "");

    // Make a histogram or two or more!
    eventStore.putHistogram("h_lyso_pedestals", std::make_shared<TH1D>("h_lyso_pedestals", "Pedestals", 2000, -2000, 0));
}

void EndOfEventAnalysis::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "EndOfEventAnalysis with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto lyso_waveforms = store.get<const dataProducts::WFD5Waveform>(lysoRecoLabel_, lysoWaveformsLabel_);

        for (int i = 0; i < lyso_waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(lyso_waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }

            // Fill the histogram
            store.GetHistogram("h_lyso_pedestals")->Fill(waveform->pedestalLevel);
 
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("EndOfEventAnalysis error: ") + e.what());
    }
}