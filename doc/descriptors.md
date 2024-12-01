## Decays from event types

Given a text file with the event types, one in each line, run:

```bash
make_fields -i decays.txt
```

to get a `decays.yaml` file with:

```yaml
Bd_Denu_Kstenu_eq_VisibleInAcceptance_HighVisMass_EGDWC:
  Bd   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  D    : '[B0  ==>  ^(  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Em   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )  ^e-  anti-nu_e  )   e+  nu_e  ]CC'
  Ep   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )  ^e+  nu_e  ]CC'
  Kp   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==> ^K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Kst  : '[B0  ==>   (  D-  ==>  ^(  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  nu   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+ ^nu_e  ]CC'
  pim  : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+ ^pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
Bd_K1gamma_Kpipi0_eq_mK1270_HighPtGamma_DPC:
  Bd   : '[  B0  ==>   (  K_1(1270)0  ==>   (  X0  ==>   K+  pi-  pi0  ))   gamma]CC'
  K1   : '[  B0  ==>  ^(  K_1(1270)0  ==>   (  X0  ==>   K+  pi-  pi0  ))   gamma]CC'
  Kp   : '[  B0  ==>   (  K_1(1270)0  ==>   (  X0  ==>  ^K+  pi-  pi0  ))   gamma]CC'
  X    : '[  B0  ==>   (  K_1(1270)0  ==>  ^(  X0  ==>   K+  pi-  pi0  ))   gamma]CC'
  gm   : '[  B0  ==>   (  K_1(1270)0  ==>   (  X0  ==>   K+  pi-  pi0  ))  ^gamma]CC'
  pi0  : '[  B0  ==>   (  K_1(1270)0  ==>   (  X0  ==>   K+  pi- ^pi0  ))   gamma]CC'
  pim  : '[  B0  ==>   (  K_1(1270)0  ==>   (  X0  ==>   K+ ^pi-  pi0  ))   gamma]CC'
Bd_KstPi0gamma_Kpi_eq_DPC:
  Bd   : '[B0  ==>   (  K*(892)0  ==>  K+  pi-  )   pi0  gamma  ]CC'
  Kp   : '[B0  ==>   (  K*(892)0  ==> ^K+  pi-  )   pi0  gamma  ]CC'
  Kst  : '[B0  ==>  ^(  K*(892)0  ==>  K+  pi-  )   pi0  gamma  ]CC'
  gm   : '[B0  ==>   (  K*(892)0  ==>  K+  pi-  )   pi0 ^gamma  ]CC'
  pi0  : '[B0  ==>   (  K*(892)0  ==>  K+  pi-  )  ^pi0  gamma  ]CC'
  pim  : '[B0  ==>   (  K*(892)0  ==>  K+ ^pi-  )   pi0  gamma  ]CC'
```

