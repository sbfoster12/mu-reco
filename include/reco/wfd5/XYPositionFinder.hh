
#ifndef XYPOSITIONFINDER_HH
#define XYPOSITIONFINDER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class XYPositionFinder : public RecoStage {
    public:
        XYPositionFinder() {}
        ~XYPositionFinder() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputFitResultsLabel_;
        std::string outputPositionLabel_;

        ClassDefOverride(XYPositionFinder, 1);
    };
}

#endif  // XYPOSITIONFINDER_HH