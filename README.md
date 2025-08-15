# mu-reco

A C++ library that provides a framework for performing reconstruction on unpacked MIDAS data. This library is used for the LYSO 2025 test beam.

## Requirements

- [CMake]
- [ROOT]
- [ZLIB]
- [nlohmann_json]
- [mu-data-products]
- [mu-unpackers]

## If you are here to learn about the reconstruction framework...
...go to Section [Reconstruction Framework](#reconstruction-framework).

## Cloning the repository

You can either clone this repository on its own via:

```bash
git clone git@github.com:PIONEER-Experiment/mu-reco.git
``` 

or you can clone the app level repository. For cloning the app level repository, follow the instructions here: git@github.com:PIONEER-Experiment/mu-app. It is recommended to clone the app level repository, as this repository does not produce a standalone executable, but rather is used as a library for the app level repository.

## Build instructions

If you are building this repository on its own (not recommended), you should first build and install the mu-data-products library and the mu-unpackers library. Details can be found here git@github.com:PIONEER-Experiment/mu-data-products.git and here git@github.com:PIONEER-Experiment/mu-unpackers.git.

You should then source the `setenv.sh` script to set up the environment, and then you can build:

```bash
source ./scripts/setenv.sh
./scripts/build.sh -o
```
Building the library will produce a shared library `libmu_reco.dylib` (or `libmu_reco.so`) in the `build/lib` directory.

## Reconstruction Framework
The reconstruction framework is designed to provide some organization and modularity around running alogorithms on data. It allows you to define a series of reconstruction stages that can process collections of data products from the unpacker, which itself operates on a raw midas file.

The framework has two key features: `RecoStages` and `Services`. A `RecoStage` is a class that runs a physics algorithm on a collection of data products (or more than one collection) and produces a new collection of data products. An example of a `RecoStage` is the JitterCorrector, which corrects the waveform's even/odd pedestal jitters. A `Service` is a class that provides commonly needed utilities to a `RecoStage` and is shared and accessible by all stages. An example of a service is the ChannelMapService, which provides a mapping from physical detector slots (crate number, AMC slot number, WFD5 channel number) to the physical detector with a detector system and sub detector name.

Each of these elements (the `RecoStages` and `Services`) have their own manager: there is the `RecoManager` and the `ServiceManager`. The `RecoManager` is responsible for configuring all the user-defined `RecoStages` and executing them in the order they are defined, while the `ServiceManager` is responsible for configuring and providing access to the user-defined `Services`.

Other important elements of the reconstruction framework include the following:
- `ConfigHolder`: This class holds the configuration for the reconstruction framework. It is loaded from a JSON file (e.g. `reco_config.json`) and provides each part of the program with access to the configuration parameters.
- `EventStore`: The event store carries around all the data products for the current event, as well as things like the run and subrun number, the odb, and histograms. The collections of data products are stored as `TClonesArray` objects. Note that the `TClonesArrays` are reused event-to-event. Importantly, the `EventStore`'s `clear` method calls `Clear("C")` on each `TClonesArray`, which clears the contents of the array to get you ready for the next event.
- `OutputManager`: This class holds the output ROOT file, the output tree, histograms, and anything else that is written to the file. One importantly thing is does is write the `EventStore` to the tree after each event. This is done with `void FillEvent(const EventStore& eventStore);` The first time this is called, the output manager will create the necessary branches in the tree and have them point to the `TClonesArray` objects in the `EventStore`. In this way, the data always lives in the `EventStore`, and the `OutputManager` just writes it to the tree. 

## JSON Configuration File
The nearline is configured via a JSON file. It configures the unpacker, which reco stages to run, which services to use, and what data products to write to the file. The default configuration file is `mu-reco/config/reco_config.json`. You can modify this file to change the configuration of the nearline. An example configuration file is shown below:
```json
{
  
  "Unpacker" : {
    "max_midas_events": 10,
    "verbosity": 0
  },
  "RecoStages": [
    {
      "recoClass": "reco::WaveformInitializer",
      "recoLabel": "initializer",
      "inputRecoLabel": "unpacker",
      "inputWaveformsLabel": "WFD5WaveformCollection",
      "outputWaveformsLabel": "waveforms",
      "failOnError":false,
      "debug":false
    },
    {
      "recoClass": "reco::JitterCorrector",
      "recoLabel": "jitter",
      "inputRecoLabel": "initializer",
      "inputWaveformsLabel": "waveforms",
      "outputWaveformsLabel": "waveforms",
      "templateServiceLabel": "templates",
      "pedestal_files":"pedestal_files.json",
      "failOnError":false,
      "debug":false
    },
    {
      "recoClass": "reco::EmptyChannelPruner",
      "recoLabel": "pruned",
      "inputRecoLabel": "jitter",
      "inputWaveformsLabel": "waveforms",
      "outputWaveformsLabel": "waveforms",
      "minAmplitude":25.0,
      "file_name":"minimum_amplitudes.json",
      "keepChannels":[
        [8,1,2],
        [8,1,4]
      ],
      "failOnError":false,
      "debug":false
    },
    {
      "recoClass": "reco::PedestalCalculator",
      "recoLabel": "pedestal",
      "inputRecoLabel": "pruned",
      "inputWaveformsLabel": "waveforms",
      "outputWaveformsLabel": "waveforms",
      "pedestalMethod" : "FirstN",
      "numSamples": 20,
      "debug": true
    }
  ],
  "_RecoPath":[],
  "RecoPath":[
    "initializer",
    "jitter",
    "pruned",
    "pedestal"
  ],
  "RecoManager": {
    "timeProfilerLabel": "timeProfiler"
  },
  "ServiceManager": {
  },
  "Services": [
    {
      "type": "reco::ChannelMapService",
      "label": "channelMap",
      "channel_map_files":"channel_map_files.json"
    },
    {
      "type": "reco::TimeProfilerService",
      "label": "timeProfiler"
    }
  ],
  "Output": {
    "drop": [
      "unpacker*",
      "#initializer*",
      "pruned*",
      "jitter*",
      "#pedestal*"
    ],
    "compressionLevel": 1,
    "compressionAlgorithm": 4
  }
}
```
- The `Unpacker` block configures the unpacker. Set `max_midas_events` to `-1` to run over all midas event.
- The `RecoStages` array defines the reconstruction stages you have access to (doesn't guarantee they are run; see `RecoPath`). Each `RecoStage` block in the array must have the `recoClass` and `recoLabel` fields. The `recoClass` is the name of the class that implements the reco stage (see all possible `RecoStages` in `mu-reco/src/common` or `mu-reco/src/wfd5`; it must derive from the `reco::RecoStage` class). The `recoLabel` is a user-defined label (whatever you want) that is used to identify the reco stage. This label is used as the prefix to all data products produced by the reco stage. You can have any other json-parsable parameters. 
- The `RecoPath` array defines the reco stages to run and the order in which they are run. You can edit this path to decided what actually gets run.
- The `RecoManager` block configures the reco manager.
- The `ServiceManager` block configures the service manager.
- The `Services` array defines the services you have access to.
- The `Output` block configures the output ROOT file. You can set which data products to drop from the output file. Provide a list of data product names. You can use the `*` wildcard to drop select multiple data products, e.g. `unpacker*` will drop all data products that start with `unpacker`.

# Configurations based on interval-of-validity (IOV)
Some configuration settings depend on an interval of validity (IOV), defined as a range of run numbers. The idea here is that the experimental conditions may change over time. To accomodate these changes, the nearline can be configured to use different configuration files based on an IOV and the run number of the file being processed.

We'll take a look at the structure through an example. We'll pick the even/odd pedestals. In `mu-reco/config/pedestals_iov.json`, we have
```json
{
    "pedestals_iov": [
        {"file":"pedestals.json", "iov":[0,30000]}
    ]
}
```
`"pedestals_iov"` is a json array that should contain the different running configurations. Each configuration is a json object with two fields: `"file"` and `"iov"`. The `"file"` field is the name of the configuration file, and the `"iov"` field is an array with two elements, the start and end run numbers of the IOV. In this case, the configuration file is `pedestals.json`, which looks like
```json
{
    "pedestals": [
      {
         "crateNum": 8,
         "amcSlotNum": 1,
         "channelNum": 0,
         "pedestal": 4.0
      },
...
}
```
Then, in the code, we can access the proper IOV like so:
```cpp
void JitterCorrector::Configure(const nlohmann::json& config, const ServiceManager& serviceManager, EventStore& eventStore) {

    inputRecoLabel_ = config.value("inputRecoLabel", "unpacker");
    inputWaveformsLabel_ = config.value("inputWaveformsLabel", "WFD5WaveformCollection");
    outputWaveformsLabel_ = config.value("outputWaveformsLabel", "WaveformsCorreted");
    templateLoaderServiceLabel_ = config.value("templateLoaderServiceLabel", "templateLoader");
    failOnError_ = config.value("failOnError", false);
    debug_ = config.value("debug",false);

    // Set up the parser
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Get the run number from the configuration
    int run = configHolder_->GetRun();
    int subrun = configHolder_->GetSubrun();

    // Get the pedestal configuration from the IOV list using the run and subrun
    auto pedestalConfig =  jsonParserUtil.GetConfigFromIOVList(config, run, subrun, "pedestals_iov", debug_);
    if (pedestalConfig.empty()) {
        throw std::runtime_error("JitterCorrector configuration file not found for run: " + std::to_string(run) + ", subrun: " + std::to_string(subrun));
    }   

    for (const auto& configi : pedestalConfig["pedestals"]) 
    {
        offsetMap_[std::make_tuple(configi["crateNum"], configi["amcSlotNum"], configi["channelNum"])] = configi["pedestal"];
        if (debug_) std::cout << "Loading configuration for odd/even difference in channel ("
            << configi["crateNum"]      << " / "
            << configi["amcSlotNum"]        << " / "
            << configi["channelNum"]    << ") -> " 
            << configi["pedestal"]
            << std::endl;
    }
}
```

## Instructions for adding a new reco stage
To add a new reco stage, you should follow the following steps:
1. Copy an existing RecoStage and modify it: there is a template you can copy here: `include/reco/common/TemplateStage.hh`; and copy the source file `src/common/TemplateStage.cc`. Your stage must derive from the `reco::RecoStage` class and must implement a couple virtual methods.
2. Add the class to the `include/reco/LinkDef.hh` file, so ROOT can generate the dictionary for it.
3. To define the reco stage in your job, add it to the `RecoStages` array in the `reco_config.json` file. You can add any parameters in the json that you then must parse in the `Configure` method of the RecoStage. If you want to actually run the reco stage, add it to the `RecoPath` array. The reco stages will be executed in the order they are defined in the `RecoPath` array. The `RecoPath` is used to decouple stage definition from execution.
4. Often times, you will want to access collections from the `EventStore`. You can do this with a get method: `TClonesArray* waveforms = store.get<const dataProducts::WFD5Waveform>(recoLabel, dataLabel);` where `recoLabel` is the label of the reco stage and `dataLabel` is the label of the data product you want to access. This will return a `TClonesArray*` object that you can loop over. A possible workflow might look like this:

```cpp
void YourRecoStage::Process(EventStore& store, const ServiceManager& serviceManager) {
    try {
         // Get the input waveforms
        auto waveforms = store.get<const dataProducts::WFD5Waveform>(inputRecoLabel, inputWaveformsLabel);

        //Make a collection new waveforms
        auto newWaveforms = store.getOrCreate<dataProducts::WFD5Waveform>(outputRecoLabel, outputWaveformsLabel);

        for (int i = 0; i < waveforms->GetEntriesFast(); ++i) {
            auto* waveform = static_cast<dataProducts::WFD5Waveform*>(waveforms->ConstructedAt(i));
            if (!waveform) {
                throw std::runtime_error("Failed to retrieve waveform at index " + std::to_string(i));
            }
            //Make the new waveform
            dataProducts::WFD5Waveform* newWaveform = new ((*newWaveforms)[i]) dataProducts::WFD5Waveform(*waveform);
            newWaveforms->Expand(i + 1);

            // Do something with the waveform here!

        }
    } catch (const std::exception& e) {
       throw std::runtime_error(std::string("YourRecoStage error: ") + e.what());
    }
}
```

## Instructions for adding a new service
To add a new service, you should follow the following steps:
1. Copy an existing Service (e.g. `include/reco/wfd5/TemplateService.hh`) and modify it. It should derive from the `reco::Service` class and must implement a couple virtual methods.
2. Add the class to the `include/reco/LinkDef.hh` file, so ROOT can generate the dictionary for it.
3. To use the service, add it to the `Services` array in the `reco_config.json` file. You can add parameters in the json that you parse in the `Configure` method.
4. Services are accessible in a `RecoStage` via the `ServiceManager` class. You can get a service by its label, e.g. `reco::Service templateFitter = serviceManager.Get<TemplateFitterService>(templateFitterLabel);` where the argument is the label given to the service in the json configuration.

## Making histograms
Histograms you make need to be added to the `EventStore`. The `EventStore` holds the collections of dataProducts for each event, the odb, and histograms that persist event-to-event. To add a histogram, you can do something like this in your `RecoStage`'s `Configure` method:
```cpp
// Create a histogram
 eventStore.putHistogram("h_pedestals", std::make_shared<TH1D>("h_pedestals", "Pedestals", 2000, -2000, 0));
```

You can then retrieve (and fill) the histogram later (say in the `Process` method) using:
```cpp
eventStore.GetHistogram("h_pedestals")->Fill(50);
```
Notice that the name of the histograms is used as the key in the `EventStore`.

Finally, writing the histograms to a ROOT file is handled by the `OutputManager` and is done in the main application for you:
```cpp
 outputManager->WriteHistograms(eventStore);
 ```
