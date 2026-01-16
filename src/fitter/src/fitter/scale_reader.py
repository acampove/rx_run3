'''
Module holding ScaleReader class
'''

# -------------------------------------
class ScaleReader:
    '''
    Class in charge of accessing mass scales, resolutions, brem fractions, etc
    and making them available
    '''
    # ----------------------
    def get_scale(
        self,
        name : str,
        block : str,
        brem  : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        name: Kind of scale, e.g. scale, reso, brem
        block: E.g. 1...8
        brem : E.g. 1, 2

        Returns
        -------------
        Tuple with value and error of scale
        '''
        return 1, 0
# -------------------------------------
