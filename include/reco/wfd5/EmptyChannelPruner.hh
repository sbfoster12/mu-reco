#ifndef EmptyChannelPruner_HH
#define EmptyChannelPruner_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class EmptyChannelPruner : public RecoStage {
    public:
        EmptyChannelPruner() : minAmplitude_() {}
        ~EmptyChannelPruner() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;
        std::string templateLoaderServiceLabel_;
        double minAmplitude_;

        std::map<dataProducts::ChannelID, int> minAmplMap_;

        bool debug_;
        bool failOnError_;


        ClassDefOverride(EmptyChannelPruner, 1);
    };
}

#endif  // EmptyChannelPruner_HH