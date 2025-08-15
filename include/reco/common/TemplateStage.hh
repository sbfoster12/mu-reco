#ifndef TEMPLATESTAGE_HH
#define TEMPLATESTAGE_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class TemplateStage : public RecoStage {
    public:
        TemplateStage() {}
        ~TemplateStage() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;

        bool debug_;
        bool failOnError_;

        ClassDefOverride(TemplateStage, 1);
    };
}

#endif  // TEMPLATESTAGE_HH