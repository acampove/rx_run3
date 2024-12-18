#include "OptimizeSelection.hpp"
#include "ParserSvc.hpp"
#include "IOSvc.hpp"

#include "CutHolder.hpp"
#include "EventType.hpp"

#include "AnaPlots.hpp"
#include "BkgComparePlots.hpp"
#include "BkgVetoPlots.hpp"
#include "EfficiencyPlots.hpp"

int main(int argc, char ** argv) {

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    EnableMultiThreads();

    vector< Q2Bin > _vQ2Bins = {Q2Bin::JPsi, Q2Bin::Psi, Q2Bin::Low, Q2Bin::Central, Q2Bin::High};
    TString _weightOption = SettingDef::Weight::option;
    TString _cutOption = SettingDef::Cut::option;
    TString _creVer = SettingDef::Tuple::creVer;
    Year _year = hash_year(SettingDef::Config::year);
    TString _prj = SettingDef::Config::project;
    TString _ana = SettingDef::Config::ana;
    vector<TString> _bkgSamples = {};
    if (_ana == "EE")
        _bkgSamples = {"Bs2PhiJPsEE", "Bs2PhiEE", "Lb2pKPsiEE", "Lb2pKJPsEE", "Lb2pKEE", "Bd2KstJPsEE_HH", "Bd2KstJPsEE_HL", "Bd2KstPsiEE_HH", "Bd2KstPsiEE_HL", "Bd2DNuKstNuEE", "Bd2DstNuDPiKPiEE", "Bu2KPiPiJPsEE", "Bu2KPiPiEE", "Bu2KJPsEE", "Bu2KEE", "Bd2KstEE_HH"};
        // _bkgSamples = {"Bd2KstEE"};
    else
        _bkgSamples = {"Bs2PhiJPsMM", "Bs2PhiMM", "Lb2pKPsiMM", "Lb2pKJPsMM", "Lb2pKMM", "Bd2KstJPsMM_HH", "Bd2KstJPsMM_HL", "Bd2KstPsiMM_HH", "Bd2KstPsiMM_HL", "Bd2DNuKstNuMM", "Bd2DstNuDPiKPiMM", "Bu2KPiPiJPsMM", "Bu2KPiPiMM", "Bu2KJPsMM", "Bu2KMM", "Bd2KstMM_HH"};
    TString _basePath("./");
    IOSvc::MakeDir(_basePath, OpenMode::WARNING);

    // Initilize summary tables
    LaTable _tableSig;
    TString _tableSigPath = _basePath + Form("Sig_stats_%s_%s_%s.tex", _prj.Data(), _ana.Data(), to_string(_year).Data());
    TString _tableSigName = TString(fmt::format("Expected background efficiencies and yields in signal region for 20{0} {1} \\{2}. If nPassed is $< 20$ a limit at $90\\%$ CL is given, else the expected yield is calculated.", to_string(_year), _ana, _prj));
    _tableSig.Create(_tableSigPath.Data(), _tableSigName.Data(), 5);
    vector< string > _headerSig = {TString("MC 20" + to_string(_year)).Data(), "$q^2$ bin", "nPassed", "efficiency", "bkg yield/limit"};
    _tableSig.AddRow(_headerSig);
    _tableSig.TopRule();
    _tableSig.TopRule();

    LaTable _tableCC;
    TString _tableCCPath = _basePath + Form("CC_stats_%s_%s_%s.tex", _prj.Data(), _ana.Data(), to_string(_year).Data());
    TString _tableCCName = TString(fmt::format("Expected background efficiencies and yields in control region for 20{0} {1} \\{2}. If nPassed is $< 20$ a limit at $90\\%$ CL is given, else the expected yield is calculated.", to_string(_year), _ana, _prj));
    _tableCC.Create(_tableCCPath.Data(), _tableCCName.Data(), 5);
    vector< string > _headerCC = {TString("MC 20" + to_string(_year)).Data(), "$q^2$ bin", "nPassed", "efficiency", "bkg yield/limit"};
    _tableCC.AddRow(_headerCC);
    _tableCC.TopRule();
    _tableCC.TopRule();

    // Setup signal and control event types
    TString _sigSample = "Bd2Kst";
    TString _conSample = _sigSample + "JPs";
    _sigSample += SettingDef::Config::ana;
    _conSample += SettingDef::Config::ana;
    TString _psiSample = _conSample;
    _psiSample = _psiSample.ReplaceAll("JPs", "Psi");

    SettingDef::Tuple::option = "cre";
    SettingDef::Config::sample = _sigSample;
    EventType _sigET           = EventType();
    _sigET.GetTupleHolder().Init();
    SettingDef::Tuple::creVer   = "FIT_noMVA_noHLT_noBKG_noMinPET_newL0I_FIX_exc"; // some creVer with noBKG
    SettingDef::Config::trigger = "L0L";
    SettingDef::Config::q2bin   = "jps";
    SettingDef::Config::sample  = _conSample;
    EventType _conET            = EventType();
    _conET.GetTupleHolder().Init();
    SettingDef::Config::q2bin   = "psi";
    SettingDef::Config::sample  = _psiSample;
    EventType _psiET            = EventType();
    _psiET.GetTupleHolder().Init();
    // SettingDef::Tuple::option = "pro";

    // Re set configs for bkg mode
    SettingDef::Tuple::creVer   = _creVer;
    SettingDef::Config::trigger = "";
    SettingDef::Config::q2bin   = "";
    SettingDef::Weight::option  = _weightOption;
    SettingDef::Config::trigger = "";

    // Loop over bkg modes
    for (auto& _bkgSampleC : _bkgSamples) {
        TString _bkgSample = _bkgSampleC;
        _bkgSample = _bkgSample.ReplaceAll("_HH", "").ReplaceAll("_HL", "").ReplaceAll("_H1L1", "").ReplaceAll("_H2L2", "");
        SettingDef::Config::sample  = _bkgSample;
        if (_bkgSample.Contains("Bs2Phi") && (_year == Year::Y2017 || _year == Year::Y2018)) continue;

        // Set up temp EventType to load cuts
        SettingDef::Config::q2bin  = "jps";
        SettingDef::Cut::option    = "-noTCKCat";
        EventType _cutET    = EventType();
        Analysis  _analysis = _cutET.GetAna();
        _cutET.GetCutHolder().Init();
        _cutET.GetWeightHolder().Init();
        if (!(_cutET.GetTupleHolder().IsSampleInCreVer(SettingDef::Tuple::creVer, _prj, to_string(_analysis), "", to_string(_year) , "", "exclusive", _bkgSample)))
            continue;

        // Set up bkg EventType
        SettingDef::Cut::option   = _cutOption;
        SettingDef::Config::q2bin = "";
        EventType _bkgET    = EventType();
        _bkgET.Init();

        auto oSel = OptimizeSelection(_bkgET, _sigET, _conET, _psiET, _bkgSample);
        if (_bkgSampleC.Contains("_HH"))   // modifys truth-matching options: HH: K<->Pi; HL: K<->L1 && Pi<->L2; H1L1: K<->L1; H2L2: Pi<->L2
            oSel.StudySwap("HH");
        else if (_bkgSampleC.Contains("_HL"))
            oSel.StudySwap("HL");
        else if (_bkgSampleC.Contains("_H1L1"))
            oSel.StudySwap("H1L1");
        else if (_bkgSampleC.Contains("_H2L2"))
            oSel.StudySwap("H2L2");
        oSel.SetBasePath(_basePath);

        vector< pair< TString, TString > > _cutsToCompare;
        TString _head = oSel.GetHead();
        oSel.AddCut("L0", _cutET.GetCutHolder().Cuts().at("cutL0"));
        oSel.AddCut("Hlt1", _cutET.GetCutHolder().Cuts().at("cutHLT1"));
        oSel.AddCut("Hlt2", _cutET.GetCutHolder().Cuts().at("cutHLT2"));
        oSel.AddCut("m_Kst", CutDefRKst::Mass::Kst);
        oSel.AddCut("Quality", _cutET.GetCutHolder().Cuts().at("cutPS") && _cutET.GetCutHolder().Cuts().at("cutSPD"));
        if (_analysis == Analysis::MM) {
            oSel.AddCut("PID_strip", CutDefRKst::PID::pidStripH && CutDefRX::PID::pidStripM);
            oSel.AddCut("ProbNNk", UpdatePIDTune(CutDefRKst::PID::probnnK, to_string(_year)) && CutDefRKst::PID::dllK);
            oSel.AddCut("ProbNNpi", UpdatePIDTune(CutDefRKst::PID::probnnPi, to_string(_year)));
            oSel.AddCut("ProbNNl", UpdatePIDTune(CutDefRKst::PID::probnnM, to_string(_year)));

            oSel.AddCutBkg("Bs2Phi", UpdatePIDTune(CutDefRKst::Background::Bs2Phi, to_string(_year)), "B0_M23_Subst3_pi2K", 1040.);
            oSel.AddCutBkg("Bd2DPiMNuPID", UpdatePIDTune(CutDefRKst::Background::Bd2DPiMNu_PID, to_string(_year)), "B0_M02_Subst0_mu2pi", 30.);
            oSel.AddCutBkg("Bd2DMNuPID", UpdatePIDTune(CutDefRKst::Background::Bd2DMNu_PID, to_string(_year)), "B0_M023_Subst0_mu2pi", 30.);
            oSel.AddCutBkg("misIDJPs_PID", UpdatePIDTune(CutDefRKst::Background::misIDJPsMM_PID, to_string(_year)), "B0_M13_Subst3_pi2mu", 60.);
            oSel.AddCutBkg("misIDPsi_PID", UpdatePIDTune(CutDefRKst::Background::misIDPsiMM_PID, to_string(_year)), "B0_M13_Subst3_pi2mu", 60.);
            oSel.AddCutBkg("Bd2DXMass", CutDefRKst::Background::Bd2DXMass || !(CutDefRKst::Mass::Q2Low || CutDefRKst::Mass::Q2Central), "B0_M023", 1780.);
            oSel.AddCutBkg("Bu2K", CutDefRKst::Background::Bu2K, "TMath::Max(B0_M012, B0_M013_Subst3_pi2K)", 5100.);
            
            // non default cuts
            // oSel.AddCutBkg("PartReco", (TCut)((TString) CutDefRX::Background::partRecoJPsMM).ReplaceAll("{HEAD}_", _head) || !CutDefRX::Mass::JPsMM, "B0_DTF_JPs_M", 5150.);

            if (_bkgSample.Contains("Bd2KstJPsMM")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                } else if (oSel.GetSwapOption().Contains("HL")) {
                    oSel.AddCutToStudy("misIDJPs_PID");
                    // oSel.AddCutBkg("misIDJPs_PID2", UpdatePIDTune(CutDefRKst::Background::misIDJPsMM_PID, to_string(_year)), "B0_M02_Subst2_K2mu", 60.);
                    // oSel.AddCutToStudy("misIDJPs_PID2");
                }
            } else if (_bkgSample.Contains("Bd2KstPsiMM")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                } else if (oSel.GetSwapOption().Contains("HL")) {
                    oSel.AddCutToStudy("misIDPsi_PID");
                    // oSel.AddCutBkg("misIDPsi_PID2", UpdatePIDTune(CutDefRKst::Background::misIDPsiMM_PID, to_string(_year)), "B0_M02_Subst2_K2mu", 60.);
                    // oSel.AddCutToStudy("misIDPsi_PID2");
                }
            } else if (_bkgSample.Contains("Bd2KstMM")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                }
            } else if (_bkgSample.Contains("Bs2Phi")) {
                oSel.AddCutToStudy("Bs2Phi");
            } else if (_bkgSample.Contains("Lb2pK")) {
                oSel.Draw("B0_M0123_Subst3_pi2p");
            } else if (_bkgSample.Contains("Bu2KPiPiJPsMM")) {
                oSel.Draw("B0_DTF_JPs_M");
            } else if (_bkgSample.Contains("Bu2KPiPiMM")) {
                oSel.Draw("B0_DTF_M");
            } else if (_bkgSample.Contains("Bu2K")) {
                oSel.AddCutToStudy("Bu2K");
            } else if (_bkgSample.Contains("Bd2DNuKstNuMM")) {
                oSel.AddCutBkg("Bd2DX", CutDefRKst::Background::Bd2DX || !(CutDefRKst::Mass::Q2Low || CutDefRKst::Mass::Q2Central), "TMath::Abs(TMath::Cos(B0_ThetaL_custom))", 0.8);
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DX");
                _cutsToCompare.push_back(make_pair("Bd2DX", "Bd2DXMass"));
            } else if (_bkgSample.Contains("Bd2DPiMM")) {
                oSel.AddCutToStudy("Bd2DMNuPID");
            } else if (_bkgSample.Contains("Bd2DstNuDPiKPiMM")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiMNuPID");
            } else if (_bkgSample.Contains("Bd2DstNuD0PiKPiMM")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiMNuPID");
            } else if (_bkgSample.Contains("Bd2D0XNuMM")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiMNuPID");
                oSel.AddCutToStudy("Bd2DMNuPID");
            }
        }
        if (_analysis == Analysis::EE) {
            oSel.AddCut("PID_strip", CutDefRKst::PID::pidStripH && CutDefRX::PID::pidStripE);
            oSel.AddCut("ProbNNk", UpdatePIDTune(CutDefRKst::PID::probnnK, to_string(_year)) && CutDefRKst::PID::dllK);
            oSel.AddCut("ProbNNpi", UpdatePIDTune(CutDefRKst::PID::probnnPi, to_string(_year)));
            oSel.AddCut("ProbNNl", UpdatePIDTune(CutDefRKst::PID::probnnE, to_string(_year)) && CutDefRKst::PID::dllE);
            oSel.AddCutBkg("Bs2Phi", UpdatePIDTune(CutDefRKst::Background::Bs2Phi, to_string(_year)), "B0_M23_Subst3_pi2K", 1040.);
            oSel.AddCutBkg("Bd2DPiENuPID", UpdatePIDTune(CutDefRKst::Background::Bd2DPiENu_PID, to_string(_year)), "B0_TRACK_M12_Subst1_e2pi", 30.);
            oSel.AddCutBkg("Bd2DENuPID", UpdatePIDTune(CutDefRKst::Background::Bd2DENu_PID, to_string(_year)), "B0_TRACK_M023_Subst0_e2pi", 30.);
            oSel.AddCutBkg("DSwapJPsConstrPID", UpdatePIDTune(CutDefRKst::Background::misIDJPsEE_PID, to_string(_year)), "B0_TRACK_M_Subst_lpi2pil_DTF_JPs", 60.);
            oSel.AddCutBkg("DSwapPsiConstrPID", UpdatePIDTune(CutDefRKst::Background::misIDPsiEE_PID, to_string(_year)), "B0_TRACK_M_Subst_lpi2pil_DTF_Psi", 60.);
            oSel.AddCutBkg("Bd2DXMass", CutDefRKst::Background::Bd2DXMass || !(CutDefRKst::Mass::Q2Low || CutDefRKst::Mass::Q2Central), "B0_M023", 1780.);
            oSel.AddCutBkg("Bu2K", CutDefRKst::Background::Bu2K, "TMath::Max(B0_M012, B0_M013_Subst3_pi2K)", 5100.);

            // non default cuts
            // oSel.AddCutBkg("Bd2DPiENu", CutDefRKst::Background::Bd2DPiENu, "B0_TRACK_M12_Subst1_e2pi", 30.);
            // oSel.AddCutBkg("Bd2DENu", CutDefRKst::Background::Bd2DENu, "B0_M023_Subst0_e2pi", 30.);
            // oSel.AddCutBkg("DSwapK", "TMath::Abs(B0_M02_Subst2_K2e - 3096.916) > 60 && TMath::Abs(B0_M13_Subst3_pi2e - 3096.916) > 60", "B0_M02_Subst2_K2e", 60.);
            // oSel.AddCutBkg("DSwapPi", "TMath::Abs(B0_M02_Subst2_K2e - 3096.916) > 60 && TMath::Abs(B0_M13_Subst3_pi2e - 3096.916) > 60", "B0_M13_Subst3_pi2e", 60.);
            // oSel.AddCutBkg("PartReco", ((TCut)((TString) CutDefRX::Background::partRecoJPsEE).ReplaceAll("{HEAD}_", _head)) || !CutDefRX::Mass::JPsEE, "B0_DTF_JPs_M", 5150.);

            if (_bkgSample.Contains("Bd2KstJPsEE")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                    // oSel.Draw("B0_M23_Subst23_Kpi2piK");
                } else if (oSel.GetSwapOption().Contains("HL")) {
                    oSel.AddCutToStudy("DSwapJPsConstrPID");
                } else if (oSel.GetSwapOption().Contains("H1L1")) {
                    oSel.AddCutBkg("DSwapJPsConstrPID2", UpdatePIDTune(CutDefRKst::Background::misIDJPsEE_PID, to_string(_year)), "B0_TRACK_M_Subst_Kl2lK_DTF_JPs", 60.);
                    oSel.AddCutToStudy("DSwapJPsConstrPID2");
                } else if (oSel.GetSwapOption().Contains("H2L2")) {
                    oSel.AddCutToStudy("DSwapJPsConstrPID");
                }
                    // oSel.AddCutBkg("DSwapJPsConstr", CutDefRKst::Background::misIDJPsEE, "B0_TRACK_M_Subst_lpi2pil_DTF_JPs", 60.);
                    // oSel.AddCutToStudy("DSwapJPsConstr");
                    // oSel.AddCutBkg("DSwapJPsConstr2", CutDefRKst::Background::misIDJPsEE, "B0_TRACK_M_Subst_Kl2lK_DTF_JPs", 60.);
                    // oSel.AddCutToStudy("DSwapJPsConstr2");
                    // oSel.AddCutBkg("DSwapKst", CutDefRKst::Background::misIDKst, "B0_TRACK_M03_Subst0_e2K", 80.);
                    // oSel.AddCutToStudy("DSwapKst");
                    // oSel.AddCutBkg("DSwapKstPID", UpdatePIDTune(CutDefRKst::Background::misIDKst_PID, to_string(_year)), "B0_TRACK_M03_Subst0_e2K", 80.);
                    // oSel.AddCutToStudy("DSwapKstPID");
                    // oSel.AddCutBkg("DSwapKst2", CutDefRKst::Background::misIDKst, "B0_TRACK_M12_Subst1_e2pi", 80.);
                    // oSel.AddCutToStudy("DSwapKst2");
                    // oSel.AddCutBkg("DSwapKstPID2", UpdatePIDTune(CutDefRKst::Background::misIDKst_PID, to_string(_year)), "B0_TRACK_M12_Subst1_e2pi", 80.);
                    // oSel.AddCutToStudy("DSwapKstPID2");
            } else if (_bkgSample.Contains("Bd2KstPsiEE")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                } else if (oSel.GetSwapOption().Contains("HL")) {
                    oSel.AddCutToStudy("DSwapPsiConstrPID");
                    // oSel.AddCutBkg("DSwapPsiConstr", CutDefRKst::Background::misIDPsiEE, "B0_TRACK_M_Subst_lpi2pil_DTF_Psi", 60.);
                    // oSel.AddCutToStudy("DSwapPsiConstr");
                    // oSel.AddCutBkg("DSwapPsiConstr2", CutDefRKst::Background::misIDPsiEE, "B0_TRACK_M_Subst_Kl2lK_DTF_Psi", 60.);
                    // oSel.AddCutToStudy("DSwapPsiConstr2");
                    // oSel.AddCutBkg("DSwapPsiConstrPID2", UpdatePIDTune(CutDefRKst::Background::misIDPsiEE_PID, to_string(_year)), "B0_TRACK_M_Subst_Kl2lK_DTF_Psi", 60.);
                    // oSel.AddCutToStudy("DSwapPsiConstrPID2");
                }
            } else if (_bkgSample.Contains("Bd2KstEE")) {
                if (oSel.GetSwapOption().Contains("HH")) {
                    oSel.AddCutBkg("DSwapHad", CutDefRKst::Background::misIDHad, "B0_M23_Subst23_Kpi2piK", 50.);
                    oSel.AddCutToStudy("DSwapHad");
                }
            } else if (_bkgSample.Contains("Bs2Phi")) {
                oSel.AddCutToStudy("Bs2Phi");
            } else if (_bkgSample.Contains("Lb2pK")) {
                // oSel.AddCutBkg("Lb2pK", "1>0", "B0_M0123_Subst3_pi2p", 0.);
                oSel.Draw("B0_M0123_Subst3_pi2p");
            } else if (_bkgSample.Contains("Bu2KPiPiJPsEE")) {
                oSel.Draw("B0_DTF_JPs_M");
            } else if (_bkgSample.Contains("Bu2KPiPiEE")) {
                oSel.Draw("B0_DTF_M");
            } else if (_bkgSample.Contains("Bu2K")) {
                oSel.AddCutToStudy("Bu2K");
            } else if (_bkgSample.Contains("Bd2DNuKstNuEE")) {
                oSel.AddCutBkg("Bd2DX", CutDefRKst::Background::Bd2DX || !(CutDefRKst::Mass::Q2Low || CutDefRKst::Mass::Q2Central), "TMath::Abs(TMath::Cos(B0_ThetaL_custom))", 0.8);
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DX");
                // _cutsToCompare.push_back(make_pair("Bd2DX", "Bd2DXMass"));
            } else if (_bkgSample.Contains("Bd2DPiEE")) {
                oSel.AddCutToStudy("Bd2DENuPID");
            } else if (_bkgSample.Contains("Bd2DstNuDPiKPiEE")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiENuPID");
            } else if (_bkgSample.Contains("Bd2DNuKstPiEE")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiENuPID");
                oSel.AddCutToStudy("Bd2DENuPID");
            } else if (_bkgSample.Contains("Bd2DstNuD0PiKPiEE")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiENuPID");
            } else if (_bkgSample.Contains("Bd2D0XNuEE")) {
                oSel.AddCutToStudy("Bd2DXMass");
                oSel.AddCutToStudy("Bd2DPiENuPID");
                oSel.AddCutToStudy("Bd2DENuPID");
            }
        }

        oSel.PrintCuts();
        oSel.TestPreSelection();
        for (auto & _q2Bin : _vQ2Bins) {
            TString _q2Str = to_string(_q2Bin);

            vector< AnaPlots * > _anaPlots;
            _anaPlots.push_back(new EfficiencyPlots(_q2Str, oSel.GetSwapOption()));
            for (auto & _sel : oSel.GetCutsToStudy()) _anaPlots.push_back(new BkgVetoPlots(_q2Str, _sel));
            if (oSel.GetDrawVar().var != "NoVar") _anaPlots.push_back(new BkgVetoPlots(_q2Str, "NoSel"));
            for (auto & _comp : _cutsToCompare) _anaPlots.push_back(new BkgComparePlots(_q2Str, _comp.first, _comp.second));
            for (auto _anaPlot : _anaPlots) {
                _anaPlot->Initialize(oSel);
                _anaPlot->Fill(oSel);
                _anaPlot->Finalize(oSel);
                _anaPlot->Draw(oSel);
            }
            vector< TCanvas * > _plotsToBeSaved;
            for (auto _anaPlot : _anaPlots) {
                vector< TCanvas * > _padsFromThisPlot = _anaPlot->SavePlots();
                _plotsToBeSaved.insert(_plotsToBeSaved.end(), _padsFromThisPlot.begin(), _padsFromThisPlot.end());
            }
            for (auto _plot : _plotsToBeSaved) { _plot->Print(oSel.GetPath() + to_string(_q2Bin) + "/" + TString(_plot->GetName()) + ".pdf"); }
        }
        oSel.CalcExpectedYield(_tableSig, _tableCC);
    }
    _tableSig.BottomRule();
    _tableSig.Close();
    _tableCC.BottomRule();
    _tableCC.Close();
    DisableMultiThreads();

    return 0;
}
