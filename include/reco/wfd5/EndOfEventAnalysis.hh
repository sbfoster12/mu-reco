
#ifndef ENDOFEVENTANALYSIS_HH
#define ENDOFEVENTANALYSIS_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class EndOfEventAnalysis : public RecoStage {
    public:
        EndOfEventAnalysis() {}
        ~EndOfEventAnalysis() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string lysoRecoLabel_;
        std::string lysoWaveformsLabel_;

        ClassDefOverride(EndOfEventAnalysis, 1);
    };
}

#endif  // ENDOFEVENTANALYSIS_HH