#ifndef OUTPUTMANAGER_HH
#define OUTPUTMANAGER_HH

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TObject.h>

#include "reco/common/EventStore.hh"

namespace reco {
    
    class OutputManager {
    public:
        OutputManager(const std::string& filename, const std::string& treename);
        virtual ~OutputManager();

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

        std::unique_ptr<TFile> file_;
        TTree* tree_;
    };
} //namespace reco


#endif  // OUTPUTMANAGER_HH
