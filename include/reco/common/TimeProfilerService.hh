#ifndef TIMEPROFILER_SERVICE_HH
#define TIMEPROFILER_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <unordered_map>

#include "reco/common/Service.hh"
#include "reco/wfd5/ChannelConfig.hh"

namespace reco {


    class TimeProfilerService : public Service {
    public:
        TimeProfilerService() = default;
        virtual ~TimeProfilerService() = default;
        void EndOfJobPrint() const override;

        void Configure(const nlohmann::json& config, EventStore& eventStore) override;

         void StartTimer(const std::string& label);
         void StopTimer(const std::string& label);

    private:
        std::unordered_map<std::string, double> totalDurations_;
        std::unordered_map<std::string, int> nEvents_;

        std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> startTime_;
        std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> endTime_;

        ClassDefOverride(TimeProfilerService, 1);

    };
}

#endif
