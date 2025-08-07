#ifndef WFD5OUTPUTMANAGER_HH
#define WFD5OUTPUTMANAGER_HH

#include "reco/common/OutputManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"

#include <data_products/wfd5/WFD5Header.hh>
#include <data_products/wfd5/WFD5ChannelHeader.hh>
#include <data_products/wfd5/WFD5WaveformHeader.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/WFD5WaveformFit.hh>
#include <data_products/wfd5/WFD5ODB.hh>

namespace reco {
    
    class WFD5OutputManager : public OutputManager {
    public:
        WFD5OutputManager(const std::string& filename);
        ~WFD5OutputManager();

        void FillEvent(const EventStore& eventStore) override;

        void WriteODB(const EventStore& eventStore) override {
            file_->cd();
            dataProducts::WFD5ODB* odb = dynamic_cast<dataProducts::WFD5ODB*>(eventStore.GetODB().get()); // we do not own this pointer
            if (!odb) {
                throw std::runtime_error("Failed to cast ODB to WFD5ODB");
            }
            file_->WriteObject(odb, "wfd5_odb");
        };

        bool starts_with(const std::string& str, const std::string& prefix) {
            return str.size() >= prefix.size() &&
                str.compare(0, prefix.size(), prefix) == 0;
        }

        bool ends_with(const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() &&
                str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        bool matchesWildcard(const std::string& pattern, const std::string& text) {
            if (pattern == "*") return true;

            auto starPos = pattern.find('*');
            if (starPos == std::string::npos) {
                return pattern == text;
            }

            std::string prefix = pattern.substr(0, starPos);
            std::string suffix = pattern.substr(starPos + 1);

            if (!prefix.empty() && !starts_with(text, prefix)) return false;
            if (!suffix.empty() && !ends_with(text, suffix)) return false;

            return true;
        }

    private:
        std::map<std::string, std::vector<dataProducts::WFD5Header>> wfd5HeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5ChannelHeader>> channelHeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5WaveformHeader>> waveformHeaderBuffers_;
        std::map<std::string, std::vector<dataProducts::WFD5Waveform>> waveformBuffers_;
        std::map<std::string, std::vector<dataProducts::WaveformFit>> waveformFitBuffers_;

    };
}

#endif  // WFD5OUTPUTMANAGER_HH
