# mu-reco

A C++ library to perform reconstruction on unpacked MIDAS data.

## Requirements

- [CMake]
- [ROOT]
- [mu-data-products]
- [mu-unpackers]

## Cloning the repository

You can either clone this repository on its own via:

```bash
git clone https://github.com/PIONEER-Experiment/mu-reco.git
``` 

or you can clone the app level repository. Follow the instructions here: https://github.com/PIONEER-Experiment/mu-app

## Build instructions

If you are building this repository on its own, you should first build and install the mu-data-products library and the mu-unpackers library. Details can be found here https://github.com/PIONEER-Experiment/mu-data-products.git and here https://github.com/PIONEER-Experiment/mu-unpackers.git.

You should then source the `setenv.sh` script to set up the environment, and then you can build:

```bash
source ./scripts/setenv.sh
./scripts/build.sh -o
```

## Running the application

Building the application will create an executable for running reco over the WFD5 unpacked data.

Assuming that `my_file.mid` is your midas file, you can run the reco via:
```bash
./install/bin/reco_wfd5 ./config/reco_config.json my_file.mid 0
```
where the `0` is the verbosity level. 

Running these commands will run the reconstruction configured in `reco_config.json` and will create a ROOT file that you can use for all your analysis needs!


## Instructions for adding a new reco stage
To add a new reco stage, you should follow the following steps:
1. Copy an existing RecoStage (e.g. `include/reco/wfd5/JitterCorrector.hh`) and modify it. It should derive from the `reco::RecoStage` class and must implement a couple virtual methods.
2. Add the class to the `include/reco/LinkDef.hh` file, so ROOT can generate the dictionary for it.
3. To use the reco stage, add it to the `RecoStages` array in the `reco_config.json` file. The reco stage will be executed in the order they are defined in the array. You can add parameters in the json that you parse in the `Configure` method.
4. Often times, you will want to access collections from the `EventStore`. You can do this with a get method: `eventStore.get<reco::WFD5Waveform>(recoLabel, dataLabel)` where `recoLabel` is the label of the reco stage and `dataLabel` is the label of the data product you want to access.

## Instructions for adding a new service
To add a new service, you should follow the following steps:
1. Copy an existing Service (e.g. `include/reco/wfd5/TemplateService.hh`) and modify it. It should derive from the `reco::Service` class and must implement a couple virtual methods.
2. Add the class to the `include/reco/LinkDef.hh` file, so ROOT can generate the dictionary for it.
3. To use the service, add it to the `Services` array in the `reco_config.json` file. You can add parameters in the json that you parse in the `Configure` method.
4. Services are accessible in a `RecoStage` via the `ServiceManager` class. You can get a service by its label, e.g. `serviceManager.Get<reco::TemplateService>("templates")` where the argument is the label given to the service in the json configuration.

## Making histograms
Histograms you make need to be added to the `EventStore`. The `EventStore` holds the collections of dataProducts for event event, the odb, and histograms that persistent event-to-event. Add a histogram like so:
```cpp
// Create some histograms
auto hist = std::make_shared<TH1D>("energy", "Energy Spectrum", 100, 0, 1000);
eventStore.putHistogram("energy", hist);
```
where it is assumed you made the `eventStore` object previously via ` reco::EventStore eventStore;`.

You can then retrieve (and fill) the histogram later using:
```cpp
eventStore.GetHistogram("energy")->Fill(50);
```

Finally, writing the histograms to a ROOT file is handled by the `OutputManager`:
```cpp
 outputManager->WriteHistograms(eventStore);
 ```
