# Analysis

## Build and Install

In order to build this project you need the dependencies in `environment.yaml`. To setup the environment do:

```bash
conda env create -f environment.yaml
```
this should work with mamba and micromamba too. Then build the libraries and executables with:

```bash
make -p build
cmake -B build
make -C build
```


