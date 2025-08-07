#ifndef TEMPLATELOADER_SERVICE_HH
#define TEMPLATELOADER_SERVICE_HH

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "reco/common/Service.hh"
#include "data_products/common/DataProduct.hh"
#include "TSpline.h"
#include "TFile.h"
#include "data_products/wfd5/CubicSpline.hh"
#include "data_products/wfd5/WFD5WaveformFit.hh"

namespace reco {

    class TemplateLoaderService : public Service {
    public:
        TemplateLoaderService() = default;
        virtual ~TemplateLoaderService() = default;

        void Configure(const nlohmann::json& config, EventStore& eventStore) override {

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
            templateConfig_ = jsonParserUtil.ParseFile(file_path_);  // Example usage of JsonParserUtil

            std::shared_ptr<dataProducts::SplineHolder> sharedHolder = std::make_shared<dataProducts::SplineHolder>();
            splineHolder_ = sharedHolder;
            
            int run = configHolder_->GetRun();
            int subrun = configHolder_->GetSubrun();
            InitializeWithRun(run);
            
            eventStore.putSplines(
                templateConfig_.value("label","templateLoader"),
                sharedHolder                
            );
            // std::vector<std::string> template_root_file = jsonObj.value("templates", {});
            // std::cout << "Loading template from file:" << template_root_file << std::endl;
            // LoadSplines(template_root_file);          

        }

        void InitializeWithRun(int run)
        {
            for (const auto& tmpl : templateConfig_["templates"]) {
                std::vector<int> iov = tmpl["iov"];
                if (run >= iov[0] && run <= iov[1])
                {
                    std::string infile = tmpl["file"];
                    LoadSplines(infile);
                    break;
                }
            }
        }

        void SetSpline(dataProducts::ChannelID id, TSpline3* sp)
        {
            // template_map_[id] = sp;
            splineHolder_->SetSpline(id, sp, 0);
        };

        void LoadSplines(std::string infile)
        {
            TFile *this_file = new TFile(infile.c_str(),"OPEN");
            for (int crate : crateNumbers_)
            {
                for (int amcNum = 1; amcNum < 13; amcNum ++)
                {
                    for (int channel = 0; channel < 5; channel ++)
                    {
                        std::string keyName = TString::Format("crate_%i_amc_%i_channel_%i",crate,amcNum, channel).Data();
                        // std::cout << "Searching for splines: " << keyName << std::endl;
                        if (this_file->GetKey(keyName.c_str())) {
                            std::cout << "   -> Key \"" << keyName << "\" exists in the file." << std::endl;
                            TSpline3* this_spline = (TSpline3*)this_file->Get(keyName.c_str());
                            std::cout << "   -> Loaded spline:" << this_spline << std::endl;
                            // this_spline->SetDirectory(0);
                            SetSpline({crate,amcNum,channel}, this_spline);
                        }
                    }
                }
            }
            this_file->Close();
        }

        TSpline3* GetTemplate(dataProducts::ChannelID id) 
        {
            if (splineHolder_->SplinePresent(id))
            {
                // return template_map_[id];
                return splineHolder_->GetTSpline(id,0);
            }
            throw;
        }

        fitter::CubicSpline* GetSpline(dataProducts::ChannelID id)
        {
            return splineHolder_->GetSpline(id, 0);
        }

        fitter::CubicSpline* buildCubicSpline(const TSpline3* tSpline, fitter::CubicSpline::BoundaryType cond  = fitter::CubicSpline::BoundaryType::first) 
        {
            unsigned int nKnots = tSpline->GetNp();

            fitter::CubicSpline::Knots knots(nKnots);

            for (unsigned int i = 0; i < nKnots; ++i) {
                tSpline->GetKnot(i, knots.xs[i], knots.ys[i]);
            }

            return new fitter::CubicSpline(knots, cond);
        }

        dataProducts::ChannelList GetValidChannels()
        {
            // dataProducts::ChannelList keys;
            // keys.reserve(template_map_.size());
            // for (const auto& pair : template_map_) {
            //     keys.push_back(pair.first);
            // }
            // return keys;
            return splineHolder_->GetIDs();
        }

        std::shared_ptr<dataProducts::SplineHolder> GetSplineHolder()
        {
            return splineHolder_;
        }

    private:
        std::string file_path_;
        // std::map<dataProducts::ChannelID, TSpline3*> template_map_;
        std::shared_ptr<dataProducts::SplineHolder> splineHolder_;
        std::vector<int> crateNumbers_ = {7,8};
        nlohmann::json templateConfig_;

        ClassDefOverride(TemplateLoaderService, 1);

    };
}

#endif
