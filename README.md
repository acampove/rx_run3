[TOC]

# $R_X$ selection

This project is meant to apply an offline selection to ntuples produced by
[post_ap](https://github.com/acampove/post_ap/tree/main/src/post_ap_scripts)
and downloaded with
[rx_data](https://github.com/acampove/rx_data).
the selection is done with jobs sent to an HTCondor cluster.

## How to pick up selection and apply it to data and MC

For this do:

```python
from rx_selection import selection as sel

# trigger : HLT2 trigger, e.g. Hlt2RD_BuToKpEE_MVA 
# q2bin   : low, central, jpsi, psi2, high
# smeared : If true (default), the selection will use cuts on smeared masses. Only makes sense for electron MC samples
# process : 
#     One of the keys in https://gitlab.cern.ch/rx_run3/rx_data/-/blob/master/src/rx_data_lfns/rx/v7/rk_samples.yaml
#     DATA will do all the data combined

d_sel = sel.selection(trigger='Hlt2RD_BuToKpEE_MVA', q2bin='jpsi', process='DATA', smeared=True)

# You can override the selection here
for cut_name, cut_value in d_sel.items():
    rdf = rdf.Filter(cut_value, cut_name)

rep = rdf.Report()
# Here you cross check that the cuts were applied and see the statistics
rep.Print()
```

## Overriding selection

The selection stored in the config files can be overriden with:

```python
from rx_selection import selection as sel

with sel.custom_selection(d_sel = {'bdt' : 'mva_cmb > 0.1'}):
    run_function_that_uses_selection()
```

This will make sure that the `bdt` field:

- Is added with a `mva_cmb > 0.1` cut, if it does not exist
- Is updated, if it exists

For `run_function_that_uses_selection` and outside that `with`
section, the nominal the selection picked is the nominal.

## Resetting overriding selection

In order to do tests of parts of the code with different selections, one would have to
override the selection multiple times. This is not allowed, unless the selection is reset with:

```python
from rx_selection import selection as sel

sel.reset_custom_selection()
```
