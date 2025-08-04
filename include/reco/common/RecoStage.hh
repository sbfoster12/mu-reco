// reco/common/RecoStage.hh
#ifndef RECOSTAGE_HH
#define RECOSTAGE_HH

#include <nlohmann/json.hpp>
#include <TObject.h>
#include <string>

using json = nlohmann::json;

namespace reco {

    class EventStore;
    class ServiceManager;

    class RecoStage : public TObject {
    public:
        RecoStage() = default;
        virtual ~RecoStage() = default;

        virtual void Configure(const nlohmann::json& config) = 0;
        virtual void Process(EventStore& eventStore, ServiceManager& serviceManager) = 0;

        void SetLabel(const std::string& label) { label_ = label; }
        const std::string& GetLabel() const { return label_; }

    private:
        std::string label_;

        ClassDef(RecoStage, 1);
    };
}

#endif
