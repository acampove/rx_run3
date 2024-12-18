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
        // _bkgSamples = {"Bu2PiJPsEE", "Bu2KJPsEE_HL", "Bu2KPsiEE_HL", "Bu2DKENuPiEE", "Bu2DKPiENuEE", "Bu2DKNuNuEE", "Bd2KstEE", "Bd2KstJPsEE", "Bd2KstPsiEE", "Bd2KPiEE"};
        _bkgSamples = {"Bu2DKNuNuEE"};
    else
        _bkgSamples = {"Bu2PiJPsMM", "Bu2KJPsMM_HL", "Bu2KPsiMM_HL", "Bu2DKMuNuPiMM", "Bu2DKPiMuNuMM", "Bd2KstMM", "Bd2KstJPsMM", "Bd2KstPsiMM", "Bd2KPiMM"};
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
    TString _sigSample = "Bu2K";
    TString _conSample = _sigSample + "JPs";
    _sigSample += SettingDef::Config::ana;
    _conSample += SettingDef::Config::ana;
    TString _psiSample = _conSample;
    _psiSample = _psiSample.ReplaceAll("JPs", "Psi");
    
    SettingDef::Tuple::option = "cre";
    SettingDef::Config::sample = _sigSample;
    EventType _sigET           = EventType();
    _sigET.GetTupleHolder().Init();
    SettingDef::Tuple::creVer   = "FIT_noMVA_noHLT_noBKG_sescher_FIX_exc"; // some creVer with noBKG
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
        _bkgSample = _bkgSample.ReplaceAll("_HL", "");
        SettingDef::Config::sample  = _bkgSample;

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
        if (_bkgSampleC.Contains("_HL"))   // modifys truth-matching options: only sensible option for RK: HL: H1<->L1
            oSel.StudySwap("HL");
        oSel.SetBasePath(_basePath);

        vector< pair< TString, TString > > _cutsToCompare;
        TString _head = oSel.GetHead();
        oSel.AddCut("L0", _cutET.GetCutHolder().Cuts().at("cutL0"));
        oSel.AddCut("Hlt1", _cutET.GetCutHolder().Cuts().at("cutHLT1"));
        oSel.AddCut("Hlt2", _cutET.GetCutHolder().Cuts().at("cutHLT2"));
        oSel.AddCut("Quality", _cutET.GetCutHolder().Cuts().at("cutPS") && _cutET.GetCutHolder().Cuts().at("cutSPD"));
        if (_analysis == Analysis::MM) {
            oSel.AddCut("PID_strip", CutDefRK::PID::pidStripH && CutDefRX::PID::pidStripM);
            oSel.AddCut("ProbNNk", UpdatePIDTune(CutDefRK::PID::probnnK, to_string(_year)) && CutDefRK::PID::dllK);
            oSel.AddCut("ProbNNl", UpdatePIDTune(CutDefRK::PID::probnnM, to_string(_year)));

            oSel.AddCutBkg("Bu2DMNuPID", UpdatePIDTune(CutDefRK::Background::Bu2DMNu_PID, to_string(_year)), "Bp_M02_Subst0_mu2pi", 1885.);
            oSel.AddCutBkg("misIDJPs_PID", UpdatePIDTune(CutDefRK::Background::misIDJPsMM_PID, to_string(_year)), "Bp_M02_Subst2_K2mu", 60.);
            oSel.AddCutBkg("misIDPsi_PID", UpdatePIDTune(CutDefRK::Background::misIDPsiMM_PID, to_string(_year)), "Bp_M02_Subst2_K2mu", 60.);
            oSel.AddCutBkg("Bu2DXMass", CutDefRK::Background::D2Klnu, "Bp_M02", 1885.);
            
            // oSel.AddCutBkg("Bu2DX", CutDefRK::Background::Bu2DX, "TMath::Abs(TMath::Cos(Bp_ThetaL))", 0.8);
            // oSel.AddCutBkg("PartReco", (TCut)((TString) CutDefRX::Background::partRecoJPsEE).ReplaceAll("{HEAD}_", _head) || !CutDefRX::Mass::JPsMM, "Bp_DTF_JPs_M", 5150.);
            if (_bkgSample.Contains("Bu2KJPs")) {
                oSel.AddCutToStudy("misIDJPs_PID");
                // oSel.AddCutBkg("misIDJPs", CutDefRK::Background::misIDJPsMM, "Bp_M02_Subst2_K2mu", 60.);
                // oSel.AddCutToStudy("misIDJPs");
            } else if (_bkgSample.Contains("Bu2KPsi")) {
                oSel.AddCutToStudy("misIDPsi_PID");
                // oSel.AddCutBkg("misIDPsi", CutDefRK::Background::misIDPsiMM, "Bp_M02_Subst2_K2mu", 60.);
                // oSel.AddCutToStudy("misIDPsi");
            } else if (_bkgSample.Contains("Bu2PiJPs")) {
                oSel.Draw("Bp_M012_Subst2_K2pi");
            } else if (_bkgSample.Contains("Bd2KstPsi")) {
                oSel.Draw("Bp_DTF_Psi_M");
            } else if (_bkgSample.Contains("Bd2KstJPs")) {
                oSel.Draw("Bp_DTF_JPs_M");
            } else if (_bkgSample.Contains("Bd2Kst")) {
                oSel.Draw("Bp_DTF_M");
            } else if (_bkgSample.Contains("Bu2DKNuNuMM")) {
                oSel.AddCutToStudy("Bu2DXMass");
                // oSel.AddCutBkg("Bu2DX", CutDefRK::Background::Bu2DX, "TMath::Abs(TMath::Cos(Bp_ThetaL_custom))", 0.8);
                // oSel.AddCutToStudy("Bu2DX");
            } else if (_bkgSample.Contains("Bu2DKMuNuPi")) {
                oSel.AddCutToStudy("Bu2DXMass");
            } else if (_bkgSample.Contains("Bu2DKPiMuNu")) {
                oSel.AddCutToStudy("Bu2DMNuPID");
                // oSel.AddCutBkg("Bu2DMNu", CutDefRK::Background::Bu2DMNu, "Bp_TRACK_M12_Subst1_mu2pi", 40.);
                // oSel.AddCutToStudy("Bu2DMNu");
            }
        }
        if (_analysis == Analysis::EE) {
            oSel.AddCut("PID_strip", CutDefRK::PID::pidStripH && CutDefRX::PID::pidStripE);
            oSel.AddCut("ProbNNk", UpdatePIDTune(CutDefRK::PID::probnnK, to_string(_year)) && CutDefRK::PID::dllK);
            oSel.AddCut("ProbNNl", UpdatePIDTune(CutDefRK::PID::probnnE, to_string(_year)) && CutDefRK::PID::dllE);

            oSel.AddCutBkg("Bu2DENuPID", UpdatePIDTune(CutDefRK::Background::Bu2DENu_PID, to_string(_year)), "Bp_TRACK_M12_Subst1_e2pi", 40.);
            oSel.AddCutBkg("DSwapJPsConstrPID", UpdatePIDTune(CutDefRK::Background::misIDJPsEE_PID, to_string(_year)), "Bp_TRACK_M_Subst_Kl2lK_DTF_JPs", 60.);
            oSel.AddCutBkg("DSwapPsiConstrPID", UpdatePIDTune(CutDefRK::Background::misIDPsiEE_PID, to_string(_year)), "Bp_TRACK_M_Subst_Kl2lK_DTF_Psi", 60.);
            oSel.AddCutBkg("Bu2DXMass", CutDefRK::Background::D2Klnu, "Bp_M02", 1885.);

            // oSel.AddCutBkg("DSwapK", "TMath::Abs(Bp_M02_Subst2_K2e - 3096.916) > 60 && TMath::Abs(Bp_M02_Subst2_K2e - 3686.109) > 60", "Bp_M02_Subst2_K2e", 60.);
            // oSel.AddCutBkg("PartReco", (TCut)((TString) CutDefRX::Background::partRecoJPsEE).ReplaceAll("{HEAD}_", _head) || !CutDefRX::Mass::JPsEE, "Bp_DTF_JPs_M", 5150.);
            if (_bkgSample.Contains("Bu2KJPs")) {
                oSel.AddCutToStudy("DSwapJPsConstrPID");
                // oSel.AddCutBkg("DSwapJPsConstr", CutDefRK::Background::misIDJPsEE, "Bp_TRACK_M_Subst_Kl2lK_DTF_JPs", 60.);
                // oSel.AddCutToStudy("DSwapJPsConstr");
            } else if (_bkgSample.Contains("Bu2KPsi")) {
                oSel.AddCutToStudy("DSwapPsiConstrPID");
                // oSel.AddCutBkg("DSwapPsiConstr", CutDefRK::Background::misIDPsiEE, "Bp_TRACK_M_Subst_Kl2lK_DTF_Psi", 60.);
                // oSel.AddCutToStudy("DSwapPsiConstr");
            } else if (_bkgSample.Contains("Bu2PiJPs")) {
                oSel.Draw("Bp_M012_Subst2_K2pi");
            } else if (_bkgSample.Contains("Bd2KstPsi")) {
                oSel.Draw("Bp_DTF_Psi_M");
            } else if (_bkgSample.Contains("Bd2KstJPs")) {
                oSel.Draw("Bp_DTF_JPs_M");
            } else if (_bkgSample.Contains("Bd2Kst")) {
                oSel.Draw("Bp_DTF_M");
            } else if (_bkgSample.Contains("Bu2DKNuNuEE")) {
                oSel.AddCutToStudy("Bu2DXMass");
                oSel.AddCutBkg("Bu2DX", CutDefRK::Background::Bu2DX, "TMath::Abs(TMath::Cos(Bp_ThetaL_custom))", 0.8);
                oSel.AddCutBkg("Bu2DX", CutDefRK::Background::Bu2DX || !(CutDefRK::Mass::Q2Low || CutDefRK::Mass::Q2Central), "TMath::Abs(TMath::Cos(Bp_ThetaL_custom))", 0.8);
                oSel.AddCutToStudy("Bu2DX");
            } else if (_bkgSample.Contains("Bu2DKENuPi")) {
                oSel.AddCutToStudy("Bu2DXMass");
            } else if (_bkgSample.Contains("Bu2DKPiENuEE")) {
                oSel.AddCutToStudy("Bu2DENuPID");
                // oSel.AddCutBkg("Bu2DENu", CutDefRK::Background::Bu2DENu, "Bp_TRACK_M12_Subst1_e2pi", 40.);
                // oSel.AddCutToStudy("Bu2DENu");
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
