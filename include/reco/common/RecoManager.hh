#ifndef RECOMANAGER_HH
#define RECOMANAGER_HH

#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "reco/common/RecoStage.hh"

class EventStore;
class ServiceManager;

namespace reco {
    
    class RecoManager {
    public:
        RecoManager() = default;

        void Configure(const nlohmann::json& config);
        void Run(EventStore& eventStore, ServiceManager& serviceManager);

    private:
        std::vector<std::shared_ptr<RecoStage>> stages_;
    };
} //namespace reco


#endif
