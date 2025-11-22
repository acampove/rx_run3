[[_TOC_]]

# Description

These are YAML and TOML config files used for different reasons:

### Selection 

These files look like `lhcb_year.yaml` and contain information about correspondences
between blocks and runs or fills.

- `good_runs_2024.yaml`: Containing the ranges of good runs for 2024, obtained with:

```bash
dirac-bookkeeping-get-run-ranges --Fast --Activity=Collision24 --DQFlag=OK --RunGap=1
```

### Local 

- `link_confg.yaml`: Used by the `link_merge` utility to assign readable directory name from event type.

### Post AP

These are files used to carry out the filtering

### Tests

These are files needed by the tests

### Samples

Here is stored the list of samples for each analysis. This is needed to do checks with `dump_samples` which samples are missing in the AP before sending the filtering jobs.

