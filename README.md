# D(ata) M(anipulation) U(tilities)

These are tools that can be used for different data analysis tasks.

# GIT

## Pushing

From the root directory of a version controlled project (i.e. a directory with the `.git` subdirectory)
using a `pyproject.toml` file, run:

```bash
publish
```

such that:

1. The `pyproject.toml` file is checked and the version of the project is extracted.
1. If a tag named as the version exists move to the steps below.
1. If it does not, make a new tag with the name as the version

Then, for each remote it pushes the tags and the commits.

*Why?*

1. Tags should be named as the project's version
1. As soon as a new version is created, that version needs to be tagged.
1. In GitHub, one can configure actions to publish projects when the commits are tagged.

# Generic

This section describes generic tools that could not be put in a specific category, but tend to be useful.

## Timer

In order to benchmark functions do:

```python
import dmu.generic.utilities as gut

# Needs to be turned on, it's off by default
gut.TIMER_ON=True
@gut.timeit
def fun():
    sleep(3)

fun()
```

## JSON dumper

The following lines will dump data (dictionaries, lists, etc) to a JSON file:

```python
import dmu.generic.utilities as gut

data = [1,2,3,4]

gut.dump_json(data, '/tmp/list.json')
```

# Physics

## Truth matching

In order to compare the truth matching efficiency and distributions after it is performed in several samples, run:

```bash
check_truth -c configuration.yaml
```

where the config file, can look like:

```yaml
# ---------
max_entries : 1000
samples:
  # Below are the samples for which the methods will be compared
  sample_a:
    file_path : /path/to/root/files/*.root
    tree_path : TreeName
    methods :
        #Below we specify the ways truth matching will be carried out
        bkg_cat : B_BKGCAT == 0 || B_BKGCAT == 10 || B_BKGCAT == 50
        true_id : TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521
    plot:
      # Below are the options used by Plottter1D (see plotting documentation below)
      definitions:
          mass : B_nopv_const_mass_M[0]
      plots:
          mass :
              binning    : [5000, 6000, 40]
              yscale     : 'linear'
              labels     : ['$M_{DTF-noPV}(B^+)$', 'Entries']
              normalized : true
      saving:
        plt_dir : /path/to/directory/with/plots
```

# Math

## PDFs

### Model building

In order to do complex fits, one often needs PDFs with many parameters, which need to be added.
In these PDFs certain parameters (e.g. $\mu$ or $\sigma$) need to be shared. This project provides
`ModelFactory`, which can do this as shown below:

```python
from dmu.stats.model_factory import ModelFactory

l_pdf = ['cbr'] + 2 * ['cbl']
l_shr = ['mu', 'sg']
mod   = ModelFactory(obs = Data.obs, l_pdf = l_pdf, l_shared=l_shr)
pdf   = mod.get_pdf()
```

where the model is a sum of three `CrystallBall` PDFs, one with a right tail and two with a left tail.
The `mu` and `sg` parameters are shared. The elementary components that can be plugged are:

```
exp: Exponential
pol1: Polynomial of degree 1
pol2: Polynomial of degree 2
cbr : CrystallBall with right tail
cbl : CrystallBall with left tail
gauss : Gaussian 
dscb : Double sided CrystallBall
```

### Printing PDFs

One can print a zfit PDF by doing:

```python
from dmu.stats.utilities   import print_pdf

print_pdf(pdf)
```

this should produce an output that will look like:

```
PDF: SumPDF
OBS: <zfit Space obs=('m',), axes=(0,), limits=(array([[-10.]]), array([[10.]])), binned=False>
Name                                                        Value            Low           HighFloating               Constraint
--------------------
fr1                                                     5.000e-01      0.000e+00      1.000e+00    1                     none
fr2                                                     5.000e-01      0.000e+00      1.000e+00    1                     none
mu1                                                     4.000e-01     -5.000e+00      5.000e+00    1                     none
mu2                                                     4.000e-01     -5.000e+00      5.000e+00    1                     none
sg1                                                     1.300e+00      0.000e+00      5.000e+00    1                     none
sg2                                                     1.300e+00      0.000e+00      5.000e+00    1                     none
```


