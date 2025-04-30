'''
File with functions to test MCScaler
'''

from rx_misid.mc_scaler import MCScaler

# -----------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    q2bin = 'central'
    sample= 'Bu_Kee_eq_btosllball05_DPC'

    scl = MCScaler(q2bin=q2bin, sample=sample)
    val = scl.get_scale()

    print(val)
# -----------------------------------------------

