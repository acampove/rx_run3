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
from rx_q2.q2smear_corrector import Q2SmearCorrector

obj          = Q2SmearCorrector()
smeared_mass = obj.get_mass(nbrem=nbrem, block=block, jpsi_mass_reco=original_mass)
```

This will smear the mass for a given block and brem category. The object should be reused
for all the candidates and declared only once.

# Visualizing scales and resolutions

The script collecting all the numbers should be run with:

```bash
dump_q2_ratios -v v2 -p rk_ee
```

for instance.
