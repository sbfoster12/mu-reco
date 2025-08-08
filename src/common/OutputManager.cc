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

// Configure the OutputManager with a ConfigHolder
void OutputManager::Configure(std::shared_ptr<const ConfigHolder> configHolder) {
    const nlohmann::json& config = configHolder->GetConfig();

    if (!config.contains("Output")) {
        throw std::runtime_error("OutputManager: Missing or invalid 'Output' config");
    }

    if ( config["Output"].contains("drop") && config["Output"]["drop"].is_array() ) {
        dropList_.clear();
        for (const auto& colName : config["Output"]["drop"]) {
            if (!colName.is_string()) {
                throw std::runtime_error("OutputManager: 'drop' must be an array of strings");
            }
            dropList_.push_back(colName.get<std::string>());
        }
    } else {
        throw std::runtime_error("OutputManager: 'drop' must be an array in 'Output' config");
    }

    //Write the configuration to the file
    dataProducts::RecoConfig recoConfig(config.dump(),configHolder->GetRun(),configHolder->GetSubrun());
    file_->cd();
    file_->WriteObject(&recoConfig, "reco_config");
}


// Fill the event data from EventStore to the TTree
void OutputManager::FillEvent(const EventStore& eventStore) {
   
    // Get the buffers
    auto buffers = eventStore.GetBuffers();
    auto bufferKeys = eventStore.GetBufferKeys();

    // Loop over buffer keys and create the branches
    for (const auto& collName : bufferKeys) {
        
        const auto& buffer = buffers.at(collName);
        if (!buffer) {
            std::cerr << "-> reco::WFD5OutputManager: No buffer found for collection '" << collName << "'." << std::endl;
            continue;
        }

        if (std::any_of(dropList_.begin(), dropList_.end(),
            [&](const std::string& pattern) {   // capture collName by value, NOT &
                return MatchesWildcard(pattern, collName);
            })) {
            continue;
        }
        CreateBranchIfMissing(collName, buffer);
    }

    // Ensure TRefs work
    tree_->BranchRef();

    // Fill the tree
    tree_->Fill();

 }


bool OutputManager::StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

bool OutputManager::EndsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool OutputManager::MatchesWildcard(const std::string& pattern, const std::string& text) {
    if (pattern == "*") return true;

    auto starPos = pattern.find('*');
    if (starPos == std::string::npos) {
        return pattern == text;
    }

    std::string prefix = pattern.substr(0, starPos);
    std::string suffix = pattern.substr(starPos + 1);

    if (!prefix.empty() && !StartsWith(text, prefix)) return false;
    if (!suffix.empty() && !EndsWith(text, suffix)) return false;

    return true;
}

// Write splines
void OutputManager::WriteSplines(const EventStore& store) {
    for (const auto& [name, splines] : store.GetAllSplines()) {
        file_->cd();
        splines->Write(name.c_str());
        std::cout << "-> reco::OutputManager: Wrote spline holder '" << name << "' to the TFile." << std::endl;
    }
}

// Write histograms
void OutputManager::WriteHistograms(const EventStore& store) {
    for (const auto& [name, hist] : store.GetAllHistograms()) {
        file_->cd();
        hist->Write(name.c_str());
        std::cout << "-> reco::OutputManager: Wrote histogram '" << name << "' to the TFile." << std::endl;
    }
}


// protected:

// Helper to create branch if missing
void OutputManager::CreateBranchIfMissing(const std::string& name, TClonesArray* buffer) {
    if (!tree_->GetBranch(name.c_str())) {
        tree_->Branch(name.c_str(), &buffer);
        std::cout << "-> reco::OutputManager: Created branch '" << name << "' in tree." << std::endl;
    }
}