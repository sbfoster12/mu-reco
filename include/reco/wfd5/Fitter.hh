#ifndef FITTER_HH
#define FITTER_HH

#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/TimeSeed.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class Fitter : public RecoStage {
    public:
        Fitter() {}
        ~Fitter() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputFitResultLabel_;
        std::string templateFitterLabel_;
        bool fit_debug;

        bool seeded_;
        bool seeded_extra_leeway_;
        std::string seededInputReco_;
        std::string seededInputLabel_;

        ClassDefOverride(Fitter, 2);
    };
}

#endif  // FITTER_HH