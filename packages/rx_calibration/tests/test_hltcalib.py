'''
Module with tests for HLT calibration class
'''

# ---------------------------
def _get_config() -> dict:
    return {}
# ---------------------------
def test_simple():
    '''
    Simplest version of test
    '''

    for cut in l_cut:
      par = ParCal(dat=rdf_dat, sim=rdf_sim, cut=cut)
      par.fit(sig=pdf_sig, bkg=pdf_bkg)

      obj = EffCal(par=par)
      eff = obj.get_eff()

