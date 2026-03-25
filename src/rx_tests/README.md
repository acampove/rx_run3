# RX Tests

This package is meant to:

- Automate sending test jobs to IHEP's condor cluster 
- Collect pytest reports and display them

Because this project contains hundreds of tests that cannot be ran in a single
machine.

## Send jobs

For this do:

```bash
rxtests test-all
```

This will send the jobs as specified in `rx_tests_data/config.yaml`
To run over only one project, use the `-p` argument.

## Visualize reports

For this run

```bash
rxtests show-report -p /path/to/directory/with/*.xml
```

which will show the reports in firefox, by default.
