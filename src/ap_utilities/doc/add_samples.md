# Add the decay matching lines 

This is done in `new_production/tupling/config/mcfuntuple.yaml`. Each section in this file looks like:

```yaml
# This is a nickname for the sample
Bd_Denu_Kstenu_eq_VisibleInAcceptance_HighVisMass_EGDWC:
# This is the decay descriptor and how to match particles to branch names
  Bd   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  D    : '[B0  ==>  ^(  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Em   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )  ^e-  anti-nu_e  )   e+  nu_e  ]CC'
  Ep   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )  ^e+  nu_e  ]CC'
  Kp   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==> ^K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Kst  : '[B0  ==>   (  D-  ==>  ^(  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  nu   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+ ^nu_e  ]CC'
  pim  : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+ ^pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
```

- To get the sample nickname follow [this](doc/nicknames.md)
- To get the descriptors one can either write them down in the case of a few samples or run follow [these](doc/descriptors.md) instructions in case of multiple.

# Updating `tupling/config/samples_turbo_lines_mapping.yaml`

This file lists the samples together with the _analyses_ like in:

```yaml
Bc_Dsst2573mumu_KKpi_eq_BcVegPy_DPC: # This is a sample
- Bc_lines                           # This is an analysis
Bc_Jpsipi_mm_eq_WeightedBcVegPy_DPC:
- Bc_lines
Bc_pimumu_eq_PHSP_BcVegPy_DPC:
- Bc_lines
```

Where the _analyses_ are sets of HLT2 lines described in `tupling/config/analyses.yaml`.
