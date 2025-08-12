#include "reco/common/TimeProfilerService.hh"

using namespace reco;

 void TimeProfilerService::Configure(const nlohmann::json& config, EventStore& eventStore) {

    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    std::cout << "-> reco::TimeProfilerService: Configuring TimeProfilerService" << std::endl;

    // For this service, we need to go a little rogue and parse the full config which is in the ConfigHolder
    if (!configHolder_) {
        throw std::runtime_error("TimeProfilerService: ConfigHolder not set");
    }

    const nlohmann::json& fullConfig = configHolder_->GetConfig();
    if (!fullConfig.contains("RecoPath") || !fullConfig["RecoPath"].is_array()) {
        throw std::runtime_error("TimeProfilerService: Missing or invalid 'RecoPath' config");
    }



    // Initialize maps
    for (const auto& label : fullConfig["RecoPath"]) {
        totalDurations_[label] = 0.0;
        nEvents_[label] = 0;
        startTime_[label] = std::chrono::high_resolution_clock::now();
        endTime_[label] = std::chrono::high_resolution_clock::now();
    }

    std::cout << "-> reco::TimeProfilerService: Initialized timing for ";
    // Loop with iterators
    for (auto it = totalDurations_.begin(); it != totalDurations_.end(); ++it) {
        if (it != totalDurations_.begin()) {
            std::cout << ", ";
        }
        std::cout << "'" << it->first << "'";
    }
    std::cout << std::endl;
 }

 void TimeProfilerService::StartTimer(const std::string& label) {
    startTime_[label] = std::chrono::high_resolution_clock::now();
 }

 void TimeProfilerService::StopTimer(const std::string& label) {
    endTime_[label] = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime_[label] - startTime_[label];
    totalDurations_[label] += elapsed.count();
    nEvents_[label]++;
 }

 void TimeProfilerService::EndOfJobPrint() const {
    std::cout << "-> reco::TimeProfilerService: Timing summary:" << std::endl;

    // Determine the width
    int width = 20;
    for (const auto& entry : totalDurations_) {
        const std::string& label = entry.first;
        width = std::max(width, static_cast<int>(label.length()));
    }
    width+=5;

    // Loop over the reco stages from the config holder (to keep the order)
    for (const auto& labelKey : configHolder_->GetConfig()["RecoPath"]) {
        const std::string& label = labelKey.get<std::string>();
        if (totalDurations_.find(label) == totalDurations_.end()) {
            std::cout << "  - " << std::left << std::setw(width) << label
                      << ": No timing data available" << std::endl;
            continue;
        } else {
            const auto& entry = totalDurations_.find(label);
            double totalDuration = entry->second;
            int nEvents = nEvents_.at(label);
            if (nEvents > 0) {
                std::cout << "  - " 
                        << std::left << std::setw(width) << label
                        << ": "
                        << std::setprecision(3) << totalDuration << "/" << nEvents
                        << " = "
                        << (totalDuration / nEvents) << " seconds/event"
                    << std::endl;
            } else {
                std::cout << std::left << std::setw(width) << label << ": No events processed" << std::endl;
            }

        }
    }
 }