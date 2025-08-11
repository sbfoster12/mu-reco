
#include "reco/wfd5/PulseIntegrator.hh"
#include <iostream>

using namespace reco;

void PulseIntegrator::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) 
{
    inputRecoLabel_ = config.value("inputRecoLabel", "grouped");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WaveformsXtal");
    outputIntegralsLabel_ = config.value("outputIntegralsLabel", "Integrals");
    file_name_ = config.value("file_name", "integrator.json");
    debug_ = config.value("debug",false);

    seeded_ = config.value("seeded",false);
    inputSeedRecoLabel_ = config.value("inputSeedRecoLabel", "grouped");
    inputSeedLabel_ = config.value("inputSeedLabel", "WaveformsXtal");

    // parse out the default integration strategy
    defaultConfig_ = {
        config.value("skipChannel", false),
        config.value("nPresamples", 10 ),
        config.value("windowLength", 50 ),
        config.value("minAmplitude", 5 ),
        config.value("strategy", 0 ),
        config.value("nSigma", 1.0 )
    };

    // parse the individual channel config
    auto& jsonParserUtil = reco::JsonParserUtil::instance();
    json_ = jsonParserUtil.GetPathAndParseFile(file_name_, debug_);

    for (const auto& configi : json_["integrators"]) {
        std::vector<int> jid = configi["channel"];
        dataProducts::ChannelID id = {jid[0],jid[1],jid[2]};
        channelConfigMap_[id] = {
            config.value("skipChannel",     defaultConfig_.skipChannel ),
            config.value("nPresamples",     defaultConfig_.nPresamples  ),
            config.value("windowLength",    defaultConfig_.windowLength  ),
            config.value("minAmplitude",    defaultConfig_.minAmplitude  ),
            config.value("strategy",    defaultConfig_.strategy  ),
            config.value("nSigma",    defaultConfig_.nSigma  )
        };
    }



}

void PulseIntegrator::Process(EventStore& store, const ServiceManager& serviceManager) 
{

    // get the input waveforms
    auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);
    

    // create the output collection
    auto integrals = store.getOrCreate<dataProducts::WaveformIntegral>(this->GetRecoLabel(), outputIntegralsLabel_);

    // loop through each of the waveforms
    PulseIntegrationConfig thisConfig;
    
    for (int i = 0; i < waveforms->GetEntriesFast(); ++i) 
    {
        auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
        if (!waveform) {
            throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
        }

        
        
        // check if ID is in the override list, otherwise do the default processing
        if (channelConfigMap_.count(waveform->GetID()))
        {
            thisConfig = channelConfigMap_[waveform->GetID()];
        }
        else
        {
            thisConfig = defaultConfig_;
        }
        // TODO

        if (thisConfig.skipChannel)
        {
            std::cout << "Skipping channel due to config" << std::endl;
            continue;
        }
        
        // do the integration, 
        if (seeded_)
        {
            if (debug_) std::cout << "Performing a seeded integration" << std::endl;
            throw; //not implemneted yet
        }
        else 
        {
            if (debug_) std::cout << "Performing an unseeded integration" << std::endl;
            dataProducts::WaveformIntegral* integral = new ((*integrals)[i]) dataProducts::WaveformIntegral(
                waveform,
                thisConfig.nSigma,
                thisConfig.strategy
            );
            integrals->Expand(i+1);

            integral->DoIntegration({
                thisConfig.nPresamples,
                thisConfig.windowLength
            },-1,-1);
        }

        // if (debug_) integral->Show();

    }
}