'''
Module with Measurement class
'''
from pydantic import BaseModel, ConfigDict

# ----------------------------------------------
class Measurement(BaseModel):
    '''
    Class meant to symbolize a set of measurements, useful to represent:

    - Parameters of fit
    '''
    data         : dict[str, tuple[float, float]]
    model_config = ConfigDict(frozen=True)
    # ----------------------
    def __getitem__(self, variable : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        variable: Name of variable associated to measurement

        Returns
        -------------
        Tuple with value and error
        '''
        if variable not in self.data:
            raise KeyError(f'Invalid variable: {variable}')

        return self.data[variable]
    # ----------------------
    def __contains__(self, variable : str) -> bool:
        '''
        Parameters
        ----------------
        variable: Name of variable sought

        Returns
        ----------------
        True if variable was found
        '''

        return variable in self.data
# ----------------------------------------------
