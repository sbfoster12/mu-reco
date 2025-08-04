#ifndef TEMPLATE_SERVICE_HH
#define TEMPLATE_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>

#include "reco/common/Service.hh"

namespace reco {

    class TemplateService : public Service {
    public:
        TemplateService() = default;
        virtual ~TemplateService() = default;

        void Configure(const nlohmann::json& config) override {
            file_path_ = config.value("file_path", "/path/to/file");
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