showing basic information on the observable, the parameter ranges and values, whether they are Gaussian constrained and floating or not.
One can add other options too:

```python
from dmu.stats.utilities   import print_pdf

# Constraints, uncorrelated for now
d_const = {'mu1' : [0.0, 0.1], 'sg1' : [1.0, 0.1]}
#-----------------
# simplest printing to screen
print_pdf(pdf)

# Will not show certain parameters
print_pdf(pdf,
          blind   = ['sg.*', 'mu.*'])

# Will add constraints
print_pdf(pdf,
          d_const = d_const,
          blind   = ['sg.*', 'mu.*'])
#-----------------
# Same as above but will dump to a text file instead of screen
#-----------------
print_pdf(pdf,
          txt_path = 'tests/stats/utilities/print_pdf/pdf.txt')

print_pdf(pdf,
          blind    =['sg.*', 'mu.*'],
          txt_path = 'tests/stats/utilities/print_pdf/pdf_blind.txt')

print_pdf(pdf,
          d_const  = d_const,
          txt_path = 'tests/stats/utilities/print_pdf/pdf_const.txt')
```

## Fits

The `Fitter` class is a wrapper to zfit, use to make fitting easier.

### Goodness of fits

Once a fit has been done, one can use `GofCalculator` to get a rough estimate of the fit quality.
This is done by:

- Binning the data and PDF.
- Calculating the reduced $\chi^2$.
- Using the $\chi^2$ and the number of degrees of freedom to get the p-value.

This class is used as shown below:

```python
from dmu.stats.gof_calculator import GofCalculator

nll = _get_nll()
res = Data.minimizer.minimize(nll)

gcl = GofCalculator(nll, ndof=10)
gof = gcl.get_gof(kind='pvalue')
```

where:

- `ndof` Is the number of degrees of freedom used in the reduced $\chi^2$ calculation
It is needed to know how many bins to use to make the histogram. The recommended value is 10.
- `kind` The argument can be `pvalue` or `chi2/ndof`.

### Simplest fit

```python
from dmu.stats.fitter      import Fitter

obj = Fitter(pdf, dat)
res = obj.fit()
```

### Customizations
In order to customize the way the fitting is done one would pass a configuration dictionary to the `fit(cfg=config)`
function. This dictionary can be represented in YAML as:

```yaml
# The strategies below are exclusive, only can should be used at a time
strategy      :
      # This strategy will fit multiple times and retry the fit until either
      # ntries is exhausted or the pvalue is reached.
      retry   :
          ntries        : 4    #Number of tries
          pvalue_thresh : 0.05 #Pvalue threshold, if the fit is better than this, the loop ends
          ignore_status : true #Will pick invalid fits if this is true, otherwise only valid fits will be counted
      # This will fit smaller datasets and get the value of the shape parameters to allow
      # these shapes to float only around this value and within nsigma
      # Fit can be carried out multiple times with larger and larger samples to tighten parameters
      steps   :
          nsteps   : [1e3, 1e4] #Number of entries to use
          nsigma   : [5.0, 2.0] #Number of sigmas for the range of the parameter, for each step
          yields   : ['ny1', 'ny2'] # in the fitting model ny1 and ny2 are the names of yields parameters, all the yield need to go in this list
# The lines below will split the range of the data [0-10] into two subranges, such that the NLL is built
# only in those ranges. The ranges need to be tuples
ranges        :
      - !!python/tuple [0, 3]
      - !!python/tuple [6, 9]
#The lines below will allow using contraints for each parameter, where the first element is the mean and the second
#the width of a Gaussian constraint. No correlations are implemented, yet.
constraints   :
      mu : [5.0, 1.0]
      sg : [1.0, 0.1]
#After each fit, the parameters spciefied below will be printed, for debugging purposes
print_pars    : ['mu', 'sg']
likelihood :
    nbins : 100 #If specified, will do binned likelihood fit instead of unbinned
```

## Minimizers

These are alternative implementations of the minimizers in zfit meant to be used for special types of fits.

### Anealing minimizer

This minimizer is meant to be used for fits to models with many parameters, where multiple minima are expected in the
likelihood. The minimizer use is illustrated in:

