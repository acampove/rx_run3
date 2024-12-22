'''
Module with functions needed to test YamlCutReader python interface
'''

from rx_kernel import YamlCutReader

# -----------------------------
def test_default():
    '''
    Tests default contructor
    '''
    rdr = YamlCutReader()
    rdr.PrintData()
# -----------------------------
