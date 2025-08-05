#ifndef DETECTORGROUPER_HH
#define DETECTORGROUPER_HH

#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/ChannelMapService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class DetectorGrouper : public RecoStage {
    public:
        DetectorGrouper() {}
        ~DetectorGrouper() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsBaseLabel_;
        std::string channelMapServiceLabel_;

        ClassDefOverride(DetectorGrouper, 1);
    };
}

#endif  // DETECTORGROUPER_HH