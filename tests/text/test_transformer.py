from dmu.text.transformer import transformer as txt_trf
from importlib.resources  import files

import logging

log=None
#-------------------------------------------------------------
class data:
    cfg=None
    txt=None
    out=None
#-------------------------------------------------------------
def test_simple():
    trf = txt_trf(txt_path=data.txt, cfg_path=data.cfg)
    trf.save_as(out_path=data.out)
#-------------------------------------------------------------
def initialize():
    logging.basicConfig()
    log=logging.getLogger('tests:text:transformer')
    log.setLevel(10)

    log.info('Loading inputs')

    data.txt = files('dmu_data').joinpath('text/transform.txt')
    data.cfg = files('dmu_data').joinpath('text/transform.toml')
#-------------------------------------------------------------
def main():
    initialize()

    test_simple()
#-------------------------------------------------------------
if __name__ == '__main__':
    main()


