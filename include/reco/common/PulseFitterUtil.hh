#pragma once

#include <TSpline.h>
#include <Math/Minimizer.h>
#include <Math/Factory.h>
#include <Math/Functor.h>
#include <vector>
#include <iostream>
#include <limits>
#include <iomanip>
#include "data_products/common/DataProduct.hh"
#include "data_products/wfd5/WFD5WaveformFit.hh"
#include "TRef.h"
// #include <omp.h>
#include "data_products/wfd5/CubicSpline.hh"
#include <nlohmann/json.hpp>


class TemplateFit {
public:
    // TemplateFit(TSpline3* spline1, TSpline3* spline2)
    TemplateFit(fitter::CubicSpline* spline1, fitter::CubicSpline* spline2)
        : minimumAmplitude(100), timeBounds(10), maxPulses(10), chi2Threshold(10), debug(false), single_spline_only(false), restricted_chi2_min(-10), restricted_chi2_max(50), amp_scale_factor(1.0) {
        if (debug) std::cout << "Creating TemplateFit from: " << spline1 << " / " << spline2 << std::endl;
        if (!spline1 || !spline2) {
            throw std::runtime_error("Error: Splines not initialized!");
        }
        if (debug) 
        {
            std::cout << "spline eval test"  << std::endl;
            // std::cout << "    -> " << spline1->Eval(0) << std::endl;
            // std::cout << "    -> " << spline2->Eval(0) << std::endl;
            std::cout << "    -> " << (*spline1)(0) << std::endl;
            std::cout << "    -> " << (*spline2)(0) << std::endl;
        }
            
        splines[0] = spline1;
        splines[1] = spline2;

        if (debug) std::cout << "Creating Minimizer..." << std::endl;
        minimizer = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
        // minimizer = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Minimize");
        if (!minimizer)
        {
            // ROOT::Math::Factory::PrintAvailableMinimizerTypes();
            throw std::runtime_error("Error: Minimzer not created!");
        }
        if (debug) std::cout << "   -> minimizer: " << minimizer << std::endl;
        
        minimizer->SetStrategy(0);
        minimizer->SetMaxFunctionCalls(1000);
        minimizer->SetTolerance(1e-6);
        // minimizer->SetTolerance(1e-3);
        minimizer->SetPrintLevel(0);
        if (debug) std::cout << "   -> TemplateFit initialized" << std::endl;

    }

    void SetValueFromConfig(nlohmann::json config)
    {
        // set to default values if no json key is found
        SetMinimumAmplitude( config.value("minimumAmplitude",100) );
        SetTimeBounds( config.value("timeBounds",10) );
        SetMaxPulses( config.value("maxPulses",10) );
        SetChi2Threshold( config.value("chi2Threshold",10) );

        setDebug( config.value("debug", false) );
        setSingleTemplate( config.value("singleTemplate", true ) );
        setTimeoutLimit( config.value("timeoutLimit", 100000 ) );
        SetAmpScale( config.value("ampScale", 1.0 ) );
        SetMinMaxClippingRange( 
            config.value("minClippingRange", -2000),
            config.value("maxClippingRange",  2000)
        );

    }

    void SetMinimumAmplitude(double val) { minimumAmplitude = val; }
    void SetTimeBounds(double val) { timeBounds = val; }
    void SetMaxPulses(double val) { maxPulses = val; }
    void SetChi2Threshold(double val) { chi2Threshold = val; }

    void SetTSpline(TSpline3* sp, int i)
    {
        tsplines[i] = sp;
    }

