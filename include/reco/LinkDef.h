#ifdef __ROOTCLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Common reco/service classes
#pragma link C++ class reco::RecoStage+;
#pragma link C++ class reco::Service+;
#pragma link C++ class reco::TimeProfilerService+;

// WFD5 RecoStages
#pragma link C++ class reco::T0Processor+;
#pragma link C++ class reco::TimeSeeder+;
#pragma link C++ class reco::EmptyChannelPruner+;
#pragma link C++ class reco::EnergyCalibration+;
#pragma link C++ class reco::JitterCorrector+;
#pragma link C++ class reco::DigitizerTimeAligner+;
#pragma link C++ class reco::PedestalCalculator+;
#pragma link C++ class reco::DetectorGrouper+;
#pragma link C++ class reco::Fitter+;
#pragma link C++ class reco::RFFitter+;
#pragma link C++ class reco::PulseIntegrator+;
#pragma link C++ class reco::PeakIdentifier+;
#pragma link C++ class reco::PileupIdentifier+;
#pragma link C++ class reco::CaloClusterFinder+;
#pragma link C++ class reco::XYPositionFinder+;
#pragma link C++ class reco::EndOfEventAnalysis+;

// WFD5 services
#pragma link C++ class reco::TemplateLoaderService+;
#pragma link C++ class reco::TemplateFitterService+;
#pragma link C++ class reco::ChannelMapService+;

#endif
