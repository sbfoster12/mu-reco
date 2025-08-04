#ifdef __ROOTCLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Common reco classes
#pragma link C++ class reco::RecoStage+;
#pragma link C++ class reco::Service+;

// WFD5 Reco classes
#pragma link C++ class reco::JitterCorrector+;
#pragma link C++ class reco::DetectorGrouper+;
#pragma link C++ class reco::TemplateService+;
#pragma link C++ class reco::ChannelMapService+;

// // Common unpackers classes
// #pragma link C++ class unpackers::Parser+;
// #pragma link C++ class unpackers::CollectionsHolder+;
// #pragma link C++ class unpackers::EventUnpacker+;
// #pragma link C++ class unpackers::BankUnpacker+;
// #pragma link C++ class unpackers::PayloadUnpacker+;

// // WFD5 unpackers classes
// #pragma link C++ class unpackers::WFD5WaveformParser+;
// #pragma link C++ class unpackers::WFD5WaveformHeaderParser+;
// #pragma link C++ class unpackers::WFD5WaveformHeaderAsyncParser+;
// #pragma link C++ class unpackers::WFD5ChannelHeaderParser+;
// #pragma link C++ class unpackers::WFD5ChannelHeaderAsyncParser+;
// #pragma link C++ class unpackers::WFD5HeaderParser+;
// #pragma link C++ class unpackers::WFD5CRBankUnpacker+;
// #pragma link C++ class unpackers::WFD5PayloadUnpacker+;
// #pragma link C++ class unpackers::WFD5EventUnpacker+;

// // FC7 unpackers classes
// #pragma link C++ class unpackers::FC7HeaderParser+;
// #pragma link C++ class unpackers::FC7PayloadUnpacker+;

// // Nalu unpackers classes
// #pragma link C++ class unpackers::NaluADBankUnpacker+;
// #pragma link C++ class unpackers::NaluATBankUnpacker+;
// #pragma link C++ class unpackers::NaluADPayloadUnpacker+;
// #pragma link C++ class unpackers::NaluATPayloadUnpacker+;
// #pragma link C++ class unpackers::NaluEventUnpacker+;

#endif