    void setFitResult(dataProducts::WaveformFit* w)
    {

        if (debug)
        {
            std::cout << "Model parameters at readout time:\n";
            std::cout << "xs size: " << xs.size() << "\n";
            std::cout << "p size: " << guesses.size() << "\n";
            for (size_t i = 0; i < guesses.size(); ++i) 
            {
                std::cout << "p[" << i << "] = " << guesses[i] << "\n";
            }
            std::cout << std::endl;
        }
        w->length = xs.size() ;
        w->pedestalLevel = guesses.at(0);
        w->converged = converged ;
        w->chi2 = chi2(guesses) ;
        w->ndf = xs.size() - guesses.size();
        w->nfit = whichSplines.size() ;
        
        w->times.clear() ;
        w->amplitudes.clear() ;
        for(unsigned int i = 0; i < whichSplines.size(); i++)
        {
            w->times.push_back( guesses[2 + 2*i ] );
            w->amplitudes.push_back( guesses[1 + 2*i ] );
            w->which_splines.push_back( whichSplines[i] );
            if (debug) std:: cout << "Params: t/amp " << i << " -> " << w->times.back() << " / " << w->amplitudes.back() << std::endl;
        }

        if(tsplines[0]) w->splines.push_back(TRef(tsplines[0])) ;
        if(tsplines[1]) w->splines.push_back(TRef(tsplines[1])) ;

        w->timeout = timeout;
        w->CalculatePulseTimeOrdering();
    }

    void setDebug(bool enableDebug) {
        debug = enableDebug;
    }

    void setSingleTemplate(bool only_use_single_template)
    {
        single_spline_only = only_use_single_template;
    }

    void setTimeoutLimit(double ding)
    {
        timeout_limit = ding;
    }

    void SetAmpScale(double ding)
    {
        amp_scale_factor = ding;
    }

    void SetMinMaxClippingRange(short min, short max)
    {
        max_val_without_clipping = max;
        min_val_without_clipping = min;
    }

    void addTrace(const std::vector<short>& trace, double timeOffset) {
        for (size_t i = 0; i < trace.size(); ++i) {
            if ((trace[i] > max_val_without_clipping) || (trace[i] < min_val_without_clipping)) continue;
            xs.push_back(i + timeOffset);
            ys.push_back(static_cast<double>(trace[i])); // Convert to double for calculations
            yerrs.push_back(1.0); // Assuming uniform errors
        }
        if (debug) {
            std::cout << "Trace added with size: " << trace.size() << ", timeOffset: " << timeOffset << "\n";
        }
        fittedTrace.resize(xs.size());
    }

    double model(const std::vector<double>& xs, const std::vector<double>& p, std::vector<double>& fittedTrace, bool fullModelEvaluation = true) {
        size_t nPulses = (p.size() - 1) / 2;
        std::fill(fittedTrace.begin(), fittedTrace.end(), p[0]);
        // if (debug) {
        //     std::cout << "Model parameters at call time:\n";
        //     std::cout << "xs size: " << xs.size() << "\n";
        //     std::cout << "p size: " << p.size() << "\n";
        //     for (size_t i = 0; i < p.size(); ++i) {
        //         std::cout << "p[" << i << "] = " << p[i] << "\n";
        //     }
        //     std::cout << std::endl;
        // }


        for (size_t n = 0; n < nPulses; ++n) {
            // double output = p[0];
            int whichTemplate = whichSplines[n];
            if (whichTemplate < 0 || whichTemplate > 1) {
                std::cerr << "Error: whichTemplate out of bounds: " << whichTemplate << std::endl;
                throw std::runtime_error("Invalid template selection"); // Exit gracefully or throw an exception
            }
            double amp = p[1 + n * 2];
            double t = p[2 + n * 2];
            // double xmin = splines[whichTemplate]->GetXmin();
            // double xmax = splines[whichTemplate]->GetXmax();
            // std::cout << "Fitted trace: [";
            for (size_t i = 0; i < xs.size(); ++i) {
                double xi = xs[i] - t;

                // this is a bad idea in general, but may be good for intermediate steps
                if ((!fullModelEvaluation) && ((xi > restricted_chi2_max) || (xi < restricted_chi2_min))) continue; //this saves time by only evaluating the spline very near the peak

                // if (xi > xmin && xi < xmax) 
                // {
                //     // fittedTrace[i] += amp * splines[whichTemplate]->Eval(xi);
                fittedTrace[i] += amp * (*splines[whichTemplate])(xi) ;
                // }
                // std::cout << " " << fittedTrace[i];
            }
            // std::cout << " ]" << std::endl;
        }
        return 0.0; // Dummy return for now
    }