```python
from dmu.stats.minimizers  import AnealingMinimizer

nll       = _get_nll()
minimizer = AnealingMinimizer(ntries=10, pvalue=0.05)
res       = minimizer.minimize(nll)
```

this will:

- Take the `NLL` object.
- Try fitting at most 10 times
- After each fit, calculate the goodness of fit (in this case the p-value)
- Stop when the number of tries has been exhausted or the p-value reached is higher than `0.05`
- If the fit has not succeeded because of convergence, validity or goodness of fit issues,
randomize the parameters and try again.
- If the desired goodness of fit has not been achieved, pick the best result.
- Return the `FitResult` object and set the PDF to the final fit result.

The $\chi^2/Ndof$ can also be used as in:

```python
from dmu.stats.minimizers  import AnealingMinimizer

nll       = _get_nll()
minimizer = AnealingMinimizer(ntries=10, chi2ndof=1.00)
res       = minimizer.minimize(nll)
```

## Fit plotting

The class `ZFitPlotter` can be used to plot fits done with zfit. For a complete set of examples of how to use
this class check the [tests](tests/stats/test_fit_plotter.py). A simple example of its usage is below:

```python
from dmu.stats.zfit_plotter import ZFitPlotter

obs = zfit.Space('m', limits=(0, 10))

# Create signal PDF
mu  = zfit.Parameter("mu", 5.0,  0, 10)
sg  = zfit.Parameter("sg", 0.5,  0,  5)
sig = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)
nsg = zfit.Parameter('nsg', 1000, 0, 10000)
esig= sig.create_extended(nsg, name='gauss')

# Create background PDF
lm  = zfit.Parameter('lm', -0.1, -1, 0)
bkg = zfit.pdf.Exponential(obs=obs, lam=lm)
nbk = zfit.Parameter('nbk', 1000, 0, 10000)
ebkg= bkg.create_extended(nbk, name='expo')

# Add them
pdf = zfit.pdf.SumPDF([ebkg, esig])
sam = pdf.create_sampler()

# Plot them
obj   = ZFitPlotter(data=sam, model=pdf)
d_leg = {'gauss': 'New Gauss'}
obj.plot(nbins=50, d_leg=d_leg, stacked=True, plot_range=(0, 10), ext_text='Extra text here')

# add a line to pull hist
obj.axs[1].plot([0, 10], [0, 0], linestyle='--', color='black')
```

this class supports:

- Handling title, legend, plots size.
- Adding pulls.
- Stacking and overlaying of PDFs.
- Blinding.

## Arrays

### Scaling by non-integer

Given an array representing a distribution, the following lines will increase its size
by `fscale`, where this number is a float, e.g. 3.4.

```python
from dmu.arrays.utilities import repeat_arr

arr_val = repeat_arr(arr_val = arr_inp, ftimes = fscale)
```

in such a way that the output array will be `fscale` larger than the input one, but will keep the same distribution.

## Functions

The project contains the `Function` class that can be used to:

- Store `(x,y)` coordinates.
- Evaluate the function by interpolating
- Storing the function as a JSON file
- Loading the function from the JSON file

It can be used as:

```python
import numpy
from dmu.stats.function    import Function

x    = numpy.linspace(0, 5, num=10)
y    = numpy.sin(x)

path = './function.json'

# By default the interpolation is 'cubic', this uses scipy's interp1d
# refer to that documentation for more information on this.
fun  = Function(x=x, y=y, kind='cubic')
fun.save(path = path)

fun  = Function.load(path)

xval = numpy.lispace(0, 5, num=100)
yval = fun(xval)
```

# Machine learning

## Classification

To train models to classify data between signal and background, starting from ROOT dataframes do:

```python
from dmu.ml.train_mva      import TrainMva

rdf_sig = _get_rdf(kind='sig')
rdf_bkg = _get_rdf(kind='bkg')
cfg     = _get_config()

obj= TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=cfg)
obj.run(skip_fit=False) # by default it will be false, if true, it will only make plots of features
```

where the settings for the training go in a config dictionary, which when written to YAML looks like:

