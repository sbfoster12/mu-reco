
#include "reco/wfd5/RFFitter.hh"
#include <iostream>

using namespace reco;

void RFFitter::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "");
    outputFitResultLabel_ = config.value("outputFitResultLabel", "");
    fitStartTime_ = config.value("fitStartTime", 0.0);
    fitEndTime_ = config.value("fitEndTime", 100.0);
    frequency_ = config.value("frequency", -1.0); // important that this is -1.0, not 1 to get a double; -1 means "use the zero crossings to estimate the frequency"
    fixFrequency_ = config.value("fixedFrequency", false);
    fitOption_ = config.value("fitOption", "R");
}

void RFFitter::Process(EventStore& store, const ServiceManager& serviceManager) const {
    // std::cout << "RFFitter with name '" << GetRecoLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Make a collection new waveforms
        auto fitResults = store.getOrCreate<dataProducts::RFWaveformFit>(this->GetRecoLabel(), outputFitResultLabel_);

        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }
            //Make the new waveform
            dataProducts::RFWaveformFit* newFitResult = new ((*fitResults)[i]) dataProducts::RFWaveformFit(waveform);
            fitResults->Expand(i + 1);

            // Do the fit
            PerformRFFit(waveform,newFitResult);

        }

    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("RFFitter error: ") + e.what());
    }
}

void RFFitter::PerformRFFit(const dataProducts::WFD5Waveform* waveform, dataProducts::RFWaveformFit* fitResult) const {
    // std::cout << "Performing RF fit for waveform with index: " << waveform->waveformIndex << std::endl;

    // Get the trace
    auto trace = waveform->trace;

    // Make a TGraph
    TGraph* graph = new TGraph(trace.size());
    for (size_t i = 0; i < trace.size(); ++i) {
        graph->SetPoint(i, i, trace[i]);
    }

    // Make the fit function
    TF1 fitFunc("fitFunc", "[1]*cos(x*[0]) + [2]*sin([0]*x) + [3]", 0, trace.size());

    // Estimate baseline as mean value
    double baseline = std::accumulate(trace.begin(), trace.end(), 0.0) / trace.size();

    // Determine the number of (positive) zero crossings to estimate frequency and phase; also estimate the amplitude
    int zeroCrossings = 0;
    double firstPosCrossingTime = 0.0;
    double amplitude = 0.0;
    int spacing = 1;
    for (size_t i = spacing; i < trace.size(); ++i) {
        if ((trace[i-spacing] < baseline && trace[i] >= baseline)) {
            zeroCrossings++;
            if (firstPosCrossingTime == 0.0) {
                firstPosCrossingTime = 0.5*(2*i - spacing);
            }
        }
        if (std::abs(trace[i] - baseline) > std::abs(amplitude)) {
            amplitude = std::abs(trace[i] - baseline);
        }
    }

    // Use the provided frequency if > 0
    double freq = frequency_;
    if (freq < 0.0) {
        freq = 2 * TMath::Pi() * (double)(zeroCrossings) / (trace.size() - spacing); // Estimate frequency
    }

    // Estimate the phase from the first zero crossing
    double phase = 0.0;
    if (firstPosCrossingTime > 0.0) {
        phase = freq * firstPosCrossingTime;
    }

    // std::cout << "Estimated frequency: " << freq << ", phase: " << phase << ", A*cos: " <<  amplitude*TMath::Cos(phase) << ", -A*sin: " << -amplitude*TMath::Sin(phase) << ", amplitude: " << amplitude << ", baseline: " << baseline << std::endl;

    // Set the initial parameters for the fit function
    fitFunc.SetParameters(freq, amplitude*TMath::Cos(phase), -amplitude*TMath::Sin(phase), baseline); // Initial parameters

    if (fixFrequency_) {
        fitFunc.FixParameter(0, freq); // Fix frequency if specified
    }

    // Do the fit!
    graph->Fit(&fitFunc, fitOption_.c_str(),"", fitStartTime_,fitEndTime_);

    // Store the fit results in the fitResult object
    fitResult->chi2 = fitFunc.GetChisquare();
    fitResult->ndf = fitFunc.GetNDF();
    fitResult->converged = fitFunc.IsValid();
    fitResult->frequency = fitFunc.GetParameter(0);
    fitResult->amplitude = std::sqrt(fitFunc.GetParameter(1)*fitFunc.GetParameter(1) + fitFunc.GetParameter(2)*fitFunc.GetParameter(2));
    fitResult->phase = std::atan2(fitFunc.GetParameter(2), fitFunc.GetParameter(1));
    fitResult->pedestalLevel = fitFunc.GetParameter(3);

    // Set the fit function
    fitResult->SetFitFunc(std::move(fitFunc));

    // Clean up
    delete graph;
}