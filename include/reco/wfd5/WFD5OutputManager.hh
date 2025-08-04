#ifndef WFD5OUTPUTMANAGER_HH
#define WFD5OUTPUTMANAGER_HH

#include "reco/common/OutputManager.hh"

#include <data_products/wfd5/WFD5Header.hh>
#include <data_products/wfd5/WFD5ChannelHeader.hh>
#include <data_products/wfd5/WFD5WaveformHeader.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/WFD5ODB.hh>

namespace reco {
    
    class WFD5OutputManager : public OutputManager {
    public:
        WFD5OutputManager(const std::string& filename);
        ~WFD5OutputManager();

        void FillEvent(const EventStore& eventStore) override;

        void WriteODB(const EventStore& eventStore) override {
            file_->cd();
            dataProducts::WFD5ODB* odb = dynamic_cast<dataProducts::WFD5ODB*>(eventStore.GetODB().get());
            if (!odb) {
                throw std::runtime_error("Failed to cast ODB to WFD5ODB");
            }
            file_->WriteObject(odb, "wfd5_odb");
        };


    private:
        std::map<std::string, std::vector<dataProducts::WFD5Header>> wfd5HeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5ChannelHeader>> channelHeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5WaveformHeader>> waveformHeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5Waveform>> waveformBuffers_;

    };
}

#endif  // WFD5OUTPUTMANAGER_HH
