# $R_X$ common

This is a thin layer of python code meant to be used as an interface to the $R_x$ run3
[github](https://github.com/acampove/rx_run3)/[gitlab](https://gitlab.cern.ch/LHCb-RD/cal-rx-run3)
tools, written in C++.

## Lists of samples needed

To configure the underlying C++ code one needs to know what samples are needed for which analysis. 
This is specified in the `sample_run12.yaml` and `sample_run3.yaml` files. The latter file is made from
the former by:

- Looping over `sample_run12.yaml`.
- Checking what projects and samples were used there.
- Checking the new names for these samples and writting them to `sample_run3.yaml`

this is done with:

```bash
make_run3_yaml
```

## Post AP samples

In order to feed these samples in the C++ code one needs:

- Text files with lists of LFNs
- YAML file with information on how the LFNs are associated to different
projects and samples.

Both of them can be made with:

```bash
pap_lfn_to_yaml -v v1 -l 10
```

which will dump the inputs in the current directory.

## AP samples

The same two ingredients as above can be made for the files before the filtering (`post_ap`) 
step, which can be ran with:

```bash
ap_lfn_to_yaml
```
