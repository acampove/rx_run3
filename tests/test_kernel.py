import os 

ld_library_path = os.environ.get('LD_LIBRARY_PATH', '')
os.environ['LD_LIBRARY_PATH'] =  f"{ld_library_path}:{os.getcwd()}/build:{os.getcwd()}/build/kernel"

import ROOT 
def _import_cal_rx_run3_():
    loaded = ROOT.gSystem.Load("kernel/libkernel.so")
    print(loaded)
    if loaded ==-1  : 
        raise ImportError( "cannot load libkernel.so compiled code")


if __name__ == "__main__" : 
    _import_cal_rx_run3_()     
    st = ROOT.LHCbStyle()
    ROOT.SettingDef.IO.exe  = "test_kernel.py"    
    """
    globally configured version
    ROOT.SettingDef.Config.project = "RK"
    ROOT.SettingDef.Config.ana = "MM"
    ROOT.SettingDef.Config.q2bin = "jpsi"
    ROOT.SettingDef.Config.year = "24"
    ROOT.SettingDef.Config.polarity="MD"
    ROOT.SettingDef.Config.trigger=""
    ROOT.SettingDef.Config.triggerConf="inclusive"
    ROOT.SettingDef.Config.brem=""
    # ROOT.SettingDef.Config.track="PASS" #or TAG etc.. (to make categories in fits for example)
    # ROOT.SettingDef.Config.track="FAIL" #or PRB etc.. (to make categories in fits for example)
    """
    #fields are : 
    # - project [RK,RKst,RPhi]
    # - analysis [EE,MM]
    # - sampleName [LPT=data, others needs some updates for run3 eventually] --> this maps to specific BKK of samples within the framework, unfortunately
    # - year [11,12,15,16,17,18, R1, R2p1,R2p2, 24,24b1,24b2...]
    # - polarity [MD,MU,""=all]
    # - trigger [L0I,L0L]
    # - triggerConf [exclusive,exclusive2,inclusive, really needed only for Run1 interaction for L0I/L0L cats]
    # - bremCat ["","0G","1G","2G"]
    # - track(can be LL/DD or anything we can think about ) ["" = default, "pass,fail" extension for use case, or "long,down" if used in LL/DD for Ks, or can extend to anything else]
    # if you supply "global" it will configure ConfigHolder from SettingDef global flags.
    chRKEE = ROOT.ConfigHolder( "RK" , "EE"  , "LPT" , "jps","24b1", "", "", "inclusive", "", "" )    
    chRKEE.Print() 
    
    
    chRKMM = ROOT.ConfigHolder( "RK" , "MM"  , "LPT" , "jps","24b1", "", "", "inclusive", "", "" )    
    chRKMM.Print()
        
        
    chRKstEE = ROOT.ConfigHolder( "RKst" , "EE"  , "LPT" , "jps","24b1", "", "", "inclusive", "", "" )    
    chRKstEE.Print() 
    
    
    chRKstMM = ROOT.ConfigHolder( "RKst" , "MM"  , "LPT" , "jps","24b1", "", "", "inclusive", "", "" )    
    chRKstMM.Print()
                