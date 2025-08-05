
#ifndef PULSEINTEGRATOR_HH
#define PULSEINTEGRATOR_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class PulseIntegrator : public RecoStage {
    public:
        PulseIntegrator() {}
        ~PulseIntegrator() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputIntegralsLabel_;

        ClassDefOverride(PulseIntegrator, 1);
    };
}

#endif  // PULSEINTEGRATOR_HH