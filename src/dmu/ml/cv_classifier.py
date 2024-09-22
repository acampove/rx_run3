'''
Module holding cv_classifier class
'''

from sklearn.ensemble        import GradientBoostingClassifier

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:CVClassifier')
# ---------------------------------------
class CVClassifier(GradientBoostingClassifier):
    '''
    Derived class meant to implement features needed for cross-validation
    '''
    # pylint: disable = too-many-ancestors, abstract-method
    # ----------------------------------
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self._l_allowed_attrs = ['dset_hash', 'sset_hash']

        self._data = {}
    # ----------------------------------
    def __setitem__(self, key, value):
        '''
        Used to set values of dataset and subdataset (from cross-validation splitting) hashes
        '''
        if key not in self._l_allowed_attrs:
            raise ValueError(f'Invalid item: {key}, choose from {self._l_allowed_attrs}')

        self._data[key] = value

    # ----------------------------------
    def __getitem__(self, key):
        '''
        Used to get values of dataset and subdataset (from cross-validation splitting) hashes
        '''
        if key not in self._l_allowed_attrs:
            raise ValueError(f'Invalid item: {key}, choose from {self._l_allowed_attrs}')

        value = self._data[key]

        return value
    # ----------------------------------
    def __str__(self):
        msg = 40 * '-' + '\n'
        msg+= f'{"Attribute":<20}{"Value":<20}\n'
        msg+= 40 * '-' + '\n'
        for attr, value in self._data.items():
            msg += f'{attr:<20}{value:<20}\n'
        msg+= 40 * '-'

        return msg
    # ----------------------------------
    def fit(self, *args, **kwargs):
        log.debug('Fitting')
        super().fit(*args, **kwargs)
# ---------------------------------------
