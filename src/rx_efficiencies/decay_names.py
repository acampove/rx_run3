'''
Module used to define variables whose values are decay nicknames
'''
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_efficiencies:decay_names')
# -----------------------------------
class DecayNames:
    '''
    Class used to hold names of decays for ease of use in the rest of the code
    '''
    # --------------------------
    # correspondence between variable and name
    # of sample when e.g. saving to disk
    # --------------------------
    # NOTE: The names have to start with bp,bu or bs,
    # so that code downstream uses this name to pick up hadronization fractions

    bpkpee           = 'bpkpee'
    bdkskpiee        = 'bdkskpiee'
    bpkskpiee        = 'bpkskpiee'
    bsphiee          = 'bsphiee'
    bpk1kpipiee      = 'bpk1kpipiee'
    bpk2kpipiee      = 'bpk2kpipiee'
    # --------------------------
    # correspondence between decay variables and latex
    # --------------------------
    tex              = {}
    tex[bdkskpiee  ] = r'$B_d\to K^{*0}(\to K^+\pi^-)e^+e^-$'
    tex[bpkskpiee  ] = r'$B^+\to K^{*+}(\to K^+\pi^0)e^+e^-$'
    tex[bpkpee     ] = r'$B^+\to K^+e^+e^-$'
    tex[bsphiee    ] = r'$B_s\to \phi(1020)e^+e^-$'
    tex[bpk1kpipiee] = r'$B^+\to K_1(1270)^+(\to K^+\pi^+\pi^-)e^+e^-$'
    tex[bpk2kpipiee] = r'$B^+\to K_2(1430)^+(\to X \to K^+\pi^+\pi^-)e^+e^-$'
    # --------------------------
    # correspondence between decay variable and sample identifier
    # --------------------------
    sam              = {}
    sam[bdkskpiee  ] = 'Bd_Kstee_eq_btosllball05_DPC'
    sam[bpkskpiee  ] = 'Bu_Kstee_Kpi0_eq_btosllball05_DPC'
    sam[bpkpee     ] = 'Bu_Kee_eq_btosllball05_DPC'
    sam[bsphiee    ] = 'Bs_phiee_eq_Ball_DPC'
    sam[bpk1kpipiee] = 'Bu_K1ee_eq_DPC'
    sam[bpk2kpipiee] = 'Bu_K2stee_Kpipi_eq_mK1430_DPC'
    # -----------------------------------
    # correspondence between decay variable and list of subdecays, needed to calculate branching
    # fraction of decay from subdecays
    # -----------------------------------
    dec              = {}
    dec[bdkskpiee  ] = ['bdks', 'k+kp']
    dec[bpkskpiee  ] = ['bpks', 'kokp']
    dec[bpkpee     ] = ['bpkp']
    dec[bsphiee    ] = ['bsph', 'phkk']
    dec[bpk1kpipiee] = ['bpk1', 'k13h']
    dec[bpk2kpipiee] = ['bpk2', 'k23h']
    # -----------------------------------
    @staticmethod
    def get_decays() -> list[str]:
        '''
        Returns list of decay nicknames
        '''
        # TODO: Need to find K1 and K2 branching fractions. Are these decays important?
        return [
                DecayNames.bpkpee,
                DecayNames.bdkskpiee,
                DecayNames.bpkskpiee,
                DecayNames.bsphiee]
                #DecayNames.bpk1kpipiee,
                #DecayNames.bpk2kpipiee]
    # -----------------------------------
    @staticmethod
    def tex_from_decay(decay : str) -> str:
        '''
        Returns latex string for a given decay nickname
        '''
        if decay not in DecayNames.tex:
            for elm in DecayNames.tex:
                log.info(elm)
            raise ValueError(f'Decay {decay} not found')

        return DecayNames.tex[decay]
    # -----------------------------------
    @staticmethod
    def sample_from_decay(decay : str) -> str:
        '''
        Takes nickname of decay, returns nickname of sample
        '''
        if decay not in DecayNames.sam:
            for elm in DecayNames.sam:
                log.info(elm)
            raise ValueError(f'Decay {decay} not found')

        return DecayNames.sam[decay]
    # -----------------------------------
    @staticmethod
    def subdecays_from_decay(decay : str) -> list[str]:
        '''
        Takes nickname of decay, returns list of subdecays, needed to build branching fractions
        '''
        if decay not in DecayNames.dec:
            for elm in DecayNames.sam:
                log.info(elm)
            raise ValueError(f'Decay {decay} not found')

        return DecayNames.dec[decay]
# -----------------------------------
