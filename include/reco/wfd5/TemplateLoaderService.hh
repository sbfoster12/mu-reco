#ifndef TEMPLATELOADER_SERVICE_HH
#define TEMPLATELOADER_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/ConfigHolder.hh"
#include "reco/common/Service.hh"
#include "reco/common/JsonParserUtil.hh"

namespace reco {

    class TemplateLoaderService : public Service {
    public:
        TemplateLoaderService() = default;
        virtual ~TemplateLoaderService() = default;

        void Configure(const nlohmann::json& config) override {

            auto& jsonParserUtil = reco::JsonParserUtil::instance();

            std::string file_name = config.value("file_name", "templates.json");
            std::string file_path_ = "";
            if (file_name.find('/') != std::string::npos) {
                // If not a base name, try using this path directly
                file_path_ = file_name;
            } else {
                // If a base name, prepend the config directory
                file_path_ = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + file_name;
            }
            if (!std::filesystem::exists(file_path_)) {
                throw std::runtime_error("TemplateLoaderService: File not found: " + file_path_);
            }
            std::cout << "-> reco::TemplateLoaderService: Configuring with file: " << file_path_ << std::endl;
            auto jsonObj = jsonParserUtil.ParseFile(file_path_);  // Example usage of JsonParserUtil
        }

        int GetTemplate() {
            return 1;
        }

    private:
        std::string file_path_;

        ClassDefOverride(TemplateLoaderService, 1);

    };
}

#endif
