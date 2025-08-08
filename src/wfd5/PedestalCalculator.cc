
#include "reco/wfd5/PedestalCalculator.hh"
#include <iostream>

using namespace reco;

void PedestalCalculator::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "timeAligned");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "Waveforms");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "Waveforms");
    pedestalMethod_ = config.value("pedestalMethod", "FirstN");
    numSamples_ = config.value("numSamples", 10);
}

void PedestalCalculator::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "PedestalCalculator with name '" << GetLabel() << "' is processing...\n";
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel_, inputWaveformsLabel_);

        //Make a collection new waveforms
        auto newWaveforms = store.getOrCreate<dataProducts::WFD5Waveform>(this->GetRecoLabel(), outputWaveformsLabel_);

        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }
            //Make the new waveform
            dataProducts::WFD5Waveform* newWaveform = new ((*newWaveforms)[i]) dataProducts::WFD5Waveform(waveform);
            newWaveforms->Expand(i + 1);

             ComputePedestal(newWaveform);
        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("PedestalCalculator error: ") + e.what());
    }
}

void PedestalCalculator::ComputePedestal(dataProducts::WFD5Waveform* wf) {
    
    // Initialize the pedestal
    double pedestal = 0.0;
    double pedestalStdev = 0.0;

    // Get the trace
    const auto& trace = wf->trace;

    int startIndex = -1;
    int endIndex = -1;
    
    if (pedestalMethod_ == "FirstN") {
        startIndex = 0;
        endIndex = std::min(numSamples_, static_cast<int>(trace.size()));
    } else if (pedestalMethod_ == "MiddleN") {
        startIndex = (trace.size() - numSamples_) / 2;
        endIndex = startIndex + numSamples_ < trace.size() ? startIndex + numSamples_ : trace.size();
    } else if (pedestalMethod_ == "LastN") {
        startIndex = std::max(0, static_cast<int>(trace.size()) - numSamples_);
        endIndex = trace.size();
    } else {
        throw std::runtime_error("Unknown pedestal method: " + pedestalMethod_);
    }

    std::vector<short> pedestalSamples;
    for (size_t i = startIndex; i < endIndex; ++i) {
        pedestalSamples.push_back(trace[i]);
        pedestal += trace[i];
    }
    pedestal /= (endIndex - startIndex);

    // Compute the standard deviation
    for (const auto& sample : pedestalSamples) {
        pedestalStdev += (sample - pedestal) * (sample - pedestal);
    }
    pedestalStdev = std::sqrt(pedestalStdev / pedestalSamples.size());

    // Set the waveform pedestal values
    wf->pedestalLevel = pedestal;
    wf->pedestalStdev = pedestalStdev;
    wf->pedestalSamples = pedestalSamples;
    wf->pedestalStartSample = startIndex;
}