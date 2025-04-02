# $R_X$ efficiencies

Project used to calculate efficiencies for different simulated datasets

## Geometric acceptance

The geometric acceptance should be calculated using [rapidsim](https://github.com/gcowan/RapidSim)
given that:

1. Accessing the generator tables is not trivial.
1. Using RapidSim is trivial.
1. One might need special definitions for the acceptance and having the ntuples with the
decay product values offers higher flexibility.

More on how to install and configure rapidsim [here](doc/rapidsim.md).

To create the ntuples with the decays, do:

```bash
create_rapidsim_ntuples
```

