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
}

void Fitter::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "Fitter with name '" << GetLabel() << "' is processing...\n";
    try {
        // Get original waveforms as const shared_ptr collection (safe because get is const)
        auto waveforms = store.get<dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);
        auto templateFitter = serviceManager.Get<TemplateFitterService>(templateFitterLabel_);

        //Make a collection of fit results
        std::vector<std::shared_ptr<dataProducts::WaveformFit>> fitResults;
        fitResults.reserve(waveforms.size());


        for (const auto wf : waveforms) {
            // // Make a fit result and add it to the collection here
            // auto corrected = std::make_shared<dataProducts::WFD5Waveform>(*wf);
            // correctedWaveforms.push_back(std::move(corrected));

            dataProducts::ChannelID id = wf->GetID();
            if (templateFitter->ValidChannel(id))
            {
                std::cout << "Performing fit on "
                << std::get<0>(id) << "/" << std::get<1>(id) << "/" << std::get<2>(id) 
                << std::endl;

                auto this_fit_result = std::make_shared<dataProducts::WaveformFit>(wf.get());

                if (fit_debug)
                {
                    std::cout << "*********************************" << std::endl;
                    std::cout << "Performing fit on channel " << wf->crateNum << "/" << wf->amcNum << "/" << wf->channelTag << std::endl;
                    std::cout << "    -> Event " << wf->eventNum << " / " << wf->waveformIndex << std::endl;
                }
                auto start = std::chrono::high_resolution_clock::now();
                auto thisfitter = templateFitter->GetFitter(id);
                thisfitter->reset();
                if (fit_debug) thisfitter->setDebug(true);
                thisfitter->addTrace(wf->trace, 0.0);
                auto intermediate = std::chrono::high_resolution_clock::now();
                // auto bestchi2 = 1;
                auto bestchi2 = thisfitter->performMinimization();
                if (bestchi2 > 0) thisfitter->setFitResult(this_fit_result.get());
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::micro> elapsed = end - start;
                std::chrono::duration<double, std::micro> elapsed2 = end - intermediate;
                if (fit_debug) std::cout << "Function call took " << elapsed.count() << " microseconds." << std::endl;
                if (fit_debug) std::cout << "   -> minimization " << elapsed2.count() << " microseconds." << std::endl;
                this_fit_result->fitTime = elapsed.count();

                if (fit_debug) std::cout << "Final chi2: " << bestchi2 << std::endl;
                if (fit_debug) std::cout << "Final Nfit: " << this_fit_result->nfit << std::endl;
                fitResults.push_back(
                    std::move(this_fit_result)
                );
            }
            else
            {
                std::cout << "No valid fitter found for channel" 
                    << std::get<0>(id) << "/" << std::get<1>(id) << "/" << std::get<2>(id) 
                    << "  -> skipping! " 
                    << std::endl;
            }
            

        }

        // Store corrected fit results under a new key
        store.put(this->GetRecoLabel(), outputFitResultLabel_, std::move(fitResults));

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("Fitter error: ") + e.what());
    }
}