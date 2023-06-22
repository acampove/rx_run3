from logzero      import logger    as log

import zfit
import logging
import matplotlib.pyplot           as plt

#--------------------------------------
class model:
    def __init__(self, d_sim_par=None):
        self._sig_pdf_splt = None
        self._sig_pdf_merg = None
        self._bkg_pdf      = None
        self._d_sim_par    = d_sim_par 

        self._obs          = zfit.Space('Jpsi_M', limits=(2450, 3600))
        self._mu           = zfit.Parameter('mu', 3060,  3040, 3100)
        self._sg           = zfit.Parameter('sg',   20,    10,  100)
        self._dmu          = zfit.Parameter('dmu', 0, 0.0, 50.0)
        self._rsg          = zfit.Parameter('rsg', 1, 0.7,  1.4)

        zfit.settings.changed_warnings.hesse_name = False
    #--------------------------------------
    def get_pdf(self, is_signal=None, split_by_nspd=None):
        if is_signal     not in [True, False]:
            log.error('Signal flag not specified')
            raise
    
        if split_by_nspd not in [True, False]:
            log.error('split_by_nspd flag not specified')
            raise
    
        pdf = self._get_signal_pdf() if is_signal else self._get_full_pdf(split_by_nspd)
    
        return pdf
    #--------------------------------------
    def _get_nspd_signal(self, l_pdf):
        nsg   = 1000 
        nsg_1 = zfit.Parameter("nsg_1", nsg, nsg - 0.1 * nsg, nsg + 0.1 * nsg)
        nsg_2 = zfit.Parameter("nsg_2", nsg, nsg - 0.1 * nsg, nsg + 0.1 * nsg)
        nsg_3 = zfit.Parameter("nsg_3", nsg, nsg - 0.1 * nsg, nsg + 0.1 * nsg)
        l_nsg = [nsg_1, nsg_2, nsg_3]
        l_epdf= [ pdf.create_extended(nsg) for pdf, nsg in zip(l_pdf, l_nsg) ]
        epdf  = zfit.pdf.SumPDF(pdfs=l_epdf)
    
        return epdf
    #--------------------------------------
    def _get_cb_pdf(self):
        ap_r  = zfit.Parameter('ap_r',  1.0,  -10.0, 10.0)
        pw_r  = zfit.Parameter('pw_r',  1.0,    0.1, 10.0)
        sig_r = zfit.pdf.CrystalBall(obs=self._obs, mu=self._mu, sigma=self._sg, alpha=ap_r, n=pw_r)
    
        ap_l  = zfit.Parameter('ap_l', -1.0,  -10.0, 10.0)
        pw_l  = zfit.Parameter('pw_l',  1.0,    0.1,  10.)
        sig_l = zfit.pdf.CrystalBall(obs=self._obs, mu=self._mu, sigma=self._sg, alpha=ap_l, n=pw_l)
    
        ncbr  = zfit.Parameter('ncbr',  10,   0,  1000000)
        sig_r = sig_r.create_extended(ncbr, name='CB1')
    
        ncbl  = zfit.Parameter('ncbl',  10,   0,  1000000)
        sig_l = sig_l.create_extended(ncbl, name='CB2') 
    
        sig   = zfit.pdf.SumPDF([sig_r, sig_l], name='Signal')
    
        return sig
    #--------------------------------------
    def _get_nspd_data_pars(self, preffix=''):
        sim_mu = zfit.param.ConstantParameter(f'sim_mu{preffix}', self._d_sim_par[f'mu{preffix}'][0])
        sim_sg = zfit.param.ConstantParameter(f'sim_sg{preffix}', self._d_sim_par[f'sg{preffix}'][0])
    
        dat_mu = zfit.ComposedParameter(f'dat_mu{preffix}', 
                                        lambda d_par : d_par['dmu'] + d_par[f'sim_mu{preffix}'], 
                                        {'dmu' : self._dmu, f'sim_mu{preffix}' : sim_mu} )
        dat_sg = zfit.ComposedParameter(f'dat_sg{preffix}', 
                                        lambda d_par : d_par['rsg'] * d_par[f'sim_sg{preffix}'], 
                                        {'rsg' : self._rsg, f'sim_sg{preffix}' : sim_sg} )
    
        return dat_mu, dat_sg
    #--------------------------------------
    def _get_cb_nspd_pdf(self, prefix=''):
        mu,sg = self._get_nspd_data_pars(prefix)
    
        ap_r  = zfit.Parameter(f'ap_r{prefix}',  1.0,  -10.0, 10.0)
        pw_r  = zfit.Parameter(f'pw_r{prefix}',  1.0,    0.1, 10.0)
        sig_r = zfit.pdf.CrystalBall(obs=self._obs, mu=mu, sigma=sg, alpha=ap_r, n=pw_r)
    
        ap_l  = zfit.Parameter(f'ap_l{prefix}', -1.0,  -10.0, 10.0)
        pw_l  = zfit.Parameter(f'pw_l{prefix}',  1.0,    0.1,  10.)
        sig_l = zfit.pdf.CrystalBall(obs=self._obs, mu=mu, sigma=sg, alpha=ap_l, n=pw_l)
    
        fr    = zfit.Parameter(f'fr_cb{prefix}', 0.5,  0.0, 1)
        sig   = zfit.pdf.SumPDF([sig_r, sig_l], fracs=[fr])
    
        return sig
    #--------------------------------------
    def _get_signal_pdf(self, split_by_nspd = False, prefix=None):
        if   self._sig_pdf_splt is not None and     split_by_nspd:
            return self._sig_pdf_splt
        elif self._sig_pdf_merg is not None and not split_by_nspd:
            return self._sig_pdf_merg
    
        if split_by_nspd:
            l_pdf = [ self._get_cb_nspd_pdf(prefix=f'_{i_nspd}') for i_nspd in [1, 2, 3] ]
            self._sig_pdf_splt = self._get_nspd_signal(l_pdf)
        else:
            self._sig_pdf_merg = self._get_cb_pdf()
    
        return self._sig_pdf_splt if split_by_nspd else self._sig_pdf_merg 
    #--------------------------------------
    def _get_full_pdf(self, split_by_nspd): 
        sig = self._get_signal_pdf(split_by_nspd)
        bkg = self._get_bkg_pdf()
        pdf = zfit.pdf.SumPDF([sig, bkg], name='Model')
    
        log.debug(f'{"Signal":<20}{str(sig):<40}')
        log.debug(f'{"Background":<20}{str(bkg):<20}')
        log.debug(f'{"Model":<20}{str(pdf):<40}')
    
        return pdf
    #--------------------------------------
    def _get_bkg_pdf(self):
        if self._bkg_pdf is not None:
            return self._bkg_pdf
    
        lam = zfit.Parameter('lam', -0.001, -0.1, -0.0001)
        bkg = zfit.pdf.Exponential(lam=lam, obs=self._obs, name='')
    
        nbk = zfit.Parameter(f'nbk', 100, 0.0, 200000)
        bkg = bkg.create_extended(nbk, name='Combinatorial')
    
        self._bkg_pdf = bkg
    
        return bkg
    #--------------------------------------
    def clean_pars(self):
        log.debug('Deallocating PDF')
        d_par = zfit.Parameter._existing_params
        l_key = list(d_par.keys())

        for key in l_key:
            del(d_par[key])
#--------------------------------------

