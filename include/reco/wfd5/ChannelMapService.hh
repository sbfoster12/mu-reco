#ifndef CHANNELMAPSERVICE_HH
#define CHANNELMAPSERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/ConfigHolder.hh"
#include "reco/common/Service.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class ChannelMapService : public Service {
    public:
        ChannelMapService() = default;
        virtual ~ChannelMapService() = default;

        void Configure(const nlohmann::json& config) override;

        const std::map<std::tuple<int,int,int>, std::pair<std::string,std::string>>& GetChannelMap() const {
            return channelMap_;
        }

    private:
        std::map<std::tuple<int,int,int>, std::pair<std::string,std::string>> channelMap_;  // key: (crate, wfd5, channel), value: (detector system, subdetector)

        ClassDefOverride(ChannelMapService, 1);

    };
}

#endif
