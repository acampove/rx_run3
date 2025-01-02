'''
Module containing functions used for truth matching
'''
# pylint: disable=line-too-long, import-error, too-many-statements, invalid-name, too-many-branches

from typing import Union

from dmu.logging.log_store  import LogStore


log=LogStore.add_logger('rx_selection:truth_matching')

# ----------------------------------------------------------
def get_truth(event_type : Union[int,str]) -> str:
    '''
    Function meant to return truth matching string from event type string
    For data it will return '(1)'
    '''
    if isinstance(event_type, int):
        event_type=str(event_type)

    if     event_type.startswith('data'):
        cut = '(1)'
    elif   event_type in ['12113001', '12113002']:
        #rare mumu
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(L1_TRUEID) == 13 && TMath::Abs(L2_TRUEID) == 13 && TMath::Abs(L1_MC_MOTHER_ID) == 521 && TMath::Abs(L2_MC_MOTHER_ID) == 521 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'
    elif event_type in ['12123002', '12123003', '12123005']:
        #rare ee
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 521 && TMath::Abs(L2_MC_MOTHER_ID) == 521 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'
    elif event_type in ['12143001']:
        #reso Jpsi mumu
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 13 && TMath::Abs(L2_TRUEID) == 13 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso Jpsi mumu
    elif event_type in ['12153001']:
        #reso Jpsi ee
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso Jpsi ee
    elif event_type in ['12143020']:
        #reso Psi mumu
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 100443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 13 && TMath::Abs(L2_TRUEID) == 13 && TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso Psi mumu
    elif event_type in ['12153012']:
        #reso Psi ee
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 100443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso Psi ee
    elif event_type in ['12143010']:
        #reso jpsi pi mumu
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 13 && TMath::Abs(L2_TRUEID) == 13 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 211 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso jpsi pi mumu
    elif event_type in ['12153020']:
        #reso jpsi pi ee
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 211 && TMath::Abs(H_MC_MOTHER_ID) == 521'#reso jpsi pi ee
    #-------------------------------------------------------------
    elif event_type in ['12952000']:
        #B+->XcHs
        ctrl_ee    = get_truth('ctrl_ee')
        psi2_ee    = get_truth('psi2_ee')
        ctrl_pi_ee = get_truth('ctrl_pi_ee')
        fail       = get_truth('fail')

        cut= f'!({fail}) && !({ctrl_ee}) && !({psi2_ee}) && !({ctrl_pi_ee})'
    elif event_type == '11453001':
        #Bd->XcHs
        ctrl_ee    = get_truth('ctrl_ee')
        psi2_ee    = get_truth('psi2_ee')
        ctrl_pi_ee = get_truth('ctrl_pi_ee')
        fail       = get_truth('fail')

        cut= f'!({fail}) && !({ctrl_ee}) && !({psi2_ee}) && !({ctrl_pi_ee})'
    elif event_type == '13454001':
        #Bs->XcHs
        ctrl_ee    = get_truth('ctrl_ee')
        psi2_ee    = get_truth('psi2_ee')
        ctrl_pi_ee = get_truth('ctrl_pi_ee')
        fail       = get_truth('fail')

        cut= f'!({fail}) && !({ctrl_ee}) && !({psi2_ee}) && !({ctrl_pi_ee})'
    elif event_type == '12442001':
        # bpXcHs_mm
        fail            = get_truth('fail')
        mm              = '((TMath::Abs(L1_TRUEID)==13) && (TMath::Abs(L2_TRUEID)==13))'
        ll_mother       = '(((TMath::Abs(Jpsi_TRUEID)==443) && (TMath::Abs(L1_MC_MOTHER_ID)==443) && (TMath::Abs(L2_MC_MOTHER_ID)==443)) || ((TMath::Abs(Jpsi_TRUEID)==100443) && (TMath::Abs(L1_MC_MOTHER_ID)==100443) && (TMath::Abs(L2_MC_MOTHER_ID)==100443)))'
        Bx              = "TMath::Abs(B_TRUEID)==521"
        Bx_psi2s_mother = "((TMath::Abs(Jpsi_MC_MOTHER_ID)==521 && TMath::Abs(Jpsi_TRUEID)==100443) || (TMath::Abs(Jpsi_TRUEID) != 100443))"

        cut             = f"!({fail}) && ({mm}) && ({ll_mother}) && ({Bx}) && ({Bx_psi2s_mother})"
    elif event_type == '11442001':
        # bdXcHs_mm
        fail            = get_truth('fail')
        mm              = '((TMath::Abs(L1_TRUEID)==13) && (TMath::Abs(L2_TRUEID)==13))'
        ll_mother       = '(((TMath::Abs(Jpsi_TRUEID)==443) && (TMath::Abs(L1_MC_MOTHER_ID)==443) && (TMath::Abs(L2_MC_MOTHER_ID)==443)) || ((TMath::Abs(Jpsi_TRUEID)==100443) && (TMath::Abs(L1_MC_MOTHER_ID)==100443) && (TMath::Abs(L2_MC_MOTHER_ID)==100443)))'
        Bx              = "TMath::Abs(B_TRUEID)==511"
        Bx_psi2s_mother = "((TMath::Abs(Jpsi_MC_MOTHER_ID)==511 && TMath::Abs(Jpsi_TRUEID)==100443) || (TMath::Abs(Jpsi_TRUEID) != 100443))"

        cut             = f"!({fail}) && ({mm}) && ({ll_mother}) && ({Bx}) && ({Bx_psi2s_mother})"
    elif event_type == '13442001':
        # bsXcHs_mm
        fail            = get_truth('fail')
        mm              = '((TMath::Abs(L1_TRUEID)==13) && (TMath::Abs(L2_TRUEID)==13))'
        ll_mother       = '(((TMath::Abs(Jpsi_TRUEID)==443) && (TMath::Abs(L1_MC_MOTHER_ID)==443) && (TMath::Abs(L2_MC_MOTHER_ID)==443)) || ((TMath::Abs(Jpsi_TRUEID)==100443) && (TMath::Abs(L1_MC_MOTHER_ID)==100443) && (TMath::Abs(L2_MC_MOTHER_ID)==100443)))'
        Bx              = "TMath::Abs(B_TRUEID)==531"
        Bx_psi2s_mother = "((TMath::Abs(Jpsi_MC_MOTHER_ID)==531 && TMath::Abs(Jpsi_TRUEID)==100443) || (TMath::Abs(Jpsi_TRUEID) != 100443))"

        cut             = f"!({fail}) && ({mm}) && ({ll_mother}) && ({Bx}) && ({Bx_psi2s_mother})"
    #-------------------------------------------------------------
    elif event_type == '12155100':
        #exclusive jpsi kst ee Bu
        cut= 'TMath::Abs(B_TRUEID) == 521 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 521 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 211 && (TMath::Abs(H_MC_MOTHER_ID) == 323 or TMath::Abs(H_MC_MOTHER_ID) == 310) && (TMath::Abs(H_MC_GD_MOTHER_ID) == 521 or TMath::Abs(H_MC_GD_MOTHER_ID) == 323)'#exclusive Jpsi kst ee
    elif event_type == '11154001':
        #exclusive jpsi kst ee Bd
        cut= 'TMath::Abs(B_TRUEID) == 511 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 511 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 313'#exclusive Jpsi kst ee Bd
    elif event_type == '13454001':
        #reso jpsi kst ee Bs
        cut= 'TMath::Abs(B_TRUEID) == 531 && TMath::Abs(Jpsi_TRUEID) == 443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 531 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 443 && TMath::Abs(L2_MC_MOTHER_ID) == 443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 313'#reso Jpsi kst ee
    elif event_type in ['11154011']:
        #Bd->psi2S(=>ee) K*
        cut= 'TMath::Abs(B_TRUEID) == 511 && TMath::Abs(Jpsi_TRUEID) == 100443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 511 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 313'#reso Psi kst ee
    elif event_type == '11453012':
        #reso Psi X
        cut= 'TMath::Abs(B_TRUEID) == 511 && TMath::Abs(Jpsi_TRUEID) == 100443 && TMath::Abs(Jpsi_MC_MOTHER_ID) == 511 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443'#reso Psi(ee) X
    elif event_type == '11124002':
        #Bd K*(k pi) ee.
        cut= 'TMath::Abs(B_TRUEID) == 511 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 511 && TMath::Abs(L2_MC_MOTHER_ID) == 511 && (TMath::Abs(H_TRUEID) == 321 or TMath::Abs(H_TRUEID) == 211) && TMath::Abs(H_MC_MOTHER_ID) == 313'
    elif event_type == '11124037':
        #Bd (k pi) ee.
        cut= 'TMath::Abs(B_TRUEID) == 511 && TMath::Abs(L1_TRUEID) == 11 && TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID) == 511 && TMath::Abs(L2_MC_MOTHER_ID) == 511 && (TMath::Abs(H_TRUEID) == 321 or TMath::Abs(H_TRUEID) == 211) && TMath::Abs(H_MC_MOTHER_ID) == 511'
    elif event_type == '12123445':
        #B+ -> K*+ ee
        cut= 'TMath::Abs(B_TRUEID) == 521 &&  TMath::Abs(L1_TRUEID) ==  11 &&  TMath::Abs(L2_TRUEID) == 11 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 521 &&  TMath::Abs(L2_MC_MOTHER_ID) == 521 &&  TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 323'
    elif event_type == '13124006':
        #Bs -> phi(-> KK) ee
        cut= 'TMath::Abs(B_TRUEID) == 531 &&  TMath::Abs(L1_TRUEID) ==  11 &&  TMath::Abs(L2_TRUEID) == 11 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 531 &&  TMath::Abs(L2_MC_MOTHER_ID) == 531 &&  TMath::Abs(H_TRUEID) == 321 && TMath::Abs(H_MC_MOTHER_ID) == 333'
    elif event_type == '12425000':
        #B+ -> K_1(K pipi) ee
        cut= 'TMath::Abs(B_TRUEID) == 521 &&  TMath::Abs(L1_TRUEID) ==  11 &&  TMath::Abs(L2_TRUEID) == 11 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 521 &&  TMath::Abs(L2_MC_MOTHER_ID) == 521 &&  (TMath::Abs(H_TRUEID) == 321 || TMath::Abs(H_TRUEID) == 211) && (TMath::Abs(H_MC_MOTHER_ID) == 10323 || TMath::Abs(H_MC_MOTHER_ID) == 113 || TMath::Abs(H_MC_MOTHER_ID) == 223 || TMath::Abs(H_MC_MOTHER_ID) == 313)'
    elif event_type == '12425011':
        #B+ -> K_2(X -> K pipi) ee
        cut= 'TMath::Abs(B_TRUEID) == 521 &&  TMath::Abs(L1_TRUEID) ==  11 &&  TMath::Abs(L2_TRUEID) == 11 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 521 &&  TMath::Abs(L2_MC_MOTHER_ID) == 521 &&  (TMath::Abs(H_TRUEID) == 321 || TMath::Abs(H_TRUEID) == 211) && (TMath::Abs(H_MC_MOTHER_ID) ==   325 || TMath::Abs(H_MC_MOTHER_ID) == 113 || TMath::Abs(H_MC_MOTHER_ID) == 223 || TMath::Abs(H_MC_MOTHER_ID) == 313)'
    elif event_type == '12155110':
        #B+->K*+ psi2S(-> ee)
        cut= 'TMath::Abs(B_TRUEID) == 521 &&  TMath::Abs(L1_TRUEID) ==  11 &&  TMath::Abs(L2_TRUEID) == 11 && TMath::Abs(L1_MC_MOTHER_ID)  == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(H_TRUEID) == 211 && (TMath::Abs(H_MC_MOTHER_ID) == 323 or TMath::Abs(H_MC_MOTHER_ID) == 310)'
    elif event_type == '12103025':
        #B+ -> K+ pi pi
        cut= 'TMath::Abs(B_TRUEID)  == 521 &&  TMath::Abs(L1_TRUEID)  == 211 &&  TMath::Abs(L2_TRUEID) == 211 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 521 &&  TMath::Abs(L2_MC_MOTHER_ID) == 521 &&  TMath::Abs(H_TRUEID) == 321 &&  TMath::Abs(H_MC_MOTHER_ID) == 521'
    elif event_type == '12103017':
        #B+ -> K+ K K
        cut= 'TMath::Abs(B_TRUEID)  == 521 &&  TMath::Abs(L1_TRUEID)  == 321 &&  TMath::Abs(L2_TRUEID) == 321 &&  TMath::Abs(L1_MC_MOTHER_ID)  == 521 &&  TMath::Abs(L2_MC_MOTHER_ID) == 521 &&  TMath::Abs(H_TRUEID) == 321 &&  TMath::Abs(H_MC_MOTHER_ID) == 521'
    elif event_type == '12583021':
        #bpd0kpenuenu
        tm_par = 'TMath::Abs(B_TRUEID)  == 521 &&  TMath::Abs(L1_TRUEID)  == 11 &&  TMath::Abs(L2_TRUEID) == 11'
        tm_dt1 = 'TMath::Abs(L1_MC_MOTHER_ID)  == 521 || TMath::Abs(L1_MC_MOTHER_ID) == 421'
        tm_dt2 = 'TMath::Abs(L2_MC_MOTHER_ID)  == 521 || TMath::Abs(L2_MC_MOTHER_ID) == 421'
        cut    = f'({tm_par}) && ({tm_dt1}) && ({tm_dt2}) && TMath::Abs(H_TRUEID) == 321 &&  TMath::Abs(H_MC_MOTHER_ID) == 421'
    elif event_type == '12183004':
        # bpd0kpenupi
        tm_par = 'TMath::Abs(B_TRUEID)  == 521 &&  (TMath::Abs(L1_TRUEID)  == 11 || TMath::Abs(L1_TRUEID)  == 211) &&  (TMath::Abs(L2_TRUEID) == 11 || TMath::Abs(L2_TRUEID) == 211)'
        tm_dt1 = 'TMath::Abs(L1_MC_MOTHER_ID)  == 521 || TMath::Abs(L1_MC_MOTHER_ID) == 421'
        tm_dt2 = 'TMath::Abs(L2_MC_MOTHER_ID)  == 521 || TMath::Abs(L2_MC_MOTHER_ID) == 421'
        cut    = f'({tm_par}) && ({tm_dt1}) && ({tm_dt2}) && TMath::Abs(H_TRUEID) == 321 &&  TMath::Abs(H_MC_MOTHER_ID) == 421'
    elif event_type == '12583013':
        # bpd0kppienu
        tm_par = 'TMath::Abs(B_TRUEID)  == 521 &&  (TMath::Abs(L1_TRUEID)  == 11 || TMath::Abs(L1_TRUEID)  == 211) &&  (TMath::Abs(L2_TRUEID) == 11 || TMath::Abs(L2_TRUEID) == 211)'
        tm_dt1 = 'TMath::Abs(L1_MC_MOTHER_ID)  == 521 || TMath::Abs(L1_MC_MOTHER_ID) == 421'
        tm_dt2 = 'TMath::Abs(L2_MC_MOTHER_ID)  == 521 || TMath::Abs(L2_MC_MOTHER_ID) == 421'
        cut    = f'({tm_par}) && ({tm_dt1}) && ({tm_dt2}) && TMath::Abs(H_TRUEID) == 321 &&  TMath::Abs(H_MC_MOTHER_ID) == 421'
    #------------------------------------------------------------
    elif event_type == 'bdpsi2kst_ee':
        tm_par = 'TMath::Abs(B_TRUEID)        == 511    && TMath::Abs(L1_TRUEID)       == 11     && TMath::Abs(L2_TRUEID)   == 11 && TMath::Abs(H_TRUEID) == 321'
        tm_psi = 'TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(Jpsi_TRUEID) == 100443'
        tm_kst = 'TMath::Abs(H_MC_MOTHER_ID)  == 313'

        cut    = f'{tm_par} && {tm_psi} && {tm_kst}'
    elif event_type == 'bdpsi2kst_mm':
        tm_par = 'TMath::Abs(B_TRUEID)        == 511    && TMath::Abs(L1_TRUEID)       == 13     && TMath::Abs(L2_TRUEID)   == 13 && TMath::Abs(H_TRUEID) == 321'
        tm_psi = 'TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(Jpsi_TRUEID) == 100443'
        tm_kst = 'TMath::Abs(H_MC_MOTHER_ID)  == 313'

        cut    = f'{tm_par} && {tm_psi} && {tm_kst}'
    #------------------------------------------------------------
    elif event_type == 'bppsi2kst_ee':
        tm_par = 'TMath::Abs(B_TRUEID)        == 521    && TMath::Abs(L1_TRUEID)       == 11     && TMath::Abs(L2_TRUEID)   == 11 && TMath::Abs(H_TRUEID) == 211'
        tm_psi = 'TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(Jpsi_TRUEID) == 100443'
        tm_kst = 'TMath::Abs(H_MC_MOTHER_ID)  == 323'

        cut    = f'{tm_par} && {tm_psi} && {tm_kst}'
    elif event_type == 'bppsi2kst_mm':
        tm_par = 'TMath::Abs(B_TRUEID)        == 521    && TMath::Abs(L1_TRUEID)       == 13     && TMath::Abs(L2_TRUEID)   == 13 && TMath::Abs(H_TRUEID) == 211'
        tm_psi = 'TMath::Abs(L1_MC_MOTHER_ID) == 100443 && TMath::Abs(L2_MC_MOTHER_ID) == 100443 && TMath::Abs(Jpsi_TRUEID) == 100443'
        tm_kst = 'TMath::Abs(H_MC_MOTHER_ID)  == 323'

        cut    = f'{tm_par} && {tm_psi} && {tm_kst}'
    #------------------------------------------------------------
    elif event_type == 'fail':
        cut= 'TMath::Abs(B_TRUEID) == 0 || TMath::Abs(Jpsi_TRUEID) == 0 || TMath::Abs(Jpsi_MC_MOTHER_ID) == 0 || TMath::Abs(L1_TRUEID) == 0 || TMath::Abs(L2_TRUEID) == 0 || TMath::Abs(L1_MC_MOTHER_ID) == 0 || TMath::Abs(L2_MC_MOTHER_ID) == 0 || TMath::Abs(H_TRUEID) == 0 || TMath::Abs(H_MC_MOTHER_ID) == 0'
    else:
        raise ValueError(f'Event type {event_type} not recognized')

    return cut
