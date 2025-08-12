#include "reco/common/RecoStage.hh"

using namespace reco;

void RecoStage::RunStage(EventStore& eventStore, const ServiceManager& serviceManager) {

    // Check if we have a TimeProfilerService configured
    if (configHolder_->GetConfig().contains("RecoManager") && configHolder_->GetConfig()["RecoManager"].contains("timeProfilerLabel")) {
        std::string timeProfilerLabel = configHolder_->GetConfig()["RecoManager"]["timeProfilerLabel"];
        auto timeProfilerService = serviceManager.Get<reco::TimeProfilerService>(timeProfilerLabel);
        if (!timeProfilerService){
            throw std::runtime_error("RecoStage: TimeProfilerService not found: " + timeProfilerLabel);
        } else {
            timeProfilerService->StartTimer(this->GetRecoLabel());
            Process(eventStore, serviceManager);
            timeProfilerService->StopTimer(this->GetRecoLabel());
        }
    } else {
        // If no TimeProfilerService, just run the stage
        Process(eventStore, serviceManager);
    }
}