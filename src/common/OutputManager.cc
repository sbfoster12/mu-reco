#include "reco/common/OutputManager.hh"

using namespace reco;

OutputManager::OutputManager(const std::string& filename)
    : file_(std::make_unique<TFile>(filename.c_str(), "RECREATE")),
      tree_(new TTree("tree", "tree")) {}

reco::OutputManager::~OutputManager() {
    file_->cd();
    tree_->Write();
    file_->Close();
}