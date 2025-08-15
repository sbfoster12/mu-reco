#ifndef WAVEFORMINITIALIZER_HH
#define WAVEFORMINITIALIZER_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/WFD5ODB.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class WaveformInitializer : public RecoStage {
    public:
        WaveformInitializer() {}
        ~WaveformInitializer() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:
        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;

        bool debug_;
        bool failOnError_;

        ClassDefOverride(WaveformInitializer, 1);
    };
}

#endif  // WAVEFORMINITIALIZER_HH