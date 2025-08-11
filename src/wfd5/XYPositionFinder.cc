
#include "reco/wfd5/XYPositionFinder.hh"
#include "data_products/wfd5/ClusteredHits.hh"
#include "data_products/wfd5/WaveformIntegral.hh"
#include "data_products/wfd5/WFD5WaveformFit.hh"
#include <iostream>

using namespace reco;

void XYPositionFinder::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "");
    inputFitResultsLabel_ = config.value("inputFitResultsLabel", "DEFAULT");
    outputPositionLabel_ = config.value("outputPositionLabel", "");
    debug_ = config.value("debug", false); 
    integrals_ = config.value("integrals", false); // same module can take integrals or fit result
    weighting_ = config.value("weightingMode", 0);

    // First fit in the pulse train
    useFirstFitinTime_ = config.value("useFirstFitInTime", false);

    // first fit in the fit sequence. This will be the maximum energy fit (locally) or the seeded fit.
    useFirstFitInFitSequence_ = config.value("useFirstFit", false);

    // highest energy fit (locally)
    useHighestEnergyFit_ = config.value("useHighestEnergyFit", false);

    if (!(useFirstFitinTime_ || useFirstFitInFitSequence_ || useHighestEnergyFit_))
    {
        if (!integrals_) std::cout << "No default fit selection method set -> defaulting to useFirstFitinTime " <<  std::endl;
        useFirstFitinTime_ = true;
    }

    if (!(useFirstFitinTime_ ^ useHighestEnergyFit_ ^ useFirstFitInFitSequence_)) // NOT XOR
    {
        std::cerr << "Configuration error: more than one of [useHighestEnergyFit, useFirstFitinTime, useFirstFitInFitSequence] set" 
        << "   -> (useFirstFitinTime_ ^ useHighestEnergyFit_ ^ useFirstFitInFitSequence_) -> " 
        << useFirstFitinTime_ << " / " << useHighestEnergyFit_ << " / " << useFirstFitInFitSequence_
        << std::endl;
        throw;
    }
}


void XYPositionFinder::ProcessIntegralsToXY(TClonesArray* input, dataProducts::ClusteredHits* thisCluster)
{
    for (int i = 0; i < input->GetEntriesFast(); ++i) {
        auto* waveform = static_cast<dataProducts::WaveformIntegral*>(input->ConstructedAt(i));
        if (!waveform) {
            throw std::runtime_error("Failed to retrieve waveform integral at index " + std::to_string(i));
        }
        thisCluster->inputs.push_back(waveform);
        //Make the new waveform
        xs.push_back(waveform->x);
        ys.push_back(waveform->y);
        energies.push_back(waveform->integral);
    }
}

void XYPositionFinder::ProcessFitsToXY(TClonesArray* input, dataProducts::ClusteredHits* thisCluster)
{
    for (int i = 0; i < input->GetEntriesFast(); ++i) {
        auto* fit = static_cast<dataProducts::WaveformFit*>(input->ConstructedAt(i));
        if (!fit) {
            throw std::runtime_error("Failed to retrieve waveform fit at index " + std::to_string(i));
        }
        thisCluster->inputs.push_back(fit);
        //Make the new waveform
        auto input_wf = (dataProducts::WFD5Waveform*) fit->waveforms[0].GetObject();
        xs.push_back(input_wf->x);
        ys.push_back(input_wf->y);
        int fit_index = 0;
        if (useFirstFitinTime_)
        {
            fit_index = fit->GetClosestPulseIndex(-100000.);
        }
        else if (useFirstFitInFitSequence_)
        {
            fit_index = 0;
        }
        else if (useHighestEnergyFit_)
        {
            fit_index = std::distance( 
                fit->amplitudes.begin(),
                std::max_element(fit->amplitudes.begin(), fit->amplitudes.end()) 
            );
        }
        else 
        {
            throw;
        }
        energies.push_back(fit->amplitudes[fit_index]);
        thisCluster->fitIndex.push_back(fit_index);
    }
    
}

void XYPositionFinder::Cluster(dataProducts::ClusteredHits* thisCluster)
{
    double avgX = 0.0;
    double avgY = 0.0;
    if (debug_ ) std::cout << "Performing average of: ";
    for (int i = 0; i < weights.size(); i++)
    {
        avgX += xs[i]*weights[i];
        avgY += ys[i]*weights[i];
        if (debug_) std:: cout << "[( " << xs[i] << ", " << weights[i] << "), (" << ys[i] << ", " << weights[i] << ")], ";
    }
    avgX /= weightSum;
    avgY /= weightSum;

    if (debug_) std::cout << std::endl << "Result: (" << avgX << ", " << avgY << ")." << std::endl;

    thisCluster->x = avgX;
    thisCluster->y = avgY;
}

