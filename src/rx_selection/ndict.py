'''
Module containing ndict class
'''
# pylint: disable=import-error, line-too-long

from collections           import UserDict
from dmu.logging.log_store import LogStore

#-----------------------
class ndict(UserDict):
    '''
    Class used to map pairs of objects to a third object
    Can enforce that all keys have been used with check()
    '''
    log = LogStore.add_logger('rx_selection:ndict')
    #--------------------------------
    def __init__(self):
        self._s_key_x = set()
        self._s_key_y = set()

        super().__init__()
    #--------------------------------
    def __setitem__(self, key, val):
        try:
            key_x, key_y = key
        except:
            self.log.error(f'Argument is not a pair of objects: {key}')
            raise

        self._s_key_x.add(key_x)
        self._s_key_y.add(key_y)

        self.data[key] = val
    #--------------------------------
    @property
    def x_axis(self):
        return self._s_key_x
    @property
    def y_axis(self):
        return self._s_key_y
    #--------------------------------
    def has_val(self, val, axis=None):
        if   axis == 'x':
            axis = [ xval for xval, _    in self.data ]
        elif axis == 'y':
            axis = [ yval for    _, yval in self.data ]
        else:
            self.log.error(f'Wrong axis: {axis}')
            raise

        return val in axis
    #--------------------------------
    def __str__(self, type_only=False):
        msg = '-' * 40
        msg+= '\nndict\n'
        msg+= '-' * 40
        for (xval, yval), value in self.data.items():
            tpval = str(type(value))
            value = str(value)
            if type_only:
                msg += f'\n{xval:<20}{yval:<20}{"->"}{tpval:>30}\n'
            else:
                msg += f'\n{xval:<20}{yval:<20}{"->"}{value:>30}\n'
        msg+= '-' * 40

        return msg
    #--------------------------------
    def check(self):
        for key_x in self._s_key_x:
            for key_y in self._s_key_y:
                t_key = (key_x, key_y)
                if t_key not in self.data:
                    self.log.error(f'No data found for: {t_key}')
                    print(self)
                    raise
    #--------------------------------
    @staticmethod
    def json_encoder(obj):
        return { ind : [key, val] for ind, (key, val) in enumerate(obj.data.items()) }

    @staticmethod
    def json_decoder(dic):
        obj=ndict()
        for [[key_1, key_2], val] in dic.values():
            obj[key_1, key_2] = val

        return obj
#-----------------------
