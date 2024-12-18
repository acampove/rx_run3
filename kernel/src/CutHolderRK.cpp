#ifndef CUTHOLDERRK_CPP
#define CUTHOLDERRK_CPP

#include "CutHolderRK.hpp"
#include "CutHolderRKst.hpp"

#include "CutDefRK.hpp"
#include "CutDefRKst.hpp"
#include "CutDefRX.hpp"
#include "SettingDef.hpp"

#include "TruthMatchingSvc.hpp"

ClassImp(CutHolderRK)

    CutHolderRK::CutHolderRK()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRK", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
}

CutHolderRK::CutHolderRK(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRK", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
}

CutHolderRK::CutHolderRK(const CutHolderRK & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRK", (TString) "CutHolderRK");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
}

ostream & operator<<(ostream & os, const CutHolderRK & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolderRK");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void CutHolderRK::Init() {
    MessageSvc::Info(Color::Cyan, "CutHolderRK", (TString) "Initialize ...");
    CreateCut();
    return;
}

void CutHolderRK::CreateCut() {
    if (m_debug) MessageSvc::Debug("CreateCut from CutHolderRKst", m_cutOption);

    m_cut = TCut(NOCUT);
    m_cuts.clear();

    //========================================================================================================================
    // Routine, take the option configured, ship to the RKst CutHolder, with "removal" of bits, make the cut available in RK
    //========================================================================================================================

    TString _cutOptionRKst = m_cutOption;
    /* Remove the -ePID option for RK, used later on eventually */
    _cutOptionRKst=_cutOptionRKst.ReplaceAll("-ePID","").ReplaceAll("ePID","");
    /* Remove the combBveto if present */
    _cutOptionRKst=_cutOptionRKst.ReplaceAll("-combBveto","").ReplaceAll("combBveto","");
    if( _cutOptionRKst.Contains("SSLep") || _cutOptionRKst.Contains("SSHad")){
        /* Remove the SSLep, SSHad tags if present*/
        MessageSvc::Warning("No Same sign tag on LPTSS samples for RK, removing this tag by construction");
        _cutOptionRKst=_cutOptionRKst.ReplaceAll("-SSLep","").ReplaceAll("SSLep","");
        _cutOptionRKst=_cutOptionRKst.ReplaceAll("-SSHad","").ReplaceAll("SSHad","");
    }
    /* Drop Preselection, BKG veto, PID, MVA q2 and Kst mass window cut in CutHolderRKst */
    _cutOptionRKst+="-noPS-noBKG-noPID-noMVA-noQ2-noKSTMASS";
    
    bool _isRKLowEE = m_configHolder.GetQ2bin() == Q2Bin::Low && m_configHolder.GetAna() == Analysis::EE;
    if( m_cutOption.Contains("SPECIAL") && m_cutOption.Contains("_RKLOW") && _isRKLowEE){
        if(m_cutOption.Contains("SPECIAL_RKLOW")){
        //no HOP and no PRMVA
        _cutOptionRKst = _cutOptionRKst.ReplaceAll("-HOP","");
        m_cutOption = m_cutOption.ReplaceAll("-HOP","");
        m_cutOption+="-noPRMVA";
      }
      if(m_cutOption.Contains("SPECIAL2_RKLOW")){
        //keep HOP and no PRMVA
        if( !_cutOptionRKst.Contains("HOP")){ _cutOptionRKst+="-HOP"; m_cutOption+="-HOP";}
        _cutOptionRKst+= "-noPRMVA";
      }
      if(m_cutOption.Contains("SPECIAL3_RKLOW")){
        //keep HOP, remove all MVAs
        if( !_cutOptionRKst.Contains("HOP")){ _cutOptionRKst+="-HOP"; m_cutOption+="-HOP";}
        m_cutOption+="-noMVA";
      }
      if(m_cutOption.Contains("SPECIAL4_RKLOW") || m_cutOption.Contains("SPECIAL5_RKLOW")){
        //Keep HOP (varied, see later )
        if( !_cutOptionRKst.Contains("HOP")){ _cutOptionRKst+="-HOP"; m_cutOption+="-HOP";}
        if(m_cutOption.Contains("SPECIAL5_RKLOW")) m_cutOption+="-noPRMVA";
      }
    }
    
    CutHolderRKst _cutHolder = CutHolderRKst(m_configHolder, _cutOptionRKst);

    _cutHolder.Init();

    m_cut  = _cutHolder.Cut();
    m_cuts = _cutHolder.Cuts();

    m_cut = CleanRKstCut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = CleanRKstCut(_cut.second); }

    // Build new cuts

    if (!m_cutOption.BeginsWith("no-")) {
        if (!m_cutOption.Contains("noPS")) m_cut  = m_cut && GetPreSelectionCut();
        if (!m_cutOption.Contains("noBKG")) m_cut = m_cut && GetBackgroundCut();
        if (!m_cutOption.Contains("noPID")) m_cut = m_cut && GetPIDCut();
        if (!m_cutOption.Contains("noMVA")) m_cut = m_cut && GetMVACut();
        if (!m_cutOption.Contains("noQ2")) m_cut = m_cut  && GetQ2Cut();
    }



    if( m_cutOption.Contains("ePID") && m_cutOption.Contains("noPID") ){
        MessageSvc::Warning("Enabling PID(e) cuts only, Be careful to use this only for weight Data/MC PIDe configurations setups");
        //--------------- Append the ePID cut here only ----------------//
        //--------------- This code is copied from GetPIDCut() call ----//
        Analysis ana  = m_configHolder.GetAna();
        if(ana== Analysis::EE) {
            Year     year = m_configHolder.GetYear();
            TCut _cut = CutDefRK::PID::pidE1E2;
            if (m_cutOption.Contains("PIDELECTRON2")){
                if (     m_cutOption.Contains("PIDELECTRON2_PROB_0p6"))   _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p55"))  _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p5"))   _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p45"))  _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p4"))   _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p35"))  _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p3"))   _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p25"))  _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p2"))   _cut = CutDefRK::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
                else _cut = CutDefRK::PID::pidE1E2_PID2;
            }
            if (m_cutOption.Contains("PIDELECTRON5")){
                if (     m_cutOption.Contains("PIDELECTRON5_PROB_0p6"))   _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p55"))  _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p5"))   _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p45"))  _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p4"))   _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p35"))  _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p3"))   _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p25"))  _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p2"))   _cut = CutDefRK::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
                else _cut = CutDefRK::PID::pidE1E2_PID5;
            }
            if (m_cutOption.Contains("PIDELECTRON7")){
                if (     m_cutOption.Contains("PIDELECTRON7_PROB_0p6"))   _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p55"))  _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p5"))   _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p45"))  _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p4"))   _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p35"))  _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p3"))   _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p25"))  _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p2"))   _cut = CutDefRK::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
                else _cut = CutDefRK::PID::pidE1E2_PID7;
            }
            if (m_cutOption.Contains("PIDELECTRON3"))       _cut = CutDefRK::PID::pidE1E2_PID3;  
            if (m_cutOption.Contains("PIDELECTRON3_PROB4")) _cut = CutDefRK::PID::pidE1E2_PID3_Prob4;
            
            if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRK::PID::probnnE), "1");
            if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRK::PID::dllE), "1");
            _cut = UpdatePIDTune(_cut, to_string(year));
            if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));
            if (IsCut(_cut)) m_cuts["cutPID"] = _cut;    
            m_cut = m_cut && _cut;
        } else {
            //non electron, make a dummy slot, to align normalization cutSetNorm.
            TCut _cut("1>0");
            m_cuts["cutPID"] = _cut;
        }
    }

    if (m_cutOption.Contains("combBveto")) m_cut = m_cut && GetCombBVeto();

    if (m_cutOption.Contains("XFeedKst")) m_cut = m_cut && GetXFeedKstCut();

    m_cut = UpdateDTFCut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = UpdateDTFCut(_cut.second); }

    if (m_configHolder.IsMC() && m_configHolder.IsSignalMC()) {
        m_cut = UpdateHLT1Cut(m_cut, m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetYear(), m_cutOption);
        for (auto _cut : m_cuts) { m_cuts[_cut.first] = UpdateHLT1Cut(_cut.second, m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetYear(), m_cutOption); }
    }

    m_cut = ReplaceProject(m_cut, m_configHolder.GetProject());
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = ReplaceProject(_cut.second, m_configHolder.GetProject()); }

    m_cut = CleanCut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = CleanCut(_cut.second); }

    if( m_cutOption.Contains("SPECIAL4_RKLOW") && m_configHolder.GetQ2bin() == Q2Bin::Low && m_configHolder.GetAna() == Analysis::EE){
        //Soften the HOP cut and PRMVA
        m_cut = ReplaceCut( m_cut,  "Bp_HOP_M > 4800", "Bp_HOP_M > 4700");
        m_cut = ReplaceCut( m_cut,  "cat_wMVA_PR_lowcen > 0.40", "cat_wMVA_PR_lowcen > 0.20");
        for( auto _cut : m_cuts ){
            m_cuts[_cut.first] = ReplaceCut( m_cuts[_cut.first], "Bp_HOP_M > 4800"    , "Bp_HOP_M > 4700");
            m_cuts[_cut.first] = ReplaceCut( m_cuts[_cut.first], "cat_wMVA_PR_lowcen > 0.40", "cat_wMVA_PR_lowcen > 0.20");
      }
    }
    if( m_cutOption.Contains("SPECIAL5_RKLOW") && m_configHolder.GetQ2bin() == Q2Bin::Low && m_configHolder.GetAna() == Analysis::EE){
        //Soften the HOP cut and PRMVA
        m_cut = ReplaceCut( m_cut,  "Bp_HOP_M > 4800", "Bp_HOP_M > 4700");
        m_cut = ReplaceCut( m_cut,  "cat_wMVA_PR_lowcen > 0.40", "(1>0)");
        for( auto _cut : m_cuts ){
            m_cuts[_cut.first] = ReplaceCut( m_cuts[_cut.first], "Bp_HOP_M > 4800"    , "Bp_HOP_M > 4700");
            m_cuts[_cut.first] = ReplaceCut( m_cuts[_cut.first], "cat_wMVA_PR_lowcen > 0.40", "(1>0)");
      }
    }
    m_cuts["cutFULL"] = m_cut;

    if (m_debug) MessageSvc::Debug("CreateCut", &m_cut);
    return;
}

