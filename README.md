# D(ata) M(anipulation) U(tilities)

These are tools that can be used for different data analysis tasks.

# Math

## Fits

The `Fitter` class is a wrapper to zfit, use to make fitting easier.

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
          pvalue        : 0.05 #Pvalue threshold, if the fit is better than this, the loop ends
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
```

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
obj.run()
```

where the settings for the training go in a config dictionary, which when written to YAML looks like:

```yaml
training :
    nfold    : 10
    features : [w, x, y, z]
    hyper    :
      loss              : log_loss
      n_estimators      : 100
      max_depth         : 3
      learning_rate     : 0.1
      min_samples_split : 2
saving:
    path : 'tests/ml/train_mva/model.pkl'
plotting:
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

If a sample exist, that was used in the training of _every_ model, no model can be chosen for the prediction and an
`CVSameData` exception will be risen.

# Rdataframes

These are utility functions meant to be used with ROOT dataframes.

## Adding a column from a numpy array

For this do:

```python
import dmu.rdataframe.utilities as ut

arr_val = numpy.array([10, 20, 30])
rdf     = ut.add_column(rdf, arr_val, 'values')
```

thed `add_column` function will check for:

1. Presence of a column with the same name
2. Same size for array and existing dataframe

and return a dataframe with the added column

# Dataframes

Polars is very fast, however the interface of polars is not simple. Therefore this project has a derived class
called `DataFrame`, which implements a more user-friendly interface. It can be used as:

```python
from dmu.dataframe.dataframe import DataFrame

df = DataFrame({
'a': [1, 2, 3],
'b': [4, 5, 6]
})


# Defining a new column
df = df.define('c', 'a + b')

```

The remaining functionality is identical to `polars`.

# Logging

The `LogStore` class is an interface to the `logging` module. It is aimed at making it easier to include
a good enough logging tool. It can be used as:

```python
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('msg')
LogStore.set_level('msg', 10)

log.debug('debug')
log.info('info')
log.warning('warning')
log.error('error')
log.critical('critical')
```

# Plotting from ROOT dataframes

Given a set of ROOT dataframes and a configuration dictionary, one can plot distributions with:

```python
from dmu.plotting.plotter import Plotter

ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
ptr.run()
```

where the config dictionary `cfg_dat` in YAML would look like:

```yaml
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
        yscale     : 'linear'
        labels     : ['x', 'Entries']
        title      : 'some title can be added for different variable plots'
    y :
        binning    : [-5.0, 8.0, 40]
        yscale     : 'linear'
        labels     : ['y', 'Entries']
    z :
        binning    : [-5.0, 8.0, 40]
        yscale     : 'linear'
        labels     : ['x + y', 'Entries']
        normalized : true #This should normalize to the area
```

it's up to the user to build this dictionary and load it.

# Manipulating ROOT files

## Printing contents

The following lines will create a `file.txt` with the contents of `file.root`, the text file will be in the same location as the
ROOT file.

```python
from dmu.rfile.rfprinter import RFPrinter

obj = RFPrinter(path='/path/to/file.root')
obj.save()
```

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

# Future tools

## coned

Utility used to edit SSH connection list, it should have the following behavior:

```bash
#Prints all connections
coned -p

#Adds a task name to a given server
coned -a server_name server_index task

#Removes a task name from a given server
coned -d server_name server_index task
```

