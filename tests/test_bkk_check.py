'''
Module with tests for BkkChecker class
'''

from ap_utilities.bookkeeping.bkk_checker import BkkChecker 

obj=BkkChecker('all_samples.yaml')
obj.save_existing(path='existing_samples.yaml')

l_sample = [
        ("Bu_KMM"                             , "12113002" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_KMM_PHSP"                        , "12113004" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_KEE"                             , "12123001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_KEE_BALL05"                      , "12123003" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiK_MM"                        , "12143001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiK_EE"                        , "12153001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiPi_EE"                       , "12153020" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiPi_MM"                       , "12143010" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_Psi2SK_MM"                       , "12143020" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_Psi2SK_EE"                       , "12153012" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_JPsiPhi_MM_CPV_Update2016_DG0"   , "13144010" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_JPsiPhi_MM_CPV_Update2016"       , "13144011" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_JPsiKst_MM"                      , "11144001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_Psi2SKst_MM"                     , "11144011" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_JPsiKst_EE"                      , "11154001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_Psi2SKst_EE"                     , "11154011" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstMM"                           , "11114002" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstMM_FlatQ2"                    , "11114014" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstEE"                           , "11124002" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KPiEE"                           , "11124037" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_PhiEE_BALL"                      , "13124006" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_KstEE"                          , "12125101",  "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ('Bd_JPsiKS_MM'                      , '11144103' , '2024.W31.34', 'MagUp'  , 'sim10-2024.Q3.4-v1.3-mu100', 'dddb-20240427', "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ('Bd_JPsiKS_EE'                      , '11154100' , '2024.W31.34', 'MagUp'  , 'sim10-2024.Q3.4-v1.3-mu100', 'dddb-20240427', "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ('Lb_JPsiLz_MM'                      , '15144100' , '2024.W31.34', 'MagUp'  , 'sim10-2024.Q3.4-v1.3-mu100', 'dddb-20240427', "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ('Lb_JPsiLz_EE'                      , '15154105' , '2024.W31.34', 'MagUp'  , 'sim10-2024.Q3.4-v1.3-mu100', 'dddb-20240427', "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstPi0_EEGDalitz"                , "11124402" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_JPsiX_EE"                        , "11453001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiX_EE"                        , "12952000" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_JPsiX_EE"                        , "13454001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Lb_JPsiX_EE"                        , "15454101" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_JPsiX_MM"                        , "11442001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_JPsiX_MM"                        , "12442001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_JPsiX_MM"                        , "13442001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Lb_JPsipKX_MM"                      , "15444001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Lb_JPsiX_MM"                        , "15442001" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("incl_Upsilon1S_MM"                  , "18112001", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),     
        ("incl_Upsilon2S_MM"                  , "18112011", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),     
        ("incl_Upsilon3S_MM"                  , "18112021", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),     
        ("incl_JPsi_MM"                       , "24142001", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),     
        ("incl_Psi2S_MM"                      , "28142001", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Dst_D0pi_KPi"                       , "27163003", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Lc_pKpi_res"                        , "25203000", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bc_JPsiPi_MM"                       , "14143013", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "BcVegPyPythia8" ),
        ("Bc_PiMM"                            , "14113032", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "BcVegPyPythia8" ),
        ("Bc_Dsst2573mumu_KKpi"               , "14175051", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "BcVegPyPythia8" ),
        ("Bu_L0pMM"                           , "12115191", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bu_L0pJPsi_MM"                      , "12145122", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstG_HighPtG"                     , "11102202", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bs_PhiG_HighPtG"                     , "13102202", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Bd_KstG_HighPtG_SS2"          , "11102202", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bs_PhiG_HighPtG_SS2"          , "13102202", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bu_KstG_HighPtG_SS2"          , "12203302", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bd_KstPi0G_GG_SS2"            , "11102211", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bd_KstEta_GG_SS2"             , "11102441", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bs_PhiEta_GG_SS2"             , "13102464", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Bs_PhiPi0_GG_SS2"             , "13102465", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d-SplitSim02", "Pythia8" ),
        ("Xib_XiMM"                           , "16115135" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xib_XiJPsi_MM"                      , "16145135" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xib_XiPsi_MM"                       , "16145137" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xib_XiEE"                           , "16125130" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xib_XiJPsi_EE"                      , "16155130" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xib_XiPsi_EE"                       , "16155131" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Omb_OmMM"                           , "16115139" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Omb_OmJPsi_MM"                      , "16145935" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Omb_OmPsi_MM"                       , "16145937" , "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xi0_ppi"      , "35102023", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" ),
        ("Xim_Lambdapi" , "35103104", "2024.W31.34", "MagUp"  , "sim10-2024.Q3.4-v1.3-mu100", "dddb-20240427", "Nu6.3", "Nu6p3", "Sim10d", "Pythia8" )
  ]


for sample in l_sample:
    sample_id, event_type, mc_path, polarity, conddb_tag, dddb_tag, nu_path, nuval, sim_version, generator = sample 

    bk_query = f'/MC/{sample_id}/Beam6800GeV-{mc_path}-{polarity}-{nu_path}-25ns-{generator}/{sim_version}/HLT2-2024.W31.34/{event_type}/DST'


