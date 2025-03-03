'''
Module with class used to swap mass hypotheses
'''

import vector
import pandas as pnd
from ROOT                  import RDataFrame, RDF
from tqdm                  import tqdm
from particle              import Particle  as part
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_data:swp_calculator')
#---------------------------------
class SWPCalculator:
    '''
    Class used to calculate di-track masses, after mass hypothesis swaps
    '''
    #---------------------------------
    def __init__(self, rdf : RDataFrame, d_lep : dict[str,int], d_had : dict[str,int]):
        self._rdf    = rdf
        self._d_lep  = d_lep
        self._d_had  = d_had

        self._extra_branches= ['EVENTNUMBER', 'RUNNUMBER']
        self._df            = self._pnd_from_root(rdf)
        self._initialized   = False
    #---------------------------------
    def _pnd_from_root(self, rdf : RDataFrame) -> pnd.DataFrame:
        s_col_all = { name.c_str() for name in rdf.GetColumnNames() }
        s_col     = { name         for name in s_col_all if self._pick_column(name) }

        ncol  = len(s_col)
        log.debug(f'Using {ncol} columns for dataframe')

        d_data= rdf.AsNumpy(list(s_col))
        df    = pnd.DataFrame(d_data)

        return df
    #---------------------------------
    def _pick_column(self, name : str) -> bool:
        l_par = list(self._d_lep) + list(self._d_had)
        l_kin = []
        for par in l_par:
            l_kin += [
                    f'{par}_PX',
                    f'{par}_PY',
                    f'{par}_PZ',
                    f'{par}_PE',
                    f'{par}_ID']

        return name in l_kin
    #---------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._check_particle(self._d_lep)
        self._check_particle(self._d_had)

        tqdm.pandas(ascii=' -')

        self._initialized=True
    #---------------------------------
    def _check_particle(self, d_part):
        if not isinstance(d_part, dict):
            raise ValueError(f'Dictionary expected, found: {d_part}')

        for pdg_id in d_part.values():
            try:
                part.from_pdgid(pdg_id)
            except Exception as exc:
                raise ValueError(f'Cannot create particle for PDGID: {pdg_id}') from exc
    #---------------------------------
    def _build_mass(self, row, d_part) -> float:
        l_vec = []
        for name, new_id in d_part.items():
            par    = part.from_pdgid(new_id)
            ms     = par.mass
            old_id = row[f'{name}_ID']

            px = row[f'{name}_PX']
            py = row[f'{name}_PY']
            pz = row[f'{name}_PZ']
            pe = row[f'{name}_PE']
            vec_1 = vector.obj(px=px, py=py, pz=pz, t=pe)
            log.debug(f'{name}: {vec_1.mass:0f}({old_id}) -> {ms:.0f}({new_id})')
            vec_2 = vector.obj(pt=vec_1.pt, phi=vec_1.phi, theta=vec_1.theta, mass=ms)
            l_vec.append(vec_2)

        if len(l_vec) != 2:
            raise ValueError('Not found two and only two particles')

        vec_1 = l_vec[0]
        vec_2 = l_vec[1]
        vec   = vec_1 + vec_2
        mass  = float(vec.mass)

        return mass
    #---------------------------------
    def _combine(self, row : pnd.Series, had_name, kind : str, new_had_id : int) ->  float:
        old_had_id = row[f'{had_name}_ID']
        had        = part.from_pdgid(old_had_id)
        l_mass     = []
        for lep_name, new_lep_id in self._d_lep.items():
            old_lep_id = row[f'{lep_name}_ID']
            lep        = part.from_pdgid(old_lep_id)

            if lep.charge == had.charge:
                continue

            lep_id = new_lep_id if kind == 'swp' else old_lep_id
            had_id = new_had_id if kind == 'swp' else old_had_id

            mass   = self._build_mass(row, {had_name : had_id, lep_name : lep_id})

            l_mass.append(mass)

        ncmb = len(l_mass)

        if ncmb == 0:
            log.warning(f'Found no combinations with masses: {l_mass}')
            return -999

        if ncmb >  1:
            log.warning(f'Found more than one combinations with masses: {l_mass}')

        return l_mass[0]
    #---------------------------------
    def get_rdf(self, preffix : str, progress_bar : bool = False) -> RDataFrame:
        '''
        Parameters:
        ------------------
        preffix: Will be used to name branches with masses as `{preffix}_mass_org/swp` for the original and swapped masses
        progress_bar: If True, will show progress bar, by default false

        Returns:
        ------------------
        Pandas dataframe with orignal and swapped masses, i.e. masses after the mass hypothesis swap
        '''
        self._initialize()

        d_comb = {}
        for had_name, new_had_id in self._d_had.items():
            for kind in ['org', 'swp']:
                log.debug(f'Adding column for {had_name}/{new_had_id}/{kind}')
                if progress_bar:
                    sr_mass = self._df.progress_apply(self._combine, args=(had_name, kind, new_had_id), axis=1)
                else:
                    sr_mass = self._df.apply(self._combine, args=(had_name, kind, new_had_id), axis=1)

                d_comb[f'{preffix}_mass_{kind}'] = sr_mass.to_numpy().flatten()

        d_extra = self._rdf.AsNumpy(self._extra_branches)
        d_comb.update(d_extra)

        rdf    = RDF.FromNumpy(d_comb)

        return rdf
#---------------------------------
