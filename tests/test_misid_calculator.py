'''
Script meant to test MisIDCalculator class
'''

from rx_misid.misid_calculator import MisIDCalculator

# ---------------------------------
def _get_config() -> dict:
    return {}
# ---------------------------------
def test_simple():
    '''
    Simplest example of misid calculator usage
    '''
    cfg = _get_config()
    obj =MisIDCalculator(cfg=cfg)
    data=obj.get_dataset()
# ---------------------------------
