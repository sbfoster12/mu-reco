
#ifndef PEDESTALCALCULATOR_HH
#define PEDESTALCALCULATOR_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class PedestalCalculator : public RecoStage {
    public:
        PedestalCalculator() {}
        ~PedestalCalculator() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;

        ClassDefOverride(PedestalCalculator, 1);
    };
}

#endif  // PEDESTALCALCULATOR_HH