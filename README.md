# $R_X$ misID

This project is meant to be used to extract the templates for the fully hadronic misID templates.

## Install

From, preferrably a micromamba/mamba virtual environment:

```bash
git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_misid.git

pip install rx_misid
```

## Usage

The code will take as inputs:

- Ntuples with real data
- MisID maps made with PIDCalib2

The configuration specifying

- The definition of the control and signal regions
- The MVA working point
- The path to the calibration maps

among others can be found in `rx_misid_data/config.yaml`.

The code will:

- Build the control regions with the weights (transfer function).
- Store them in a pandas dataframe
- Plot the data in different control regions.
- Provide a KDE PDF that will be used for fits.

## Building the Pandas dataframe with the weighted control regions

For this run:

```bash
make_misid -s leakage -q central -v v1
```

where:

```
options:
  -h, --help            show this help message and exit
  -s {data,signal,leakage}, --sample {data,signal,leakage}
                        Sample name
  -q {low,central,high}, --q2bin {low,central,high}
                        Q2 bin
  -v VERSION, --version VERSION
                        Version
```

and leakage refers to the resonant mode's. Regarding the version, this corresponds to the
configurations needed, which are stored in YAML files in `rx_misid_data/misid_v*.yaml`.
The output will go to a pandas dataframe, which will be saved in `parquet` format.

## Plotting

Run:

```bash
# This plots from a file holding the pandas dataframe
plot_misid -p /path/to/parquet/file.parquet
```

and the plots will end up in the same directory as the `parquet` file.
