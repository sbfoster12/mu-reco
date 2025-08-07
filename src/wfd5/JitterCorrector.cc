#include "reco/wfd5/JitterCorrector.hh"
#include <iostream>

using namespace reco;

void JitterCorrector::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    correctionFactor_ = config.value("correctionFactor", 1.0);
    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WaveformsCorreted");
    templateLoaderServiceLabel_ = config.value("templateLoaderServiceLabel", "templateLoader");
    file_name_ = config.value("file_name", "pedestals.json");
    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);

    // std::string file_name = config.value("file_name", "templates.json");
    std::string file_path_ = "";
    if (file_name_.find('/') != std::string::npos) {
        // If not a base name, try using this path directly
        file_path_ = file_name_;
    } else {
        // If a base name, prepend the config directory
        file_path_ = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + file_name_;
    }
    if (!std::filesystem::exists(file_path_)) {
        throw std::runtime_error("JitterCorrector: File not found: " + file_path_);
    }
    std::cout << "-> reco::JitterCorrector: Configuring with file: " << file_path_ << std::endl;
    
    auto& jsonParserUtil = reco::JsonParserUtil::instance();
    auto pedestalConfig_ = jsonParserUtil.ParseFile(file_path_);  

    for (const auto& configi : pedestalConfig_["pedestals"]) 
    {
        offsetMap_[std::make_tuple(configi["crateNum"], configi["amcSlotNum"], configi["channelNum"])] = configi["pedestal"];
        if (debug_) std::cout << "Loading configuration for odd/even difference in channel ("
            << configi["crateNum"]      << " / "
            << configi["amcSlotNum"]        << " / "
            << configi["channelNum"]    << ") -> " 
            << configi["pedestal"]
            << std::endl;
    }

    // Create some histograms
    // auto hist = std::make_shared<TH1D>("energy", "Energy Spectrum", 100, 0, 1000);
    // eventStore.putHistogram("energy", std::move(hist));
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
        auto templateLoaderService = serviceManager.Get<TemplateLoaderService>(templateLoaderServiceLabel_);

        for (const auto& wf : waveforms) {
            // Make a copy for correction
            auto corrected = std::make_shared<dataProducts::WFD5Waveform>(*wf);

            ApplyJitterCorrection(corrected);

            //Fill a histogram for fun
            // store.GetHistogram("energy")->Fill(50);

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
    if (offsetMap_.count(wf->GetID()))
    {
        if (debug_) std::cout << "Correcting pedestal difference found for channel"
            << std::get<0>(wf->GetID()) << " / "
            << std::get<1>(wf->GetID()) << " / "
            << std::get<2>(wf->GetID()) << " with " 
            << offsetMap_[wf->GetID()]
            << std::endl;
        wf->JitterCorrect(
            offsetMap_[wf->GetID()]
        );
    }
    else if (failOnError_)
    {
        std::cerr << "Warning: no odd/even pedestal difference found for channel"
            << std::get<0>(wf->GetID()) << " / "
            << std::get<1>(wf->GetID()) << " / "
            << std::get<2>(wf->GetID()) 
            << std::endl;
        if (failOnError_) throw;
    }
}