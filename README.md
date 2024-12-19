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

## Usage

Documentation is currently being built, check 
[this](https://indico.cern.ch/event/758800/contributions/3153862/attachments/1721824/2780090/Slides_RXMeeting_25September.pdf)
for documentation on the old version of this project, used for Run1 and 2, which will share some of the code with the current version



## Usage on lxplus [Renato] out of conda-dev (also gitlab-CI based on it)

```
lb-conda-dev virtual-env default myenv
./myenv
mkdir build
cd build
cmake ../
make -j5
#see tests/ folder to get inspiration on few functionalities
```