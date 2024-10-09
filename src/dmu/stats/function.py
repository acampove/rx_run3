'''
Module containing the Function class
'''
import os
import json

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:stats:function')
#---------------------------------------------------------
class Function:
    '''
    Class meant to represent a 1D function created from (x, y) coordinates
    '''
    #------------------------------------------------
    def __init__(self, x : list, y : list):
        '''
        x (list) : List with x coordinates
        y (list) : List with y coordinates
        '''

        if len(x) != len(y):
            raise ValueError('X and Y coordinates have different lengths')

        self._l_x = x
        self._l_y = y
    #------------------------------------------------
    def __eq__(self, other):
        if not isinstance(other, Function):
            log.warning('Comparison not done with instance of Function')
            return False

        return self.__dict__ == other.__dict__
    #------------------------------------------------
    def _json_encoder(self, obj):
        '''
        Takes Function object
        Returns dictionary of attributes for encoding
        '''
        return obj.__dict__
    #------------------------------------------------
    def save(self, path : str):
        '''
        Saves current object to JSON

        path (str): Path to file, needs to end in .json
        '''

        if not path.endswith('.json'):
            raise ValueError(f'Output path does not end in .json: {path}')

        with open(path, 'w', encoding='utf-8') as ofile:
            json.dump(self, ofile, indent=4, default=self._json_encoder)

        log.info(f'Saved to: {path}')
    #------------------------------------------------
    @staticmethod
    def json_decoder(d_attr):
        '''
        Takes dictionary of attributes from JSON serialization
        Returns instance of Function
        '''

        if '_l_x' not in d_attr:
            raise KeyError('X values not found')

        if '_l_y' not in d_attr:
            raise KeyError('Y values not found')

        x = d_attr['_l_x']
        y = d_attr['_l_y']

        return Function(x=x, y=y)
    #------------------------------------------------
    @staticmethod
    def load(path : str):
        '''
        Will take path to JSON file with serialized function
        Will return function instance
        '''

        if not os.path.isfile(path):
            raise FileNotFoundError(f'Cannot find: {path}')

        with open(path, encoding='utf-8') as ifile:
            fun = json.loads(ifile.read(), object_hook=Function.json_decoder)

        log.info(f'Loaded from: {path}')

        return fun
    #------------------------------------------------
    def __str__(self):
        npoints = len(self._l_x)
        max_x   = max(self._l_x)
        min_x   = min(self._l_x)

        max_y   = max(self._l_y)
        min_y   = min(self._l_y)

        line = f'\n{"Points":<20}{npoints:<20}\n'
        line+= '-------------------------\n'
        line+= f'{"x-max":<20}{max_x:<20}\n'
        line+= f'{"x-min":<20}{min_x:<20}\n'
        line+= f'{"y-max":<20}{max_y:<20}\n'
        line+= f'{"y-min":<20}{min_y:<20}'

        return line
    #------------------------------------------------
