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

        // Configure method
        void Configure(std::shared_ptr<const ConfigHolder> configHolder, reco::EventStore& eventStore);

        // Templated Add method
        template<typename T>
        void Add(const std::string& label, std::shared_ptr<T> service) {
            static_assert(std::is_base_of<Service, T>::value, "T must derive from Service");
            if (services_.count(label)) {
                throw std::runtime_error("Service label already exists: " + label);
            }
            services_[label] = std::static_pointer_cast<Service>(service);
        }

        // Templated Get method
        template<typename T>
        std::shared_ptr<T> Get(const std::string& label) const {
            auto it = services_.find(label);
            if (it == services_.end()) {
                throw std::runtime_error("Service not found: " + label);
            }
            return std::dynamic_pointer_cast<T>(it->second);
        }

        // Get all services
        const std::map<std::string, std::shared_ptr<Service>>& GetServices() const {
            return services_;
        }

        void EndOfJobPrint() const;

    private:
        std::map<std::string, std::shared_ptr<Service>> services_;
    };
}

#endif // SERVICE_MANAGER_HH
