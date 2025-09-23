import os
ANADIR = os.environ['ANADIR']
PLTDIR = f'{ANADIR}/plots'

configfile: 'configs/rx_plots/compare.yaml'

rule compare:
    output: 
        f'{PLTDIR}/comparison_brem_track_2/brem_track_2/Bd_JpsiKst_ee_eq_DPC/Hlt2RD_B0ToKpPimEE_MVA/jpsi/2_2/with_mva'
    shell:
        'compare -q jpsi -s 'DATA_24*' -t Hlt2RD_B0ToKpPimEE_MVA -c resolution -b 2 -B 2'
