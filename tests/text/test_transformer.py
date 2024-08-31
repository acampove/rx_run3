from dmu.text.transformer import transformer as txt_trf
from importlib.resources  import files

import logging

log=None
#-------------------------------------------------------------
class data:
    cfg=None
    txt=None
#-------------------------------------------------------------
def test_with_path_ext():
    data.out = '/tmp/dmu_test/with_path_ext.txt' 

    trf=txt_trf(txt_path=data.txt, cfg_path=data.cfg)
    trf.save_as(out_path=data.out)
#-------------------------------------------------------------
def test_with_path():
    data.out = '/tmp/dmu_test/with_path' 

    trf=txt_trf(txt_path=data.txt, cfg_path=data.cfg)
    trf.save_as(out_path=data.out)
#-------------------------------------------------------------
def test_settings():
    data.out = '/tmp/dmu_test/settings' 
    cfg      = files('dmu_data').joinpath('text/transform_set.toml')
    txt      = files('dmu_data').joinpath('text/transform_set.txt')

    trf=txt_trf(txt_path=txt, cfg_path=cfg)
    trf.save_as(out_path=data.out)
#-------------------------------------------------------------
def initialize():
    logging.basicConfig()
    log=logging.getLogger('tests:text:transformer')
    log.setLevel(10)

    log.info('Loading inputs')

    data.txt = files('dmu_data').joinpath('text/transform.txt')
    data.cfg = files('dmu_data').joinpath('text/transform.toml')

    log_trf=logging.getLogger('dmu:text:transformer')
    log_trf.setLevel(10)
#-------------------------------------------------------------
def main():
    initialize()

    test_settings()
    test_with_path()
    test_with_path_ext()
#-------------------------------------------------------------
if __name__ == '__main__':
    main()


