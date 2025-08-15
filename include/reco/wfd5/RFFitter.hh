
#ifndef RFFFITTER_HH
#define RFFFITTER_HH

#include <TGraph.h>
#include <TF1.h>
#include <TMath.h>

#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/RFWaveformFit.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class RFFitter : public RecoStage {
    public:
        RFFitter() {}
        ~RFFitter() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

        void PerformRFFit(const dataProducts::WFD5Waveform* waveform, dataProducts::RFWaveformFit* fitResult) const;


    private:

        std::string inputRecoLabel_;
        std::string inputWaveformsLabel_;
        std::string outputFitResultLabel_;
        double fitStartTime_;
        double fitEndTime_;
        double frequency_;
        bool fixFrequency_;
        std::string fitOption_;

        ClassDefOverride(RFFitter, 1);
    };
}

#endif  // RFFFITTER_HH