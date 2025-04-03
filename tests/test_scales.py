from scales    import scales          as scl
from scales    import get_proc_labels 
from log_store import log_store

import os
import mplhep
import pandas            as pnd
import utils_noroot      as utnr
import matplotlib.pyplot as plt


log=log_store.add_logger('rk_extractor:test_scales')
#-------------------------------
def plot_df(df, trig):
    df = df[df.trig == trig]
    ax = None
    d_proc_lab = get_proc_labels()
    for proc, df_p in df.groupby('kind'):
        ax=df_p.plot(x='year', y='val', yerr='err', ax=ax, label=d_proc_lab[proc], linestyle='none', marker='o')

    os.makedirs('tests/scales/all_datasets', exist_ok=True)

    plt_path = f'tests/scales/all_datasets/{trig}.png'
    log.info(f'Saving to: {plt_path}')
    plt.grid()
    plt.ylabel('Scale')
    plt.ylim(0.0, 0.5)
    plt.xlabel('')
    plt.tight_layout()
    plt.savefig(plt_path)
    plt.close('all')
#-------------------------------
def test_all_datasets():
    df = pnd.DataFrame(columns=['year', 'trig', 'kind', 'val', 'err'])
    for year in ['2011', '2012', '2015', '2016', '2017', '2018']:
        for trig in ['ETOS']:
            for kind in ['bpks', 'bdks', 'bsph']:
                obj      = scl(dset=year, trig=trig, kind=kind)
                val, err = obj.get_scale()

                df = utnr.add_row_to_df(df, [year, trig, kind, val, err])

    plot_df(df, 'ETOS')
#-------------------------------
def test_simple():
    obj      = scl(dset='2011', trig='ETOS', kind='bpks')
    val, err = obj.get_scale()
#-------------------------------
def test_dset(sample):
    l_scl = []
    l_err = []
    l_dst = []
    for dset in ['2011', '2012', '2015', '2016', '2017', '2018']:
        obj      = scl(dset=dset, trig='ETOS', kind=sample)
        val, err = obj.get_scale()

        l_scl.append(val)
        l_err.append(err)
        l_dst.append(dset)

    obj      = scl(dset='all', trig='ETOS', kind=sample)
    val, err = obj.get_scale()

    os.makedirs('tests/scales/dset', exist_ok=True)

    plt.errorbar(l_dst, l_scl, yerr=l_err)
    plt.axhline(y=val - err, linestyle=':', color='r')
    plt.axhline(y=val + err, linestyle=':', color='r')
    plt.savefig(f'tests/scales/dset/{sample}.png')
    plt.close('all')
#-------------------------------
def main():
    plt.style.use(mplhep.style.LHCb2)

    for sample in ['bpks', 'bdks', 'bsph']:
        test_dset(sample)

    test_simple()
    test_all_datasets()
#-------------------------------
if __name__ == '__main__':
    main()

