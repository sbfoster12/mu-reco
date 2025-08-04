#include "reco/common/OutputManager.hh"

using namespace reco;

OutputManager::OutputManager(const std::string& filename, const std::string& treename)
    : file_(std::make_unique<TFile>(filename.c_str(), "RECREATE")),
      tree_(new TTree(treename.c_str(), treename.c_str())) {}

reco::OutputManager::~OutputManager() {
    file_->cd();
    tree_->Write();
    file_->Close();
}