```yaml
dataset:
    # Before training, new features can be defined as below
    define :
        x : v + w
        y : v - w
    # If the key is found to be NaN, replace its value with the number provided
    # This will be used in the training.
    # Otherwise the entries with NaNs will be dropped
    nan:
        x : 0
        y : 0
        z : -999
training :
    nfold    : 10
    features : [x, y, z]
    hyper    :
      loss              : log_loss
      n_estimators      : 100
      max_depth         : 3
      learning_rate     : 0.1
      min_samples_split : 2
saving:
    # The actual model names are model_001.pkl, model_002.pkl, etc, one for each fold
    path : 'tests/ml/train_mva/model.pkl'
plotting:
    roc :
        min : [0.0, 0.0] # Optional, controls where the ROC curve starts and ends
        max : [1.2, 1.2] # By default it does from 0 to 1 in both axes
        # The section below is optional and will annotate the ROC curve with
        # values for the score at different signal efficiencies
        annotate:
          sig_eff : [0.5, 0.6, 0.7, 0.8, 0.9] # Values of signal efficiency at which to show the scores
          form    : '{:.2f}' # Use two decimals for scores
          color   : 'green'  # Color for text and marker
          xoff    : -15      # Offsets in X and Y
          yoff    : -15
          size    :  10      # Size of text
    correlation: # Adds correlation matrix for training datasets
      title      : 'Correlation matrix'
      size       : [10, 10]
      mask_value : 0                # Where correlation is zero, the bin will appear white
    val_dir : 'tests/ml/train_mva'
    features:
        saving:
            plt_dir : 'tests/ml/train_mva/features'
        plots:
          w :
            binning : [-4, 4, 100]
            yscale  : 'linear'
            labels  : ['w', '']
          x :
            binning : [-4, 4, 100]
            yscale  : 'linear'
            labels  : ['x', '']
          y :
            binning : [-4, 4, 100]
            yscale  : 'linear'
            labels  : ['y', '']
          z :
            binning : [-4, 4, 100]
            yscale  : 'linear'
            labels  : ['z', '']
```

the `TrainMva` is just a wrapper to `scikit-learn` that enables cross-validation (and therefore that explains the `nfolds` setting).

### Caveats

When training on real data, several things might go wrong and the code will try to deal with them in the following ways:

- **Repeated entries**: Entire rows with features might appear multiple times. When doing cross-validation, this might mean that two identical entries
will end up in different folds. The tool checks for wether a model is evaluated for an entry that was used for training and raise an exception. Thus, repeated
entries will be removed before training.

- **NaNs**: Entries with NaNs will break the training with the scikit `GradientBoostClassifier` base class. Thus, we:
    - Can use the `nan` section shown above to replace `NaN` values with something else
    - For whatever remains we remove the entries from the training.

## Application

Given the models already trained, one can use them with:

```python
from dmu.ml.cv_predict     import CVPredict

#Build predictor with list of models and ROOT dataframe with data
cvp     = CVPredict(models=l_model, rdf=rdf)

#This will return an array of probabilibies
arr_prb = cvp.predict()
```

If the entries in the input dataframe were used for the training of some of the models, the model that was not used
will be _automatically_ picked for the prediction of a specific sample.

The picking process happens through the comparison of hashes between the samples in `rdf` and the training samples.
The hashes of the training samples are stored in the pickled model itself; which therefore is a reimplementation of
`GradientBoostClassifier`, here called `CVClassifier`.

If a sample exists, that was used in the training of _every_ model, no model can be chosen for the prediction and a
`CVSameData` exception will be risen.

During training, the configuration will be stored in the model. Therefore, variable definitions can be picked up for evaluation
from that configuration and the user does not need to define extra columns.

### Caveats

When evaluating the model with real data, problems might occur, we deal with them as follows:

- **Repeated entries**: When there are repeated features in the dataset to be evaluated we assign the same probabilities, no filtering is used.
- **NaNs**: Entries with NaNs will break the evaluation. These entries will be:
    - Replaced by other values before evaluation IF a replacement was specified during training. The training configuration will be stored in the model
    and can be accessed through:
    ```python
    model.cfg
    ```
    - For whatever features that are still NaN, they will be _patched_  with zeros when evaluated. However, the returned probabilities will be
