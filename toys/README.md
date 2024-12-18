# Toys

Toy studys are run in consicutive steps
1. **fitGenerator.out --yaml yaml/fitter/config-FitHolder.yaml** : produces a ToyConfiguration.root file to creat the ToyTuples
2. **toyGenerator.out --yaml config-FitHolder.yaml** : creates ToyTuples that store the toy data
3. **toyStudy.out --yaml config-FitHolder.yaml** : fits the toy data stored in the  ToyTuples 

Don't be alarmed that r-ratio gives some incredibly bad results since efficiencies are not calculated properly yet

## Step 1: Fit
This is the normal fitting executable of the framework which fits for B->KstJPs in the JPs decaying to di-electron and di-muon final states simultaneously to measure `rJPs`. This will produce a `FitManager` object saved to disk which will be used for the next step. 

- Generating `ToyTupleConfig` saved in `.root` files  
**testToyTupleConfigGenerator.out --yaml $ANASYS/yaml/toy/config-ConfigGenerator-JPs.yaml**  
Loads the `FitManager_q2jps_after.root` object stored in the `build` directory and produce a `ToyTupleConfig` for each component in the fit. These are used to configure the generator and reader properly. The important information of each component saved are the PDFs, observable variable (needed by RooFit for loading and generation) and the nominal yields.

## Step 2: Generate ToyTuples
These generators take YAMLs as option to configure the `ToyYieldConfig`s to generate them. These data objects basically tells the generator how many events of each component to generate. You can drive them via `ScalingFactor` which scales the Poisson mean of the generator to `nominalYield * ScalingFactor`. The PDFs and nominal yield of the components are retrieved from the `config.root` files produced in the previous step.

## Step 3: Study ToyTuples
This part requires a config file to configure the fitter produced via `FitGenerator` and a config file to configure how toys are read. The fitter config file is written just like those used in Step 1 while the toy reader configuration YAML are also written in the same format as those used in Step 3. **Note**: Generate the same number of toy nTuples as the number of toy fit loops you want to perform. 

Small disclaimer: Right now the executables are hard-coded to generate 10 nTuples and run 10 toy fit loops for testing purposes. This will change in a future commit.

# Toy Classes
- Data Classes
    - ToyTupleConfig
    - ToyTupleComponentHeader
    - ToyYieldConfig
- General Classes
    - ToyFileHandler
    - ToyTupleHeaderHandler
    - ToyParser
- ToyGenerator
    - ToyTupleGenerator
    - ToyTupleComponentGenerator
    - ToyTupleConfigGenerator
    - ToyTupleConfigSaver
- Readers
    - ToyReader
    - ToyTupleReader
    - ToyTupleComponentReader
    - ToyTupleConfigLoader
- Fitters
    - ToyStudy
    - ToyFitResultLogger
    - VariableResetter
    - VariableSmearer

## ToyTupleConfig
One ToyTupleConfig configures one component. It contains 4 important data members:

1. Sample key for bookkeeping
2. PDF (via pointers) for the component's shape
3. Nominal yield
4. Observable key (so RooFit knows which RooAbsReal is your axis)

They are saved to files by ToyTupleConfigGenerator which adapts ToyTupleConfigSaver and ToyTupleConfig to fitter objects saved to disk. ToyTupleConfigLoader is responsible for loading them.

## ToyYieldConfig
Configures the generation/reading of a single toy component. Contains as data members:

1. Sample key for bookkeeping
2. Scaling Factor
3. Mean Events

## ToyTupleComponentHeader
Contains information of the generated toy component TTree.

1. Sample key for bookkeeping.
2. Mean events requested.
3. Number of generated events.
4. Poisson CDF of generated events given the mean events requested.

## Generating Toy nTuples.
At the top-level is `ToyGenerator` which can generate many toy nTuple by via YAML configuration. When you pass a YAML configuration, `ToyGenerator` will construct many `ToyTupleGenerator`. One `ToyTupleGenerator` is responsible for one toy `nTuple`.

`ToyTupleGenerator` is configured by `ConfigHolder` and a vector of `ToyYieldConfig` -- these are read from YAML files. It will look for pdf shapes automatically from `ToyTupleConfig` saved as ROOT files. Then, it matches the requested shape and their yields by matching the `Sample` keys in `ToyTupleConfig` read from ROOT files and the vector of `ToyYieldConfig` passed. Each pair of `ToyTupleConfig` and `ToyYieldConfig` is passed to `ToyTupleComponentGenerator` which is generates of a single component's `TTree` within the toy nTuple.

After generating the toy nTuples, each `ToyTupleComponentGenerator` is queried for a `ToyTupleComponentHeader` which contains information of the generated events. This is saved as a TTree by a `ToyTupleHeaderHandler` owned by `ToyTupleGenerator`. 

So, one toy nTuple will have one `header` TTree containing the `ToyTupleComponentHeader`s and as many TTree as there are components.

## Reading Toy nTuples.
At the top-level is `ToyReader` which can read many toy nTuple by via YAML configuration. When you pass a YAML configuration, `ToyReader` will construct many `ToyTupleReader`. One `ToyTupleReader` is responsible for reading one toy `nTuple`.

`ToyTupleReader` is configured by `ConfigHolder` and a vector of `ToyYieldConfig`. It will query for `Nominal Yield`s automatically from `ToyTupleConfig` saved as ROOT files in case the yield is passed as a `Scaling Factor` in `ToyYieldConfig`. After evaluating the `Mean Events` either directly or via `Scaling Factor` it passes this information to `ToyTupleComponentReader` with a matching `ToyTupleComponentHeader` . `ToyTupleComponentReader` is responsible for reading a single component's `TTree` within the nTuple.

When reading, if the requested mean number of events is the same as that used in the generator step the whole TTree is used. However, if the requested mean number of events is less than that in the generator, an inverse Poisson CDF function is used to find the number of events to read given a reduced mean value. This is the main purpose of `ToyTupleComponentHeader`.

## Toy Fit
The fitter is configured like usual via `FitGenerator` with a caveat that the initial state is saved by `VariableResetter` and constraints are smeared by `VariableSmearer`. An instance of `ToyStudy` will have an instance of `ToyReader`. Internally, `ToyStudy` will check that the keys used to bookkeep the `RooDataSet` and `RooDataHist` used by `FitterTool` matches those used by `ToyReader`. This matching is crucial to ensure toy nTuple data are read and fed into the appropriate containers.

On every fit loop, the variables are reset to their initial state and constraints are smeared. Then, all the toy nTuple data are fed from `ToyReader` to `FitterTool` via matching keys. `FitterTool` will then run a single iteration its fitting procedure. `ToyFitResultLogger` will then log the convergence of variables, covariance and correlation matrices and fit status code.