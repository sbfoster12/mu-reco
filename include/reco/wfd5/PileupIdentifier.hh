
#ifndef PILEUPIDENTIFIER_HH
#define PILEUPIDENTIFIER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class PileupIdentifier : public RecoStage {
    public:
        PileupIdentifier() {}
        ~PileupIdentifier() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputPeaksLabel_;
        std::string outputPileupLabel_;

        ClassDefOverride(PileupIdentifier, 1);
    };
}

#endif  // PILEUPIDENTIFIER_HH