#ifndef CUTHOLDERRKST_CPP
#define CUTHOLDERRKST_CPP

#include "CutHolderRKst.hpp"

#include "CutDefRKst.hpp"
#include "CutDefRK.hpp"
#include "CutDefRX.hpp"
#include "SettingDef.hpp"
#include "TruthMatchingSvc.hpp"

ClassImp(CutHolderRKst)

    CutHolderRKst::CutHolderRKst()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKst", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
}

CutHolderRKst::CutHolderRKst(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKst", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
}

CutHolderRKst::CutHolderRKst(const CutHolderRKst & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKst", (TString) "CutHolderRKst");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
}

ostream & operator<<(ostream & os, const CutHolderRKst & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolderRKst");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void CutHolderRKst::Init() {
    MessageSvc::Info(Color::Cyan, "CutHolderRKst", (TString) "Initialize ...");
    CreateCut();
    return;
}

void CutHolderRKst::CreateCut() {
    if (m_debug) MessageSvc::Debug("CreateCut", m_cutOption);

    m_cut = TCut(NOCUT);
    m_cuts.clear();

    if (m_cutOption.Contains("TCKADC")) m_cut = m_cut && GetTCKCut();
    if (m_cutOption.Contains("HLT1TCK")) m_cut = m_cut && GetHLT1TCKCutAlignment();

    if (!m_cutOption.BeginsWith("no-")) {
        if (!m_cutOption.Contains("noSPD")) m_cut = m_cut && GetSPDCut();
        if (!m_cutOption.Contains("noTRG")) m_cut = m_cut && GetTriggerCut();
        if (!m_cutOption.Contains("noPS")) m_cut = m_cut && GetPreSelectionCut();
        if (!m_cutOption.Contains("noKSTMASS")) m_cut = m_cut && GetKstMassCut();
        if (!m_cutOption.Contains("noBKG")) m_cut = m_cut && GetBackgroundCut();
        if (!m_cutOption.Contains("noPID")) m_cut = m_cut && GetPIDCut();
        if (!m_cutOption.Contains("noMVA")) m_cut = m_cut && GetMVACut();
        if (!m_cutOption.Contains("noQ2")) m_cut = m_cut && GetQ2Cut();
    }
    if (!m_cutOption.Contains("noBREM")) m_cut = m_cut && GetBremCut();
    if (!m_cutOption.Contains("noTRACK")) m_cut = m_cut && GetTrackCut();

    if( m_cutOption.Contains("ePID") && m_cutOption.Contains("noPID")){
        MessageSvc::Warning("Enabling PID(e) cuts only, Be careful to use this only for weight Data/MC PIDe configurations setups");
        Analysis ana  = m_configHolder.GetAna();
        if(ana == Analysis::EE) {
            Year     year = m_configHolder.GetYear();    
            TCut _cut = CutDefRKst::PID::pidE1E2;
            //WARNING orders matters 
            if (m_cutOption.Contains("PIDELECTRON2")){
                if (     m_cutOption.Contains("PIDELECTRON2_PROB_0p6"))   _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p55"))  _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p5"))   _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p45"))  _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p4"))   _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p35"))  _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p3"))   _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p25"))  _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");    
                else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p2"))   _cut = CutDefRKst::PID::pidE1E2_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");                   
                else _cut = CutDefRKst::PID::pidE1E2_PID2;
            }
            if (m_cutOption.Contains("PIDELECTRON5")){
                if (     m_cutOption.Contains("PIDELECTRON5_PROB_0p6"))   _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p55"))  _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p5"))   _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p45"))  _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p4"))   _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p35"))  _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p3"))   _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p25"))  _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");    
                else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p2"))   _cut = CutDefRKst::PID::pidE1E2_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");                   
                else _cut = CutDefRKst::PID::pidE1E2_PID5;
            }
            if (m_cutOption.Contains("PIDELECTRON7")){
                if (     m_cutOption.Contains("PIDELECTRON7_PROB_0p6"))   _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p55"))  _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p5"))   _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p45"))  _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p4"))   _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p35"))  _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p3"))   _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p25"))  _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");    
                else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p2"))   _cut = CutDefRKst::PID::pidE1E2_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");                   
                else _cut = CutDefRKst::PID::pidE1E2_PID7;
            }
            if (m_cutOption.Contains("PIDELECTRON3"))       _cut = CutDefRKst::PID::pidE1E2_PID3;
            if (m_cutOption.Contains("PIDELECTRON3_PROB4")) _cut =  CutDefRKst::PID::pidE1E2_PID3_Prob4;        
            if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::probnnE), "1");
            if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::dllE), "1");
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

    if (m_cutOption.Contains("HOP")) m_cut = m_cut && GetHOPCut();
    
    if (m_cutOption.Contains("addPR")) m_cut = m_cut && GetPartRecoCut();

    if (m_cutOption.Contains("SB") || m_configHolder.IsSB("SB")) m_cut = m_cut && GetSideBandCut();
    if (m_cutOption.Contains("massB")) m_cut = m_cut && GetBMassCut();
    
    if (m_cutOption.Contains("combBveto")) m_cut = m_cut && GetCombBVeto();    
    if (m_cutOption.Contains("SPlotnTracks")) m_cut = m_cut && GetSPlotnTracks();

    if (m_cutOption.Contains("tm"))       m_cut = GetTruthMatchCut() && m_cut;
    if (m_cutOption.Contains("isSingle")) m_cut = GetIsSingleCut() && m_cut;


    if(m_configHolder.GetSample() == "LPTSS"){
        TCut _cut(NOCUT);
        if(m_cutOption.Contains("SSHad")) _cut = "SSID==0";
        if(m_cutOption.Contains("SSLep")) _cut = "SSID==1";
        if (IsCut(_cut)){
            m_cuts["cutSSID"] = _cut;
            m_cut = m_cut && _cut;
        }
    }


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

    m_cuts["cutFULL"] = m_cut;

    if (m_debug) MessageSvc::Debug("CreateCut", &m_cut);
    return;
}

