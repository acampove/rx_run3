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

Will be updated soon

## Plotting

Run:

```bash
# This plots from a file holding the pandas dataframe
plot_misid -p /path/to/parquet/file.parquet
```

