
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


    // parse the individual channel config
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    json_ = jsonParserUtil.GetPathAndParseFile(file_name_, debug_);


}

void PulseIntegrator::Process(EventStore& store, const ServiceManager& serviceManager) 
{



}