void XYPositionFinder::Process(EventStore& store, const ServiceManager& serviceManager) {
    // std::cout << "XYPositionFinder with name '" << this->GetLabel() << "' is processing...\n";
    try {
        // // Get original waveforms as const shared_ptr collection (safe because get is const)
        TClonesArray* inputCollection;
        // TObject* inputObject;
        int i = 0;
        dataProducts::ClusteredHits* thisCluster;
        auto xyPositions = store.getOrCreate<dataProducts::ClusteredHits>(this->GetRecoLabel(), outputPositionLabel_);

        if (debug_) std::cout << "Looking for collection with name: '" << inputRecoLabel_ << "' / '" << inputFitResultsLabel_ << "'" << std::endl;

        if (integrals_)
        {
            // process integral results
            inputCollection = store.get<const dataProducts::WaveformIntegral>(inputRecoLabel_, inputFitResultsLabel_);
            if (inputCollection->GetEntriesFast() < 1) 
            {
                if(debug_) std::cout << "Warning: No inputs found for xyClustering" << std::endl;
                return;
            }
            if (debug_) std::cout << "Found " << inputCollection->GetEntriesFast() << " objects to cluster" << std::endl;
            auto inputObject = (dataProducts::WaveformIntegral*) inputCollection->At(0);
            thisCluster = new ((*xyPositions)[i]) dataProducts::ClusteredHits(inputObject);
        }
        else 
        {
            // process fit results
            inputCollection = store.get<const dataProducts::WaveformFit>(inputRecoLabel_, inputFitResultsLabel_);
            if (inputCollection->GetEntriesFast() < 1) 
            {
                if(debug_) std::cout << "Warning: No inputs found for xyClustering" << std::endl;
                return;
            }
            if (debug_) std::cout << "Found " << inputCollection->GetEntriesFast() << " objects to cluster" << std::endl;
            auto inputObject = (dataProducts::WaveformFit*) inputCollection->At(0);
            thisCluster = new ((*xyPositions)[i]) dataProducts::ClusteredHits(inputObject);
        }
        xyPositions->Expand(i + 1);

        if (debug_) std::cout << "Beginning clustering process" << std::endl;

        xs.clear();
        ys.clear();
        weights.clear();
        energies.clear();

        double xi,yi,wi;
        
        thisCluster->inputs.reserve(inputCollection->GetEntriesFast());
        xs.reserve(inputCollection->GetEntriesFast());
        ys.reserve(inputCollection->GetEntriesFast());
        weights.reserve(inputCollection->GetEntriesFast());
        energies.reserve(inputCollection->GetEntriesFast());

        if (integrals_)
        {
            ProcessIntegralsToXY(inputCollection, thisCluster);
        }
        else
        {
            ProcessFitsToXY(inputCollection, thisCluster);
        }

        bool doCluster = true;
        double max_ei;
        switch (weighting_)
        {
            case 0:
                // unit weighting
                if (debug_) std::cout << "Weighting method " << weighting_ << " -> Unit Weighting" << std::endl;
                for (auto ei: energies) { weights.push_back(1.0); }
                break;
            case 1:
                // energy weigting
                if (debug_) std::cout << "Weighting method " << weighting_ << " -> Energy Weighting" << std::endl;
                for (auto ei: energies) { weights.push_back(ei); }
                break;
            case 2:
                // take highest energy
                if (debug_) std::cout << "Weighting method " << weighting_ << " -> Picking Largest Energy" << std::endl;
                max_ei = *std::max_element(energies.begin(), energies.end());
                for (auto ei: energies) 
                { 
                    if (ei == max_ei)
                    {
                        weights.push_back(1.0);
                    }
                    else 
                    {
                        weights.push_back(0.0);
                    }
                }
                break;
            default:
                std::cerr<< "Weighting method '" << weighting_ << "' is not implemented" << std::endl;
                throw;
        }
        weightSum = std::accumulate(weights.begin(), weights.end(), 0.0);
        // thisCluster.fitIndex = 

        if (doCluster) Cluster(thisCluster); // only don't do this if we override somehow.


    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("XYPositionFinder error: ") + e.what());
    }
}