    double abbreviated_chi2(const std::vector<double>& p) {
        model(xs, p, fittedTrace, false);
        double sum = 0.0;
        for (size_t i = 0; i < xs.size(); ++i) {
            double diff = ys[i] - fittedTrace[i];
            // sum += diff * diff/ (yerrs[i] * yerrs[i]);
            sum += diff * diff; // assumes uniform bin errors
        }
        return sum;
    }

    double chi2(const std::vector<double>& p) {
        model(xs, p, fittedTrace, true);
        double sum = 0.0;
        for (size_t i = 0; i < xs.size(); ++i) {
            double diff = ys[i] - fittedTrace[i];
            // sum += diff * diff/ (yerrs[i] * yerrs[i]);
            sum += diff * diff; // assumes uniform bin errors
        }
        return sum;
    }

    std::pair<double, std::vector<double>> minimize(const std::vector<double>& guess, bool use_full_chi2=true) {
        
        auto minimization_start = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed;

        minimizer->Clear();
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Clearing took: " << elapsed.count() << " microseconds." << std::endl;
        minimization_start = std::chrono::high_resolution_clock::now();


        ROOT::Math::Functor chi2Functor([this](const double* params) {
            std::vector<double> p(params, params + minimizer->NDim());
            return this->chi2(p);
        }, guess.size());

        ROOT::Math::Functor abbreviatedChi2Functor([this](const double* params) {
            std::vector<double> p(params, params + minimizer->NDim());
            return this->abbreviated_chi2(p);
        }, guess.size());

        if (use_full_chi2)
        {
            if (debug) std::cout << "Using full chi2 functions" << std::endl;
            minimizer->SetFunction(chi2Functor);

        }
        {
            if (debug) std::cout << "Using abbreviated chi2 function evaluated only between " << restricted_chi2_min << " and " << restricted_chi2_max << std::endl;
            minimizer->SetFunction(abbreviatedChi2Functor);
        }
        
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Functor creation took: " << elapsed.count() << " microseconds." << std::endl;
        minimization_start = std::chrono::high_resolution_clock::now();

        const double step_size = 1;
        for (size_t i = 0; i < guess.size(); ++i) {
            if (i == 0)
            {
                // set pedestal limits
                minimizer->SetVariable(i, "pedestal", guess[i], step_size);
                minimizer->SetVariableLimits(i, -2000, 2000 );
            }
            else if ((i-1) % 2 == 0)
            {
                // set amplitude limits
                minimizer->SetVariable(i, "A" + std::to_string(i), guess[i], step_size);
                minimizer->SetVariableLimits(i, minimumAmplitude, 50000 );
            }
            else if ((i-1)%2 == 1)
            {
                // set time limits
                minimizer->SetVariable(i, "t" + std::to_string(i), guess[i], step_size);
                minimizer->SetVariableLimits(i, guess[i] - timeBounds/2., guess[i] + timeBounds/2. );
            }
        }

        
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Parameter setting took: " << elapsed.count() << " microseconds." << std::endl;
        minimization_start = std::chrono::high_resolution_clock::now();

        if (debug) {
            std::cout << "Starting minimization with guess: ";
            for (const auto& g : guess) {
                std::cout << g << " ";
            }
            std::cout << "\n";
        }

        minimizer->Minimize();
        // minimizer->Simplex();
        
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Minimization took: " << elapsed.count() << " microseconds." << std::endl;
        minimization_start = std::chrono::high_resolution_clock::now();

        const double* results = minimizer->X();
        std::vector<double> values(results, results + guess.size());

        if (debug) {
            std::cout << "Minimization result: chi2 = " << minimizer->MinValue() << ", params: ";
            for (const auto& val : values) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }

        
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Results prep took: " << elapsed.count() << " microseconds." << std::endl;
        minimization_start = std::chrono::high_resolution_clock::now();

        return {minimizer->MinValue(), values};
    }

