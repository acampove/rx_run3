from data_checks.selector import selector 
from log_store            import log_store

import ROOT

#--------------------------------------
class data:
    file_path='/home/acampove/data/aprod/fil/00231366_00000013_1.ftuple.root'
#--------------------------------------
def set_log():
    log_store.set_level('data_checks:selector'  , 10)
    log_store.set_level('rx_scripts:atr_mgr:mgr', 30)
#--------------------------------------
def test_simple():
    rdf=ROOT.RDataFrame('Hlt2RD_B0ToKpKmEE', data.file_path)
    rdf.name = 'simple_test'

    obj=selector(rdf=rdf, cfg_nam='dt_2024_turbo')
    rdf=obj.run()
#--------------------------------------
def main():
    set_log()

    test_simple()
#--------------------------------------
if __name__ == '__main__':
    main()

