#include "reco/common/ServiceManager.hh"

using namespace reco;

// Create and configure services from JSON array (each service config must have "type" and "label")
void ServiceManager::Configure(std::shared_ptr<const ConfigHolder> configHolder, reco::EventStore& eventStore) {
    const nlohmann::json& config = configHolder->GetConfig();
    if (!config.contains("Services") || !config["Services"].is_array()) {
        throw std::runtime_error("ServiceManager: Missing or invalid 'Services' config");
    }

    for (const auto& servicesConfig : config["Services"]) {
        if (!servicesConfig.contains("type") || !servicesConfig.contains("label")) {
            throw std::runtime_error("Service config missing 'type' or 'label'");
        }
        std::string className = servicesConfig["type"];
        std::string label = servicesConfig["label"];

        // Use ROOT RTTI to create instance
        TClass* cl = TClass::GetClass(className.c_str());
        if (!cl || !cl->InheritsFrom(Service::Class())) {
            throw std::runtime_error("Invalid or unknown service type: " + className);
        }

        TObject* obj = static_cast<TObject*>(cl->New());
        Service* service = dynamic_cast<Service*>(obj);
        if (!service) {
            delete obj;
            throw std::runtime_error("Failed to cast to Service for type: " + className);
        }

        // Configure the service with full JSON object
        service->SetServiceManager(this); // this lets later services access previously created services
        service->SetConfigHolder(configHolder);
        service->Configure(servicesConfig, eventStore);
        service->SetLabel(label.c_str());

        // Store with label
        Add(label, std::shared_ptr<Service>(service));
    }
}