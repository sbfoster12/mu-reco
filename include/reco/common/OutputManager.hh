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
#include "TClonesArray.h"

#include <data_products/common/RecoConfig.hh>

#include "reco/common/EventStore.hh"
#include "reco/common/ConfigHolder.hh"

using json = nlohmann::json;

namespace reco {
    
    class OutputManager {
    public:
        OutputManager(const std::string& filename);
        virtual ~OutputManager();

        void Configure(std::shared_ptr<const ConfigHolder> configHolder);

        // Write event data from EventStore to tree
        void FillEvent(const EventStore& eventStore);

        // Virtual method for writing the ODB
        virtual void WriteODB(const EventStore& eventStore) = 0;
        void WriteHistograms(const EventStore& store);
        void WriteSplines(const EventStore& store);

        bool MatchesWildcard(const std::string& pattern, const std::string& text);
        bool StartsWith(const std::string& str, const std::string& prefix);
        bool EndsWith(const std::string& str, const std::string& suffix);

    protected:

        // Helper to create branch if missing
        void CreateBranchIfMissing(const std::string& name, TClonesArray* buffer);

        // Collections to not write to the tree
        std::vector<std::string> dropList_; 

        std::unique_ptr<TFile> file_;
        TTree* tree_;
        int compressionLevel_;
        int compressionAlgorithm_;
    };
} //namespace reco


#endif  // OUTPUTMANAGER_HH
