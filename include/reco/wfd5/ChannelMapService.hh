#ifndef CHANNELMAPSERVICE_HH
#define CHANNELMAPSERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>

#include "reco/common/ConfigHolder.hh"
#include "reco/common/Service.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class ChannelMapService : public Service {
    public:
        ChannelMapService() = default;
        virtual ~ChannelMapService() = default;

        void Configure(const nlohmann::json& config) override {

            auto& jsonParserUtil = reco::JsonParserUtil::instance();

            //TODO: parse the configuration channel map and store it in a map
            channelMap_[std::make_tuple(0, 1, 0)] = "LYSO"; // this is just an example for now            
        }

        const std::map<std::tuple<int,int,int>, std::string>& GetChannelMap() const {
            return channelMap_;
        }

    private:
        std::map<std::tuple<int,int,int>, std::string> channelMap_;  // key: (crate, wfd5, channel), value: channel name

        ClassDefOverride(ChannelMapService, 1);

    };
}

#endif