TCut CutHolderRK::CleanRKstCut(TCut _cut) {
    if (m_debug) MessageSvc::Debug("CleanRKstCut", &_cut);
    _cut = ReplaceCut(_cut, "B0_", "Bp_");
    _cut = ReplaceCut(_cut, "Pi_", "K_");
    if (m_debug) MessageSvc::Debug("CleanRKstCut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetBackgroundCut() {
    if (m_debug) MessageSvc::Debug("GetBackgroundCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();
    Year     year  = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    if (m_configHolder.GetSample() != "LPTSS") {
        switch (ana) {
            case Analysis::MM: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRK::Background::peakingBkgMM : CutDefRK::Background::peakingBkgMM_PID; break;
            case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRK::Background::peakingBkgEE : CutDefRK::Background::peakingBkgEE_PID; break;
            default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }
    if (m_configHolder.GetSample() == "LPTSS" && m_cutOption.Contains("keepSSBKGCuts")) {
        switch (ana) {
            case Analysis::MM: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRK::Background::peakingBkgMM : CutDefRK::Background::peakingBkgMM_PID; break;
            case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRK::Background::peakingBkgEE : CutDefRK::Background::peakingBkgEE_PID; break;
            default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }        
    }
    //We can keep all of them for RK (they exists in the ntuples!)


    if (m_cutOption.Contains("BKGCOS")) {
        switch (q2bin) {
            case Q2Bin::Low: _cut = _cut && CutDefRK::Background::Bu2DX; break;
            case Q2Bin::Central: _cut = _cut && CutDefRK::Background::Bu2DX; break;
            default: break;
        }
    }
    //Remove the Psi-> X J/Psi in q2 = J/Psi for electrons only
    if( m_cutOption.Contains("vetoPsi") && !m_cutOption.Contains("novetoPsi")){
        if( ana == Analysis::EE){
            switch (q2bin){
                case Q2Bin::JPsi: _cut = _cut &&  "TMath::Abs(Bp_DTF_Psi_M-5280)>200"; break;
                default: break;
            }
        }
    }
    //Remove the JPsi-> ee leakage in Psi(2S) for electrons only
    if( m_cutOption.Contains("vetoJPs") && !m_cutOption.Contains("novetoJPs")){
        if( ana == Analysis::EE){
            switch (q2bin) {
                case Q2Bin::Psi: _cut = _cut &&  "TMath::Abs(Bp_DTF_JPs_M-5280)>200"; break;
                default: break;
            }
        }
    }
    if (!m_cutOption.Contains("noSL")) {
	    // TODO : add in v11 , see connected change in CutDefRK. 
        // Change is about using the D2Klnu cut only for Low/Central q2. 
        // Atm, the D2Klnu cut is applied everywhere because of CutDefRK.hpp definitions.
        if (m_configHolder.GetSample() != "LPTSS") {
            switch (ana) {
                case Analysis::MM: _cut = m_cutOption.Contains("BKGwoPID") ? _cut && CutDefRK::Background::semilepBkgMM : _cut && CutDefRK::Background::semilepBkgMM_PID; break;
                case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? _cut && CutDefRK::Background::semilepBkgEE : _cut && CutDefRK::Background::semilepBkgEE_PID; break;
                default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
            }
        }
        if (m_configHolder.GetSample() == "LPTSS" && m_cutOption.Contains("keepSSBKGCuts")){
            //in semilepBkgEE/MM cuts we have a pure kinematic cut which we might want to keep
            switch (ana) {
                case Analysis::MM: _cut =  _cut && CutDefRK::Background::D2Klnu ; break;
                case Analysis::EE: _cut =  _cut && CutDefRK::Background::D2Klnu ; break;
                default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
            }
        }
    }

    if (!m_cutOption.Contains("BKGwoPID")) {
        _cut = UpdatePIDTune(_cut, to_string(year));
        if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));
    }

    if (IsCut(_cut)) m_cuts["cutBKG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBackgroundCut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetMVACut() {
    if (m_debug) MessageSvc::Debug("GetMVACut", m_cutOption);

    Q2Bin q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    if (q2bin != Q2Bin::All) {
        Analysis ana  = m_configHolder.GetAna();
        Year     year = m_configHolder.GetYear();
        if (year != Year::All) {
            year = hash_year(GetRunFromYear(to_string(m_configHolder.GetYear())));
        } else {
            year = hash_year(GetRunFromYear(SettingDef::Config::year));
            MessageSvc::Warning("GetMVACut", (TString) "Setting year to", to_string(year));
        }
        //Forcing MVA cut value to be picked from a given Run settings ( Special configuration setup from FitComponent.cpp when -add[2XJPs] sample is used and mixing of years can happen)
        if( m_cutOption.Contains("MVAR1"))   year = Year::Run1;
        if( m_cutOption.Contains("MVAR2p1")) year = Year::Run2p1;
        if( m_cutOption.Contains("MVAR2p2")) year = Year::Run2p2;
        auto _config = make_tuple(ana, year, q2bin);
        
        if( m_cutOption.Contains("MVACentral") && q2bin == Q2Bin::JPsi && ana == Analysis::EE){
            MessageSvc::Warning("J/Psi q2 EE, setting MVA cut as in central q2");
            _config = make_tuple(ana, year, Q2Bin::Central);
        }
        if( m_cutOption.Contains("MuonCentralMVA") && q2bin == Q2Bin::JPsi && ana == Analysis::MM){
            MessageSvc::Warning("J/Psi q2 MM, setting MVA cut as in central q2");
            _config = make_tuple(ana, year, Q2Bin::Central);
        }
        
        map< tuple< Analysis, Year, Q2Bin >, TString > _mvaCut = CutDefRK::MVA::CAT;
        // if (m_cutOption.Contains("MVALOOSE") && ((ana == Analysis::EE) && ((q2bin == Q2Bin::Low) || (q2bin == Q2Bin::Central)))) _mvaCut = CutDefRK::MVA::CAT_LOOSE;
        if (_mvaCut.find(_config) == _mvaCut.end()) {
            MessageSvc::Error("Configuration", to_string(get< 0 >(_config)), ",", to_string(get< 1 >(_config)), ",", to_string(get< 2 >(_config)));
            MessageSvc::Error("GetMVACut", (TString) "Invalid configuration (please implement it or use -noMVA)", "EXIT_FAILURE");
        }else {
            _cut = _mvaCut[_config];
            // if (m_cutOption.Contains("noPR") && (ana == Analysis::EE) && (q2bin == Q2Bin::JPsi)) { _cut = TCut(RemoveStringAfter(TString(_cut), " && ")); }
        }
    }

    
    if( m_cutOption.Contains("noPRMVA")) {
        _cut = TCut(TString(_cut).ReplaceAll("cat_wMVA_PR_lowcen", "1"));
    }
    if (IsCut(_cut)) m_cuts["cutMVA"] = _cut;
    
    if (m_debug) MessageSvc::Debug("GetMVACut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetPIDCut() {
    if (m_debug) MessageSvc::Debug("GetPIDCut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRK::PID::pidMM; break;
        case Analysis::EE: _cut = CutDefRK::PID::pidEE; break;
        default: MessageSvc::Error("GetPIDCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    //Warning , order matters
    if (ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON2")){
        if (     m_cutOption.Contains("PIDELECTRON2_PROB_0p6"))   _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p55"))  _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p5"))   _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p45"))  _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p4"))   _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p35"))  _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p3"))   _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p25"))  _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p2"))   _cut = CutDefRK::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRK::PID::pidEE_PID2;    
    }
    if (ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON5")){
        if (     m_cutOption.Contains("PIDELECTRON5_PROB_0p6"))   _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p55"))  _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p5"))   _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p45"))  _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p4"))   _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p35"))  _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p3"))   _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p25"))  _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p2"))   _cut = CutDefRK::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRK::PID::pidEE_PID5;    
    }
    if (ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON7")){
        if (     m_cutOption.Contains("PIDELECTRON7_PROB_0p6"))   _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p55"))  _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p5"))   _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p45"))  _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p4"))   _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p35"))  _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p3"))   _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p25"))  _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p2"))   _cut = CutDefRK::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRK::PID::pidEE_PID7;    
    }
    if (ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON3"))       _cut = CutDefRK::PID::pidEE_PID3;
    if (ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON3_PROB4")) _cut = CutDefRK::PID::pidEE_PID3_Prob4;


    if (m_cutOption.Contains("noProbHH")) _cut = ReplaceCut(_cut, TString(CutDefRK::PID::probnnK), "1");
    if (m_cutOption.Contains("noPIDHHforEE") && ana == Analysis::EE) {
        _cut = ReplaceCut(_cut, TString(CutDefRK::PID::probnnK), "1");
        _cut = ReplaceCut(_cut, TString(CutDefRK::PID::dllK), "1");
    }
    if (m_cutOption.Contains("noProbMM")) _cut = ReplaceCut(_cut, TString(CutDefRK::PID::probnnM), "1");
    if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRK::PID::probnnE), "1");

    _cut = UpdatePIDTune(_cut, to_string(year));

    if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));

    if (IsCut(_cut)) m_cuts["cutPID"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPIDCut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetPreSelectionCut() {
    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", m_cutOption);

    Analysis ana     = m_configHolder.GetAna();
    Year     year    = m_configHolder.GetYear();
    Trigger  trigger = m_configHolder.GetTrigger();

    // TODO  : improve minLPET cut logic implementation
    /* 
        The minLPET cut is imported using a CutHolderRKst, the cutLPET flag will be embedded in the CutHolder
        We ideally want to decouple the minLPET cut in the m_cuts map
    */
    CutHolderRKst _cutHolder = CutHolderRKst(m_configHolder, m_cutOption);
    _cutHolder.Init();
    TCut _cutMinLPET(NOCUT);
    if (IsCutInMap("cutLPET", _cutHolder.Cuts())){
        _cutMinLPET = _cutHolder.Cuts().at("cutLPET");
        m_cuts["cutLPET"] = _cutMinLPET;
    }
    bool _includeIsMuon = !m_cutOption.Contains("noMuIsMuon");
    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = _includeIsMuon? CutDefRK::Quality::qualityMM :  CutDefRK::Quality::qualityMM_noIsMuon ; break;
        case Analysis::EE:
            _cut = CutDefRK::Quality::qualityEE;
            if (!m_cutOption.Contains("noMinLPET")) {
                if (IsCut(_cutMinLPET)) {
                    switch (trigger) {
                        case Trigger::L0I: break;
                        case Trigger::L0L: _cut = _cut && _cutMinLPET; break;
                        case Trigger::L0H: break;
                        case Trigger::All: _cut = _cut && (CutDefRX::Trigger::L0I || _cutMinLPET); break;
                        default: MessageSvc::Error("GetTCKCut", (TString) "Invalid trg", to_string(trigger), "EXIT_FAILURE"); break;
                    }
                }
            }
            break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_cutOption.Contains("noMinLPET")) {
        if (IsCutInMap("cutLPET", _cutHolder.Cuts())) {
            TCut _cutMinLPET_noMinLPET = _cutHolder.Cuts().at("cutLPET");
            if (IsCut(_cutMinLPET_noMinLPET)) {
                _cut              = ReplaceCut(_cut, TString(_cutMinLPET_noMinLPET), "1");
            }
        }
    }

    if (m_cutOption.Contains("noMinHPT")) {
        _cut = ReplaceCut(_cut, TString("K_PT > 250"), "1");
        _cut = ReplaceCut(_cut, TString("K_PT > 400"), "1");
    }

    /*
        TODO : enable this by default if OA flatness syst is not fixed ? 
        Simple Functor to append a max(P) cut on final states. Currently done to investigate OA issue. 
    */                
    auto _getMaxPCut = [](const Analysis & _ana){        
        TCut _maxP(NOCUT);
        switch (_ana){
            case Analysis::EE : _maxP = "E1_P<200E3 && E2_P<200E3 && K_P<200E3"; break;
            case Analysis::MM : _maxP = "M1_P<200E3 && M2_P<200E3 && K_P<200E3"; break;
            case Analysis::ME : _maxP = "M1_P<200E3 && E2_P<200E3 && K_P<200E3"; break;
            default : MessageSvc::Error("CutHolderRK::GetPreSelectionCut::_getMaxPCut functor, invalid switch", "","EXIT_FAILURE");
        }
        return _maxP ;
    };
    if(m_cutOption.Contains("MAXP"))  _cut = _cut && _getMaxPCut( ana);


    if(m_cutOption.Contains("cutECALDistance") && ana == Analysis::EE){
        //Flat , all q2 bins, RK, RKst
        _cut = _cut && CutDefRX::Quality::ECALDistance;
        m_cuts["cutECAL"] = CutDefRX::Quality::ECALDistance;
    }else{
        m_cuts["cutECAL"] = TCut(NOCUT);
    }
    if( m_cutOption.Contains("noOpeningAngles")){
        MessageSvc::Warning("Removing opening angle cuts");
        _cut = ReplaceCut( _cut, TString(CutDefRK::Quality::openingAngle), "1" );        
    }

    if (IsCut(_cut)) m_cuts["cutPS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetQ2Cut() {
    if (m_debug) MessageSvc::Debug("GetQ2Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();
    TString _q2 =  "TMath::Sq(JPs_M/1000.)";
    TCut _cut(NOCUT);
    switch (q2bin) {
        case Q2Bin::All: {
            if(m_cutOption.Contains("q2All[")) _cut = CustomizedCut( m_cutOption, "q2All[", _q2);
            break;
        }
        case Q2Bin::Low: {
            bool _useTighterLowQ2 = SettingDef::Cut::tightLowQ2 && m_configHolder.GetAna() == Analysis::MM;
            _cut = _useTighterLowQ2 ?  CutDefRK::Mass::Q2LowTight : CutDefRK::Mass::Q2Low;
            if(m_cutOption.Contains("q2Low[")) _cut = CustomizedCut( m_cutOption, "q2Low[", _q2);
            break; 
        }
        case Q2Bin::Central: {
            _cut = CutDefRK::Mass::Q2Central; 
            if(m_cutOption.Contains("q2Central[")) _cut = CustomizedCut( m_cutOption, "q2Central[", _q2);
            break;
        }
        case Q2Bin::High: {
            _cut = ana == Analysis::MM ? CutDefRK::Mass::Q2HighMM : CutDefRK::Mass::Q2HighEE;
            if(m_cutOption.Contains("q2High[")) _cut = CustomizedCut( m_cutOption, "q2High[", _q2);
            break;
        }
        case Q2Bin::JPsi: {
            _cut = ana == Analysis::MM ? CutDefRX::Mass::JPsMM : CutDefRX::Mass::JPsEE; 
            if(m_cutOption.Contains("q2JPsi[")) _cut = CustomizedCut( m_cutOption, "q2JPsi[", _q2);
            break; //default 
        }
        case Q2Bin::Psi: {
            switch( ana){
                case Analysis::MM : _cut = CutDefRX::Mass::PsiMM; break;
                case Analysis::EE : _cut = m_cutOption.Contains("q2PsiWide")? CutDefRX::Mass::PsiWideEE : CutDefRX::Mass::PsiEE; break;
                default : MessageSvc::Error("GetQ2Cut Psi Slot", (TString) "Invalid Analysis", to_string(ana), "EXIT_FAILURE");  break;
            }
            if(m_cutOption.Contains("q2Psi[")) _cut = CustomizedCut( m_cutOption, "q2Psi[", _q2);
            break;
        }
        default: MessageSvc::Error("GetQ2Cut", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
    }
    //Fast cut definer for smeared JPS_M values!
    auto UpdateQ2Cut = []( TCut _cutIN, TString _cutOption ){
        TString _newCut = TString(_cutIN); 
        if(      _cutOption.Contains("q2SmearMCDTBp")) _newCut =  _newCut.ReplaceAll("JPs_M", "JPs_M_smear_Bp_fromMCDT_wMC");
        else if( _cutOption.Contains("q2SmearMCDTB0")) _newCut =  _newCut.ReplaceAll("JPs_M", "JPs_M_smear_B0_fromMCDT_wMC");
        else if( _cutOption.Contains("q2SmearBp") )    _newCut =  _newCut.ReplaceAll("JPs_M", "JPs_M_smear_Bp_wMC");
        else if( _cutOption.Contains("q2SmearB0"))     _newCut =  _newCut.ReplaceAll("JPs_M", "JPs_M_smear_B0_wMC");
        return TCut( _newCut);
    };


    //Append to map of cuts the TRUEQ2 cut (useful for MCDT efficiency estimation)
    if (IsCut(_cut)){
        m_cuts["cutQ2TRUE"] = _cut;
        // m_cuts["cutQ2TRUE"] = ReplaceCut(m_cuts["cutQ2TRUE"], "JPs_M", "JPs_TRUEM");
        m_cuts["cutQ2TRUE"] = ReplaceCut(m_cuts["cutQ2TRUE"], "JPs_M", "JPs_TRUEM"); //the DiLepM is correct only for E1_E2 trueM
    }

    //Convert cut to the Smeared one    
    if( ana == Analysis::EE && !m_configHolder.IsLeakageSample()) _cut = UpdateQ2Cut( _cut, m_cutOption);

    
    if (IsCut(_cut)) m_cuts["cutQ2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetQ2Cut", &_cut);
    return _cut;
}

TCut CutHolderRK::GetCombBVeto() {
    TCut _cut(NOCUT);
    //Special cut which can be only applied on SS data samples , avoid all the others
    if( m_configHolder.GetSample() != "LPTSS")  return _cut;
    
    if (m_debug) MessageSvc::Debug("GetCombBVeto", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    if (ana == Analysis::EE) {
        TCut _cutMVA = TCut("cat_wMVA_lowcen < 0.8");
        //Do NOT apply the MVA cut for Central q2
        //Do NOT apply the J/Psi veto for Central and Low 
        switch (q2bin) {
            case Q2Bin::Central: _cut =  CutDefRK::Mass::CombBVetoJPs; break;
            case Q2Bin::JPsi:    _cut = _cutMVA && CutDefRK::Mass::CombBVetoJPs; break;
            case Q2Bin::Psi:     _cut = _cutMVA && CutDefRK::Mass::CombBVetoPsi; break;
            default: break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutCombBVeto"] = _cut;

    if (m_debug) MessageSvc::Debug("GetCombBVeto", &_cut);
    return _cut;
}

TCut CutHolderRK::GetXFeedKstCut(){
    TCut _cut(NOCUT);
    if( m_configHolder.IsCrossFeedSample()){
        if( m_cutOption.Contains("XFeedKstIN")){
            _cut = CutDefRKst::Mass::Kst_Port; 
        }else if( m_cutOption.Contains("XFeedKstOUT")){
            _cut = !CutDefRKst::Mass::Kst_Port; 
        }else{
            MessageSvc::Error("CutHolderRK::GetXFeedKstCut", "Please use XFeedKstIN OR XFeedKstOUT as option","EXIT_FAILURE");
        }
    }else{
        return _cut;
    }
    if (IsCut(_cut)) m_cuts["cutCrossFeedKstMass"] = _cut;
    return _cut;
}

#endif
