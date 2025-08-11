#ifndef EnergyCalibration_HH
#define EnergyCalibration_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class EnergyCalibration : public RecoStage {
    public:
        EnergyCalibration() : correctionFactor_() {}
        ~EnergyCalibration() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:
        // void ApplyJitterCorrection(dataProducts::WFD5Waveform* wf);

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;
        std::string templateLoaderServiceLabel_;
        double correctionFactor_;
        bool integrals_;

        std::map<dataProducts::ChannelID, double> calibrationMap_;

        std::string file_name_;
        bool debug_;
        bool failOnError_;


        ClassDefOverride(EnergyCalibration, 1);
    };
}

#endif  // EnergyCalibration_HH