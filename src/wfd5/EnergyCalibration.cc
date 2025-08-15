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

   // Get the energy calibration configuration from the IOV list using the run and subrun
    auto energyCalibConfig =  jsonParserUtil.GetConfigFromIOVList(config, run, subrun, "energy_calibration_iov", debug_);
    if (energyCalibConfig.empty()) {
        if (failOnError_) {
            throw std::runtime_error("EnergyCalibration configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
        } else {
            std::cout << "-> reco::EnergyCalibration: Warning, no configuration found, but proceeding, because failOnError is false" << std::endl;
        }
    }

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