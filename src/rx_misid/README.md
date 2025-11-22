[TOC]

# $R_X$ misID

This project is meant to be used to extract the templates for the fully hadronic misID templates.

## Install

From, preferrably a micromamba/mamba virtual environment:

```bash
git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_misid.git

pip install rx_misid
```

## Usage

This package does not have anymore `PassFail` functionalities. Instead it will:

- Pick a simulated `$B\to K^+ h^+ h^-$` as ROOT dataframe.
- Transform it to smaller friendlier pandas dataframe.
- Attach weights from Calibration maps and return dataframe.

### Providing pandas dataframe

This can be done with:

```python
from dmu.generic              import utilities   as gut
from rx_misid.sample_splitter import SampleSplitter

cfg   = gut.load_conf(package='rx_misid_data', fpath='splitting.yaml')
spl   = SampleSplitter(rdf = rdf, cfg = cfg)
df    = spl.get_sample()
```

### Attaching weights

This can be done with:

```python
from dmu.generic              import utilities   as gut
from rx_misid.sample_weighter import SampleWeighter

cfg = gut.load_conf(package='rx_misid_data', fpath='weights.yaml')
wgt = SampleWeighter(
    df    = df,
    cfg   = cfg,
    sample= sample, # Bu_KplKplKmn_eq_sqDalitz_DPC or Bu_piplpimnKpl_eq_sqDalitz_DPC
    is_sig= is_sig) # True for signal region weights, false for control region
df  = wgt.get_weighted_data()
```

where the input dataframe is the one from the last section.
The weights are selected such that:

- They are from the signal or control maps
- They are for pion or kaon maps
- They are picked up based on the Bremsstrahlung category

The weights go in the `weight` column

### Validating maps

In order to validate the maps one needs to:

#### Overlay the maps with the misID samples

In order to check the coverage, this is done with:

```bash
plot_samples
```

which will:

- Open the calibration maps and plot them
- Open the misID samples
- Apply the full selection but the PID
- Apply the block requirement (plots are per block)
- Apply `brem!=0` requirement
- Calculate the $1\sigma$, $2\sigma$ and $3\sigma$ contours
- Overlay them with the maps

Separate, bins, blocks, samples, etc can be specified separately,
check with `-h`.
