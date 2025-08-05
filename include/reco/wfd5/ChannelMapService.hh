#ifndef CHANNELMAPSERVICE_HH
#define CHANNELMAPSERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/ConfigHolder.hh"
#include "reco/common/Service.hh"
#include "reco/common/JsonParserUtil.hh"
#include "reco/wfd5/ChannelConfig.hh"

namespace reco {


    class ChannelMapService : public Service {
    public:
        ChannelMapService() = default;
        virtual ~ChannelMapService() = default;

        void Configure(const nlohmann::json& config) override;

        const std::map<std::tuple<int,int,int>, ChannelConfig>& GetChannelMap() const {
            return channelConfigMap_;
        }

        //Get channel mapping file from specified run and subrun
        const nlohmann::json& GetFileFromRunSubrun(int run, int subrun, const nlohmann::json& configFiles);

    private:
        std::map<std::tuple<int,int,int>, ChannelConfig> channelConfigMap_;  // key: (crate, wfd5, channel), value: ChannelConfig

        ClassDefOverride(ChannelMapService, 1);

    };
}

#endif
