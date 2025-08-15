#include "reco/wfd5/Fitter.hh"
#include "reco/wfd5/TemplateFitterService.hh"
#include <iostream>

using namespace reco;

void Fitter::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "jitter");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "CorrectedWaveforms");
    outputFitResultLabel_ = config.value("outputFitResultLabel", "FitResultXtal");
    templateFitterLabel_ = config.value("templateFitterLabel", "templateFitter");
    fit_debug = config.value("debug", false);

    seeded_ = config.value("seeded",false);
    seeded_extra_leeway_ = config.value("seededExtraLeeway",false);
    seededInputReco_ = config.value("intputSeededTime", "timeSeedFinder");
    seededInputLabel_ = config.value("intputSeededTimeLabel", "seed");

}

void Fitter::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "Fitter with name '" << GetRecoLabel() << "' is processing...\n";
    try {

        // Get the template fitter service
        auto templateFitter = serviceManager.Get<TemplateFitterService>(templateFitterLabel_);

         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        TClonesArray* seeded_input;
        dataProducts::TimeSeed* seed;
        // double seed
        if(seeded_)
        {
            seeded_input = store.get<const dataProducts::TimeSeed>(seededInputReco_,seededInputLabel_);
            seed = static_cast<dataProducts::TimeSeed*>(seeded_input->ConstructedAt(0));
            if (!seed) {
                throw std::runtime_error("Failed to retrieve seeded time");
            }
            if (fit_debug) std::cout << "Performing a seeded fit around time: " << seed->GetTimeSeed() << std::endl;
        }

        //Make a collection new waveforms
        auto fitResults = store.getOrCreate<dataProducts::WaveformFit>(this->GetRecoLabel(), outputFitResultLabel_);

        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* wf = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!wf) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }


            dataProducts::ChannelID id = wf->GetID();
            if (templateFitter->ValidChannel(id))
            {
                 if (fit_debug) std::cout << "Performing fit on "
                    << std::get<0>(id) << "/" << std::get<1>(id) << "/" << std::get<2>(id) 
                    << std::endl;

                // Create a new fit result for this waveform
                int idx = fitResults->GetEntriesFast();
                dataProducts::WaveformFit* this_fit_result = new ((*fitResults)[idx]) dataProducts::WaveformFit(wf);
                fitResults->Expand(idx + 1);

                if (fit_debug)
                {
                    std::cout << "*********************************" << std::endl;
                    std::cout << "*********************************" << std::endl;
                    std::cout << "*********************************" << std::endl;
                    std::cout << "*********************************" << std::endl;
                    std::cout << "Performing fit on channel " << wf->crateNum << "/" << wf->amcNum << "/" << wf->channelTag << std::endl;
                    std::cout << "    -> Event " << wf->eventNum << " / " << wf->waveformIndex << std::endl;
                }
                auto start = std::chrono::high_resolution_clock::now();
                auto thisfitter = templateFitter->GetFitter(id);
                // if (see)
                thisfitter->reset();
                if (fit_debug) thisfitter->setDebug(true);
                thisfitter->addTrace(wf->trace, 0.0);
                auto intermediate = std::chrono::high_resolution_clock::now();
                // auto bestchi2 = 1;
                
                thisfitter->SetSeeded(seeded_,seeded_extra_leeway_);
                if (seeded_)
                {
                    if (fit_debug) std::cout << "Adding a guess based on the seed amp/time of " 
                        <<  wf->PeakToPeak() 
                        << " / " 
                        << seed->GetTimeSeed() 
                        << std::endl;
                    thisfitter->AddGuess(seed->GetTimeSeed(), wf->PeakToPeak());
                    this_fit_result->seed = seed;
                    this_fit_result->is_seeded = true;
                }

                auto bestchi2 = thisfitter->performMinimization();
                if (bestchi2 > 0) thisfitter->setFitResult(this_fit_result);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::micro> elapsed = end - start;
                std::chrono::duration<double, std::micro> elapsed2 = end - intermediate;
                if (fit_debug) std::cout << "Function call took " << elapsed.count() << " microseconds." << std::endl;
                if (fit_debug) std::cout << "   -> minimization " << elapsed2.count() << " microseconds." << std::endl;
                this_fit_result->fitTime = elapsed.count();

                if (fit_debug) std::cout << "Final chi2: " << bestchi2 << std::endl;
                if (fit_debug) std::cout << "Final Nfit: " << this_fit_result->nfit << std::endl;
            }
            else if (fit_debug)
            {
                std::cout << "No valid fitter found for channel" 
                    << std::get<0>(id) << "/" << std::get<1>(id) << "/" << std::get<2>(id) 
                    << "  -> skipping! " 
                    << std::endl;
            }
            

        }

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("Fitter error: ") + e.what());
    }
}