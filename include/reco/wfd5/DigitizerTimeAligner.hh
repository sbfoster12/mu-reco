#ifndef DIGITIZERTIMEALIGNER_HH
#define DIGITIZERTIMEALIGNER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class DigitizerTimeAligner : public RecoStage {
    public:
        DigitizerTimeAligner() {}
        ~DigitizerTimeAligner() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;

        ClassDefOverride(DigitizerTimeAligner, 1);
    };
}

#endif  // DIGITIZERTIMEALIGNER_HH