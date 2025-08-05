#ifndef TEMPLATEFITTER_SERVICE_HH
#define TEMPLATEFITTER_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/Service.hh"
#include "reco/wfd5/TemplateLoaderService.hh"

namespace reco {

    class TemplateFitterService : public Service {
    public:
        TemplateFitterService() = default;
        virtual ~TemplateFitterService() = default;

        void Configure(const nlohmann::json& config, EventStore& eventStore) override {

            // auto& jsonParserUtil = reco::JsonParserUtil::instance();
            templateLoaderLabel_ = config.value("templateLoaderLabel", "templateLoader");

            // Get the loader templates from the services (Note that the template loader must be configure first!)
            auto templateLoader = GetServiceManager()->Get<TemplateLoaderService>(templateLoaderLabel_);

            std::cout << "-> reco::TemplateFitterService: Using TemplateLoaderService with label: " << templateLoaderLabel_ << std::endl;
        }

    private:

        std::string templateLoaderLabel_;

        ClassDefOverride(TemplateFitterService, 1);

    };
}

#endif