TCut CutHolderRKst::GetBackgroundCut() {
    if (m_debug) MessageSvc::Debug("GetBackgroundCut", m_cutOption);
    Analysis ana = m_configHolder.GetAna();
    TCut _cut(NOCUT);
    if (ana == Analysis::ME) return _cut;
    Q2Bin q2bin = m_configHolder.GetQ2bin();
    Year  year  = m_configHolder.GetYear();

    //TODO : remake LPTSS skimming with those branches in.... ( issues on B_M012.... with swaps but not with B_M012 itself)
    if (m_configHolder.GetSample() != "LPTSS") {
        switch (ana) {
            case Analysis::MM: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRKst::Background::peakingBkgMM : CutDefRKst::Background::peakingBkgMM_PID; break;
            case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRKst::Background::peakingBkgEE : CutDefRKst::Background::peakingBkgEE_PID; break;
            default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
        if(q2bin == Q2Bin::Central && ana== Analysis::EE && m_cutOption.Contains("noPKGOverRecoedCentral")){
            MessageSvc::Info("Override BKG cut removing overreconstructed cut of m(Kee)");
           _cut = m_cutOption.Contains("BKGwoPID") ? CutDefRKst::Background::peakingBkgEE_noOverReco  : CutDefRKst::Background::peakingBkgEE_PID_noOverReco;
        }
    }
    //Special SS data 
    if( m_configHolder.GetSample() == "LPTSS" && m_cutOption.Contains("keepSSBKGCuts")){
        //Keep as many cuts as possible from the semilepBkgCuts for SS data
        switch (ana){
            case Analysis::MM: _cut = CutDefRKst::Background::peakingBkgMM_SSData; break;
            case Analysis::EE: _cut = CutDefRKst::Background::peakingBkgEE_SSData; break;
            default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    
    if (m_cutOption.Contains("BKGCOS")) {
        switch (q2bin) {
            case Q2Bin::Gamma: _cut = _cut && CutDefRKst::Background::Bd2DX; break;
            case Q2Bin::Low: _cut = _cut && CutDefRKst::Background::Bd2DX; break;
            case Q2Bin::Central: _cut = _cut && CutDefRKst::Background::Bd2DX; break;
            default: break;
        }
    }

    if (!m_cutOption.Contains("noSL")) {
        switch (q2bin) {
            case Q2Bin::Gamma: _cut = _cut && CutDefRKst::Background::Bd2DXMass; break;
            case Q2Bin::Low: _cut = _cut && CutDefRKst::Background::Bd2DXMass; break;
            case Q2Bin::Central: _cut = _cut && CutDefRKst::Background::Bd2DXMass; break;
            default: break;
        }
        if( q2bin == Q2Bin::Central && ana == Analysis::EE && m_cutOption.Contains("Bs2DsXMassCentralEE")){
            //Tight up the SL cut to have also the Bs2DX vetoed out
            _cut = _cut && CutDefRKst::Background::Bs2DXMass;
        }
        //TODO : remake LPTSS skimming with those branches in.... ( issues on B_M012....)
        if (m_configHolder.GetSample() != "LPTSS") {        
            switch (ana) {
                case Analysis::MM: _cut = m_cutOption.Contains("BKGwoPID") ? _cut && CutDefRKst::Background::semilepBkgMM : _cut && CutDefRKst::Background::semilepBkgMM_PID; break;
                case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? _cut && CutDefRKst::Background::semilepBkgEE : _cut && CutDefRKst::Background::semilepBkgEE_PID; break;
                default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
            }
        }
        if( m_configHolder.GetSample() == "LPTSS" && m_cutOption.Contains("keepSSBKGCuts")){
            //Keep as many cuts as possible from the Semileptonic SS Data
            switch (ana) {
                case Analysis::MM: _cut = _cut; break;//Nothing we can really add, all cuts are with Subst branches
                case Analysis::EE: _cut = m_cutOption.Contains("BKGwoPID") ? _cut && CutDefRKst::Background::semilepBkgEE_SSData : _cut && CutDefRKst::Background::semilepBkgEE_SSData_PID; break;
                default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
            }
        }
    }
    //Remove the Psi-> X J/Psi in q2 = J/Psi for electrons only
    if( m_cutOption.Contains("vetoPsi") && !m_cutOption.Contains("novetoPsi")){
        if( ana == Analysis::EE){
            switch (q2bin){
                case Q2Bin::JPsi: _cut = _cut &&  "TMath::Abs(B0_DTF_Psi_M-5280)>200"; break;
                default: break;
            }
        }
    }
    //Remove the JPsi-> ee leakage in Psi(2S) for electrons only
    if( m_cutOption.Contains("vetoJPs") && !m_cutOption.Contains("novetoJPs")){
        if( ana == Analysis::EE){
            switch (q2bin) {
                case Q2Bin::Psi: _cut = _cut &&  "TMath::Abs(B0_DTF_JPs_M-5280)>200"; break;
                default: break;
            }
        }
    }
    if (!m_cutOption.Contains("BKGwoPID")) {
        _cut = UpdatePIDTune(_cut, to_string(year));
        if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));
    }

    if( ana == Analysis::EE && q2bin == Q2Bin::Central && m_cutOption.Contains("SLCTL")){
        if( m_cutOption.Contains("SLCTLMKE")){
            MessageSvc::Debug("Adding SL cut veto with m(K+e-) > 1885");
            _cut = _cut && CutDefRKst::Background::SLCTLMKE;        
        }else if( m_cutOption.Contains("SLCTLABSTHETA")){
            _cut = _cut && "TMath::Abs( TMath::Cos(B0_ThetaL_custom)) < 0.6";
        }else{
            MessageSvc::Debug("Adding SL cut veto with Cost(ThetaL) < 0.6");
            _cut = _cut && CutDefRKst::Background::SLCTL;   
        }
    }


    if (IsCut(_cut)) m_cuts["cutBKG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBackgroundCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetBremCut() {
    if (m_debug) MessageSvc::Debug("GetBremCut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Brem brem = m_configHolder.GetBrem();

    TCut _cut(NOCUT);
    switch (brem) {
        case Brem::All: _cut = TCut(NOCUT); break;
        case Brem::G0: _cut = CutDefRX::Brem::G0; break;
        case Brem::G1: _cut = CutDefRX::Brem::G1; break;
        case Brem::G2: _cut = CutDefRX::Brem::G2; break;
        default: MessageSvc::Error("GetBremCut", (TString) "Invalid brem", to_string(brem), "EXIT_FAILURE"); break;
    }
    if (m_cutOption.Contains("Brem")) {
        switch(ana) {
        case Analysis::MM: _cut = TCut(NOCUT); break;
        case Analysis::EE: {
            if      (m_cutOption.Contains("Brem01")) { _cut = CutDefRX::Brem::G0 || CutDefRX::Brem::G1; }
            else if (m_cutOption.Contains("Brem12")) { _cut = CutDefRX::Brem::G1 || CutDefRX::Brem::G2; }
            else if (m_cutOption.Contains("Brem0")) {  _cut = CutDefRX::Brem::G0;                       }
            else if (m_cutOption.Contains("Brem1")) {  _cut = CutDefRX::Brem::G1;                       } 
            else if (m_cutOption.Contains("Brem2")) {  _cut = CutDefRX::Brem::G2;                       }
            else { 
            MessageSvc::Error("GetBremCut", (TString) "Invalid cutOption", m_cutOption, "EXIT_FAILURE"); 
            }
            break;
        }
        default: MessageSvc::Error("GetBremCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (m_cutOption.Contains("BREM")) _cut = GetBremETCut();

    if (IsCut(_cut)) m_cuts["cutBREM"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBremCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetBremETCut() {
    if (m_debug) MessageSvc::Debug("GetBremETCut", m_cutOption);

    TString _bremET = "";
    if (m_cutOption.Contains("BREMET")) {
        for (auto _opt : *(m_cutOption.Tokenize("-"))) {
            if (((TObjString *) _opt)->String().Contains("BREMET")) { _bremET = ((TObjString *) _opt)->String().ReplaceAll("BREMET", ""); }
        }
    }

    TCut cut0BREM00 = TCut("(!E1_HasBremAdded && !E2_HasBremAdded)");
    TCut cut0BREM10 = TCut("(E1_HasBremAdded && !E2_HasBremAdded)");
    TCut cut0BREM01 = TCut("(!E1_HasBremAdded && E2_HasBremAdded)");
    TCut cut0BREM11 = TCut("(E1_HasBremAdded && E2_HasBremAdded)");

    if (_bremET != "") {
        m_cuts["cutBREM00"] = TCut("(TMath::Sqrt(E1_BremPX**2+E1_BremPY**2) < " + _bremET + " && TMath::Sqrt(E2_BremPX**2+E2_BremPY**2) < " + _bremET + ")");
        m_cuts["cutBREM10"] = TCut("(TMath::Sqrt(E1_BremPX**2+E1_BremPY**2) > " + _bremET + " && TMath::Sqrt(E2_BremPX**2+E2_BremPY**2) < " + _bremET + ")");
        m_cuts["cutBREM01"] = TCut("(TMath::Sqrt(E1_BremPX**2+E1_BremPY**2) < " + _bremET + " && TMath::Sqrt(E2_BremPX**2+E2_BremPY**2) > " + _bremET + ")");
        m_cuts["cutBREM11"] = TCut("(TMath::Sqrt(E1_BremPX**2+E1_BremPY**2) > " + _bremET + " && TMath::Sqrt(E2_BremPX**2+E2_BremPY**2) > " + _bremET + ")");
    } else {
        m_cuts["cutBREM00"] = cut0BREM00;
        m_cuts["cutBREM10"] = cut0BREM10;
        m_cuts["cutBREM01"] = cut0BREM01;
        m_cuts["cutBREM11"] = cut0BREM11;
    }
    m_cuts["cutBREMI0"] = m_cuts["cutBREM00"];
    m_cuts["cutBREMI1"] = m_cuts["cutBREM10"] || m_cuts["cutBREM01"];
    m_cuts["cutBREMI2"] = m_cuts["cutBREM11"];

    m_cuts["cutBREME0000"] = m_cuts["cutBREM00"] && cut0BREM00;
    m_cuts["cutBREME0010"] = m_cuts["cutBREM00"] && cut0BREM10;
    m_cuts["cutBREME0001"] = m_cuts["cutBREM00"] && cut0BREM01;
    m_cuts["cutBREME0011"] = m_cuts["cutBREM00"] && cut0BREM11;

    m_cuts["cutBREME1010"] = m_cuts["cutBREM10"] && cut0BREM10;
    m_cuts["cutBREME0101"] = m_cuts["cutBREM01"] && cut0BREM01;
    m_cuts["cutBREME1011"] = m_cuts["cutBREM10"] && cut0BREM11;
    m_cuts["cutBREME0111"] = m_cuts["cutBREM01"] && cut0BREM11;

    m_cuts["cutBREME1111"] = m_cuts["cutBREM11"] && cut0BREM11;

    TCut _cut(NOCUT);

    if (m_cutOption.Contains("BREMI0")) _cut = m_cuts["cutBREMI0"];
    if (m_cutOption.Contains("BREMI1")) _cut = m_cuts["cutBREMI1"];
    if (m_cutOption.Contains("BREMI2")) _cut = m_cuts["cutBREMI2"];

    if (m_cutOption.Contains("BREME0000")) _cut = m_cuts["cutBREME0000"];
    if (m_cutOption.Contains("BREME0010")) _cut = m_cuts["cutBREME0010"];
    if (m_cutOption.Contains("BREME0001")) _cut = m_cuts["cutBREME0001"];
    if (m_cutOption.Contains("BREME0011")) _cut = m_cuts["cutBREME0011"];

    if (m_cutOption.Contains("BREME1010")) _cut = m_cuts["cutBREME1010"];
    if (m_cutOption.Contains("BREME0101")) _cut = m_cuts["cutBREME0101"];
    if (m_cutOption.Contains("BREME1011")) _cut = m_cuts["cutBREME1011"];
    if (m_cutOption.Contains("BREME0111")) _cut = m_cuts["cutBREME0111"];
    if (m_cutOption.Contains("BREME1111")) _cut = m_cuts["cutBREME1111"];

    if (m_debug) MessageSvc::Debug("GetBremETCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetHOPCut() {
    if (m_debug) MessageSvc::Debug("GetHOPCut", m_cutOption);    
    TCut _cut(NOCUT);
    if (m_configHolder.GetAna() == Analysis::EE) {
        if (m_configHolder.GetQ2bin() == Q2Bin::Low) {
            if (m_cutOption.Contains("HOP2D")){
                _cut = CutDefRX::Mass::HOP2D_low;
            }else{
                _cut = CutDefRX::Mass::HOP_low;
            }
        } else if (m_configHolder.GetQ2bin() == Q2Bin::Central) {
            if (m_cutOption.Contains("HOP2D")){
                _cut = CutDefRX::Mass::HOP2D_central;
            }else{
                _cut = CutDefRX::Mass::HOP_central;
            }
        } else if ( m_configHolder.GetQ2bin() == Q2Bin::JPsi && m_cutOption.Contains("HOPJPs")) { 
            _cut = CutDefRX::Mass::HOP_central;
        }
    }

    if (IsCut(_cut)) m_cuts["cutHOP"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHOPCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetIsSingleCut() {
    if (m_debug) MessageSvc::Debug("GetIsSingleCut", m_cutOption);

    TCut _cut(NOCUT);
    if (m_configHolder.IsMC()){
        _cut = "isSingle_BKGCAT";
    }else{
        _cut = "isSingle_RND";
    }
    
    if( m_cutOption.Contains("isSingleRND")){
      _cut = "isSingle_RND";
    }

    if (IsCut(_cut)) m_cuts["cutSINGLE"] = _cut;

    if (m_debug) MessageSvc::Debug("GetIsSingleCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetBMassCut() {
    if (m_debug) MessageSvc::Debug("GetBMassCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    if (m_cutOption.Contains("massBT")) {
        _cut = CutDefRX::Mass::BT;
    } else if (m_cutOption.Contains("massBL")) {
        _cut = CutDefRX::Mass::BL;
    } else {
        _cut = ana == Analysis::MM ? CutDefRX::Mass::BMM : CutDefRX::Mass::BEE;
        if (m_cutOption.Contains("massBDTF")) {
            if (q2bin == Q2Bin::JPsi) _cut = ana == Analysis::MM ? CutDefRX::Mass::BDTFJPsMM : CutDefRX::Mass::BDTFJPsEE;
            if (q2bin == Q2Bin::Psi) _cut = ana == Analysis::MM ? CutDefRX::Mass::BDTFPsiMM : CutDefRX::Mass::BDTFPsiEE;
        }
    }

    if (IsCut(_cut)) m_cuts["cutBMASS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBMassCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetCombBVeto() {
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
            case Q2Bin::Central: _cut =  CutDefRKst::Mass::CombBVetoJPs; break;
            case Q2Bin::JPsi:    _cut = _cutMVA && CutDefRKst::Mass::CombBVetoJPs; break;
            case Q2Bin::Psi:     _cut = _cutMVA && CutDefRKst::Mass::CombBVetoPsi; break;
            default: break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutCombBVeto"] = _cut;

    if (m_debug) MessageSvc::Debug("GetCombBVeto", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetSPlotnTracks() {
    if (m_debug) MessageSvc::Debug("GetSPlotnTracks", m_cutOption);

    Year year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    if (!(year == Year::Y2011 || year == Year::Y2012 || year == Year::Run1)) {
        _cut = CutDefRX::Trigger::SPlotnTracks;
    }

    if (IsCut(_cut)) m_cuts["cutGetSPlotnTracks"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSPlotnTracks", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetMVACut() {
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
            MessageSvc::Warning("J/Psi q2 Muons, setting MVA cut as in central q2");
            _config = make_tuple(ana, year, Q2Bin::Central);
        }

        map< tuple< Analysis, Year, Q2Bin >, TString > _mvaCut = CutDefRKst::MVA::CAT;
        // if (m_cutOption.Contains("MVALOOSE") && ((ana == Analysis::EE) && ((q2bin == Q2Bin::Low) || (q2bin == Q2Bin::Central)))) _mvaCut = CutDefRKst::MVA::CAT_LOOSE;

        if (_mvaCut.find(_config) == _mvaCut.end()) {
            MessageSvc::Error("Configuration", to_string(get< 0 >(_config)), ",", to_string(get< 1 >(_config)), ",", to_string(get< 2 >(_config)));
            MessageSvc::Error("GetMVACut", (TString) "Invalid configuration (please implement it or use -noMVA)", "EXIT_FAILURE");
        } else {
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

TCut CutHolderRKst::GetPartRecoCut() {
    if (m_debug) MessageSvc::Debug("GetPartRecoCut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);

    if (ana == Analysis::ME) return _cut;

    Q2Bin q2bin = m_configHolder.GetQ2bin();

    if (q2bin == Q2Bin::JPsi) {
        switch (ana) {
            case Analysis::MM: break;
            // case Analysis::MM: _cut = CutDefRX::Background::partRecoJPsMM; break;
            case Analysis::EE: _cut = CutDefRX::Background::partRecoJPsEE; break;
            default: MessageSvc::Error("GetPartRecoCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutPR"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPartRecoCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetPIDCut() {
    if (m_debug) MessageSvc::Debug("GetPIDCut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRKst::PID::pidMM; break;
        case Analysis::EE: _cut = CutDefRKst::PID::pidEE; break;
        case Analysis::ME: _cut = CutDefRKst::PID::pidME; break;
        default: MessageSvc::Error("GetPIDCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    //Warning Order matters.
    if( ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON2")){
        if (     m_cutOption.Contains("PIDELECTRON2_PROB_0p6"))   _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p55"))  _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p5"))   _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p45"))  _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p4"))   _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p35"))  _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p3"))   _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p25"))  _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON2_PROB_0p2"))   _cut = CutDefRKst::PID::pidEE_PID2 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRKst::PID::pidEE_PID2;
    }
    if( ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON5")){
        if (     m_cutOption.Contains("PIDELECTRON5_PROB_0p6"))   _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p55"))  _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p5"))   _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p45"))  _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p4"))   _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p35"))  _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p3"))   _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p25"))  _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON5_PROB_0p2"))   _cut = CutDefRKst::PID::pidEE_PID5 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRKst::PID::pidEE_PID5;
    }
    if( ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON7")){
        if (     m_cutOption.Contains("PIDELECTRON7_PROB_0p6"))   _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.6");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p55"))  _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.55");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p5"))   _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.5");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p45"))  _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.45");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p4"))   _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p35"))  _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.35");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p3"))   _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.3");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p25"))  _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.25");
        else if (m_cutOption.Contains("PIDELECTRON7_PROB_0p2"))   _cut = CutDefRKst::PID::pidEE_PID7 && TCut("TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2");
        else _cut = CutDefRKst::PID::pidEE_PID7;
    }
    if( ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON3"))       _cut = CutDefRKst::PID::pidEE_PID3;
    if( ana == Analysis::EE && m_cutOption.Contains("PIDELECTRON3_PROB4")) _cut = CutDefRKst::PID::pidEE_PID3_Prob4;

    if (m_cutOption.Contains("noProbHH")) {
        _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::probnnK), "1");
        _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::dllK), "1");
        _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::probnnPi), "1");
    }
    if (m_cutOption.Contains("noProbMM")) _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::probnnM), "1");
    if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::probnnE), "1");
    if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRKst::PID::dllE), "1");

    _cut = UpdatePIDTune(_cut, to_string(year));

    if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));

    if (IsCut(_cut)) m_cuts["cutPID"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPIDCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetKstMassCut(){
    if (m_debug) MessageSvc::Debug("GetKstMassCut", m_cutOption);

    Analysis ana     = m_configHolder.GetAna();
    Year     year    = m_configHolder.GetYear();
    Trigger  trigger = m_configHolder.GetTrigger();

    TCut _cut =  CutDefRKst::Mass::Kst; //for MM, ME
    if( ana == Analysis::EE && ( m_cutOption.Contains("SBU") || m_configHolder.IsSB("SBU") ) ) _cut = CutDefRKst::Mass::KstLoose;
    if (IsCut(_cut)) m_cuts["cutKSTMASS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetKstMassCut", &_cut);
    return _cut;

}
TCut CutHolderRKst::GetPreSelectionCut() {
    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", m_cutOption);

    Analysis ana     = m_configHolder.GetAna();
    Year     year    = m_configHolder.GetYear();
    Trigger  trigger = m_configHolder.GetTrigger();

    TCut _cutMinLPET = GetMinLPETCut();
    bool _includeIsMuon = !m_cutOption.Contains("noMuIsMuon");
    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = _includeIsMuon ? CutDefRKst::Quality::qualityMM  : CutDefRKst::Quality::qualityMM_noIsMuon ; break;
        case Analysis::EE: _cut = CutDefRKst::Quality::qualityEE; 
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
        case Analysis::ME: _cut = CutDefRKst::Quality::qualityME ; break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    
    if (m_cutOption.Contains("noMinLPET")) {
        if (IsCut(_cutMinLPET)) _cut = ReplaceCut(_cut, TString(_cutMinLPET), "1");
    }

    if (m_cutOption.Contains("noMinHPT")) {
        _cut = ReplaceCut(_cut, TString("K_PT > 250 && Pi_PT > 250"), "1");
        _cut = ReplaceCut(_cut, TString("Kst_PT > 500"), "1");
    }

    //=========================================================
    //TODO : enable this by default if OA flatness syst is not fixed ? 
    //Simple Functor to append a max(P) cut on final states. Currently done to investigate OA issue. 
    //=========================================================        
    auto _getMaxPCut = [](const Analysis & _ana){        
        TCut _maxP(NOCUT);
        switch (_ana){
            case Analysis::EE : _maxP = "E1_P<200E3 && E2_P<200E3 && K_P<200E3 && Pi_P<200E3"; break;
            case Analysis::MM : _maxP = "M1_P<200E3 && M2_P<200E3 && K_P<200E3 && Pi_P<200E3"; break;
            case Analysis::ME : _maxP = "M1_P<200E3 && E2_P<200E3 && K_P<200E3 && Pi_P<200E3"; break;
            default : MessageSvc::Error("CutHolderRK::GetPreSelectionCut::_getMaxPCut functor, invalid switch", "","EXIT_FAILURE");break;
        }
        return _maxP ;
    };
    if(m_cutOption.Contains("MAXP")) _cut = _cut && _getMaxPCut( ana );

    if(m_cutOption.Contains("cutECALDistance") && ana == Analysis::EE){
        //Flat , all q2 bins, RK, RKst
        _cut = _cut && CutDefRX::Quality::ECALDistance;
        m_cuts["cutECAL"] = CutDefRX::Quality::ECALDistance;
    }else{
        m_cuts["cutECAL"] = TCut(NOCUT);
    }

    if( m_cutOption.Contains("noOpeningAngles")){
        MessageSvc::Warning("Removing opening angle cuts");
        _cut = ReplaceCut( _cut, TString(CutDefRKst::Quality::openingAngle), "1" );        
    }

    if (IsCut(_cut)) m_cuts["cutPS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetMinLPETCut() {
    if (m_debug) MessageSvc::Debug("GetMinLPETCut", m_cutOption);
    
    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();
    bool _MinLPETV1 = m_cutOption.Contains("L0LEv1") ? true : false;
    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = "TMath::Min(M1_PT,M2_PT) > 800"; break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run1::Y2011::trgAccL0E_ET_v1   : CutDefRX::Quality::Run1::Y2011::trgAccL0E_ET; break;
                case Year::Y2012:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run1::Y2012::trgAccL0E_ET_v1   : CutDefRX::Quality::Run1::Y2012::trgAccL0E_ET; break;
                case Year::Run1:   _cut = _MinLPETV1 ?  CutDefRX::Quality::Run1::trgAccL0E_ET_v1          : CutDefRX::Quality::Run1::trgAccL0E_ET; break;
                case Year::Y2015:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p1::Y2015::trgAccL0E_ET_v1 : CutDefRX::Quality::Run2p1::Y2015::trgAccL0E_ET; break;
                case Year::Y2016:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p1::Y2016::trgAccL0E_ET_v1 : CutDefRX::Quality::Run2p1::Y2016::trgAccL0E_ET; break;
                case Year::Run2p1: _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p1::trgAccL0E_ET_v1        : CutDefRX::Quality::Run2p1::trgAccL0E_ET; break;
                case Year::Y2017:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p2::Y2017::trgAccL0E_ET_v1 : CutDefRX::Quality::Run2p2::Y2017::trgAccL0E_ET; break;
                case Year::Y2018:  _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p2::Y2018::trgAccL0E_ET_v1 : CutDefRX::Quality::Run2p2::Y2018::trgAccL0E_ET; break;
                case Year::Run2p2: _cut = _MinLPETV1 ?  CutDefRX::Quality::Run2p2::trgAccL0E_ET_v1        : CutDefRX::Quality::Run2p2::trgAccL0E_ET; break;
                case Year::Run2  : {
                    TCut _run2p1("Year==15 || Year==16)");
                    TCut _run2p1_ET = _MinLPETV1 ?  CutDefRX::Quality::Run2p1::trgAccL0E_ET_v1          : CutDefRX::Quality::Run2p1::trgAccL0E_ET; 
                    TCut _run2p2("Year==17 || Year==18)");
                    TCut _run2p2_ET = _MinLPETV1 ?  CutDefRX::Quality::Run2p2::trgAccL0E_ET_v1          : CutDefRX::Quality::Run2p2::trgAccL0E_ET;                                         
                    _cut =  (_run2p1 && _run2p1_ET) || (_run2p2 && _run2p2_ET);
                }break;                
                case Year::All: {
                    TCut _run1("(Year==11 || Year==12)");
                    TCut _run1_ET   = _MinLPETV1 ?  CutDefRX::Quality::Run1::trgAccL0E_ET_v1            : CutDefRX::Quality::Run1::trgAccL0E_ET; 
                    TCut _run2p1("(Year==15 || Year==16)");
                    TCut _run2p1_ET = _MinLPETV1 ?  CutDefRX::Quality::Run2p1::trgAccL0E_ET_v1          : CutDefRX::Quality::Run2p1::trgAccL0E_ET; 
                    TCut _run2p2("(Year==17 || Year==18)");
                    TCut _run2p2_ET = _MinLPETV1 ?  CutDefRX::Quality::Run2p2::trgAccL0E_ET_v1          : CutDefRX::Quality::Run2p2::trgAccL0E_ET;                                                             
                    _cut =  (_run1 && _run1_ET) || (_run2p1 && _run2p1_ET) || (_run2p2 && _run2p2_ET);
                }break;
                default: MessageSvc::Error("GetMinLPETCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME: _cut = "TMath::Min(M1_PT,E2_PT) > 500"; break;
        default: MessageSvc::Error("GetMinLPETCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutLPET"] = _cut;

    if (m_debug) MessageSvc::Debug("GetMinLPETCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetQ2Cut() {
    if (m_debug) MessageSvc::Debug("GetQ2Cut", m_cutOption);
    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();
    TCut _cut(NOCUT);
    TString _q2 =  "TMath::Sq(JPs_M/1000.)";
    switch (q2bin) {
        case Q2Bin::All: {
            if(m_cutOption.Contains("q2All[")) _cut = CustomizedCut( m_cutOption, "q2All[", _q2);
            break;
        }
        case Q2Bin::Low: {
            bool _useTighterLowQ2 = SettingDef::Cut::tightLowQ2 && m_configHolder.GetAna() == Analysis::MM;
            if( _useTighterLowQ2) MessageSvc::Warning("Using tighter low q2 definition, only for muons in low Q2 enabled");
            _cut = _useTighterLowQ2 ?  CutDefRKst::Mass::Q2LowTight : CutDefRKst::Mass::Q2Low;
            if(m_cutOption.Contains("q2Low[")) _cut = CustomizedCut( m_cutOption, "q2Low[", _q2);            
            break;
        }
        case Q2Bin::Central:{
            _cut = CutDefRKst::Mass::Q2Central;            
            if(m_cutOption.Contains("q2Central[")) _cut = CustomizedCut( m_cutOption, "q2Central[", _q2);
            break;
        }
        case Q2Bin::High: {
            _cut = ana == Analysis::MM ? CutDefRKst::Mass::Q2HighMM : CutDefRKst::Mass::Q2HighEE;
            if(m_cutOption.Contains("q2High[")) _cut = CustomizedCut( m_cutOption, "q2High[", _q2);
            break;
        }
        case Q2Bin::Gamma: {
            _cut = CutDefRKst::Mass::Q2Gamma; 
            if(m_cutOption.Contains("q2Gamma[")) _cut = CustomizedCut( m_cutOption, "q2Gamma[", _q2);            
            break;
        }
        case Q2Bin::JPsi: {
            _cut = ana == Analysis::MM ? CutDefRX::Mass::JPsMM : CutDefRX::Mass::JPsEE; 
            if(m_cutOption.Contains("q2JPsi[")) _cut = CustomizedCut( m_cutOption, "q2JPsi[", _q2);        
            break;
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
        //Correct but JPs_TRUEM_REPLACED would be needed 
        m_cuts["cutQ2TRUE"] = ReplaceCut(m_cuts["cutQ2TRUE"], "JPs_M", "JPs_TRUEM");
    }
    //Convert cut to the Smeared one    
    if( ana == Analysis::EE && !m_configHolder.IsLeakageSample()) _cut = UpdateQ2Cut( _cut, m_cutOption);

    if (IsCut(_cut)) m_cuts["cutQ2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetQ2Cut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetSideBandCut() {
    if (m_debug) MessageSvc::Debug("GetSideBandCut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);

    if (ana == Analysis::ME) return _cut;

    if (m_cutOption.Contains("SBU") || m_configHolder.IsSB("SBU")) {
        switch (ana) {
            case Analysis::MM: _cut = CutDefRX::Background::upperSBMM; break;
            case Analysis::EE: _cut = CutDefRX::Background::upperSBEE; break;
            case Analysis::All: MessageSvc::Warning("GetSideBandCut", (TString) "Invalid analyses", to_string(ana)); break;
            default: MessageSvc::Error("GetSideBandCut", (TString) "Invalid analyses", to_string(ana), "EXIT_FAILURE"); break;
        }
    }
    if (m_cutOption.Contains("SBL") || m_configHolder.IsSB("SBL")) { _cut = CutDefRX::Background::lowerSB; }

    if (IsCut(_cut)) m_cuts["cutSB"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSideBandCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetSPDCut() {
    if (m_debug) MessageSvc::Debug("GetSPDCut", m_cutOption);

    Year year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (year) {
        case Year::Y2011: _cut = CutDefRX::Trigger::Run1::nSPD; break;
        case Year::Y2012: _cut = CutDefRX::Trigger::Run1::nSPD; break;
        case Year::Run1: _cut = CutDefRX::Trigger::Run1::nSPD; break;
        case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::nSPD; break;
        case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::nSPD; break;
        case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::nSPD; break;
        case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::nSPD; break;
        case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::nSPD; break;
        case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::nSPD; break;
        case Year::All: MessageSvc::Warning("GetSPDCut", (TString) "Invalid year", to_string(year)); break;
        default: MessageSvc::Error("GetSPDCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutSPD"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSPDCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetTCKCut() {
    if (m_debug) MessageSvc::Debug("GetTCKCut", m_cutOption);

    Analysis ana     = m_configHolder.GetAna();
    Year     year    = m_configHolder.GetYear();
    Trigger  trigger = m_configHolder.GetTrigger();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Quality::Run1::Y2011::trgAccL0M_ADCPT; break;
                case Year::Y2012: _cut = CutDefRX::Quality::Run1::Y2012::trgAccL0M_ADCPT; break;
                case Year::Run1: _cut = CutDefRX::Quality::Run1::trgAccL0M_ADCPT; break;
                case Year::Y2015: _cut = CutDefRX::Quality::Run2p1::Y2015::trgAccL0M_ADCPT; break;
                case Year::Y2016: _cut = CutDefRX::Quality::Run2p1::Y2016::trgAccL0M_ADCPT; break;
                case Year::Run2p1: _cut = CutDefRX::Quality::Run2p1::trgAccL0M_ADCPT; break;
                case Year::Y2017: _cut = CutDefRX::Quality::Run2p2::Y2017::trgAccL0M_ADCPT; break;
                case Year::Y2018: _cut = CutDefRX::Quality::Run2p2::Y2018::trgAccL0M_ADCPT; break;
                case Year::Run2p2: _cut = CutDefRX::Quality::Run2p2::trgAccL0M_ADCPT; break;
                case Year::All: MessageSvc::Warning("GetTCKCut", (TString) "Invalid year", to_string(year)); break;
                default: MessageSvc::Error("GetTCKCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Quality::Run1::Y2011::trgAccL0E_ADCET; break;
                case Year::Y2012: _cut = CutDefRX::Quality::Run1::Y2012::trgAccL0E_ADCET; break;
                case Year::Run1: _cut = CutDefRX::Quality::Run1::trgAccL0E_ADCET; break;
                case Year::Y2015: _cut = CutDefRX::Quality::Run2p1::Y2015::trgAccL0E_ADCET; break;
                case Year::Y2016: _cut = CutDefRX::Quality::Run2p1::Y2016::trgAccL0E_ADCET; break;
                case Year::Run2p1: _cut = CutDefRX::Quality::Run2p1::trgAccL0E_ADCET; break;
                case Year::Y2017: _cut = CutDefRX::Quality::Run2p2::Y2017::trgAccL0E_ADCET; break;
                case Year::Y2018: _cut = CutDefRX::Quality::Run2p2::Y2018::trgAccL0E_ADCET; break;
                case Year::Run2p2: _cut = CutDefRX::Quality::Run2p2::trgAccL0E_ADCET; break;
                case Year::All: MessageSvc::Warning("GetTCKCut", (TString) "Invalid year", to_string(year)); break;
                default: MessageSvc::Error("GetTCKCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME: _cut = TCut(NOCUT); break;
        default: MessageSvc::Error("GetTCKCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    if (IsCut(_cut)) {
        switch (trigger) {
            case Trigger::L0I: _cut = TCut(NOCUT); break;
            case Trigger::L0L: _cut = _cut; break;
            case Trigger::All: _cut = CutDefRX::Trigger::L0I || _cut; break;
            default: MessageSvc::Error("GetTCKCut", (TString) "Invalid trg", to_string(trigger), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutTCK"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTCKCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetHLT1TCKCutAlignment(){
    if (m_debug) MessageSvc::Debug("GetHLT1TCKCutAlignment", m_cutOption);    
    Year    year = m_configHolder.GetYear();    
    Trigger  trg = m_configHolder.GetTrigger();    
    Analysis ana = m_configHolder.GetAna();    

    TCut _cut(NOCUT);     

    if ( m_cutOption.Contains("HLT1TCK") && !m_configHolder.IsMC() && ana == Analysis::MM && trg == Trigger::L0L ) {
        switch( year) {
            case Year::Y2011 : _cut = GetHLT1TCKCut(year); break;
            case Year::Y2012 : _cut = GetHLT1TCKCut(year); break;
            case Year::Run1  : _cut = GetHLT1TCKCut(Year::Run1); break;
            case Year::Y2015 : _cut = GetHLT1TCKCut(year); break;
            case Year::Y2016 : _cut = GetHLT1TCKCut(year); break;
            case Year::Run2p1: _cut = GetHLT1TCKCut(Year::Run2p1); break;
            case Year::Y2017 : _cut = GetHLT1TCKCut(year); break;
            case Year::Y2018 : _cut = TCut(NOCUT); break;
            case Year::Run2p2: _cut = (GetHLT1TCKCut(Year::Y2017) || TCut("Year==18")); break;
            case Year::All : MessageSvc::Warning("GetHLT1Cut [ HLT1TCK ] ALL years-> no alignment");break;
            default: MessageSvc::Error("GetHLT1Cut [HLT1TCK]", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
        }
        const bool _addOrRemove = CutDefRX::Quality::HLT1TCK.at(year).first;
        if (!_addOrRemove) _cut = TCut(TString("!(") + _cut.GetTitle() + ")");        
    }else{
        MessageSvc::Warning("HLT1TCKCut applicable only for Data, Muon, L0L category");
    }
    if( IsCut(_cut)) m_cuts["HLT1TCK"] = _cut;
    if (m_debug) MessageSvc::Debug("GetHLT1TCKCutAlignment", &_cut);    
    return _cut;
}
TCut CutHolderRKst::GetTrackCut() {
    if (m_debug) MessageSvc::Debug("GetTrackCut", m_cutOption);
    Track track = m_configHolder.GetTrack();
    Year  year  = m_configHolder.GetYear();
    TCut _cut = TCut(NOCUT);
    bool _add = false;
    if( track != Track::All && m_cutOption.Contains("noHLT")){
        //you are tistossing HLT 
        map<Year,map<TString,vector<TString>>> ALL_tags_hlt1 = 
        {   { Year::Y2011 , { { "ORALL" , { "Hlt1TrackAllL0Decision", "Hlt1TrackMuonDecision",   "Hlt1TrackDiMuonDecision"} } } },
            { Year::Y2012 , { { "ORALL" , { "Hlt1TrackAllL0Decision", "Hlt1TrackMuonDecision",   "Hlt1TrackDiMuonDecision"} } } }, 
            { Year::Run1 ,  { { "ORALL" , { "Hlt1TrackAllL0Decision", "Hlt1TrackMuonDecision",   "Hlt1TrackDiMuonDecision"} } } },
            { Year::Y2015 , { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision"} } } }, //< on EE sample the TrackMuon is not present as branch!
            { Year::Y2016 , { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision", "Hlt1TrackMuonMVADecision"} } } }, 
            { Year::Run2p1, { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision"} } } },
            { Year::Y2017,  { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision","Hlt1TrackMuonMVADecision"} } } },
            { Year::Y2018,  { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision","Hlt1TrackMuonMVADecision"} } } },                                                         
            { Year::Run2p2, { { "ORALL" , { "Hlt1TrackMVADecision",   "Hlt1TwoTrackMVADecision", "Hlt1TrackMuonDecision","Hlt1TrackMuonMVADecision"} } } }
        };
        auto _allyears = {Year::Y2011, Year::Y2012, Year::Run1, Year::Y2015, Year::Y2016, Year::Run2p1,  Year::Y2017, Year::Y2018, Year::Run2p2};
        //add everywhere the slot for PHYS
        for( auto & _year : _allyears){
            ALL_tags_hlt1[_year]["PHYS"] = {"Hlt1Phys_TIS"};
        }
        //EE sample in different years has not TrackMuon at HLT1 decisions, remove from vector.
        if( m_configHolder.GetAna() == Analysis::EE){
            vector<TString> _toRemove = {"Hlt1TrackMuonDecision", "Hlt1TrackMuonMVADecision"};
            for( auto & _year : { Year::Y2015, Year::Y2016, Year::Run2p1, Year::Y2017, Year::Y2018, Year::Run2p1}){
                for( TString & cutNotExist : _toRemove  ){
                    ALL_tags_hlt1[_year]["ORALL"].erase(std::remove(ALL_tags_hlt1[_year]["ORALL"].begin(), ALL_tags_hlt1[_year]["ORALL"].end(), cutNotExist), ALL_tags_hlt1[_year]["ORALL"].end());
                }
            }
        }
        map<Year,map<TString,vector<TString>>> ALL_tags_hlt2 = 
            {   { Year::Y2011 , { { "ORALL" ,{ "Hlt2Topo2BodyBBDTDecision"   , "Hlt2Topo3BodyBBDTDecision"  , "Hlt2Topo4BodyBBDTDecision", //Topo{2,3,4}BodyBBDTDecision
                                               "Hlt2TopoMu2BodyBBDTDecision" , "Hlt2TopoMu3BodyBBDTDecision","Hlt2TopoMu4BodyBBDTDecision","Hlt2DiMuonDetachedDecision", 
                                               "Hlt2TopoE2BodyBBDTDecision"  , "Hlt2TopoE3BodyBBDTDecision" ,"Hlt2TopoE4BodyBBDTDecision" } } }} ,
                { Year::Y2012 , { { "ORALL" , { "Hlt2Topo2BodyBBDTDecision"  , "Hlt2Topo3BodyBBDTDecision"  , "Hlt2Topo4BodyBBDTDecision", 
                                              "Hlt2TopoMu2BodyBBDTDecision", "Hlt2TopoMu3BodyBBDTDecision","Hlt2TopoMu4BodyBBDTDecision","Hlt2DiMuonDetachedDecision", 
                                              "Hlt2TopoE2BodyBBDTDecision" , "Hlt2TopoE3BodyBBDTDecision" ,"Hlt2TopoE4BodyBBDTDecision" }  } }} , 
                { Year::Run1 ,  { { "ORALL" , { "Hlt2Topo2BodyBBDTDecision"  , "Hlt2Topo3BodyBBDTDecision"  , "Hlt2Topo4BodyBBDTDecision", 
                                              "Hlt2TopoMu2BodyBBDTDecision","Hlt2TopoMu3BodyBBDTDecision" ,"Hlt2TopoMu4BodyBBDTDecision","Hlt2DiMuonDetachedDecision",
                                              "Hlt2TopoE2BodyBBDTDecision", "Hlt2TopoE3BodyBBDTDecision"  ,"Hlt2TopoE4BodyBBDTDecision" }  } }} ,
                { Year::Y2015 , { { "ORALL" , { "Hlt2Topo2BodyDecision"   ,   "Hlt2Topo3BodyDecision"  ,  "Hlt2Topo4BodyDecision", //ok EE/MM
                                              "Hlt2TopoMu2BodyDecision" ,   "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision"} } } },  //ok MM only
                { Year::Y2016 , { { "ORALL" , { "Hlt2Topo2BodyDecision",          "Hlt2Topo3BodyDecision",    "Hlt2Topo4BodyDecision",
                                              "Hlt2TopoMu2BodyDecision",    "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision",
                                              "Hlt2TopoMuMu2BodyDecision",  "Hlt2TopoMuMu3BodyDecision","Hlt2TopoMuMu4BodyDecision",
                                              "Hlt2TopoE2BodyDecision",     "Hlt2TopoE3BodyDecision",   "Hlt2TopoE4BodyDecision",
                                              "Hlt2TopoEE2BodyDecision",    "Hlt2TopoEE3BodyDecision",  "Hlt2TopoEE4BodyDecision"} } }} ,  //ok MM only
                { Year::Run2p1, { { "ORALL" , { "Hlt2Topo2BodyDecision",      "Hlt2Topo3BodyDecision",    "Hlt2Topo4BodyDecision", 
                                              "Hlt2TopoMu2BodyDecision",    "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision"} } } },
                { Year::Y2017,  { { "ORALL" , { "Hlt2Topo2BodyDecision",          "Hlt2Topo3BodyDecision",    "Hlt2Topo4BodyDecision",
                                              "Hlt2TopoMu2BodyDecision",    "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision",
                                              "Hlt2TopoMuMu2BodyDecision",  "Hlt2TopoMuMu3BodyDecision","Hlt2TopoMuMu4BodyDecision",
                                              "Hlt2TopoE2BodyDecision",     "Hlt2TopoE3BodyDecision",   "Hlt2TopoE4BodyDecision",
                                              "Hlt2TopoEE2BodyDecision",    "Hlt2TopoEE3BodyDecision",  "Hlt2TopoEE4BodyDecision"} } } } ,  //ok MM only
                { Year::Y2018,  { { "ORALL" ,  { "Hlt2Topo2BodyDecision",         "Hlt2Topo3BodyDecision",    "Hlt2Topo4BodyDecision",
                                              "Hlt2TopoMu2BodyDecision",    "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision",
                                              "Hlt2TopoMuMu2BodyDecision",  "Hlt2TopoMuMu3BodyDecision","Hlt2TopoMuMu4BodyDecision",
                                              "Hlt2TopoE2BodyDecision",     "Hlt2TopoE3BodyDecision",   "Hlt2TopoE4BodyDecision",
                                              "Hlt2TopoEE2BodyDecision",    "Hlt2TopoEE3BodyDecision",  "Hlt2TopoEE4BodyDecision"} } } },  //ok MM only 
                { Year::Run2p2,  { { "ORALL" , { "Hlt2Topo2BodyDecision",          "Hlt2Topo3BodyDecision",    "Hlt2Topo4BodyDecision",
                                              "Hlt2TopoMu2BodyDecision",    "Hlt2TopoMu3BodyDecision",  "Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision",
                                              "Hlt2TopoMuMu2BodyDecision",  "Hlt2TopoMuMu3BodyDecision","Hlt2TopoMuMu4BodyDecision",
                                              "Hlt2TopoE2BodyDecision",     "Hlt2TopoE3BodyDecision",   "Hlt2TopoE4BodyDecision",
                                              "Hlt2TopoEE2BodyDecision",    "Hlt2TopoEE3BodyDecision",  "Hlt2TopoEE4BodyDecision"} } } }  //ok MM only 
        };
        if( m_configHolder.GetAna() == Analysis::EE){
           vector<TString>  _toRemove = { "Hlt2TopoMu2BodyDecision","Hlt2TopoMu3BodyDecision","Hlt2TopoMu4BodyDecision","Hlt2DiMuonDetachedHeavyDecision",
                                          "Hlt2TopoMuMu2BodyDecision","Hlt2TopoMuMu3BodyDecision","Hlt2TopoMuMu4BodyDecision"};
            for( auto & _year : { Year::Y2015, Year::Y2016, Year::Run2p1, Year::Y2017, Year::Y2018, Year::Run2p1}){
                for( TString & cutNotExist : _toRemove ){
                    ALL_tags_hlt2[_year]["ORALL"].erase(std::remove(ALL_tags_hlt2[_year]["ORALL"].begin(), ALL_tags_hlt2[_year]["ORALL"].end(), cutNotExist), ALL_tags_hlt2[_year]["ORALL"].end());
                }
            }
        }
        if( m_configHolder.GetAna() == Analysis::MM){
           vector<TString>  _toRemove = { "Hlt2TopoE2BodyDecision","Hlt2TopoE3BodyDecision","Hlt2TopoE4BodyDecision",
                                          "Hlt2TopoEE2BodyDecision","Hlt2TopoEE3BodyDecision","Hlt2TopoEE4BodyDecision"};
            for( auto & _year : { Year::Y2015, Year::Y2016, Year::Run2p1, Year::Y2017, Year::Y2018, Year::Run2p1}){
                for( TString & cutNotExist : _toRemove ){
                    ALL_tags_hlt2[_year]["ORALL"].erase(std::remove(ALL_tags_hlt2[_year]["ORALL"].begin(), ALL_tags_hlt2[_year]["ORALL"].end(), cutNotExist), ALL_tags_hlt2[_year]["ORALL"].end());
                }
            }
        }
        //fill up the PHYS slot and update the cut name with {HEAD} in front.
        for( auto & _year : _allyears){
            ALL_tags_hlt1[_year]["PHYS"] = {"{HEAD}_Hlt1Phys_TIS"};
            ALL_tags_hlt2[_year]["PHYS"] = {"{HEAD}_Hlt2Phys_TIS"};
            for( int i = 0 ; i < ALL_tags_hlt1[_year]["ORALL"].size(); ++i ){ ALL_tags_hlt1[_year]["ORALL"][i] = TString("{HEAD}_") + ALL_tags_hlt1[_year]["ORALL"][i] +"_TIS";  }
            for( int i = 0 ; i < ALL_tags_hlt2[_year]["ORALL"].size(); ++i ){ ALL_tags_hlt2[_year]["ORALL"][i] = TString("{HEAD}_") + ALL_tags_hlt2[_year]["ORALL"][i] +"_TIS";  }
        }
        auto tags_hlt1 = ALL_tags_hlt1.at(m_configHolder.GetYear());
        auto tags_hlt2 = ALL_tags_hlt2.at(m_configHolder.GetYear());

        //Our TOS HLT1 , HLT2
        TCut _hlt1TOSCut = GetHLT1Cut();
        TCut _hlt2TOSCut = GetHLT2Cut();
        m_cuts["cutHLT1"] = TCut(NOCUT);
        m_cuts["cutHLT2"] = TCut(NOCUT);
        //Grab the HLT1 TIS category for TISTOSSing
        //options available : (TAGHLT1ORALL,TAGHLTORALL,TAGHLT1PHYS,TAGHLTPHYS,TAGHLT2ORALL,  )
        TCut _hlt1TISCut = TCut(NOCUT);    
        if( m_cutOption.Contains("TAGHLTORALL")){
            _hlt1TISCut = JoinCut( tags_hlt1["ORALL"], "||");
        }else if( m_cutOption.Contains("TAGHLTPHYS")) {
            _hlt1TISCut = JoinCut( tags_hlt1["PHYS"], "||");
        }else{
            MessageSvc::Error("TISTO HL1 , add one option for TRACK!=All noHLT-(TAGHLT1ORALL,TAGHLTORALL,TAGHLT1PHYS,TAGHLTPHYS)",m_cutOption,"EXIT_FAILURE");
        }
        //Grab the HLT2 TIS category for TISTOSSing
        TCut _hlt2TISCut = TCut(NOCUT);
        if( m_cutOption.Contains("TAGHLTORALL")){
            _hlt2TISCut = JoinCut( tags_hlt2["ORALL"],"||");
        }else if( m_cutOption.Contains("TAGHLTPHYS")){
            _hlt2TISCut = JoinCut( tags_hlt2["PHYS"],"||");
        }else{
            MessageSvc::Error("TISTO HL2 , add one option noHLT-(TAGHLT1ORALL,TAGHLTORALL,TAGHLT1PHYS,TAGHLTPHYS)",m_cutOption,"EXIT_FAILURE");
        }
        //Grab the  TISTOS logic for PRB = ( TIS & TOS ), TAG = ( TIS )
        if( track == Track::PRB){
            _cut = (_hlt1TOSCut &&  _hlt2TOSCut) && ( _hlt1TISCut && _hlt2TISCut);
            m_cuts["cutHLT1"] = _hlt1TOSCut &&  _hlt1TISCut;
            m_cuts["cutHLT2"] = _hlt2TOSCut &&  _hlt2TISCut;
            if(m_cutOption.Contains("TISTOSHLT1") ){
                _cut = ( _hlt1TOSCut ) && ( _hlt1TISCut && _hlt2TISCut);
                m_cuts["cutHLT1"] = _hlt1TOSCut &&  _hlt1TISCut;
                m_cuts["cutHLT2"] = _hlt2TISCut;                
            }
        }else if( track == Track::TAG){            
            _cut = (_hlt1TISCut && _hlt2TISCut);
            m_cuts["cutHLT1"] = _hlt1TISCut;
            m_cuts["cutHLT2"] = _hlt2TISCut;           
            if(m_cutOption.Contains("TISTOSHLT1") ) {
                _cut = ( _hlt1TISCut && _hlt2TISCut);
                m_cuts["cutHLT1"] = _hlt1TISCut;
                m_cuts["cutHLT2"] = _hlt2TISCut;                
            }
        }        
        _add = true;   
    }
    if (IsCut(_cut) && _add) m_cuts["cutTRACK"] = _cut;
    if (m_debug) MessageSvc::Debug("GetTrackCut", &_cut);
    return _cut;
} 

TCut CutHolderRKst::GetTriggerCut() {
    if (m_debug) MessageSvc::Debug("GetTriggerCut", m_cutOption);

    Analysis    ana  = m_configHolder.GetAna();
    Year        year = m_configHolder.GetYear();
    Trigger     trg  = m_configHolder.GetTrigger();
    TriggerConf trgc = m_configHolder.GetTriggerConf();

    TCut _cut(NOCUT);
    if (!m_cutOption.Contains("noL0")) _cut = _cut && GetL0Cut();
    if (!m_cutOption.Contains("noHLT")) _cut = _cut && GetHLT1Cut() && GetHLT2Cut();
    if ( m_cutOption.Contains("noHLT1") && m_cutOption.Contains("noHLT2")){
        MessageSvc::Warning("dropping both HLT1 and HLT2 cuts");
    }
    else if (m_cutOption.Contains("noHLT1")) _cut = _cut && GetHLT2Cut();
    else if (m_cutOption.Contains("noHLT2")) _cut = _cut && GetHLT1Cut();    
    
    if (IsCut(_cut)) m_cuts["cutTRG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTriggerCut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetL0Cut() {
    if (m_debug) MessageSvc::Debug("GetL0Cut", m_cutOption);

    Analysis    ana  = m_configHolder.GetAna();
    Year        year = m_configHolder.GetYear();
    Trigger     trg  = m_configHolder.GetTrigger();
    TriggerConf trgc = m_configHolder.GetTriggerConf();
    Prj         prj  = m_configHolder.GetProject();

    TCut _cut(NOCUT);
    if (trg == Trigger::All) {
        if (m_debug) MessageSvc::Debug("Trigger::All", m_cutOption);
        switch (trgc) {
            default:
                if (m_debug) MessageSvc::Debug("TriggerConf::default", m_cutOption);
                switch (ana) {
                    case Analysis::MM: _cut = (CutDefRX::Trigger::L0I || CutDefRX::Trigger::L0M); break;
                    case Analysis::EE: _cut = (CutDefRX::Trigger::L0I || CutDefRX::Trigger::L0E); break;
                    case Analysis::ME: _cut = (CutDefRX::Trigger::L0I || CutDefRX::Trigger::L0E2); break;
                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                }
                break;
        }
    } else {
        switch (trgc) {
            case TriggerConf::Inclusive:
                if (m_debug) MessageSvc::Debug("TriggerConf::Inclusive", m_cutOption);
                switch (trg) {
                    case Trigger::L0I:
                        if (m_debug) MessageSvc::Debug("Trigger::L0I", m_cutOption);
                        _cut = CutDefRX::Trigger::L0I;
                        break;
                    case Trigger::L0L:
                        if (m_debug) MessageSvc::Debug("Trigger::L0L", m_cutOption);
                        switch (ana) {
                            case Analysis::MM: _cut = CutDefRX::Trigger::L0M; break;
                            case Analysis::EE: _cut = CutDefRX::Trigger::L0E; break;
                            case Analysis::ME: _cut = CutDefRX::Trigger::L0E2; break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                        }
                        break;
                    case Trigger::L0H:
                        if (m_debug) MessageSvc::Debug("Trigger::L0H", m_cutOption);
                        switch (prj) {
                            case Prj::RKst: _cut = CutDefRKst::Trigger::L0H; break;
                            case Prj::RK:   _cut = CutDefRK::Trigger::L0H; break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid project", to_string(prj), "EXIT_FAILURE"); break;
                        }
                        break;
                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
                }
                break;

            case TriggerConf::Exclusive:
                if (m_debug) MessageSvc::Debug("TriggerConf::Exclusive", m_cutOption);
                switch (trg) {
                    case Trigger::L0I:
                        if (m_debug) MessageSvc::Debug("Trigger::L0I", m_cutOption);
                        _cut = CutDefRX::Trigger::L0I;
                        break;
                    case Trigger::L0L:
                        if (m_debug) MessageSvc::Debug("Trigger::L0L", m_cutOption);
                        switch (ana) {
                            case Analysis::MM: _cut = (CutDefRX::Trigger::L0M && !CutDefRX::Trigger::L0I); break;
                            case Analysis::EE: _cut = (CutDefRX::Trigger::L0E && !CutDefRX::Trigger::L0I); break;
                            case Analysis::ME: _cut = (CutDefRX::Trigger::L0E2 && !CutDefRX::Trigger::L0I); break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                        }
                        break;
                    case Trigger::L0H:
                        if (m_debug) MessageSvc::Debug("Trigger::L0H", m_cutOption);
                        switch (prj) {
                            case Prj::RKst:
                                switch (ana) {
                                    case Analysis::MM: _cut = (CutDefRKst::Trigger::L0H && !CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0M); break;
                                    case Analysis::EE: _cut = (CutDefRKst::Trigger::L0H && !CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0E); break;
                                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                                }
                                break;
                            case Prj::RK:
                                switch (ana) {
                                    case Analysis::MM: _cut = (CutDefRK::Trigger::L0H && !CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0M); break;
                                    case Analysis::EE: _cut = (CutDefRK::Trigger::L0H && !CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0E); break;
                                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                                }
                                break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid project", to_string(prj), "EXIT_FAILURE"); break;
                        }
                        break;
                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
                }
                break;

            case TriggerConf::Exclusive2:
                if (m_debug) MessageSvc::Debug("TriggerConf::Exclusive2", m_cutOption);
                switch (trg) {
                    case Trigger::L0L:
                        if (m_debug) MessageSvc::Debug("Trigger::L0L", m_cutOption);
                        switch (ana) {
                            case Analysis::MM: _cut = CutDefRX::Trigger::L0M; break;
                            case Analysis::EE: _cut = CutDefRX::Trigger::L0E; break;
                            case Analysis::ME: _cut = CutDefRX::Trigger::L0E2; break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                        }
                        break;
                    case Trigger::L0I:
                        if (m_debug) MessageSvc::Debug("Trigger::L0I", m_cutOption);
                        switch (ana) {
                            case Analysis::MM: _cut = (CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0M); break;
                            case Analysis::EE: _cut = (CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0E); break;
                            case Analysis::ME: _cut = (CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0E2); break;
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                        }
                        break;
                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
                }
                break;

            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trgc", to_string(trgc), "EXIT_FAILURE"); break;
        }
    }


    if (m_cutOption.Contains("L0TIS")) {
        /*
          Cut options used for triggering TISTOS method for L0 module.
          Which TIS category to use?
          Defaults are the ones in the maps. 
          Customization achieved via  customL0TIS{}, customL0TISTOSL1{}, customL0TISTOSL2{} and configuring SettingDef::Cut::L0TAG
          As this is done in python  , we have to configure r.SettingDef.Cut.L0TAG along the way.
        */      
        map< pair< Trigger, Analysis >  , vector<TCut> > _L0tiscut = {
           { { Trigger::L0I , Analysis::MM }, { CutDefRX::Trigger::L0O }},
           { { Trigger::L0I , Analysis::EE }, { CutDefRX::Trigger::L0O }},
           { { Trigger::L0L , Analysis::MM }, { CutDefRX::Trigger::L0I , CutDefRX::Trigger::L0I } },
           { { Trigger::L0L , Analysis::EE }, { CutDefRX::Trigger::L0I , CutDefRX::Trigger::L0I } },
           { { Trigger::L0H , Analysis::MM }, { CutDefRX::Trigger::L0I }},
           { { Trigger::L0H , Analysis::EE }, { CutDefRX::Trigger::L0I }},
        };
        TCut _BMuonTIS    = TCut(Form("({HEAD}_L0MuonDecision_TIS && L0Data_Muon1_Pt > %i)", CutDefRX::Quality::L0Muon_ADCThreshold.at( year)));
        TCut _l1MuonTOS   = "M1_L0MuonDecision_TOS";
        TCut _l2MuonTOS   = "M2_L0MuonDecision_TOS";
        TCut _l1l2MuonTOS =  TCut(Form("((M1_L0MuonDecision_TOS || M2_L0MuonDecision_TOS) && L0Data_Muon1_Pt > %i)", CutDefRX::Quality::L0Muon_ADCThreshold.at( year)));
        TCut _hadAccCut   = prj == Prj::RK ? TCut("K_L0Calo_HCAL_region >= 0") : TCut("K_L0Calo_HCAL_region >= 0 && Pi_L0Calo_HCAL_region >= 0");

        //we can try different tags for the TIS category configurable
        if ( m_cutOption.Contains("-NominalTAG")) {
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)] = { TCut("({HEAD}_L0HadronDecision_TIS || {HEAD}_L0ElectronDecision_TIS)"), TCut("({HEAD}_L0HadronDecision_TIS || {HEAD}_L0ElectronDecision_TIS)") };
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::EE)] = { TCut(Form("({HEAD}_L0HadronDecision_TIS || %s)", _BMuonTIS.GetTitle())), TCut(Form("({HEAD}_L0HadronDecision_TIS || %s)", _BMuonTIS.GetTitle())) };
            
            _L0tiscut[ make_pair( Trigger::L0I , Analysis::MM)] = { TCut(Form("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500) || (%s)", _l1l2MuonTOS.GetTitle())) };
            _L0tiscut[ make_pair( Trigger::L0I , Analysis::EE)] = { TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3200) || (E2_L0ElectronDecision_TOS &&  E2_L0Calo_ECAL_realET > 3200)") };

            _L0tiscut[ make_pair( Trigger::L0H , Analysis::MM)] = { TCut(Form("%s && (%s)", _l1l2MuonTOS.GetTitle(), _hadAccCut.GetTitle())) };
            _L0tiscut[ make_pair( Trigger::L0H , Analysis::EE)] = { TCut(Form("((E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3200) || (E2_L0ElectronDecision_TOS &&  E2_L0Calo_ECAL_realET > 3200)) && (%s)", _hadAccCut.GetTitle())) };
        }
        if ( m_cutOption.Contains("-HadronTAG")) {
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)] = { TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)"), TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)") };
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::EE)] = { TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)"), TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)") };

            _L0tiscut[ make_pair( Trigger::L0I , Analysis::MM)] = { TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)") };
            _L0tiscut[ make_pair( Trigger::L0I , Analysis::EE)] = { TCut("({H1}_L0HadronDecision_TOS && {H1}_L0Calo_HCAL_realET > 3500) || ({H2}_L0HadronDecision_TOS && {H2}_L0Calo_HCAL_realET > 3500)") };

            _L0tiscut[ make_pair( Trigger::L0H , Analysis::MM)] = { TCut(Form("{HEAD}_L0HadronDecision_TIS && (%s)", _hadAccCut.GetTitle())) };
            _L0tiscut[ make_pair( Trigger::L0H , Analysis::EE)] = { TCut(Form("{HEAD}_L0HadronDecision_TIS && (%s)", _hadAccCut.GetTitle())) };
        }
        if ( m_cutOption.Contains("-LeptonTAG")) {
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)] = { TCut("(M2_L0MuonDecision_TOS && M2_PT > 2000)"), TCut("(M1_L0MuonDecision_TOS && M1_PT > 2000)") };
            _L0tiscut[ make_pair( Trigger::L0L , Analysis::EE)] = { TCut("(E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3200)"), TCut("(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3200)") };

            _L0tiscut[ make_pair( Trigger::L0I , Analysis::MM)] = { TCut(Form("%s", _l1l2MuonTOS.GetTitle())) };
            _L0tiscut[ make_pair( Trigger::L0I , Analysis::EE)] = { TCut("(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3200) || (E2_L0ElectronDecision_TOS &&  E2_L0Calo_ECAL_realET > 3200)") };
            
            _L0tiscut[ make_pair( Trigger::L0H , Analysis::MM)] = { TCut(Form("({HEAD}_L0ElectronDecision_TIS || %s) && (%s)", _BMuonTIS.GetTitle(), _hadAccCut.GetTitle())) };
            _L0tiscut[ make_pair( Trigger::L0H , Analysis::EE)] = { TCut(Form("({HEAD}_L0ElectronDecision_TIS || %s) && (%s)", _BMuonTIS.GetTitle(), _hadAccCut.GetTitle())) };
        }
        _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)][0] = _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)][0] && "M2_PT>800";
        _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)][1] = _L0tiscut[ make_pair( Trigger::L0L , Analysis::MM)][1] && "M1_PT>800";
        map< pair< Trigger, Analysis > , vector<TCut> > _L0toscut = { 
           { { Trigger::L0I , Analysis::MM }, { CutDefRX::Trigger::L0I }}, 
           { { Trigger::L0I , Analysis::EE }, { CutDefRX::Trigger::L0I }} ,
           { { Trigger::L0L , Analysis::MM }, { TCut(_l1MuonTOS), TCut(_l2MuonTOS) }} ,
           { { Trigger::L0L , Analysis::EE }, { CutDefRX::Trigger::L0E1 , CutDefRX::Trigger::L0E2 }} ,
           { { Trigger::L0H , Analysis::MM }, { prj == Prj::RK ? CutDefRK::Trigger::L0H : CutDefRKst::Trigger::L0H }}, 
           { { Trigger::L0H , Analysis::EE }, { prj == Prj::RK ? CutDefRK::Trigger::L0H : CutDefRKst::Trigger::L0H }}, 
        };
        //wrap the TIS cut ( Tag cut )
        auto  _tisCut = _L0tiscut.at( { trg, ana}).at(0);
        if ( m_cutOption.Contains("L0TISL1") ||  m_cutOption.Contains("L0TISTOSL1")) {
            _tisCut = _L0tiscut.at( { trg, ana}).at(0);
            if( m_cutOption.Contains("TISTOSBREM0") && ana == Analysis::EE){
                _tisCut = _tisCut && "E1_HasBremAdded==0";
            }
            if( m_cutOption.Contains("TISTOSBREM1") && ana == Analysis::EE){
                _tisCut = _tisCut && "E1_HasBremAdded==1";
            }            
        } else if ( m_cutOption.Contains("L0TISL2") ||  m_cutOption.Contains("L0TISTOSL2")) {
            _tisCut = _L0tiscut.at( { trg, ana}).at(1);
            if( m_cutOption.Contains("TISTOSBREM0") && ana == Analysis::EE){
                _tisCut = _tisCut && "E2_HasBremAdded==0";
            }
            if( m_cutOption.Contains("TISTOSBREM1") && ana == Analysis::EE){
                _tisCut = _tisCut && "E2_HasBremAdded==1";
            }
        } else if ( m_cutOption.Contains("L0TISLComb") ||  m_cutOption.Contains("L0TISTOSLComb")) {
            _tisCut = _L0tiscut.at( { trg, ana}).at(0);
        }
        //wrap the TOS cut ( Probe cut )
        auto _tosCut = _L0toscut.at( { trg, ana}).at(0);
        if( m_cutOption.Contains("L0TISTOSL2") && trg == Trigger::L0L) {
            //Pick particle 2 TISTOS cut
            _tosCut = _L0toscut.at( { trg, ana}).at(1);
        } else if ( m_cutOption.Contains("L0TISTOSLComb") && trg == Trigger::L0L) {
            _tosCut = _L0toscut.at( { trg, ana}).at(0) || _L0toscut.at( { trg, ana}).at(1);
        }
        //Assign the cut
        _cut = _tisCut;
        if ( m_cutOption.Contains("L0TISTOS")){
            _cut = _tisCut && _tosCut;
        }
    }

    if (m_cutOption.Contains("noMinHPT")) {
        _cut = ReplaceCut(_cut, TString("Kst_PT > 3000"), "1");
        _cut = ReplaceCut(_cut, TString("K_L0Calo_HCAL_realET > 4000"), "1");
    }
    
    if (IsCut(_cut)) m_cuts["cutL0"] = _cut;

    if (m_debug) MessageSvc::Debug("GetL0Cut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetHLT1Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT1Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();
    Trigger  trg  = m_configHolder.GetTrigger();

    TCut _cut(NOCUT);
    auto HLT1MMCut = [&](const Year _year , TString _option ){
        TCut _CUTRETURN_ = TCut(NOCUT);
        if(GetRunFromYear(_year) == Year::Run1 ){
            _CUTRETURN_ = CutDefRX::Trigger::Run1::Hlt1MM;
            if( _option.Contains("TrackMuonOR"))    _CUTRETURN_ = _CUTRETURN_ || CutDefRX::Trigger::Run1::Hlt1TrkM;        
            if( _option.Contains("TrackMuonALONE")) _CUTRETURN_ = CutDefRX::Trigger::Run1::Hlt1TrkM;
        }else if( GetRunFromYear(_year) == Year::Run2p1 ){
            _CUTRETURN_ = CutDefRX::Trigger::Run2p1::Hlt1MM;
            // if( _option.Contains("TrackMuonOR")) _cut = _cut || CutDefRX::Trigger::Run2p1::Hlt1TrkM;        
            // if( _option.Contains("TrackMuonALONE")) _cut = CutDefRX::Trigger::Run2p1::Hlt1TrkM;
        }else if( GetRunFromYear(_year) == Year::Run2p2 ){
            _CUTRETURN_ = CutDefRX::Trigger::Run2p2::Hlt1MM;
            // if( _option.Contains("TrackMuonOR")) _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt1TrkM;        
            // if( _option.Contains("TrackMuonALONE")) _cut = CutDefRX::Trigger::Run2p2::Hlt1TrkM;            
        }
        return _CUTRETURN_;
    };
    switch (ana) {
        case Analysis::MM:  
            switch (year) {
                case Year::Y2011: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Y2012: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Run1:  _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Y2015: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Y2016: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Run2p1:_cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Y2017: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Y2018: _cut = HLT1MMCut( year , m_cutOption); break;
                case Year::Run2p2:_cut = HLT1MMCut( year , m_cutOption); break;
                case Year::All : MessageSvc::Warning("GetHLT1Cut ALL years-> no alignment");break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Run1:  _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Run2p1:_cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                case Year::Run2p2:_cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutHLT1"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT1Cut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetHLT2Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT2Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    bool _dropTopoMuLines = m_cutOption.Contains("noTopoMuHLT2") ? true : false;
    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = _dropTopoMuLines ?  CutDefRX::Trigger::Run1::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Y2012: _cut = _dropTopoMuLines ?  CutDefRX::Trigger::Run1::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Run1: _cut = _dropTopoMuLines  ?   CutDefRX::Trigger::Run1::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Y2015: {
                    _cut =                                            _dropTopoMuLines? CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = _dropTopoMuLines? CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Hlt2MM;
                } break;
                case Year::Y2016: {
                    _cut = _dropTopoMuLines ?  CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = _dropTopoMuLines? CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Hlt2MM;
                } break;
                case Year::Run2p1: {
                    _cut = _dropTopoMuLines? CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM_noTopoMu || CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM || CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = _dropTopoMuLines? CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM_noTopoMu || CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p1::Hlt2MM;
                    } break;
                case Year::Y2017: _cut = _dropTopoMuLines ?  CutDefRX::Trigger::Run2p2::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::Y2018: _cut = _dropTopoMuLines ?  CutDefRX::Trigger::Run2p2::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::Run2p2: _cut = _dropTopoMuLines? CutDefRX::Trigger::Run2p2::Hlt2MM_noTopoMu : CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::All : MessageSvc::Warning("GetHLT2Cut ALL years-> no alignment");break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2015: {
                    _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2EE;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = CutDefRX::Trigger::Run2p1::Hlt2EE;
                } break;
                case Year::Y2016: {
                    _cut = CutDefRX::Trigger::Run2p1::Y2016::Hlt2EE;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = CutDefRX::Trigger::Run2p1::Hlt2EE;
                } break;
                case Year::Run2p1: {
                    _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2EE || CutDefRX::Trigger::Run2p1::Y2016::Hlt2EE;
                    if (m_cutOption.Contains("alignR2p1HLT2")) _cut = CutDefRX::Trigger::Run2p1::Hlt2EE;
                    } break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::All : MessageSvc::Warning("GetHLT2Cut ALL years-> no alignment");break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2EE; break;
                case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::Y2016::Hlt2EE; break;
                case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2EE || CutDefRX::Trigger::Run2p1::Y2016::Hlt2EE; break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::All : MessageSvc::Warning("GetHLT2Cut ALL years-> no alignment");break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutHLT2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT2Cut", &_cut);
    return _cut;
}

TCut CutHolderRKst::GetTruthMatchCut() {
    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_configHolder.IsMC()) return _cut;

    _cut = TruthMatching(m_configHolder, m_cutOption, m_debug);

    if (IsCut(_cut)) m_cuts["cutMCT"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", &_cut);
    return _cut;
}

#endif
