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


    auto& jsonParserUtil = reco::JsonParserUtil::instance();
    // std::cout << "-> reco::EnergyCalibration: Configuring with file: " << file_name_ << std::endl;
    // auto calibConfig = jsonParserUtil.GetPathAndParseFile(file_name_);
    
    int runNum = 0;
    bool found_calib = false;
    json thisConfig;
    for (auto &configi: config["constant_files"])
    {
        auto iov = configi["iov"];
        if (runNum >= iov[0] && runNum <= iov[1])
        {
            found_calib = true;
            if (debug_) std::cout << "Found calibration file for IOV [" 
                << iov[0] << " - " << iov[1] << ") -> " 
                << configi["file"] << std::endl;
            thisConfig = jsonParserUtil.GetPathAndParseFile(
                configi["file"]
            );
        }
    }
    if (failOnError_ && !found_calib)
    {
        std::cerr << "Unable to find energy calibration constants" << std::endl;
        throw;
    }

    for (const auto& configi : thisConfig["calibration"]) 
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

    // Create some histograms
    // auto hist = std::make_shared<TH1D>("energy", "Energy Spectrum", 100, 0, 1000);
    // eventStore.putHistogram("energy", std::move(hist));
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

        //Make a collection new waveforms

        // for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
        //     auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
        //     if (!waveform) {
        //         throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
        //     }
        //     //Make the new waveform
        //     dataProducts::WFD5Waveform* newWaveform = new ((*newWaveforms)[i]) dataProducts::WFD5Waveform(waveform);
        //     newWaveforms->Expand(i + 1);

        //     // ApplyJitterCorrection(newWaveform);
        // }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("EnergyCalibration error: ") + e.what());
    }
}

// void EnergyCalibration::CorrectEnergy(dataProducts::WFD5Waveform* wf) {
    // Implement jitter correction here
    
// }