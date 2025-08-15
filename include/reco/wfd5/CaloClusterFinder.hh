
#ifndef CALOCLUSTERFINDER_HH
#define CALOCLUSTERFINDER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class CaloClusterFinder : public RecoStage {
    public:
        CaloClusterFinder() {}
        ~CaloClusterFinder() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:

        std::string inputRecoLabel_;
        std::string inputFitResultsLabel_;
        std::string outputCaloClusterLabel_;

        ClassDefOverride(CaloClusterFinder, 1);
    };
}

#endif  // CALOCLUSTERFINDER_HH