    double performMinimization() {
        double bestChi2 = std::numeric_limits<double>::max();
        double better_chi2 = std::numeric_limits<double>::max();
        double ti = -1;


        // double timeout_limit = 100000; // us
        timeout = false;
        auto minimization_start = std::chrono::high_resolution_clock::now();
        bool prematureExit = false;

        // fittedTrace.clear();

        std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "reserving space took " << elapsed.count() << " microseconds." << std::endl;

        for (size_t npulses = 0; npulses <= maxPulses; ++npulses) {
            elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
            if (debug) std::cout << "Starting evaluation of pulse " << npulses << " after " << elapsed.count() << " microseconds." << std::endl;
            if (debug) std::cout << "   -> Looking to beat: " << bestChi2 << std::endl;

            double this_chi2 = std::numeric_limits<double>::max();
            if (npulses > 0) 
            {
                whichSplines.back() = 0;
                auto [chi2_0, params_0] = minimize(guesses, false);

                if (chi2_0 + chi2Threshold > bestChi2) {
                    if (debug) {
                        std::cout << "Prospects for chi2 improvements not great after first spline (" << chi2_0 << " + " << chi2Threshold <<  " > " << bestChi2 << "); stopping minimization.\n";
                    }
                    prematureExit = true;
                    break;
                }
                else 
                {
                    if (debug)
                    {
                        std::cout << "Prospects for chi2 improvements are good! Trying second spline.\n";
                    }
                }

                elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
                if (elapsed.count() > timeout_limit)
                {
                    timeout = true;
                    prematureExit = true;
                    break;
                }

                if (!single_spline_only)
                {
                    whichSplines.back() = 1;
                    auto [chi2_1, params_1] = minimize(guesses, false);
                    better_chi2 = std::min(chi2_0, chi2_1);
                    if (chi2_0 < chi2_1) {
                        whichSplines.back() = 0;
                        if (debug)
                            std::cout << "   -> Spline chosen: " << whichSplines.back() << std::endl;
                        guesses = params_0;
                    } else {
                        whichSplines.back() = 1;
                        if (debug)
                            std::cout << "   -> Spline chosen: " << whichSplines.back() << std::endl;
                        guesses = params_1;
                    }
                    this_chi2 = better_chi2;
                }
                else
                {
                    whichSplines.back() = 0;
                    guesses = params_0;
                    this_chi2 = chi2_0;
                }


                elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
                if (elapsed.count() > timeout_limit)
                {
                    timeout = true;
                    prematureExit = true;
                    break;
                }
            } 
            else 
            {
                auto [chi2, params] = minimize(guesses,false);
                this_chi2 = chi2;
                guesses = params;
            }

            elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
            if (debug) std::cout << "After evaluation of pulse " << npulses << " -> " << elapsed.count() << " microseconds." << std::endl;


            if (debug) {
                std::cout << "Iteration " << npulses << ": chi2 = " << this_chi2 << ", guesses: ";
                for (const auto& g : guesses) {
                    std::cout << g << " ";
                }
                std::cout << "\n";
            }

            if (this_chi2 + chi2Threshold > bestChi2) {
                if (debug) {
                    std::cout << "Chi2 threshold not improved; stopping minimization.\n";
                }
                prematureExit = true;
                break;
            }

            if ( (guesses.size() > 1) && (guesses[guesses.size() -2] < minimumAmplitude + 1) ) {
                if (debug) {
                    std::cout << "Amplitude of latest pulse is at limit; stopping minimization.\n";
                }
                prematureExit = true;
                break;
            }

            if (debug) std::cout << "Setting bestChi2 to this_chi2 (" << this_chi2 << ")" << std::endl;
            bestChi2 = this_chi2;

            if (npulses == maxPulses || timeout) 
            {
                // prematureExit = true;
                break;
            }

            double maxResidual = findMaxResidual();
            double maxTime = findMaxTime();
            if (maxResidual < minimumAmplitude) 
            {
                if(debug) std::cout << "Residual amplitude (" << maxResidual << ") is below threshold (" << minimumAmplitude << ") -> exiting" << std::endl;
                break;
            }
            for (unsigned int i = 0; i < whichSplines.size(); i++)
            {
                ti = guesses[2 + i*2];
                if ((maxTime > ti - timeBounds) && (maxTime < ti + timeBounds))
                {
                    if (debug) 
                        std::cout << "Time guess overlap between " << maxTime << " and previous fit " << i << " " << ti << std::endl;
                    maxTime = (maxTime < ti) ? maxTime - timeBounds : maxTime + timeBounds;
                    if (debug) 
                        std::cout << "    -> adjusted guess: " << maxTime << "+/-" << timeBounds/2. << std::endl;
                }
            }

            
            guesses.push_back(amp_scale_factor*maxResidual);
            if(debug) std::cout << "Setting guess of amplitude to: " << guesses.back() << " based on max residual " << maxResidual << std::endl;
            
            guesses.push_back(maxTime);
            whichSplines.push_back(0);
        }

        // if the last pulse is at limit, remove it from the system
        // if ( (guesses.size() > 1) && (guesses[guesses.size() -2] < minimumAmplitude + 1) ) 
        if ( (guesses.size() > 1) && prematureExit ) 
        {
            if (debug) {
                std::cout << "Premature exit of minimuzation loop detected. Removing last guess." << std::endl;
                if (guesses[guesses.size() -2] < minimumAmplitude + 1)
                {   
                    std::cout << "   -> Amplitude of latest pulse is at limit; resizing the vectors.\n";
                }
                else if (timeout)
                {
                    std::cout << "   -> Exiting early due to timeout  -> " << elapsed.count() <<  " > " << timeout_limit << std::endl;
                }
                else 
                {
                    std::cout << "   -> Exiting early for other reason..." << std::endl;
                }
            }
            guesses.pop_back();
            guesses.pop_back();
            whichSplines.pop_back();
        }

        // perform one last iteration with final parameters and full template evaluation
        if (debug) std::cout << "Performing final minimization with full template evaluation" << std::endl;
        auto [chi2_final, params_final] = minimize(guesses, true);
        guesses = params_final; 
        bestChi2 = chi2_final;

        converged = true;
        elapsed = std::chrono::high_resolution_clock::now() - minimization_start;
        if (debug) std::cout << "Minimization ending after " << elapsed.count() << " microseconds." << std::endl;

        return bestChi2;
    }

