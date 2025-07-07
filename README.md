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

The code will calculate the misID through Pass-Fail and control region fits 
The configuration is located in `rx_misid_data/misid.yaml`.

The code structure can be seen below:

<div style="text-align: center;">
  <img src="doc/images/rx_misid.svg" alt="Rx MisID Diagram" style="width: 50%;">
</div>

### Pass-Fail approach

- Build the control regions with the weights (transfer function).
- Store them in a pandas dataframe
- Plot the data in different control regions.
- Provide a KDE PDF that will be used for fits.

### Fitting approach

- Pick up the data and provide it in control region
- Pick up noPID simulation and apply PID maps to put it in control region.
- Fit data and extract normalization factors
- Use PID maps for signal region to build misID model.

## Classes

### MisIDSplitter

This class is meant to split data into `Pass-Fail`, `Fail-Pass` and
`Fail-Fail` regions with the output filtered for:

- The hadron to be either a kaon or pion.
- The `B` meson to be a positive or negative

```python
from rx_misid.sample_splitter import SampleSplitter

spl   = SampleSplitter(
    rdf      = rdf,
    sample   = sample,
    hadron_id= 'kaon', # or pion
    is_bplus = True,   # or False
    cfg      = cfg)    # loaded misid.yaml
df    = spl.get_samples()
```

### SampleWeighter

This class is meant to apply PID weights to data or simulation.

- **Data** : Add transfer weights, meant to provide move data
from control region to signal region.

- **Simulation**: Add PID efficiencies, meant to move noPID samples
to control or signal region.

It's used as:

```python
from rx_misid.sample_weighter import SampleWeighter

wgt = SampleWeighter(
    df    = df,     # From SampleSplitter
    cfg   = cfg,    # loaded misid.yaml
    sample= sample, # e.g. DATA_24_MagUp_24c2
    is_sig= True)   # or False for the control region
df  = wgt.get_weighted_data()
```

### MisIDCalculator

This class accesses the ntuples and:

- Plugs them in the `SampleSplitter`.
- With the outputs, puts them _together_, given that they are 
for a given `B` charge and hadron species.
- Plugs them into the `SampleWeighter` to get them in the signal
or control regions.
- And provides a dataframe with the data in the region requested.

It is used as below:

```python
from rx_misid.misid_calculator import MisIDCalculator

cfg = load('path/to/misid.yaml')
cfg['input']['sample' ] = 'Bu_piplpimnKpl_eq_sqDalitz_DPC' # or other sample
cfg['input']['q2bin'  ] = 'central'                        # or low or high
cfg['input']['project'] = 'nopid'                          # or rx for data
cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_noPID'      # or Hlt2RD_BuToKpEE_MVA_ext for data

obj = MisIDCalculator(cfg=cfg, is_sig=True) # or false for control region
df  = obj.get_misid()
```

For data, the trigger and samples are different, because we cannot remove the
PID from the data.

### PDFMaker

This class will create a PDF from an MC sample that:

- Belongs to the noPID lines
- Belongs to the control or signal region, through PID weights 
  applied in the form of PIDCalib efficiencies.

```python
import zfit
from rx_misid.pdf_maker     import PDFMaker

obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
mkr = PDFMaker(
    sample ='Bu_piplpimnKpl_eq_sqDalitz_DPC', # This should be MC always 
    q2bin  ='central',                        # or low or high 
    trigger='Hlt2RD_BuToKpEE_MVA_noPID')      # Always the noPID triggers
pdf = mkr.get_pdf(obs=obs, is_sig=False)
```

this PDF should be usable to fit the control region and extract
a data-driven estimage of the misID.

**TODO:**
- Can a parametric approach be used instead of KDE?
- KDE is not reliable, due to underfitting or overfitting.

