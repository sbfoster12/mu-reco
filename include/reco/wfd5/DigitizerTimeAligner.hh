#ifndef DIGITIZERTIMEALIGNER_HH
#define DIGITIZERTIMEALIGNER_HH

#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/TimeSeed.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"
#include "reco/wfd5/ChannelMapService.hh"

namespace reco {

    class DigitizerTimeAligner : public RecoStage {
    public:
        DigitizerTimeAligner() {}
        ~DigitizerTimeAligner() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

        void ApplyTimeAligner(dataProducts::WFD5Waveform* wf, dataProducts::TimeSeed* seed, dataProducts::WFD5Waveform* seed_wf, bool foundSeed) const;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;
        std::string channelMapServiceLabel_;


        std::string inputT0Reco_;
        std::string inputT0Label_;
        bool requireT0Seed_;
        bool debug_;

        std::map<dataProducts::ChannelID, double> knownTimeOffsetMap_;

        ClassDefOverride(DigitizerTimeAligner, 1);
    };
}

#endif  // DIGITIZERTIMEALIGNER_HH