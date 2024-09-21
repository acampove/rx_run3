# D(ata) M(anipulation) U(tilities)

These are tools that can be used for different data analysis tasks.

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

