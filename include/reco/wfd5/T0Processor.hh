#ifndef T0Processor_HH
#define T0Processor_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/TimeSeed.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/wfd5/ChannelMapService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class T0Processor : public RecoStage {
    public:
        T0Processor() : defaultTime_(0.0) {}
        ~T0Processor() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputT0TimeRefLabel_;
        std::pair<int,int> triggerSearchWindow_;
        bool failIfT0OutsideWindow_;
        bool failIfT0NotFound_;
        dataProducts::ChannelID t0Channel_;
        std::string channelMapServiceLabel_;
        double defaultTime_;

        bool debug_;
        bool failOnError_;


        ClassDefOverride(T0Processor, 1);
    };
}

#endif  // T0Processor_HH