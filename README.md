# D(ata) M(anipulation) U(tilities)

These are tools that can be used for different data analysis tasks.

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
saving:
    plt_dir : tests/plotting/simple

plots:
    x : 
        binning : [-5.0, 8.0, 40]
        yscale  : 'linear' 
        labels  : ['x', 'Entries']
    y : 
        binning : [-5.0, 8.0, 40]
        yscale  : 'linear' 
        labels  : ['y', 'Entries']
```

it's up to the user to build this dictionary and load it.

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

