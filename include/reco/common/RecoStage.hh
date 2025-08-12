// reco/common/RecoStage.hh
#ifndef RECOSTAGE_HH
#define RECOSTAGE_HH

#include <nlohmann/json.hpp>
#include <TObject.h>
#include <string>

#include "reco/common/ServiceManager.hh"
#include "reco/common/TimeProfilerService.hh"

using json = nlohmann::json;

namespace reco {

    class EventStore;
    // class ServiceManager;
    class ConfigHolder;

    class RecoStage : public TObject {
    public:
        RecoStage() = default;
        virtual ~RecoStage() = default;

        virtual void Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) = 0;
        virtual void Process(EventStore& eventStore, const ServiceManager& serviceManager) = 0;
        void RunStage(EventStore& eventStore, const ServiceManager& serviceManager);

        void SetRecoLabel(const std::string& recoLabel) { recoLabel_ = recoLabel; }
        const std::string& GetRecoLabel() const { return recoLabel_; }

        void SetConfigHolder(std::shared_ptr<const ConfigHolder> configHolder) {
            configHolder_ = configHolder;
        }

    protected:
        std::string recoLabel_;

        std::shared_ptr<const ConfigHolder> configHolder_;

        ClassDef(RecoStage, 1);
    };
}

#endif
