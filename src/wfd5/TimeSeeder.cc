#include "reco/wfd5/TimeSeeder.hh"
#include <iostream>

using namespace reco;

void TimeSeeder::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputFitResultsLabel_ = config.value("inputFitResultsLabel", "WFD5WaveformCollection");
    outputSeedLabel_ = config.value("outputSeedLabel", "seed");
    debug_ = config.value("debug",false);
    failOnError_ = config.value("failOnError",false);
    seedFromMaxAmplitudeFit_ = config.value("seedFromMaxAmplitudeFit", false);
    seedFromFirstFit_ = config.value("seedFromFirstFit", false);
    seedFromConfig_ = config.value("seedFromConfig", false);
    defaultSeed_ = config.value("defaultSeed", 100.0);

    std::vector<bool> options = {
        seedFromMaxAmplitudeFit_,
        seedFromFirstFit_,
        seedFromConfig_
    };

    bool more_than_one_found = !(std::accumulate(options.begin(),
                options.end(),
                0,
                std::bit_xor<void>())
    );

    bool none_found = !(std::accumulate(options.begin(),
                options.end(),
                0,
                std::bit_or<void>())
    );

    if(more_than_one_found) throw std::runtime_error("More than one option selected");

    if (none_found && !failOnError_)
    {
        seedFromConfig_ = true; // default to a simple seedn
    }

}

void TimeSeeder::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "TimeSeeder with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
         auto output = store.getOrCreate<dataProducts::TimeSeed>(this->GetRecoLabel(), outputSeedLabel_);
         int i = 0;
         dataProducts::TimeSeed* seed = new ((*output)[i]) dataProducts::TimeSeed();
         output->Expand(i + 1);

        if (seedFromFirstFit_)
        {
            if (debug_) std::cout << "Seeding with option: seedFromFirstFit_" << std::endl;
            throw std::runtime_error("Not implemented");

            // get the first fit from the first object in the collection

            auto input = store.get<const dataProducts::WaveformFit>(inputRecoLabel_, inputFitResultsLabel_);
            auto* fiti = static_cast<dataProducts::WaveformFit*>(input->ConstructedAt(0));

            seed->SetSeed(fiti->GetClosestPulseTime(-1e10)); // TODO: update for various time corrections, etc.

        }
        else if (seedFromMaxAmplitudeFit_)
        {
            if (debug_) std::cout << "Seeding with option: seedFromMaxAmplitudeFit_" << std::endl;
            throw std::runtime_error("Not implemented");
            auto input = store.get<const dataProducts::WaveformFit>(inputRecoLabel_, inputFitResultsLabel_);
        }
        else if (seedFromConfig_)
        {
            if (debug_) std::cout << "Seeding with option: seedFromConfig_" << std::endl;
            seed->SetSeed(defaultSeed_); 
        }


        if (debug_) std::cout << "Created a seed at time:" << seed->GetTimeSeed() << std::endl;

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("TimeSeeder error: ") + e.what());
    }
}
