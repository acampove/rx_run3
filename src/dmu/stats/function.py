'''
Module containing the Function class
'''
import os
import json

from typing import Any

import numpy
import scipy

from scipy.interpolate     import interp1d
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:stats:function')
#---------------------------------------------------------
class Function:
    '''
    Class meant to represent a 1D function created from (x, y) coordinates
    '''
    #------------------------------------------------
    def __init__(self, x : list | numpy.ndarray, y : list | numpy.ndarray, kind : str = 'cubic'):
        '''
        x (list) : List with x coordinates
        y (list) : List with y coordinates
        '''

        x = self._array_to_list(x)
        y = self._array_to_list(y)

        if len(x) != len(y):
            raise ValueError('X and Y coordinates have different lengths')

        self._l_x = x
        self._l_y = y
        self._kind= kind

        self._interpolator : scipy.interpolate._interpolate.interp1d = None
    #------------------------------------------------
    def __eq__(self, othr):
        if not isinstance(othr, Function):
            log.warning('Comparison not done with instance of Function')
            return False

        d_self = self.__dict__
        d_othr = othr.__dict__

        if '_interpolator' in d_self:
            del d_self['_interpolator']

        if '_interpolator' in d_othr:
            del d_othr['_interpolator']

        return d_self == d_othr
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
    def __call__(self, xval : float | numpy.ndarray | list) -> numpy.ndarray :
        '''
        Class taking value of x coordinates as a float, numpy array or list
        It will interpolate y value and return value
        '''
        self._make_interpolator()
        self._check_xval_validity(xval)

        return self._interpolator(xval)
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

        x    = d_attr['_l_x' ]
        y    = d_attr['_l_y' ]
        kind = d_attr['_kind']

        return Function(x=x, y=y, kind=kind)
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
    def _array_to_list(self, x : Any):
        '''
        Transform from ndarray to list
        Return x if already list
        Raise otherwise
        '''
        if isinstance(x, list):
            log.debug('Already found list')
            return x

        if isinstance(x, numpy.ndarray):
            log.debug('Transforming argument to list')
            return x.tolist()

        raise ValueError('Object introduced is neither a list nor a numpy array')
    #------------------------------------------------
    def _make_interpolator(self):
        '''
        If not found, will make interpolator and put it in self._interpolator
        '''
        if self._interpolator is not None:
            return

        log.debug('Making interpolator')

        self._interpolator = interp1d(self._l_x, self._l_y, kind=self._kind)
    #------------------------------------------------
    def _check_xval_validity(self, xval : float | numpy.ndarray | list):
        '''
        Will check that xval is an acceptable value for the function to be evaluated at
        '''

        if isinstance(xval, list):
            xval = numpy.array(xval)

        if not isinstance(xval, (float, numpy.ndarray)):
            raise ValueError(f'x value is not a float or numpy array: {xval}')

        check_within_bounds_vect = numpy.vectorize(self._check_within_bounds)
        check_within_bounds_vect(xval)
    #------------------------------------------------
    def _check_within_bounds(self, xval : float):
        '''
        Check that xval is within bounds of function
        '''

        if xval < min(self._l_x) or xval > max(self._l_x):
            print(self)
            raise ValueError(f'x value outside bounds: {xval}')
    #------------------------------------------------
    def _json_encoder(self, obj):
        '''
        Takes Function object
        Returns dictionary of attributes for encoding
        '''
        d_data = obj.__dict__

        if '_interpolator' in d_data:
            del d_data['_interpolator']

        return d_data
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
