#ifndef TEMPLATE_SERVICE_HH
#define TEMPLATE_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

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

            std::string dir = std::getenv("MU_RECO_PATH");
            file_path_ = dir + "/config/" + config.value("file_path", "templates.json");
            if (!std::filesystem::exists(file_path_)) {
                throw std::runtime_error("TemplateService: File not found: " + file_path_);
            }
            std::cout << "-> reco::TemplateService: Configuring with file: " << file_path_ << std::endl;
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
