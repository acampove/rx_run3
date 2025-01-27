# $R_X$ c++ libraries

The instructions below describe how to obtain the inputs for Run3 in order to plug them into the
c++ RX code. The usage of this code is still been settled.

## Lists of samples needed

To configure the underlying C++ code one needs to know what samples are needed for which analysis. 
This is specified in the `sample_run12.yaml` and `sample_run3.yaml` files. The latter file is made from
the former by:

- Looping over `sample_run12.yaml`.
- Checking what projects and samples were used there.
- Checking the new names for these samples and writing them to `sample_run3.yaml`

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

Which will dump the inputs in the current directory.

## AP samples

The same two ingredients as above can be made for the files before the filtering (`post_ap`) 
step, which can be run with:

```bash
ap_lfn_to_yaml
```
