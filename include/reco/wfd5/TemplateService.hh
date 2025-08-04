#ifndef TEMPLATE_SERVICE_HH
#define TEMPLATE_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>

#include "reco/common/ConfigHolder.hh"
#include "reco/common/Service.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class TemplateService : public Service {
    public:
        TemplateService() = default;
        virtual ~TemplateService() = default;

        void Configure(const nlohmann::json& config) override {

            auto& jsonParserUtil = reco::JsonParserUtil::instance();

            file_path_ = config.value("file_path", "/path/to/file");
            
            auto jsonObj = jsonParserUtil.ParseFile(file_path_);  // Example usage of JsonParserUtil
        }

        int GetTemplate() {
            return 1;
        }

    private:
        std::string file_path_;

        ClassDefOverride(TemplateService, 1);

    };
}

#endif
