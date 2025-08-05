// Service.hh
#ifndef SERVICE_HH
#define SERVICE_HH

#include <TObject.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace reco {

    class ConfigHolder;
    class ServiceManager;

    class Service : public TObject {
    public:
        virtual ~Service() = default;
        virtual void Configure(const nlohmann::json& config) = 0;

        void SetLabel(const std::string& label) { label_ = label; }
        const std::string& GetLabel() const { return label_; }

        void SetConfigHolder(std::shared_ptr<const ConfigHolder> configHolder) {
            configHolder_ = configHolder;
        }

        void SetServiceManager(const ServiceManager* serviceManager) {
            serviceManager_ = serviceManager;
        }

        const ServiceManager* GetServiceManager() const {
            return serviceManager_;
        }

    protected:
        std::string label_;

        std::shared_ptr<const ConfigHolder> configHolder_;
        const ServiceManager* serviceManager_;

        ClassDef(Service, 1);
    };
}

#endif
