'''
Module holding cv_classifier class
'''
import hashlib

import pandas as pnd

from sklearn.ensemble        import GradientBoostingClassifier

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:CVClassifier')

# ---------------------------------------
class CVSameData(Exception):
    '''
    Will be raised if a model is been evaluated with a dataset such that at least one of the
    samples was also used for the training
    '''
# ---------------------------------------
class CVClassifier(GradientBoostingClassifier):
    '''
    Derived class meant to implement features needed for cross-validation
    '''
    # pylint: disable = too-many-ancestors, abstract-method
    # ----------------------------------
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self._s_hash    = set()
        self._data      = {}
        self._l_ft_name = None
    # ----------------------------------
    @property
    def features(self):
        '''
        Returns list of feature names used in training dataset
        '''
        return self._l_ft_name
    # ----------------------------------
    def __str__(self):
        nhash = len(self._s_hash)

        msg = 40 * '-' + '\n'
        msg+= f'{"Attribute":<20}{"Value":<20}\n'
        msg+= 40 * '-' + '\n'
        msg += f'{"Hashes":<20}{nhash:<20}\n'
        msg+= 40 * '-'

        return msg
    # ----------------------------------
    def _get_hashes(self, df_ft):
        '''
        Will return hashes for each row in the feature dataframe
        '''
        if not isinstance(df_ft, pnd.DataFrame):
            log.error('Features need to be in a pandas dataframe')
            raise ValueError

        s_hash = { self._hash_from_row(row) for _, row in df_ft.iterrows() }

        return s_hash
    # ----------------------------------
    def _hash_from_row(self, row):
        '''
        Will return a hash from a pandas dataframe row
        corresponding to an event
        '''
        l_val   = [ str(val) for val in row ]
        row_str = ','.join(l_val)
        row_str = row_str.encode('utf-8')

        hsh = hashlib.sha256()
        hsh.update(row_str)

        hsh_val = hsh.digest()

        return hsh_val
    # ----------------------------------
    def fit(self, *args, **kwargs):
        '''
        Runs the training of the model
        '''
        log.debug('Fitting')

        df_ft           = args[0]
        self._l_ft_name = list(df_ft.columns)

        self._s_hash = self._get_hashes(df_ft)
        log.debug(f'Saving {len(self._s_hash)} hashes')

        super().fit(*args, **kwargs)

        return self
    # ----------------------------------
    def _check_hashes(self, df_ft):
        '''
        Will check that the hashes of the passed features do not intersect with the
        hashes of the features used for the training.
        Else it will raise CVSameData exception
        '''

        if len(self._s_hash) == 0:
            raise ValueError('Found no hashes in model')

        s_hash  = self._get_hashes(df_ft)
        s_inter = self._s_hash.intersection(s_hash)

        nh1 = len(self._s_hash)
        nh2 = len(      s_hash)
        nh3 = len(s_inter)

        if nh3 > 0:
            raise CVSameData(f'Found non empty intersection of size: {nh1} ^ {nh2} = {nh3}')
    # ----------------------------------
    def predict_proba(self, X, on_training_ok=False):
        '''
        Takes pandas dataframe with features
        Will first check hashes to make sure none of the events/samples
        used for the training of this model are in the prediction

        on_training_ok (bool): True if the dataset is expected to contain samples used for training, default is False
        '''
        if not on_training_ok:
            self._check_hashes(X)

        return super().predict_proba(X)
# ---------------------------------------
