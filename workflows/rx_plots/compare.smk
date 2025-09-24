configfile: 'configs/rx_plots/compare.yaml'

import os
os.environ['CUDA_VISIBLE_DEVICES'] = ''

ANADIR = os.environ['ANADIR']
PLTDIR = f'{ANADIR}/plots/comparison/brem_track_2'
# ---------------------------------
def _get_plots() -> list[str]:
    '''
    Returns
    ----------
    List of paths to PNG files needed
    '''
    l_plot = []
    for args in config['args']:
        sample = args['sample' ]
        trigger= args['trigger']
        q2bin  = args['q2bin'  ]
        for block in [12, 5, 6, 78]:
            if block == 78 and sample == 'Bd_JpsiKst_ee_eq_DPC':
                continue

            for brem in [1, 2]:
                plot=f'{PLTDIR}/{sample}/{trigger}/{q2bin}/{brem}_{block}/drop_mva/bmass_correction.png'
                l_plot.append(plot)

    return l_plot
# ---------------------------------
rule all:
    input:
        l_plot = _get_plots()
rule compare:
    output: 
        '{PLTDIR}/{sample}/{trigger}/{q2bin}/{brem}_{block}/drop_mva/bmass_correction.png'
    shell:
        '''
        compare -q {wildcards.q2bin} -s {wildcards.sample} -t {wildcards.trigger} -c resolution -b {wildcards.brem} -B {wildcards.block} --nomva
        '''
