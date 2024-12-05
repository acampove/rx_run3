## Notes

- The downloads can be ran many times, if a file has been downloaded already, it will not be downloaded again.

# Linking and merging

Once the ntuples are downloaded these need to be linked and merged with:

```bash
link_merge -j flt_002 -v v1
```

where `-j` is the name of the job and the files are linked to a directory named as `-v v1`. For tests run:

```bash
link_merge -j flt_002 -d 1 -m 10 -v v1
```

which will do the same with at most `10` files, can use debug messages with `-l 10`.

# Making basic plots

For this run:

```bash
plot_vars -y 2024 -v v2 -c bukee_opt -d data_ana_cut_bp_ee:Data ctrl_BuToKpEE_ana_ee:Simulation
```

which will run the plotting of the variables in a config specified by `bukee_opt` where also axis, names, ranges, etc are
specified. This config is in `post_ap_data`.
The script above will overlay data and MC.

