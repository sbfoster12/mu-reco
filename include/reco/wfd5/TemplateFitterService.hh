#ifndef TEMPLATEFITTER_SERVICE_HH
#define TEMPLATEFITTER_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/Service.hh"
#include "reco/wfd5/TemplateLoaderService.hh"
#include "reco/common/PulseFitterUtil.hh"

namespace reco {

    class TemplateFitterService : public Service {
    public:
        TemplateFitterService() = default;
        virtual ~TemplateFitterService() = default;

        void Configure(const nlohmann::json& config, EventStore& eventStore) override {

            // load the json config
            debug_ = config.value("debug", false);
            auto& jsonParserUtil = reco::JsonParserUtil::instance();

            std::string file_name = config.value("file_name", "fitters.json");
            std::string file_path_ = "";
            if (file_name.find('/') != std::string::npos) {
                // If not a base name, try using this path directly
                file_path_ = file_name;
            } else {
                // If a base name, prepend the config directory
                file_path_ = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + file_name;
            }
            if (!std::filesystem::exists(file_path_)) {
                throw std::runtime_error("TemplateFitterService: File not found: " + file_path_);
            }
            if (debug_) std::cout << "-> reco::TemplateFitterService: Configuring with file: " << file_path_ << std::endl;
            fitterConfig_ = jsonParserUtil.ParseFile(file_path_);  // Example usage of JsonParserUtil

            // auto& jsonParserUtil = reco::JsonParserUtil::instance();
            templateLoaderLabel_ = config.value("templateLoaderLabel", "templateLoader");

            // Get the loader templates from the services (Note that the template loader must be configure first!)
            auto templateLoader = GetServiceManager()->Get<TemplateLoaderService>(templateLoaderLabel_);

            if (debug_) std::cout << "-> reco::TemplateFitterService: Using TemplateLoaderService with label: " << templateLoaderLabel_ << std::endl;

            // loop through templates in the TemplateFitterService
            for (dataProducts::ChannelID id: templateLoader->GetValidChannels())
            {
                // create a template fitter for this channel
                if (debug_) std::cout << "Creating a template fit for channel:" 
                    << std::get<0>(id) << "/" << std::get<1>(id) << "/" << std::get<2>(id) 
                    << std::endl;
                pulseFitterHolder_[id] = new TemplateFit(
                    templateLoader->GetSpline(id),
                    templateLoader->GetSpline(id) // potential for a second spline (i.e. particle vs. laser pulse shape)
                );
                
                pulseFitterHolder_[id]->SetTSpline( templateLoader->GetTemplate(id),0 );
                pulseFitterHolder_[id]->SetTSpline( templateLoader->GetTemplate(id),1 );
                if (debug_) std::cout << "    -> Loading default config " << std::endl;
                pulseFitterHolder_[id]->SetValueFromConfig(config);
                if (debug_) std::cout << "    -> Done loading default config " << std::endl;

                // configure this template fitter based on the json file
            }

            // loop through custom configurations
            if (debug_) std::cout << "Looking for override config" << std::endl;
            for (const auto& configi : fitterConfig_["fitters"]) {
                std::vector<int> jid = configi["channel"];
                dataProducts::ChannelID id = {jid[0],jid[1],jid[2]};
                if (pulseFitterHolder_.count(id))
                {
                    if (debug_) std::cout << "overriding default fitter values for channel: " << jid[0] << "/" <<jid[1] << "/" <<jid[2] << std::endl;
                    pulseFitterHolder_[id]->SetValueFromConfig(configi);
                    if (debug_) std::cout << "    -> Done loading override config " << std::endl;
                    
                }
            }


        }

        bool ValidChannel(dataProducts::ChannelID id)
        {
            return pulseFitterHolder_.count(id);
        }

        TemplateFit* GetFitter( dataProducts::ChannelID id )
        {
            return pulseFitterHolder_[id];
        }

    private:

        std::string templateLoaderLabel_;
        std::map<dataProducts::ChannelID, TemplateFit*> pulseFitterHolder_;
        nlohmann::json fitterConfig_;
        bool debug_;


        ClassDefOverride(TemplateFitterService, 1);

    };
}

#endif
