#include "reco/wfd5/EnergyCalibration.hh"
#include "data_products/wfd5/WaveformIntegral.hh"
#include "data_products/wfd5/WFD5WaveformFit.hh"
#include <iostream>

using namespace reco;

void EnergyCalibration::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    correctionFactor_ = config.value("correctionFactor", 1.0);
    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WaveformsCorreted");
    file_name_ = config.value("file_name", "calibration.json");
    failOnError_ = config.value("failOnError", true);
    debug_ = config.value("debug",false);    
    integrals_ = config.value("integrals",false);

    // Set up the parser
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();

    // Check the parameter exists
    if (!config.contains("energy_calibration_iov")) {
        throw std::runtime_error("EnergyCalibration configuration must contain 'energy_calibration_iov' key");
    }
    
    // Get the iov json file
    auto iovConfigListFileName = config.value("energy_calibration_iov","");
    std::string iovConfigListFilePath = "";
    auto iovConfigJson = jsonParserUtil.GetPathAndParseFile(iovConfigListFileName, iovConfigListFilePath, debug_);
    
    // Check that the list exists in the json file
    if (!iovConfigJson.contains("energy_calibration_iov")) {
        throw std::runtime_error("EnergyCalibration: File " + iovConfigListFilePath + " must contain 'energy_calibration_iov' key");
    }

    // Get the iov list and check it is an array
    auto iovConfigList = iovConfigJson["energy_calibration_iov"];
    if (!iovConfigList.is_array()) {
        throw std::runtime_error("'energy_calibration_iov' key must contain an array");
    }
    
    // Determine the correct configuration based on run and subrun from the iov list
    auto iovConfigMatch= jsonParserUtil.GetIOVMatch(iovConfigList, run, subrun);

    // Now get the actual configuration
    std::string configFileName = iovConfigMatch.value("file","");
    std::string configFilePath = "";
    auto energyCalibConfig = jsonParserUtil.GetPathAndParseFile(configFileName, configFilePath, debug_);
    std::cout << "-> reco::EnergyCalibration: Configuring with file: " << configFilePath << std::endl;

    for (const auto& configi : energyCalibConfig["calibration"]) 
    {
        calibrationMap_[std::make_tuple(configi["crateNum"], configi["amcSlotNum"], configi["channelNum"])] = configi["calib"];
        // if (debug_) 
        std::cout << "Loading configuration for energy calibration in channel ("
            << configi["crateNum"]      << " / "
            << configi["amcSlotNum"]        << " / "
            << configi["channelNum"]    << ") -> " 
            << configi["calib"]
            << std::endl;
    }
}

void EnergyCalibration::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "EnergyCalibration with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        TClonesArray *input;
        double scale = 1.0;
        if(integrals_)
        {
            input = store.get<const dataProducts::WaveformIntegral>(inputRecoLabel_, inputWaveformsLabel_);
            auto output = store.getOrCreate<dataProducts::WaveformIntegral>(this->GetRecoLabel(), outputWaveformsLabel_);
            for (int i = 0; i < input->GetEntriesFast(); i++)
            {
                auto inputObject = (dataProducts::WaveformIntegral*) input->At(i);
                auto outputObject = new ((*output)[i]) dataProducts::WaveformIntegral(inputObject);
                if (!calibrationMap_.count(inputObject->GetID()))
                {
                    if(debug_) std::cout << "Warning: no calibration constant found for channel ("
                        << inputObject->crateNum << " / "
                        << inputObject->amcNum << " / "
                        << inputObject->channelTag << " ) "
                        << std::endl;
                    if (failOnError_) throw; 
                    scale = 1.0;
                }
                else
                {
                    scale = calibrationMap_.at(inputObject->GetID());
                    outputObject->CalibrateEnergies(scale);
                }
                output->Expand(i + 1);
            }
        }
        else 
        {
            input = store.get<const dataProducts::WaveformFit>(inputRecoLabel_, inputWaveformsLabel_);
            auto output = store.getOrCreate<dataProducts::WaveformFit>(this->GetRecoLabel(), outputWaveformsLabel_);
            for (int i = 0; i < input->GetEntriesFast(); i++)
            {
                auto inputObject = (dataProducts::WaveformFit*) input->At(i);
                auto outputObject = new ((*output)[i]) dataProducts::WaveformFit(inputObject);
                if (!calibrationMap_.count(inputObject->GetID()))
                {
                    if(debug_) std::cout << "Warning: no calibration constant found for channel ("
                        << inputObject->crateNum << " / "
                        << inputObject->amcNum << " / "
                        << inputObject->channelTag << " ) "
                        << std::endl;
                    if (failOnError_) throw; 
                    scale = 1.0;
                }
                else
                {
                    scale = calibrationMap_.at(inputObject->GetID());
                    outputObject->CalibrateEnergies(scale);
                }
                output->Expand(i + 1);   
            }
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("EnergyCalibration error: ") + e.what());
    }
}