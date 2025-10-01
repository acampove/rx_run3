[TOC]

This project is meant to be used to make classifiers for the RX analyses

# Training

For that run:

```bash
train_classifier -v v6 -c cmb -q low
train_classifier -v v6 -c cmb -q central
train_classifier -v v6 -c cmb -q high

train_classifier -v v6 -c prc -q low
train_classifier -v v6 -c prc -q central
train_classifier -v v6 -c prc -q high
```

Which will train the classifiers for different $q^2$ bins and with different
settings. The settings are part of `YAML` files and are stored in:

```bash
rx_classifier/src/rx_classifier_data/v6
```

The underlying tool doing all the training can be found in 
[dmu](https://github.com/acampove/dmu?tab=readme-ov-file#machine-learning)

other options that this utility takes are:

```
options:
  -h, --help            show this help message and exit
  -v VERSION, --version VERSION
                        Version of config files
  -c CFG_NAME, --cfg_name CFG_NAME
                        Kind of config file
  -q {low,central,jpsi,psi2S,high}, --q2bin {low,central,jpsi,psi2S,high}
                        q2bin
  -l {10,20,30}, --log_level {10,20,30}
                        Logging level
  -m MAX_ENTRIES, --max_entries MAX_ENTRIES
                        Limit datasets entries to this value
  -p, --plot_only       If used, will only do plots of feature distributions, not
                        training
  -L, --load_trained    Nothing changes, but instead of training models, will load
                        trained models, which should exist
```

# Output

The output of this training are, for each fold:

- Pickle file with the classifier
- ROC curve
- Importance table
- Classifier score
- Hyper-parameters table
- Plots of the features
- Covariance matrix

# Performance

In order to compare the performances use:

```bash
compare_classifier -c cmb_high -n 10000
```

where:

- `-c` specifies the name of the config file in `rx_classifier_data/performance/{name}.yaml`
- `-n` specifies the number of entries to use, these are chosen randomly.

The config file will specify:

- How to make the plots, ranges, colors, etc.
- What models to test.
- What samples to pick for signal and proxy backgrounds, including the selections.

**Note:** By selection, we mean what we add **on top** of the default analysis selection.
# TODO

Some things that can be tried are:

- The hyper-parameters need to be optimized
- One could try MLPs or other type of classifiers.

# Getting MVA scores for ntuples 

For this run:

```bash
apply_classifier -c application.yaml
```

where the YAML file has configurations that look like:

```yaml
# This should be the directory with the fully selected (except for the MVA) data and or MC
input_dir : /publicfs/ucas/user/campoverde/Data/RX_run3/v4/NO_q2_bdt_mass_Q2_central_VR_v1
# This section specifies the locations of the MVA pickle files for each type of MVA
mva :
  cmb :
    low     : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/cmb/low
    central : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/cmb/central
    high    : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/cmb/high
  prc :
    low     : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/prc/low
    central : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/prc/central
    high    : /publicfs/ucas/user/campoverde/Data/RK/MVA/run3/v5/RK/prc/high
```
the script above will run over each input file, ending in `_sample.root` and will produce a file
with the mva scores `mva_cmb` and `mva_prc` alongside the `EVENTNUMBER` and `RUNNUMBER` branches.
These files can be used to create friend trees with the latter two branches used for indexing as
specified [here](https://root.cern/manual/trees/#widening-a-ttree-through-friends).

In case multiple samples are present in the directory one can target a specific sample only with:

```bash
apply_classifier -c application.yaml -s DATA_24_MagUp_24c1
```

Dry runs can be used to make sure the right files will be processed with `-d`.

If 10 files will be processed, a specific file can be targeted with:

```bash
apply_classifier -c application.yaml -s DATA_24_MagUp_24c1 -i 2
```

`-i` can also be used to parametrize jobs in a cluster, like HTCondor.

# Using cluster jobs to apply scores

Given that there are thousands of files, in around 80 samples, this should be run in a computing cluster.
The line:

```bash
build_job_script -p /publicfs/ucas/user/campoverde/Data/RX_run3/v5/rx_samples.yaml -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -s DATA
```

Where the data samples were skipped with `-s`, this is optional. The command will create `jobs.txt` with contents like:

```
apply_classifier -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -t Hlt2RD_BuToKpEE_MVA       -s Bd_Denu_Kstenu_eq_VIA_HVM_EGDWC
apply_classifier -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -t Hlt2RD_BuToKpEE_MVA       -s Bd_Dmnpipl_eq_DPC
apply_classifier -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -t Hlt2RD_BuToKpMuMu_MVA     -s Bd_Dmunu_Kstmunu_eq_DPC
apply_classifier -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -t Hlt2RD_BuToKpEE_MVA       -s Bd_Dstplenu_eq_PHSP_TC
apply_classifier -c /publicfs/ucas/user/campoverde/Packages/RK/rx_classifier/application.yaml -t Hlt2RD_BuToKpEE_MVA       -s Bd_JpsiKS_ee_eq_CPV_DPC
...
```
which can be used to apply the selection in a computing cluster, assuming that:

- The job runs in a specific virtual environment where `apply_classifier` is available.
- Each job runs one line

# MVA Optimization

For this one needs:

- Expected signal yields for different working points
- Background yields from fits to actual data
- A utility that puts them together to find the optimal significance

### Signal yields

To get the grid of signal yields do:

```python
from dmu.generic                     import utilities as gut
from rx_classifier.signal_calculator import SignalCalculator

q2bin = 'central'

cfg = gut.load_data(package='rx_classifier_data', fpath='optimization/scanning.yaml')
cal = SignalCalculator(cfg=cfg, q2bin=q2bin)
df  = cal.get_signal()
```

which will provide a dataframe with the expected signal yield for each working point.
I.e. with columns `mva_cmb`, `mva_prc` and `sig`.

The settings are stored in the `scanning.yaml` config file.

