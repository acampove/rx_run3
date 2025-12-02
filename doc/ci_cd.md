# CI/CD

For this we will use:

## A testing framework
Which in python can be offered by `pytest`. All the tests can be ran interactively with

```bash
pytest src/XXX/tests
```

where `XXX` denotes a given project.

## A parallelization framework

Which will rely on `REANA` and will be in charge of:

- Running tests using multiple CPUs, ideally 30-50.
- Running the analysis code and sending the outputs to EOS (i.e. the delivery).

Due to `REANA` capabilities of:

- Accessing a computing cluster at CERN using Kubernetes or HTCondor
- Accessing EOS, where the ntuples are located.
- Accessing the analysis code in `gitlab.cern.ch`, such that the workflow runs on each push.

## A software environment

Which will be provided by docker containers. This will ensure that:

- We have available an environment with frozen versions of the depdencies (e.g. pandas, numpy, ROOT)
- We do not need to install everything all over again, only the analysis code needs to be installed.
- We do not rely on anyone to provide the software infrastructure (e.g. LCGEnv, Lbconda) and thus we are flexible
on what we need and when.

Currently, the docker container is hosted [here](https://hub.docker.com/r/acampove/rx_run3) and it can be used with:

```bash
podman pull acampove/rx_run3:v4.6

podman run -it --privileged --name rx_run3_001 -v $ANADIR:/eos/lhcb/wg/RD/RX_run3 rx_run3:v4.6
```

where `-v` in the last step will expose `ANADIR` (the path to the inputs) to the container.

# Further documentation

[On testing](doc/testing.md)
