#ifndef TimeSeeder_HH
#define TimeSeeder_HH

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/TimeSeed.hh>

#include "reco/common/RecoStage.hh"
#include "reco/common/EventStore.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class TimeSeeder : public RecoStage {
    public:
        TimeSeeder() : correctionFactor_() {}
        ~TimeSeeder() override = default;

        void Configure(const json& config, const ServiceManager& serviceManager, EventStore& eventStore) override;

        void Process(EventStore& store, const ServiceManager& serviceManager) const override;

    private:

        std::string inputRecoLabel_;
        double correctionFactor_;
        std::string inputFitResultsLabel_;
        std::string outputSeedLabel_;

        bool seedFromMaxAmplitudeFit_;
        bool seedFromFirstFit_;
        bool seedFromConfig_;
        double defaultSeed_;
        bool failOnError_;


        bool debug_;


        ClassDefOverride(TimeSeeder, 1);
    };
}

#endif  // TimeSeeder_HH