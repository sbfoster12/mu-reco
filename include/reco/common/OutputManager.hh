#ifndef OUTPUTMANAGER_HH
#define OUTPUTMANAGER_HH

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

#include <nlohmann/json.hpp>
#include <TFile.h>
#include <TTree.h>
#include <TObject.h>

#include <data_products/common/RecoConfig.hh>

#include "reco/common/EventStore.hh"
#include "reco/common/ConfigHolder.hh"

using json = nlohmann::json;

namespace reco {
    
    class OutputManager {
    public:
        OutputManager(const std::string& filename);
        virtual ~OutputManager();

        void Configure(const ConfigHolder& configHolder) {
            const nlohmann::json& config = configHolder.GetConfig();

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
            dataProducts::RecoConfig recoConfig(config.dump(),configHolder.GetRun(),configHolder.GetSubrun());
            file_->cd();
            file_->WriteObject(&recoConfig, "reco_config");
        }

        // Write event data from EventStore to tree
        virtual void FillEvent(const EventStore& eventStore) = 0;

        virtual void WriteODB(const EventStore& eventStore) = 0;

        void WriteHistograms(const EventStore& store) {
            for (const auto& [name, hist] : store.GetAllHistograms()) {
                file_->cd();
                hist->Write(name.c_str());
            }
        }

    protected:
        // Helper to create branch if missing
        template <typename T>
        void CreateBranchIfMissing(const std::string& name, std::vector<T>* buffer) {
            if (!tree_->GetBranch(name.c_str())) {
                tree_->Branch(name.c_str(), buffer);
            }
        }

        // Collections to not write to the tree
        std::vector<std::string> dropList_; 

        std::unique_ptr<TFile> file_;
        TTree* tree_;
    };
} //namespace reco


#endif  // OUTPUTMANAGER_HH
