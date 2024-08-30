from data_checks.selector import selector 
from log_store            import log_store

import ROOT

#--------------------------------------
class data:
    dt_path = '/home/acampove/data/aprod/downloads/flt_27_08_2024_dt_2024_turbo/00231366_00000001_1.ftuple.root'
    mc_path = '/home/acampove/data/aprod/downloads/flt_29_08_2024_mc_2024_turbo_comp/mc_bu_jpsik_ee_12153001_nu4p3_magdown_turbo_hlt1_2_tupling_00231483_00000002_1.tuple.root'
#--------------------------------------
def set_log():
    log_store.set_level('data_checks:selector'  , 10)
    log_store.set_level('rx_scripts:atr_mgr:mgr', 30)
#--------------------------------------
def test_mc():
    rdf= ROOT.RDataFrame('Hlt2RD_BuToKpEE', data.mc_path)

    obj=selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=True)
    rdf=obj.run()

    rep=rdf.Report()
    rep.Print()
#--------------------------------------
def test_dt():
    rdf= ROOT.RDataFrame('Hlt2RD_BuToKpEE', data.dt_path)

    obj=selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=False)
    rdf=obj.run()

    rep=rdf.Report()
    rep.Print()
#--------------------------------------
def test_mc():
    rdf= ROOT.RDataFrame('Hlt2RD_BuToKpEE', data.mc_path)

    obj=selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=True)
    rdf=obj.run()

    rep=rdf.Report()
    rep.Print()

#--------------------------------------
def main():
    set_log()

    test_dt()
    test_mc()
#--------------------------------------
if __name__ == '__main__':
    main()

