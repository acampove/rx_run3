'''
This module contains the MVACalculator class
'''

import os
import glob
import joblib
import numpy

from ROOT                  import RDF
from dmu.ml.cv_predict     import CVClassifier, CVPredict
from dmu.logging.log_store import LogStore
from rx_selection          import selection as sel

log = LogStore.add_logger('rx_classifier:mva_calculator')
#---------------------------------
class MVACalculator:
    '''
    This class is meant to plug the ROOT dataframe with data and MC
    with the classifiers and produce friend trees in the form of a dataframe
    '''
    # ----------------------
    def __init__(
        self,
        rdf     : RDF.RNode,
        sample  : str,
        trigger : str,
        version : str,
        nfold   : int  = 10,
        dry_run : bool = False) -> None:
        '''
        Parameters
        -------------
        rdf    : Dataframe with main sample
        version: Version of classifier
        sample : E.g. DATA_24... # Needed to get q2 selection
        trigger: HLT2 triger     # to switch models
        nfold  : Number of expected folds, 10 by default. Used to validate inputs
        dry_run: If true, will not evaluate models, but stop early and assign 1s
        '''
        rdf = rdf.Define('index', 'rdfentry_')

        self._rdf         = rdf
        self._sample      = sample
        self._trigger     = trigger
        self._max_path    = 700
        self._nfold       = nfold
        self._ana_dir     = os.environ['ANADIR']
        # TODO: Update this.
        # Jpsi and Psi2 should use central MVA
        # Above high data should use high MVA
        self._default_q2  = 'central' # Any entry not in [low, central, high] bins will go to this bin for prediction
        self._version     = version
        self._l_model     : list[CVClassifier]
        self._dry_run     = dry_run
    #---------------------------------
    def _get_q2_selection(self, q2bin : str) -> str:
        '''
        Parameters
        ---------------
        q2bin: E.g. central, needed to retrieve selection used to switch classifiers

        Returns
        ---------------
        string defining q2 selection
        '''
        d_sel = sel.selection(
                trigger=self._trigger,
                q2bin  =q2bin,
                process=self._sample)

        q2_cut = d_sel['q2']

        return q2_cut
    #---------------------------------
    def _apply_q2_cut(
        self,
        rdf   : RDF.RNode,
        q2bin : str) -> RDF.RNode:
        '''
        Parameters
        --------------
        rdf  : ROOT dataframe with contents of main tree
        q2bin: E.g. central

        Returns
        --------------
        Dataframe after q2 selection
        '''
        if q2bin == 'rest':
            low     = self._get_q2_selection(q2bin='low')
            central = self._get_q2_selection(q2bin='central')
            high    = self._get_q2_selection(q2bin='high')
            q2_cut  = f'!({low}) && !({central}) && !({high})'
        else:
            q2_cut  = self._get_q2_selection(q2bin=q2bin)

        log.debug(f'{q2bin:<10}{q2_cut}')
        rdf = rdf.Filter(q2_cut, 'q2')

        return rdf
    # ----------------------------------------
    def _q2_scores_from_rdf(
        self,
        rdf    : RDF.RNode,
        d_path : dict[str,str],
        q2bin  : str) -> numpy.ndarray:
        '''
        Parameters
        -----------
        rdf   : DataFrame with input data, it has to be indexed with an `index` column
        d_path: Dictionary mapping q2bin to path to models
        q2bin : q2 bin

        Returns
        -----------
        2D Array with indexes and MVA scores
        '''
        rdf     = self._apply_q2_cut(rdf=rdf, q2bin=q2bin)
        nentries= rdf.Count().GetValue()
        if nentries == 0:
            log.warning(f'No entries found for q2 bin: {q2bin}')
            return numpy.column_stack(([], []))

        # The dataframe has the correct cut applied
        # From here onwards, if the q2bin is non-rare (rest)
        # Will use default_q2 model
        if q2bin == 'rest':
            q2bin = self._default_q2

        path   = d_path[q2bin]
        l_pkl  = glob.glob(f'{path}/*.pkl')

        npkl   = len(l_pkl)
        if npkl == 0:
            raise ValueError(f'No pickle files found in {path}')

        log.info(f'Using {npkl} pickle files from: {path}')
        l_model = [ joblib.load(pkl_path) for pkl_path in l_pkl ]

        cvp     = CVPredict(models=l_model, rdf=rdf)
        if self._dry_run:
            log.warning(f'Using {nentries} ones for dry run MVA scores')
            arr_prb = numpy.ones(nentries)
        else:
            arr_prb = cvp.predict()

        arr_ind = rdf.AsNumpy(['index'])['index']
        arr_res = numpy.column_stack((arr_ind, arr_prb))

        log.debug(f'Shape: {arr_res.shape}')

        return arr_res
    # ----------------------------------------
    def _scores_from_rdf(
        self,
        rdf    : RDF.RNode,
        d_path : dict[str,str]) -> numpy.ndarray:
        '''
        Parameters
        ------------------
        rdf   : DataFrame with inputs
        d_path: Dictionary mapping q2bin to path to models

        Returns
        ------------------
        Array of signal probabilities
        '''
        nentries = rdf.Count().GetValue()

        arr_low     = self._q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='low'    )
        arr_central = self._q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='central')
        arr_high    = self._q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='high'   )
        arr_rest    = self._q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='rest'   )
        arr_all     = numpy.concatenate((arr_low, arr_central, arr_high, arr_rest))

        arr_ind = arr_all.T[0]
        arr_val = arr_all.T[1]

        arr_obtained = numpy.sort(arr_ind)
        arr_expected = numpy.arange(nentries + 1)
        if  numpy.array_equal(arr_obtained, arr_expected):
            raise ValueError('Array of indexes has the wrong values')

        arr_ord = numpy.argsort(arr_ind)
        arr_mva = arr_val[arr_ord]

        return arr_mva
    # ----------------------
    def _get_q2_path(self, q2bin : str) -> str:
        '''
        Parameters
        -------------
        q2bin: E.g. central

        Returns
        -------------
        Path to directory with classifier models
        '''
        path = f'{self._ana_dir}/mva/cmb/{self._version}/{q2bin}'
        fail = False
        for ifold in range(self._nfold):
            model_path = f'{path}/model_{ifold:03}.pkl'
            if not os.path.isfile(model_path):
                log.error(f'Missing: {model_path}')
                fail = True

        if fail:
            raise FileNotFoundError('At least one model is missing')

        return path
    # ----------------------
    def _get_mva_dir(self) -> dict:
        '''
        Returns
        -----------
        Dictionary with paths to directories with classifier models
        '''
        l_q2bin    = ['low', 'central', 'high']
        d_path_cmb = { q2bin : self._get_q2_path(q2bin=q2bin) for q2bin in l_q2bin }
        d_path_prc = { q2bin : self._get_q2_path(q2bin=q2bin) for q2bin in l_q2bin }

        return {'cmb' : d_path_cmb, 'prc' : d_path_prc}
    # ----------------------------------------
    def apply_classifier(self, rdf : RDF.RNode) -> RDF.RNode:
        '''
        Takes name of dataset and corresponding ROOT dataframe
        return dataframe with a classifier probability column added
        '''
        d_mva_kind  = self._get_mva_dir()
        d_mva_score = {}
        for name, d_path in d_mva_kind.items():
            arr_score = self._scores_from_rdf(rdf=rdf, d_path=d_path)
            d_mva_score[f'mva_{name}'] = arr_score

        d_data      = rdf.AsNumpy(['RUNNUMBER', 'EVENTNUMBER'])
        d_data.update(d_mva_score)
        rdf         = RDF.FromNumpy(d_data)

        return rdf
#---------------------------------
