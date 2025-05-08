[[_TOC_]]

# q2 systematics

This project is used to assess the systematics associated to q2 smearing. 

# Installation

Make a virtual environment like:

```bash
mamba create -n q2_systematics root=6.28 python=3.9
```

# Performing the fits

In order to get the fits to the $J/\psi(\to ee)$ mass do:

```bash
get_q2_tables -v v1 -t Hlt2RD_BuToKpEE_MVA -y 2024 -x nom -b 0 -B 0 -k sim
get_q2_tables -v v1 -t Hlt2RD_BuToKpEE_MVA -y 2024 -x nom -b 0 -B 0 -k dat
```

this will fit for a given:

1. Trigger
1. Year 
1. Statistical fluctuation
1. Brem category
1. Block of data
1. MC or actual data

The fit needs to be done for MC and THEN data, in order to fix the tails

# Extracting the smeared $q^2$ values

This is done with:

```python
from rx_q2.q2smear_calculator import Q2SmearCalculator

obj = Q2SmearCalculator(rdf=rdf)
rdf = obj.get_rdf()
```

This will create a dataframe with the smeared mass and a `EVENTNUMBER` and `RUNNUMBER`
which are needed to use the dataframe as a friend tree.
