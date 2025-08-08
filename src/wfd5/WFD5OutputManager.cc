#include "reco/wfd5/WFD5OutputManager.hh"

using namespace reco;

WFD5OutputManager::WFD5OutputManager(const std::string& filename)
    : OutputManager(filename) {}

WFD5OutputManager::~WFD5OutputManager() {}

void WFD5OutputManager::WriteODB(const EventStore& eventStore) {
    file_->cd();
    dataProducts::WFD5ODB* odb = dynamic_cast<dataProducts::WFD5ODB*>(eventStore.GetODB().get()); // we do not own this pointer
    if (!odb) {
        throw std::runtime_error("Failed to cast ODB to WFD5ODB");
    }
    file_->WriteObject(odb, "wfd5_odb");
};
