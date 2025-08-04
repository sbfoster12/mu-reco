#ifndef SERVICE_MANAGER_HH
#define SERVICE_MANAGER_HH

#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <TClass.h>
#include <nlohmann/json.hpp>

#include "reco/common/Service.hh"
#include "reco/common/ConfigHolder.hh"

namespace reco {
    class ServiceManager {
    public:
        ServiceManager() = default;
        ~ServiceManager() = default;

        // Create and configure services from JSON array (each service config must have "class" and "label")
        void Configure(std::shared_ptr<const ConfigHolder> configHolder) {
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
                service->SetConfigHolder(configHolder);
                service->Configure(servicesConfig);
                service->SetLabel(label.c_str());

                // Store with label
                Add(label, std::shared_ptr<Service>(service));
            }
        }

        template<typename T>
        void Add(const std::string& label, std::shared_ptr<T> service) {
            static_assert(std::is_base_of<Service, T>::value, "T must derive from Service");
            if (services_.count(label)) {
                throw std::runtime_error("Service label already exists: " + label);
            }
            services_[label] = std::static_pointer_cast<Service>(service);
        }

        template<typename T>
        std::shared_ptr<T> Get(const std::string& label) const {
            auto it = services_.find(label);
            if (it == services_.end()) {
                throw std::runtime_error("Service not found: " + label);
            }
            return std::dynamic_pointer_cast<T>(it->second);
        }

    private:
        std::map<std::string, std::shared_ptr<Service>> services_;
    };
}

#endif // SERVICE_MANAGER_HH
