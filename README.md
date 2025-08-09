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

## Changing selection

### Overriding default

The selection stored in the config files can be overriden with:

```python
from rx_selection import selection as sel

with sel.custom_selection(d_sel = {'bdt' : 'mva_cmb > 0.1'}):
    run_function_that_uses_selection()
```

This will make sure that the `bdt` field:

- Is added with a `mva_cmb > 0.1` cut, if it does not exist
- Is updated, if it exists

inside the context, outside, the nominal selection will be used.

This manager implements a _lock_ that prevents custom selections
from been set more than once in a nested way.

If the user **has to** use a ustom selection inside a context 
already using a custom selection, he has to use:

```python
from rx_selection import selection as sel

with sel.custom_selection(d_sel = {'bdt_cmb' : 'mva_cmb > 0.1'}):
    # mva_cmb cut added on top of default selection
    run_with_cmb_cut()
    with sel.custom_selection(d_sel = {'bdt_prc' : 'mva_prc > 0.2'}, force_override=True):
        # mva_prc cut added on top of default selection
        run_with_prc_cut()
```

### Overriding current selection

In order to override whatever is the current selection do:

```python
from rx_selection import selection as sel

with sel.custom_selection(d_sel = {'bdt' : 'mva_cmb > 0.1'}):
    # Here you run with mva_cmb only
    with sel.update_selection(d_sel = {'bdt_prc' : 'mva_prc > 0.2'}):
        # Here you run with both mva_cmb and mva_prc
        run_function_that_uses_selection()
```

This will not override the default selection, as `custom_selection` does.
It will override the **current** selection, which might be a custom selection.
