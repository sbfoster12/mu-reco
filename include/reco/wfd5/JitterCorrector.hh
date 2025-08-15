#ifndef JITTERCORRECTOR_HH
#define JITTERCORRECTOR_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class JitterCorrector : public RecoStage {
    public:
        JitterCorrector() : correctionFactor_() {}
        ~JitterCorrector() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:
        void ApplyJitterCorrection(dataProducts::WFD5Waveform* wf) const;

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;
        std::string templateLoaderServiceLabel_;
        double correctionFactor_;

        std::map<dataProducts::ChannelID, int> offsetMap_;

        std::string pedestal_files_;
        bool debug_;
        bool failOnError_;


        ClassDefOverride(JitterCorrector, 1);
    };
}

#endif  // JITTERCORRECTOR_HH