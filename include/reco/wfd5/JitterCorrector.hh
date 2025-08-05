#ifndef JITTERCORRECTOR_HH
#define JITTERCORRECTOR_HH

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

        void Configure(const json& config, const ServiceManager& serviceManager) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) override;

    private:
        void ApplyJitterCorrection(std::shared_ptr<dataProducts::WFD5Waveform>& wf);

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputWaveformsLabel_;
        std::string templateLoaderServiceLabel_;
        double correctionFactor_;


        ClassDefOverride(JitterCorrector, 1);
    };
}

#endif  // JITTERCORRECTOR_HH