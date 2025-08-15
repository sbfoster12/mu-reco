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

        void Configure(const nlohmann::json& config, EventStore& eventStore) override;

        void InitializeWithRun(int run);

        void SetSpline(dataProducts::ChannelID id, TSpline3* sp);

        void LoadSplines(std::string infile);

        TSpline3* GetTemplate(dataProducts::ChannelID id);

        fitter::CubicSpline* GetSpline(dataProducts::ChannelID id);

        fitter::CubicSpline* buildCubicSpline(const TSpline3* tSpline, fitter::CubicSpline::BoundaryType cond);

        dataProducts::ChannelList GetValidChannels();

        std::shared_ptr<dataProducts::SplineHolder> GetSplineHolder();

    private:
        std::string file_path_;
        // std::map<dataProducts::ChannelID, TSpline3*> template_map_;
        std::shared_ptr<dataProducts::SplineHolder> splineHolder_;
        std::vector<int> crateNumbers_ = {7,8};
        nlohmann::json templateConfig_;
        bool debug_;

        ClassDefOverride(TemplateLoaderService, 1);

    };
}

#endif
