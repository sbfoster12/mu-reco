#ifndef RECOMANAGER_HH
#define RECOMANAGER_HH

#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "reco/common/RecoStage.hh"
#include "reco/common/ConfigHolder.hh"
#include "reco/common/ServiceManager.hh"
#include "reco/common/EventStore.hh"

namespace reco {
    
    class RecoManager {
    public:
        RecoManager() = default;

        void Configure(std::shared_ptr<const ConfigHolder> configHolder, const ServiceManager& serviceManager);
        void Run(EventStore& eventStore, const ServiceManager& serviceManager);

    private:
        std::vector<std::shared_ptr<RecoStage>> stages_;
    };
} //namespace reco


#endif
