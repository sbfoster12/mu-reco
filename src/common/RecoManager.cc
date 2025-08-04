#include "reco/common/RecoManager.hh"
#include "reco/common/EventStore.hh"

#include <iostream>
#include <stdexcept>
#include <TClass.h>

using namespace reco;

void RecoManager::Configure(const nlohmann::json& config) {
    if (!config.contains("RecoStages") || !config["RecoStages"].is_array()) {
        throw std::runtime_error("RecoManager: Missing or invalid 'RecoStages' config");
    }

    for (const auto& stageConfig : config["RecoStages"]) {
        if (!stageConfig.contains("type") || !stageConfig.contains("label")) {
            throw std::runtime_error("RecoManager: RecoStage missing 'type' or 'label'");
        }

        const std::string& type = stageConfig["type"];
        const std::string& label = stageConfig["label"];

        TClass* cl = TClass::GetClass(type.c_str());
        if (!cl || !cl->InheritsFrom(RecoStage::Class())) {
            throw std::runtime_error("RecoManager: Cannot find or cast RecoStage type: " + type);
        }

        TObject* obj = static_cast<TObject*>(cl->New());
        if (!obj) {
            throw std::runtime_error("RecoManager: Failed to instantiate " + type);
        }

        auto* stage = dynamic_cast<RecoStage*>(obj);
        if (!stage) {
            throw std::runtime_error("RecoManager: Instantiated object is not a RecoStage");
        }

        stage->SetLabel(label);
        stage->Configure(stageConfig);
        stages_.emplace_back(stage);
    }
}

void RecoManager::Run(EventStore& eventStore, ServiceManager& serviceManager) {
    for (const auto& stage : stages_) {
        stage->Process(eventStore, serviceManager);
    }
}
