# Analysis

To setup the proper environment
```bash
source scripts/setup.sh
```

To build the code with `make`(`ninja`) [aliased to `cMake`(`cNinja`)]
```bash
./scripts/build.sh make (ninja)
```
For more info `cMake h`(`cNinja h`)

To access the module functionalities from an interactive ROOT session
```bash 
root -l $ANASYS/root/root_logon.C
```
or execute
```cpp
gROOT->ProcessLine(".L $ANASYS/root/root_logon.C");
```
To access the module functionalities from a ROOT macro add at the top
```cpp
R__ADD_LIBRARY_PATH($BUILDSYS) // if needed
R__LOAD_LIBRARY(libvelo)
//R__LOAD_LIBRARY(libpentaquark) if needed
R__LOAD_LIBRARY(libroofit)
R__LOAD_LIBRARY(libkernel)
R__LOAD_LIBRARY(libtuples)
R__LOAD_LIBRARY(libefficiencies)
R__LOAD_LIBRARY(libfitter)
R__LOAD_LIBRARY(libtoys)
```

The module is divided in packages containing

- `roofit`      : custom RooFit classes
- `velo`        : VeloMaterial classes
- `kernel`      : classes to access cuts, weights and tuples
- `tuples`      : classes to process tuples
- `efficiencies`: classes to compute efficiencies
- `fitter`      : classes to perform fits
- `targets`     : `cpp`(`cxx`) executables compiled against static (shared) libraries
- `data`        : cards, weight maps
- `python`      : python classes
- `yaml`        : YAML configurations
