#include "reco/common/RecoManager.hh"

#include <iostream>
#include <stdexcept>
#include <TClass.h>

using namespace reco;

void RecoManager::Configure(std::shared_ptr<const ConfigHolder> configHolder, const ServiceManager& serviceManager, EventStore& eventStore) {

    const nlohmann::json& config = configHolder->GetConfig();
    if (!config.contains("RecoStages") || !config["RecoStages"].is_array()) {
        throw std::runtime_error("RecoManager: Missing or invalid 'RecoStages' config");
    }

    if (!config.contains("RecoPath") || !config["RecoPath"].is_array()) {
        throw std::runtime_error("RecoManager: Missing or invalid 'RecoPath' config");
    }

    std::cout << "-> reco::RecoManager: Configuring with " << config["RecoPath"].size() << " stages.\n";    
    for (const auto& label : config["RecoPath"]) {
        auto it = std::find_if(config["RecoStages"].begin(), config["RecoStages"].end(),
                               [&](const json& stage) {
                                   return stage["recoLabel"] == label;
                               });

        if (it != config["RecoStages"].end()) {
            const auto& stageConfig = *it;

            const std::string& type = stageConfig["recoClass"];
            const std::string& recoLabel = stageConfig["recoLabel"];

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

            stage->SetConfigHolder(configHolder);
            stage->SetRecoLabel(recoLabel);
            stage->Configure(stageConfig, serviceManager, eventStore);
            stages_.emplace_back(stage);

            std::cout << "-> reco::RecoManager: Added RecoStage of type '" << type << "' with label '" << recoLabel << "'\n";

        } else {
            std::cerr << "Stage not found for label: " << label << "\n";
        }
    }
}

void RecoManager::Run(EventStore& eventStore, const ServiceManager& serviceManager) {
    for (const auto& stage : stages_) {
        stage->RunStage(eventStore, serviceManager);
    }
}

