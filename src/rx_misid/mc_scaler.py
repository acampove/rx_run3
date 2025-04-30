'''
Module containing MCScaler class
'''

# ----------------------------------
class MCScaler:
    '''
    Class meant to provide scale factor to extrapolate leakage and signal yields
    to misID control regions
    '''
    # ----------------------------------
    def __init__(self, q2bin : str, sample : str):
        '''
        q2bin : q2 bin
        sample: Name of MC sample, e.g. Bu_12345_Kee_btosll...
        '''
        self._q2bin = q2bin
        self._sample= sample
    # ----------------------------------
    def get_scale(self) -> float:
        '''
        Returns scale factor:
        Signal yield x MC control / MC signal
        '''

        return 1
# ----------------------------------
