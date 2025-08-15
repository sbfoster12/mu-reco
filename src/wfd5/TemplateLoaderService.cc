#include "reco/wfd5/TemplateLoaderService.hh"

using namespace reco;

void TemplateLoaderService::Configure(const nlohmann::json& config, EventStore& eventStore) {

    debug_ = config.value("debug", false);
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();
    
    // Get the energy calibration configuration from the IOV list using the run and subrun
    auto templateConfigJson =  jsonParserUtil.GetConfigFromIOVList(config, run, subrun, "templates_iov", debug_);
    if (templateConfigJson.empty()) {
        throw std::runtime_error("TemplateLoaderService configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
    }
    
    // Now parse the configuration
    if (!templateConfigJson.contains("templates")) {
        throw std::runtime_error("Template configuration JSON must contain 'templates' key");
    }

    templateConfig_ = templateConfigJson["templates"];
 
    std::shared_ptr<dataProducts::SplineHolder> sharedHolder = std::make_shared<dataProducts::SplineHolder>();
    splineHolder_ = sharedHolder;
    

    // Set the splines
    std::string infile = templateConfig_["file"];
    LoadSplines(infile);
    
    eventStore.putSplines(
        config.value("label","templateLoader"),
        sharedHolder                
    );
    // std::vector<std::string> template_root_file = jsonObj.value("templates", {});
    // std::cout << "Loading template from file:" << template_root_file << std::endl;
    // LoadSplines(template_root_file);          

}

void TemplateLoaderService::SetSpline(dataProducts::ChannelID id, TSpline3* sp)
{
    // template_map_[id] = sp;
    splineHolder_->SetSpline(id, sp, 0);
};

void TemplateLoaderService::LoadSplines(std::string infile)
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
                    if (debug_) std::cout << "   -> Key \"" << keyName << "\" exists in the file." << std::endl;
                    TSpline3* this_spline = (TSpline3*)this_file->Get(keyName.c_str());
                    if (debug_) std::cout << "   -> Loaded spline:" << this_spline << std::endl;
                    // this_spline->SetDirectory(0);
                    SetSpline({crate,amcNum,channel}, this_spline);
                }
            }
        }
    }
    this_file->Close();
}

TSpline3* TemplateLoaderService::GetTemplate(dataProducts::ChannelID id) 
{
    if (splineHolder_->SplinePresent(id))
    {
        // return template_map_[id];
        return splineHolder_->GetTSpline(id,0);
    }
    throw;
}

fitter::CubicSpline* TemplateLoaderService::GetSpline(dataProducts::ChannelID id)
{
    return splineHolder_->GetSpline(id, 0);
}

fitter::CubicSpline* TemplateLoaderService::buildCubicSpline(const TSpline3* tSpline, fitter::CubicSpline::BoundaryType cond  = fitter::CubicSpline::BoundaryType::first) 
{
    unsigned int nKnots = tSpline->GetNp();

    fitter::CubicSpline::Knots knots(nKnots);

    for (unsigned int i = 0; i < nKnots; ++i) {
        tSpline->GetKnot(i, knots.xs[i], knots.ys[i]);
    }

    return new fitter::CubicSpline(knots, cond);
}

dataProducts::ChannelList TemplateLoaderService::GetValidChannels()
{
    // dataProducts::ChannelList keys;
    // keys.reserve(template_map_.size());
    // for (const auto& pair : template_map_) {
    //     keys.push_back(pair.first);
    // }
    // return keys;
    return splineHolder_->GetIDs();
}

std::shared_ptr<dataProducts::SplineHolder> TemplateLoaderService::GetSplineHolder()
{
    return splineHolder_;
}