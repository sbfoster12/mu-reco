#include "reco/wfd5/JitterCorrector.hh"
#include <iostream>

using namespace reco;

void JitterCorrector::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WaveformsCorreted");
    templateLoaderServiceLabel_ = config.value("templateLoaderServiceLabel", "templateLoader");
    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);

    // Set up the parser
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();

    // Get the pedestal configuration from the IOV list using the run and subrun
    auto pedestalConfig =  jsonParserUtil.GetConfigFromIOVList(config, run, subrun, "pedestals_iov", debug_);
    if (pedestalConfig.empty()) {
        throw std::runtime_error("JitterCorrector configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
    }   

    for (const auto& configi : pedestalConfig["pedestals"]) 
    {
        offsetMap_[std::make_tuple(configi["crateNum"], configi["amcSlotNum"], configi["channelNum"])] = configi["pedestal"];
        if (debug_) std::cout << "Loading configuration for odd/even difference in channel ("
            << configi["crateNum"]      << " / "
            << configi["amcSlotNum"]        << " / "
            << configi["channelNum"]    << ") -> " 
            << configi["pedestal"]
            << std::endl;
    }
}

void JitterCorrector::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "JitterCorrector with name '" << GetRecoLabel() << "' is processing...\n";
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

            ApplyJitterCorrection(newWaveform);
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("JitterCorrector error: ") + e.what());
    }
}

void JitterCorrector::ApplyJitterCorrection(dataProducts::WFD5Waveform* wf) const {
    // Implement jitter correction here
    if (offsetMap_.count(wf->GetID()))
    {
        if (debug_) std::cout << "Correcting pedestal difference found for channel"
            << std::get<0>(wf->GetID()) << " / "
            << std::get<1>(wf->GetID()) << " / "
            << std::get<2>(wf->GetID()) << " with " 
            << offsetMap_.at(wf->GetID())
            << std::endl;
        wf->JitterCorrect(
            offsetMap_.at(wf->GetID())
        );
    }
    else if (failOnError_)
    {
        std::cerr << "Warning: no odd/even pedestal difference found for channel"
            << std::get<0>(wf->GetID()) << " / "
            << std::get<1>(wf->GetID()) << " / "
            << std::get<2>(wf->GetID()) 
            << std::endl;
        throw;
    }
}