#include "reco/wfd5/WFD5OutputManager.hh"

using namespace reco;

WFD5OutputManager::WFD5OutputManager(const std::string& filename)
    : OutputManager(filename) {}

WFD5OutputManager::~WFD5OutputManager() {}

void WFD5OutputManager::FillEvent(const EventStore& eventStore) {
   
    auto store = eventStore.GetStore();

    for (const auto& [collName, baseVec] : store) {
        if (baseVec.empty()) continue;

        if (std::find(keepList_.begin(), keepList_.end(), collName) == keepList_.end()) {
            // Skip collections not in keep list
            continue;
        }

        const TObject* firstObj = baseVec.front().get();
        if (!firstObj) continue;

        std::string className = firstObj->IsA()->GetName();

        if (className == "dataProducts::WFD5Header") {
            auto& typedVec = wfd5HeaderBuffers_[collName];
            typedVec.clear();
            typedVec.reserve(baseVec.size());

            for (const auto& basePtr : baseVec) {
                auto* hd = dynamic_cast<dataProducts::WFD5Header*>(basePtr.get());
                if (!hd) throw std::runtime_error("Bad cast to dataProducts::WFD5Header for collection " + collName);
                typedVec.push_back(*hd);
            }

            CreateBranchIfMissing(collName, &typedVec);

        } else if (className == "dataProducts::WFD5ChannelHeader") {
            auto& typedVec = channelHeaderBuffers_[collName];
            typedVec.clear();
            typedVec.reserve(baseVec.size());

            for (const auto& basePtr : baseVec) {
                auto* hd = dynamic_cast<dataProducts::WFD5ChannelHeader*>(basePtr.get());
                if (!hd) throw std::runtime_error("Bad cast to dataProducts::WFD5ChannelHeader for collection " + collName);
                typedVec.push_back(*hd);
            }

            CreateBranchIfMissing(collName, &typedVec);

        } else if (className == "dataProducts::WFD5WaveformHeader") {
            auto& typedVec = waveformHeaderBuffers_[collName];
            typedVec.clear();
            typedVec.reserve(baseVec.size());

            for (const auto& basePtr : baseVec) {
                auto* hd = dynamic_cast<dataProducts::WFD5WaveformHeader*>(basePtr.get());
                if (!hd) throw std::runtime_error("Bad cast to dataProducts::WFD5WaveformHeader for collection " + collName);
                typedVec.push_back(*hd);
            }

            CreateBranchIfMissing(collName, &typedVec);
        
        } else if (className == "dataProducts::WFD5Waveform") {
            auto& typedVec = waveformBuffers_[collName];
            typedVec.clear();
            typedVec.reserve(baseVec.size());

            for (const auto& basePtr : baseVec) {
                auto* wf = dynamic_cast<dataProducts::WFD5Waveform*>(basePtr.get());
                if (!wf) throw std::runtime_error("Bad cast to dataProducts::WFD5Waveform for collection " + collName);
                typedVec.push_back(*wf);
            }

            CreateBranchIfMissing(collName, &typedVec);

        } else {
            // throw and exception for unsupported types
            throw std::runtime_error("Unsupported collection type: " + className + " for collection " + collName);
        }
    }

    // Fill the tree
    tree_->Fill();
 }