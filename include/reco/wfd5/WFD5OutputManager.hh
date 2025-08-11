#ifndef WFD5OUTPUTMANAGER_HH
#define WFD5OUTPUTMANAGER_HH

#include "reco/common/OutputManager.hh"
#include "reco/wfd5/TemplateLoaderService.hh"

// any new dataproduct to be written to the tree must be included here!
#include <data_products/wfd5/WFD5Header.hh>
#include <data_products/wfd5/WFD5ChannelHeader.hh>
#include <data_products/wfd5/WFD5WaveformHeader.hh>
#include <data_products/wfd5/WFD5Waveform.hh>
#include <data_products/wfd5/WFD5WaveformFit.hh>
#include <data_products/wfd5/WaveformIntegral.hh>
#include <data_products/wfd5/WFD5ODB.hh>

namespace reco {
    
    class WFD5OutputManager : public OutputManager {
    public:
        WFD5OutputManager(const std::string& filename);
        ~WFD5OutputManager();

        void WriteODB(const EventStore& eventStore) override;

    private:
        
    };
}

#endif  // WFD5OUTPUTMANAGER_HH