    void reset() {
        // Reset to initial state while retaining the splines
        xs.clear();
        ys.clear();
        yerrs.clear();
        whichSplines.clear();
        guesses = {-1700.0}; // Initial guess with baseline only, pedestal is usually around this
    }

    void setRestrictedChi2Range(double min, double max)
    {
        restricted_chi2_min = min;
        restricted_chi2_max = max;
        if (debug) std::cout << "Setting resttructed chi2 range to: " << restricted_chi2_min << " -> " << restricted_chi2_max << std::endl;
    }

private:
    double findMaxResidual() {
        std::vector<double> residuals = getResidual();
        return *std::max_element(residuals.begin(), residuals.end());
    }

    double findMaxTime() {
        std::vector<double> residuals = getResidual();
        auto maxIt = std::max_element(residuals.begin(), residuals.end());
        return xs[std::distance(residuals.begin(), maxIt)];
    }

    std::vector<double> getResidual() {
        // std::vector<double> fittedTrace;
        model(xs, guesses, fittedTrace);

        std::vector<double> residuals(ys.size());
        for (size_t i = 0; i < ys.size(); ++i) {
            residuals[i] = ys[i] - fittedTrace[i];
        }
        return residuals;
    }

    TSpline3* tsplines[2];
    fitter::CubicSpline* splines[2];
    std::vector<double> xs, ys, yerrs;
    std::vector<int> whichSplines;
    std::vector<double> guesses = {0.0};
    // double chi2 = -1;
    bool converged = false;

    double minimumAmplitude;
    double timeBounds;
    size_t maxPulses;
    double chi2Threshold;
    bool debug;
    ROOT::Math::Minimizer*  minimizer;
    std::vector<double> fittedTrace;
    bool timeout;
    bool single_spline_only;

    double restricted_chi2_min;
    double restricted_chi2_max;
    double timeout_limit;
    double amp_scale_factor;
    short max_val_without_clipping;
    short min_val_without_clipping;

};
