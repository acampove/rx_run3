# $R_X$ orchestration

This project is meant to chain all the subtasks in the $R_X$ project.
The dependencies are listed in the `pyproject.toml` and are meant to
be taken from gitlab's master branch, where the latest valid version should be.

# Installation

Run

```bash
git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_orchestration.git
pip install rx_orchestration
```

which should install all the `RX` projects

# Usage

## Snakemake

In the main folder there is a `Snakemake` file and run:

```bash
Snakemake -c 8 all
```

## Prefect

To run the comparisons between original and brem corrected line-shape do:

```bash
cutflow_wf
```