saved as -1. I.e. entries with NaNs will have probabilities of -1.

# Pandas dataframes

## Utilities

These are thin layers of code that take pandas dataframes and carry out specific tasks

### Dataframe to latex

One can save a dataframe to latex with:

```python
import pandas as pnd
import dmu.pdataframe.utilities as put

d_data = {}
d_data['a'] = [1,2,3]
d_data['b'] = [4,5,6]
df = pnd.DataFrame(d_data)

d_format = {
        'a' : '{:.0f}',
        'b' : '{:.3f}'}

df = _get_df()
put.df_to_tex(df,
        './table.tex',
        d_format = d_format,
        caption  = 'some caption')
```

# Rdataframes

These are utility functions meant to be used with ROOT dataframes.

## Adding a column from a numpy array

### With numba

For this do:

```python
import dmu.rdataframe.utilities as ut

arr_val = numpy.array([10, 20, 30])
rdf     = ut.add_column_with_numba(rdf, arr_val, 'values', identifier='some_name')
```

where the identifier needs to be unique, every time the function is called.
This is the case, because the addition is done internally by declaring a numba function whose name
cannot be repeated as mentioned
[here](https://root-forum.cern.ch/t/ways-to-work-around-the-redefinition-of-compiled-functions-in-one-single-notebook-session/41442/1)

### With awkward

For this do:

```python
import dmu.rdataframe.utilities as ut

arr_val = numpy.array([10, 20, 30])
rdf     = ut.add_column(rdf, arr_val, 'values')
```

the `add_column` function will check for:

1. Presence of a column with the same name
2. Same size for array and existing dataframe

and return a dataframe with the added column

## Attaching attributes

**Use case** When performing operations in dataframes, like `Filter`, `Range` etc; a new instance of the dataframe
will be created. One might want to attach attributes to the dataframe, like the name of the file or the tree, etc.
Those attributes will thus be dropped. In order to deal with this one can do:

```python
from dmu.rdataframe.atr_mgr import AtrMgr
# Pick up the attributes
obj = AtrMgr(rdf)

# Do things to dataframe
rdf = rdf.Filter(x, y)
rdf = rdf.Define('a', 'b')

# Put back the attributes
rdf = obj.add_atr(rdf)
```

The attributes can also be saved to JSON with:

```python
obj = AtrMgr(rdf)
...
obj.to_json('/path/to/file.json')
```

# Logging

The `LogStore` class is an interface to the `logging` module. It is aimed at making it easier to include
a good enough logging tool. It can be used as:

```python
from dmu.logging.log_store import LogStore

LogStore.backend = 'logging' # This line is optional, the default backend is logging, but logzero is also supported
log = LogStore.add_logger('msg')
LogStore.set_level('msg', 10)

log.debug('debug')
log.info('info')
log.warning('warning')
log.error('error')
log.critical('critical')
```

# Plotting from ROOT dataframes

## 1D plots

Given a set of ROOT dataframes and a configuration dictionary, one can plot distributions with:

```python
from dmu.plotting.plotter_1d import Plotter1D as Plotter

ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
ptr.run()
```

where the config dictionary `cfg_dat` in YAML would look like:

```yaml
general:
    # This will set the figure size
    size : [20, 10]
selection:
    #Will do at most 50K random entries. Will only happen if the dataset has more than 50K entries
    max_ran_entries : 50000
    cuts:
    #Will only use entries with z > 0
      z : 'z > 0'
saving:
    #Will save lots to this directory
    plt_dir : tests/plotting/high_stat
definitions:
    #Will define extra variables
    z : 'x + y'
#Settings to make histograms for differen variables
plots:
    x :
        binning    : [0.98, 0.98, 40] # Here bounds agree => tool will calculate bounds making sure that they are the 2% and 98% quantile
        yscale     : 'linear' # Optional, if not passed, will do linear, can be log
        labels     : ['x', 'Entries'] # Labels are optional, will use varname and Entries as labels if not present
        title      : 'some title can be added for different variable plots'
        name       : 'plot_of_x' # This will ensure that one gets plot_of_x.png as a result, if missing x.png would be saved
    y :
        binning    : [-5.0, 8.0, 40]
        yscale     : 'linear'
        labels     : ['y', 'Entries']
    z :
        binning    : [-5.0, 8.0, 40]
        yscale     : 'linear'
        labels     : ['x + y', 'Entries']
        normalized : true #This should normalize to the area
# Some vertical dashed lines are drawn by default
# If you see them, you can turn them off with this
style:
  skip_lines : true
  # This can pass arguments to legend making function `plt.legend()` in matplotlib
  legend:
    # The line below would place the legend outside the figure to avoid ovelaps with the histogram
    bbox_to_anchor : [1.2, 1]
stats:
  nentries : '{:.2e}' # This will add number of entries in legend box
```

it's up to the user to build this dictionary and load it.

## 2D plots

For the 2D case it would look like:

```python
from dmu.plotting.plotter_2d import Plotter2D as Plotter

ptr=Plotter(rdf=rdf, cfg=cfg_dat)
ptr.run()
```

where one would introduce only one dataframe instead of a dictionary, given that overlaying 2D plots is not possible.
The config would look like:

```yaml
saving:
    plt_dir : tests/plotting/2d
selection:
  cuts:
    xlow : x > -1.5
general:
    size : [20, 10]
plots_2d:
    # Column x and y
    # Name of column where weights are, null for not weights
    # Name of output plot, e.g. xy_x.png
    # Book signaling to use log scale for z axis
    - [x, y, weights, 'xy_w', false]
    - [x, y,    null, 'xy_r', false]
    - [x, y,    null, 'xy_l',  true]
axes:
    x :
        binning : [-5.0, 8.0, 40]
        label   : 'x'
    y :
        binning : [-5.0, 8.0, 40]
        label   : 'y'
```

# Other plots

## Matrices

This can be done with `MatrixPlotter`, whose usage is illustrated below:

```python
import numpy
import matplotlib.pyplot as plt

from dmu.plotting.matrix import MatrixPlotter

cfg = {
        'labels'     : ['x', 'y', 'z'], # Used to label the matrix axes
        'title'      : 'Some title',    # Optional, title of plot
        'label_angle': 45,              # Labels will be rotated by 45 degrees
        'upper'      : True,            # Useful in case this is a symmetric matrix
        'zrange'     : [0, 10],         # Controls the z axis range
        'size'       : [7, 7],          # Plot size
        'format'     : '{:.3f}',        # Optional, if used will add numerical values to the contents, otherwise a color bar is used
        'fontsize'   : 12,              # Font size associated to `format`
        'mask_value' : 0,               # These values will appear white in the plot
        }

mat = [
        [1, 2, 3],
        [2, 0, 4],
        [3, 4, numpy.nan]
        ]

mat = numpy.array(mat)

obj = MatrixPlotter(mat=mat, cfg=cfg)
obj.plot()
plt.show()
```

# Manipulating ROOT files

## Getting trees from file

The lines below will return a dictionary with trees from the handle to a ROOT file:

```python
import dmu.rfile.utilities   as rfut

ifile  = TFile("/path/to/root/file.root")

d_tree = rfut.get_trees_from_file(ifile)
```

## Printing contents

The following lines will create a `file.txt` with the contents of `file.root`, the text file will be in the same location as the
ROOT file.

```python
from dmu.rfile.rfprinter import RFPrinter

obj = RFPrinter(path='/path/to/file.root')
obj.save()
```

## Printing from the command line

This is mostly needed from the command line and can be done with:

```bash
print_trees -p /path/to/file.root
```

which would produce a `/pat/to/file.txt` file with the contents, which would look like:

```
Directory/Treename
    B_CHI2                        Double_t
    B_CHI2DOF                     Double_t
    B_DIRA_OWNPV                  Float_t
    B_ENDVERTEX_CHI2              Double_t
    B_ENDVERTEX_CHI2DOF           Double_t
```

## Comparing ROOT files

Given two ROOT files the command below:

```bash
compare_root_files -f file_1.root file_2.root
```

will check if:

1. The files have the same trees. If not it will print which files are in the first file but not in the second
and vice versa.
1. The trees have the same branches. The same checks as above will be carried out here.
1. The branches of the corresponding trees have the same values.

the output will also go to a `summary.yaml` file that will look like:

```yaml
'Branches that differ for tree: Hlt2RD_BToMuE/DecayTree':
  - L2_BREMHYPOENERGY
  - L2_ECALPIDMU
  - L1_IS_NOT_H
'Branches that differ for tree: Hlt2RD_LbToLMuMu_LL/DecayTree':
  - P_CaloNeutralHcal2EcalEnergyRatio
  - P_BREMENERGY
  - Pi_IS_NOT_H
  - P_BREMPIDE
Trees only in file_1.root: []
Trees only in file_2.root:
  - Hlt2RD_BuToKpEE_MVA_misid/DecayTree
  - Hlt2RD_BsToPhiMuMu_MVA/DecayTree
```

# File system

## Versions

The utilities below allow the user to deal with versioned files and directories

```python
from dmu.generic.version_management import get_last_version
from dmu.generic.version_management import get_next_version
from dmu.generic.version_management import get_latest_file

# get_next_version will take a version and provide the next one, e.g.
get_next_version('v1')           # -> 'v2'
get_next_version('v1.1')         # -> 'v2.1'
get_next_version('v10.1')        # -> 'v11.1'

get_next_version('/a/b/c/v1')    # -> '/a/b/c/v2'
get_next_version('/a/b/c/v1.1')  # -> '/a/b/c/v2.1'
get_next_version('/a/b/c/v10.1') # -> '/a/b/c/v11.1'

# `get_latest_file` will return the path to the file with the highest version
# in the `dir_path` directory that matches a wildcard, e.g.:

last_file = get_latest_file(dir_path = file_dir, wc='name_*.txt')

# `get_last_version` will return the string with the latest version
# of directories in `dir_path`, e.g.:

oversion=get_last_version(dir_path=dir_path, version_only=True)  # This will return only the version, e.g. v3.2
oversion=get_last_version(dir_path=dir_path, version_only=False) # This will return full path, e.g. /a/b/c/v3.2
```

The function above should work for numeric (e.g. `v1.2`) and non-numeric (e.g. `va`, `vb`) versions.

# Text manipulation

## Transformations

Run:

```bash
transform_text -i ./transform.txt -c ./transform.toml
```
to apply a transformation to `transform.txt` following the transformations in `transform.toml`.

The tool can be imported from another file like:

```python
from dmu.text.transformer import transformer as txt_trf

trf=txt_trf(txt_path=data.txt, cfg_path=data.cfg)
trf.save_as(out_path=data.out)
```

Currently the supported transformations are:

### append

Which will apppend to a given line a set of lines, the config lines could look like:

```toml
[settings]
as_substring=true
format      ='--> {} <--'

[append]
'primes are'=['2', '3', '5']
'days are'=['Monday', 'Tuesday', 'Wednesday']
```

`as_substring` is a flag that will allow matches if the line in the text file only contains the key in the config
e.g.:

```
the
first
primes are:
and
the first
days are:
```

`format` will format the lines to be inserted, e.g.:

```
the
first
primes are:
--> 2 <--
--> 3 <--
--> 5 <--
and
the first
days are:
--> Monday <--
--> Tuesday <--
--> Wednesday <--
```

## coned

Utility used to edit SSH connection list, has the following behavior:

```bash
#Prints all connections
coned -p

#Adds a task name to a given server
coned -a server_name server_index task

#Removes a task name from a given server
coned -d server_name server_index task
```

the list of servers with tasks and machines is specified in a YAML file that can look like:

```yaml
ihep:
    '001' :
        - checks
        - extractor
        - dsmanager
        - classifier
    '002' :
        - checks
        - hqm2
        - dotfiles
        - data_checks
    '003' :
        - setup
        - ntupling
        - preselection
    '004' :
        - scripts
        - tools
        - dmu
        - ap
lxplus:
    '984' :
        - ap
```

and should be placed in `$HOME/.config/dmu/ssh/servers.yaml`
