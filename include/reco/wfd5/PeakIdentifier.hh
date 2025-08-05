
#ifndef PEAKIDENTIFIER_HH
#define PEAKIDENTIFIER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class PeakIdentifier : public RecoStage {
    public:
        PeakIdentifier() {}
        ~PeakIdentifier() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputPeaksLabel_;

        ClassDefOverride(PeakIdentifier, 1);
    };
}

#endif  // PEAKIDENTIFIER_HH