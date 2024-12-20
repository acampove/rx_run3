import os 
import tempfile

ld_library_path = os.environ.get('LD_LIBRARY_PATH', '')
os.environ['LD_LIBRARY_PATH'] =  f"{ld_library_path}:{os.getcwd()}/build:{os.getcwd()}/build/kernel"

import ROOT 
def _import_cal_rx_run3_():
    loaded = ROOT.gSystem.Load("kernel/libkernel.so")
    print(loaded)
    if loaded ==-1  : 
        raise ImportError( "cannot load libkernel.so compiled code")



if __name__ == "__main__": 
    
    yaml_content = """
# define your categories of cuts
Categories :     
    electron_acceptance       : "(L1_INECAL==1 && L2_INECAL==1 && L1_PPHASRICH==1 && L2_PPHASRICH==1)"
    hadron_acceptance         : "(K1_PPHASRICH==1 && K2_PPHASRICH==1)"
    pid_electrons             : "(L1_PID_E > 2 && L2_PID_E > 2)"
    pid_kaons                 : "(K1_PID_K > 3 && K2_PID_K > 3)" 
    dihadron_mass_loose       : "(TMath::Abs(Phi_M - 1020) < 50)"
    dihadron_mass_tight       : "(TMath::Abs(Phi_M - 1020) < 20)"
    electron_kinematic        : "(L1_PT > 500 && L2_PT > 500)"
    hadron_kinematic          : "(K1_PT > 250 && K2_PT > 25)"
    long_tracks               : "(L1_TRACKTYPE==3 && L2_TRACKTYPE==3 && K1_TRACKTYPE==3 && K2_TRACKTYPE==3)"
    nobrem                    : "((L1_HASBREMADDED + L2_HASBREMADDED)==0)"
    onebrem                   : "((L1_HASBREMADDED + L2_HASBREMADDED)==1)"
    twobrem                   : "((L1_HASBREMADDED + L2_HASBREMADDED)> 1)"
    jpsi_qsquare              : "(TMath::Sq(Jpsi_M/1000)>6 && TMath::Sq(Jpsi_M/1000)<11)"
    gamma_qsquare             : "Jpsi_M < 50"
    vlow_qsquare              : "Jpsi_M > 50 && Jpsi_M < 1000"
    trigger                   : "(B_Hlt1DiElectronDisplacedDecision_TOS==1 || B_Hlt1TrackElectronMVADecision_TOS || B_Hlt1TrackMVADecision_TOS==1 || B_Hlt1TwoTrackMVADecision_TOS==1)"
    # rx analysis had 
    clone_veto                : "(L1K1_TRACK_OA>0.001 &&  L1K2_TRACK_OA>0.001 && L2K1_TRACK_OA>0.001 && L2K2_TRACK_OA>0.001 && K1K2_TRACK_OA>0.001)"
# define various selection 'chains' by keywords (see reader.py for info on cut-reader)
Selections : 
    jpsi_predefine :
        - jpsi_qsquare
        - electron_acceptance
        - hadron_acceptance
        - pid_electrons
        - pid_kaons
        - dihadron_mass_loose
        - long_tracks
        - trigger
    jpsi_postdefine : 
        - clone_veto
        - electron_kinematic
        - hadron_kinematic
"""
    with tempfile.NamedTemporaryFile(suffix=".yaml", delete=False, mode="w") as temp_file:
        temp_file.write(yaml_content)
        temp_yaml_path = temp_file.name
    
    print(f"Temporary YAML file created at: {temp_yaml_path}")

    # this is just an example for normal YamlCutReader usage
    SelectionReader = ROOT.YamlCutReader(temp_yaml_path)
    SelectionReader.PrintData()
    cuts_pre  = [ sel.GetTitle() for sel in SelectionReader.GetSelections("jpsi_predefine")]
    cuts_post = [ sel.GetTitle() for sel in SelectionReader.GetSelections("jpsi_postdefine")]
    print(cuts_pre)
    print(cuts_post)
    
    #this is just an example of using CutHolder combined to reader-mode (useful for Run3!)    
    configH = ROOT.ConfigHolder( "RK" , "EE"  , "LPT" , "jps","24b1", "", "", "inclusive", "", "" )    
    cutHold = ROOT.CutHolder( configH, f"-yaml[{temp_yaml_path}:jpsi_predefine]")
    cutHold.Init()
    cutHold.PrintInline()
    cutHold.PrintCuts()
    
