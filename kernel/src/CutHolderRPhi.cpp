#ifndef CUTHOLDERRPHI_CPP
#define CUTHOLDERRPHI_CPP

#include "CutHolderRPhi.hpp"

#include "CutDefRPhi.hpp"
#include "CutDefRX.hpp"
#include "SettingDef.hpp"

#include "TruthMatchingSvc.hpp"

ClassImp(CutHolderRPhi)

    CutHolderRPhi::CutHolderRPhi()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRPhi", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
}

CutHolderRPhi::CutHolderRPhi(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRPhi", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
}

CutHolderRPhi::CutHolderRPhi(const CutHolderRPhi & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRPhi", (TString) "CutHolderRPhi");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
}

ostream & operator<<(ostream & os, const CutHolderRPhi & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolderRPhi");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void CutHolderRPhi::Init() {
    MessageSvc::Info(Color::Cyan, "CutHolderRPhi", (TString) "Initialize ...");
    CreateCut();
    return;
}

Bool_t CutHolderRPhi::CheckCutOptionKnown() {
    if (m_debug) MessageSvc::Debug("CheckCutOptionKnown", m_cutOption);

    // Cut Options that CutHolderRPhi knows what to do with
    vector< TString > _knownCutOptions = {
        "no-",                                                                                                                                     // no
        "noSPD",     "noPS",      "noMVA",    "noBREM",       "noBKG",      "noPR",       "noQ2", "cutECALDistance",                               // misc.
        "tm",        "isSingle",  "tmSig",    "tmSigNoGhost", "tmBkg",      "tmSwap",     "tmCustom",   "tmSigTight", "tmCustomSwap", "noGhost",   // truthmatching
        "massBT",    "massBL",    "massB",    "SB",           "massSB",     "SBU",        "SBL",                                                   // mass regions
        "noPID",     "noProbEE",  "noProbMM", "noProbHH",     "PIDMeerkat", "loosePID",                                                            // PID
        "noPhi",     "loosePhi",  "noFid",    "noOA",                                                                                              // fiducial
        "noMinLPET", "noTRG",     "noL0",     "L0TIS",        "L0TISTOS",   "L0TISTOSL1", "L0TISTOSL2",                                            // L0 and TCK
        "HLTTIS",    "HLTTISTOS", "HLT2T4B",  "HLTPHYS",      "HLT2PHYS",   "noHLT1",     "noHLT2",     "noHLT"                                    // HLT
    };

    vector< TString > _cutOptions = TokenizeString(m_cutOption, "-");
    for (int i = 0; i < _cutOptions.size(); i++) {
        if (!(find(_knownCutOptions.begin(), _knownCutOptions.end(), _cutOptions.at(i)) != _knownCutOptions.end())) return false;
    }
    return true;
}

void CutHolderRPhi::CreateCut() {
    if (m_debug) MessageSvc::Debug("CreateCut", m_cutOption);

    if (!CheckCutOptionKnown()) MessageSvc::Error("CreateCut", (TString) "Unknown option", m_cutOption, "(should be an) EXIT FAILURE");

    m_cut = TCut(NOCUT);
    m_cuts.clear();

    if (!m_cutOption.BeginsWith("no-")) {
        if (!m_cutOption.Contains("noSPD")) m_cut = GetSPDCut();
        if (!m_cutOption.Contains("noTRG")) m_cut = m_cut && GetTriggerCut();
        if (!m_cutOption.Contains("noPS")) m_cut = m_cut && GetPreSelectionCut();
        if (!m_cutOption.Contains("noBKG")) m_cut = m_cut && GetBackgroundCut();
        if (!m_cutOption.Contains("noPID")) m_cut = m_cut && GetPIDCut();
        if (!m_cutOption.Contains("noPR")) m_cut = m_cut && GetPartRecoCut();   // TODO
        if (!m_cutOption.Contains("noMVA")) m_cut = m_cut && GetMVACut();
        if (!m_cutOption.Contains("noQ2")) m_cut = m_cut && GetQ2Cut();
    }
    if (!m_cutOption.Contains("noBREM")) m_cut = m_cut && GetBremCut();   // can be dropped i guess
    if (!m_cutOption.Contains("noTRACK")) m_cut = m_cut && GetTrackCut();

    // if (m_cutOption.Contains("HOP")) m_cut = m_cut && GetHOPCut(); //TODO

    if (m_cutOption.Contains("SB") || m_configHolder.IsSB("SB")) m_cut = m_cut && GetSideBandCut();
    if (m_cutOption.Contains("massB")) m_cut = m_cut && GetBMassCut();   // TODO

    if (m_cutOption.Contains("tm")) m_cut = m_cut && GetTruthMatchCut();
    if (m_cutOption.Contains("isSingle")) m_cut = m_cut && GetIsSingleCut();

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

TCut CutHolderRPhi::GetBackgroundCut() {
    if (m_debug) MessageSvc::Debug("GetBackgroundCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    // TODO
    // switch (ana) {
    //    case Analysis::MM : _cut =  CutDefRPhi::Background::peakingBkgMM; break;
    //    case Analysis::EE : _cut =  CutDefRPhi::Background::peakingBkgEE; break;
    //    default           : MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    //}

    if (IsCut(_cut)) m_cuts["cutBKG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBackgroundCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetBremCut() {
    if (m_debug) MessageSvc::Debug("GetBremCut", m_cutOption);

    Brem brem = m_configHolder.GetBrem();

    TCut _cut(NOCUT);

    switch (brem) {
        case Brem::All: _cut = TCut(NOCUT); break;
        case Brem::G0: _cut = CutDefRX::Brem::G0; break;
        case Brem::G1: _cut = CutDefRX::Brem::G1; break;
        case Brem::G2: _cut = CutDefRX::Brem::G2; break;
        default: MessageSvc::Error("GetBremCut", (TString) "Invalid brem", to_string(brem), "EXIT_FAILURE"); break;
    }
    if (IsCut(_cut)) m_cuts["cutBREM"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBremCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetIsSingleCut() {
    if (m_debug) MessageSvc::Debug("GetIsSingleCut", m_cutOption);

    TCut _cut(NOCUT);
    if (m_configHolder.IsMC())
        _cut = "isSingle_BKGCAT";
    else
        _cut = "isSingle_RND";

    if (IsCut(_cut)) m_cuts["cutSINGLE"] = _cut;

    if (m_debug) MessageSvc::Debug("GetIsSingleCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetBMassCut() {
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
        if (m_cutOption.Contains("DTF")) {
            if (q2bin == Q2Bin::JPsi) _cut = ana == Analysis::MM ? CutDefRX::Mass::BDTFJPsMM : CutDefRX::Mass::BDTFJPsEE;
            if (q2bin == Q2Bin::Psi) _cut = ana == Analysis::MM ? CutDefRX::Mass::BDTFPsiMM : CutDefRX::Mass::BDTFPsiEE;
        }
    }

    if (IsCut(_cut)) m_cuts["cutBMASS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBMassCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetMVACut() {
    if (m_debug) MessageSvc::Debug("GetMVACut", m_cutOption);

    Q2Bin q2bin = m_configHolder.GetQ2bin();
    TCut  _cut(NOCUT);
    if (q2bin != Q2Bin::All) {
        Analysis ana  = m_configHolder.GetAna();
        Year     year = hash_year(GetRunFromYear(to_string(m_configHolder.GetYear())));

        auto _config = make_tuple(ana, year, q2bin);

        map< tuple< Analysis, Year, Q2Bin >, TString > _mvaCut = CutDefRPhi::MVA::CAT;

        if (_mvaCut.find(_config) == _mvaCut.end()) {
            MessageSvc::Error("Configuration", to_string(get< 0 >(_config)), ",", to_string(get< 1 >(_config)), ",", to_string(get< 2 >(_config)));
            MessageSvc::Error("GetMVACut", (TString) "Invalid configuration (please implement it or use -noMVA)", "EXIT_FAILURE");
        } else {
            _cut = _mvaCut[_config];
        }
    }

    if ((SettingDef::IO::exe == "optimise.py") && !((TString) _cut).Contains("lowcen_")) _cut = ReplaceCut(_cut, "lowcen", "lowcen_v" + SettingDef::Cut::mvaVer);

    if (IsCut(_cut)) m_cuts["cutMVA"] = _cut;

    if (m_debug) MessageSvc::Debug("GetMVACut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetPartRecoCut() {
    if (m_debug) MessageSvc::Debug("GetPartRecoCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);

    // TODO
    // if (q2bin == Q2Bin::JPsi) {
    //    switch (ana) {
    //    case Analysis::MM : _cut = CutDefRPhi::Background::partRecoJPsMM; break;
    //    case Analysis::EE : _cut = CutDefRPhi::Background::partRecoJPsEE; break;
    //    default           : MessageSvc::Error("GetPartRecoCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    //    }
    //}

    if (IsCut(_cut)) m_cuts["cutPR"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPartRecoCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetPIDCut() {
    if (m_debug) MessageSvc::Debug("GetPIDCut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = (m_cutOption.Contains("loosePID") ? CutDefRPhi::PID::pidMML : CutDefRPhi::PID::pidMM); break;
        case Analysis::EE: _cut = (m_cutOption.Contains("loosePID") ? CutDefRPhi::PID::pidEEL : CutDefRPhi::PID::pidEE); break;
        default: MessageSvc::Error("GetPIDCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_cutOption.Contains("noProbHH")) _cut = ReplaceCut(_cut, TString(CutDefRPhi::PID::probnnK), "1");
    if (m_cutOption.Contains("noProbMM")) _cut = ReplaceCut(_cut, TString(CutDefRPhi::PID::probnnM), "1");
    if (m_cutOption.Contains("noProbEE")) _cut = ReplaceCut(_cut, TString(CutDefRPhi::PID::probnnE), "1");

    _cut = UpdatePIDTune(_cut, to_string(year));

    if (m_cutOption.Contains("PIDMeerkat")) _cut = UpdatePIDMeerkat(_cut, to_string(year));

    if (IsCut(_cut)) m_cuts["cutPID"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPIDCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetPreSelectionCut() {
    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", m_cutOption);

    Analysis ana     = m_configHolder.GetAna();
    Year     year    = m_configHolder.GetYear();
    Trigger  trigger = m_configHolder.GetTrigger();

    TCut _cut(NOCUT);

    TCut _cutMinLPET = GetMinLPETCut();

    if (!m_cutOption.Contains("noPhi")) { _cut = m_cutOption.Contains("loosePhi") ? CutDefRPhi::Mass::PhiL : CutDefRPhi::Mass::Phi; }

    if (!m_cutOption.Contains("noGhost")) { _cut = _cut && CutDefRPhi::Quality::Ghost; }

    if (!m_cutOption.Contains("noFid")) {
        switch (ana) {
            case Analysis::MM:
                switch (year) {
                    case Year::Y2011: [[fallthrough]];
                    case Year::Y2012: [[fallthrough]];
                    case Year::Run1: _cut = _cut && CutDefRPhi::Fiducial::Run1::fidMM; break;
                    case Year::Y2015: [[fallthrough]];
                    case Year::Y2016: [[fallthrough]];
                    case Year::Run2p1: _cut = _cut && CutDefRPhi::Fiducial::Run2p1::fidMM; break;
                    case Year::All: MessageSvc::Warning("GetPreSelectionCut", (TString) "Invalid year", to_string(year)); break;
                    default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            case Analysis::EE:
                switch (year) {
                    case Year::Y2011: [[fallthrough]];
                    case Year::Y2012: [[fallthrough]];
                    case Year::Run1: _cut = _cut && CutDefRPhi::Fiducial::Run1::fidEE; break;
                    case Year::Y2015: [[fallthrough]];
                    case Year::Y2016: [[fallthrough]];
                    case Year::Run2p1: _cut = _cut && CutDefRPhi::Fiducial::Run2p1::fidEE; break;
                    case Year::All: MessageSvc::Warning("GetPreSelectionCut", (TString) "Invalid year", to_string(year)); break;
                    default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    switch (ana) {
        case Analysis::MM: _cut = _cut && CutDefRPhi::Quality::qualMM; break;
        case Analysis::EE:
            _cut = _cut && CutDefRPhi::Quality::qualEE;
            if (!m_cutOption.Contains("noMinLPET")) {
                if (IsCut(_cutMinLPET)) {
                    switch (trigger) {
                        case Trigger::L0I: break;
                        case Trigger::L0L: _cut = _cut && _cutMinLPET; break;
                        case Trigger::All: _cut = _cut && (CutDefRX::Trigger::L0I || _cutMinLPET); break;
                        default: MessageSvc::Error("GetTCKCut", (TString) "Invalid trg", to_string(trigger), "EXIT FAILURE"); break;
                    }
                }
            }
            break;
        case Analysis::ME: MessageSvc::Error("Analysis::ME", (TString) "Invalid Ana", to_string(ana), "EXIT FAILURE"); break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_cutOption.Contains("noMinLPET")) {
        if (IsCut(_cutMinLPET)) _cut = ReplaceCut(_cut, TString(_cutMinLPET), "1");
    }

    if (!m_cutOption.Contains("noOA")) { _cut = _cut && CutDefRPhi::Fiducial::openingAngle; }

    if (IsCut(_cut)) m_cuts["cutPS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetMinLPETCut() {
    if (m_debug) { MessageSvc::Debug("GetMinLPETCut", m_cutOption); }

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = "TMath::Min(M1_PT, M2_PT) > 800"; break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Quality::Run1::Y2011::trgAccL0E_ET; break;
                case Year::Y2012: _cut = CutDefRX::Quality::Run1::Y2012::trgAccL0E_ET; break;
                case Year::Run1: _cut = CutDefRX::Quality::Run1::trgAccL0E_ET; break;
                case Year::Y2015: _cut = CutDefRX::Quality::Run2p1::Y2015::trgAccL0E_ET; break;
                case Year::Y2016: _cut = CutDefRX::Quality::Run2p1::Y2016::trgAccL0E_ET; break;
                case Year::Run2p1: _cut = CutDefRX::Quality::Run2p1::trgAccL0E_ET; break;
                case Year::Y2017: _cut = CutDefRX::Quality::Run2p2::Y2017::trgAccL0E_ET; break;
                case Year::Y2018: _cut = CutDefRX::Quality::Run2p2::Y2018::trgAccL0E_ET; break;
                case Year::Run2p2: _cut = CutDefRX::Quality::Run2p2::trgAccL0E_ET; break;
                case Year::All: MessageSvc::Warning("GetMinLPETCut", (TString) "Invalid Year", to_string(year)); break;
                default: MessageSvc::Error("GetMinLPETCut", (TString) "Invalid year", to_string(year), "EXIT FAILURE"); break;
            }
            break;
        case Analysis::ME: MessageSvc::Error("GetMinLPETCut", "Invalid Ana", to_string(ana), "EXIT FAILURE"); break;
        default: MessageSvc::Error("GetMinLPETCut", "Invalid Ana", to_string(ana), "EXIT FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutLPET"] = _cut;

    if (m_debug) MessageSvc::Debug("GetMinLPETCut", &_cut);

    return _cut;
}

TCut CutHolderRPhi::GetQ2Cut() {
    if (m_debug) MessageSvc::Debug("GetQ2Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);

    switch (q2bin) {
        case Q2Bin::All: break;
        case Q2Bin::Low: _cut = CutDefRPhi::Mass::Q2Low; break;
        case Q2Bin::Central: _cut = CutDefRPhi::Mass::Q2Central; break;
        case Q2Bin::High: _cut = CutDefRPhi::Mass::Q2High; break;
        case Q2Bin::Gamma: _cut = CutDefRPhi::Mass::Q2Gamma; break;
        case Q2Bin::JPsi: _cut = ana == Analysis::MM ? CutDefRX::Mass::JPsMM : CutDefRX::Mass::JPsEE; break;
        case Q2Bin::Psi: _cut = ana == Analysis::MM ? CutDefRX::Mass::PsiMM : CutDefRX::Mass::PsiEE; break;
        default: MessageSvc::Error("GetQ2Cut", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
    }
    auto UpdateQ2Cut = []( TCut _cutIN, TString _cutOption ){
        TString _newCut = TString(_cutIN); 
        if( _cutOption.Contains("q2SmearBp") ){
            _newCut =  _newCut.ReplaceAll("JPs_M", "JPs_M_smear_Bp");
        }else if( _cutOption.Contains("q2SmearB0")){
            _newCut = _newCut.ReplaceAll("JPs_M", "JPs_M_smear_B0");
        }
        return TCut( _newCut);
    };
    //Append to map of cuts the TRUEQ2 cut (useful for MCDT efficiency estimation)
    if (IsCut(_cut)){
        m_cuts["cutQ2TRUE"] = _cut;
        m_cuts["cutQ2TRUE"] = ReplaceCut(m_cuts["cutQ2TRUE"], "JPs_M", "JPs_TRUEM");
    }
    if( ana == Analysis::EE) _cut = UpdateQ2Cut( _cut, m_cutOption);

    if (IsCut(_cut)) m_cuts["cutQ2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetQ2Cut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetSideBandCut() {
    if (m_debug) MessageSvc::Debug("GetSideBandCut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);
    if (m_cutOption.Contains("SBU") || m_configHolder.IsSB("SBU")) {
        switch (ana) {
            case Analysis::MM: _cut = CutDefRPhi::Background::upperSBMM; break;
            case Analysis::EE: _cut = CutDefRPhi::Background::upperSBEE; break;
            case Analysis::All: MessageSvc::Warning("GetSideBandCut", (TString) "Invalid analyses", to_string(ana)); break;
            default: MessageSvc::Error("GetSideBandCut", (TString) "Invalid analyses", to_string(ana), "EXIT_FAILURE"); break;
        }
    }
    if (m_cutOption.Contains("SBL") || m_configHolder.IsSB("SBL")) { _cut = CutDefRPhi::Background::lowerSB; }

    if (IsCut(_cut)) m_cuts["cutSB"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSideBandCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetSPDCut() {
    if (m_debug) MessageSvc::Debug("GetSPDCut", m_cutOption);

    Year year = m_configHolder.GetYear();

    TCut _cut(NOCUT);

    switch (year) {
        case Year::Y2011: [[fallthrough]];
        case Year::Y2012: [[fallthrough]];
        case Year::Run1: _cut = CutDefRX::Trigger::Run1::nSPD; break;
        case Year::Y2015: [[fallthrough]];
        case Year::Y2016: [[fallthrough]];
        case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::nSPD; break;
        case Year::All: MessageSvc::Warning("GetSPDCut", (TString) "Invalid year", to_string(year)); break;
        default: MessageSvc::Error("GetSPDCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutSPD"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSPDCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetTrackCut() {

    if (m_debug) MessageSvc::Debug("GetTrackCut", m_cutOption);

    Track track = m_configHolder.GetTrack();

    bool    _add       = true;
    TString _cutOption = m_cutOption;

    TCut _cut(NOCUT);

    switch (track) {
        case Track::All: _cut = TCut(NOCUT); break;
        case Track::TAG:
            if (m_cutOption.Contains("noHLT")) {
                m_cutOption += "-HLTTIS";
                _cut = GetHLT1Cut() && GetHLT2Cut();
                _add = false;
            }
            break;
        case Track::PRB:
            if (m_cutOption.Contains("noHLT")) {
                m_cutOption += "-HLTTISTOS";
                _cut = GetHLT1Cut() && GetHLT2Cut();
                _add = false;
            }
            break;
        default: MessageSvc::Error("GetTrackCut", (TString) "Invalid track", to_string(track), "EXIT FAILURE"); break;
    }

    m_cutOption = _cutOption;

    if (IsCut(_cut) && _add) m_cuts["cutTrack"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTrackCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetTriggerCut() {
    if (m_debug) MessageSvc::Debug("GetTriggerCut", m_cutOption);

    Analysis    ana  = m_configHolder.GetAna();
    Year        year = m_configHolder.GetYear();
    Trigger     trg  = m_configHolder.GetTrigger();
    TriggerConf trgc = m_configHolder.GetTriggerConf();

    TCut _cut(NOCUT);
    if (!m_cutOption.Contains("noL0")) _cut = _cut && GetL0Cut();
    if (!m_cutOption.Contains("noHLT") && !m_cutOption.Contains("noHLT1")) _cut = _cut && GetHLT1Cut();
    if (!m_cutOption.Contains("noHLT") && !m_cutOption.Contains("noHLT2")) _cut = _cut && GetHLT2Cut();

    if (IsCut(_cut)) m_cuts["cutTRG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTriggerCut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetL0Cut() {
    if (m_debug) MessageSvc::Debug("GetL0Cut", m_cutOption);

    Analysis    ana  = m_configHolder.GetAna();
    Year        year = m_configHolder.GetYear();
    Trigger     trg  = m_configHolder.GetTrigger();
    TriggerConf trgc = m_configHolder.GetTriggerConf();

    TCut _cut(NOCUT);
    if (trg == Trigger::All) {
        if (m_debug) MessageSvc::Debug("Trigger::All", m_cutOption);
        switch (ana) {
            case Analysis::MM: _cut = (CutDefRX::Trigger::L0I || CutDefRX::Trigger::L0M); break;
            case Analysis::EE: _cut = (CutDefRX::Trigger::L0I || CutDefRX::Trigger::L0E); break;
            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
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
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
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
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
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
                            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                        }
                        break;
                    case Trigger::L0I:
                        if (m_debug) MessageSvc::Debug("Trigger::L0I", m_cutOption);
                        switch (ana) {
                            case Analysis::MM: _cut = (CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0M); break;
                            case Analysis::EE: _cut = (CutDefRX::Trigger::L0I && !CutDefRX::Trigger::L0E); break;
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
        if (m_cutOption.Contains("L0TISTOS")) {
            if (m_cutOption.Contains("L0TISTOSL")) {
                switch (trg) {
                    case Trigger::L0L:
                        if (m_cutOption.Contains("L0TISTOSL1")) {
                            switch (ana) {
                                case Analysis::MM: _cut = CutDefRX::Trigger::L0M1; break;
                                case Analysis::EE: _cut = CutDefRX::Trigger::L0E1; break;
                                default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                            }
                        } else if (m_cutOption.Contains("L0TISTOSL2")) {
                            switch (ana) {
                                case Analysis::MM: _cut = CutDefRX::Trigger::L0M2; break;
                                case Analysis::EE: _cut = CutDefRX::Trigger::L0E2; break;
                                default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                            }
                        } else {
                            MessageSvc::Error("GetL0Cut", (TString) "Invalid option", m_cutOption, "EXIT_FAILURE");
                        }
                        break;
                    default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
                }
            }
            TString _cutOption = m_cutOption;
            m_cutOption.ReplaceAll("L0TISTOS", "L0TIS");
            _cut        = GetL0Cut() && _cut;
            m_cutOption = _cutOption;
        } else {
            switch (trg) {
                case Trigger::L0I: _cut = CutDefRX::Trigger::L0O; break;
                case Trigger::L0L:
                    switch (ana) {
                        case Analysis::MM: _cut = CutDefRX::Trigger::L0I; break;
                        case Analysis::EE: _cut = CutDefRX::Trigger::L0I; break;
                        default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
                    }
                    break;
                default: MessageSvc::Error("GetL0Cut", (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
            }
        }
    }

    if (IsCut(_cut)) m_cuts["cutL0"] = _cut;

    if (m_debug) MessageSvc::Debug("GetL0Cut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetHLT1Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT1Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: [[fallthrough]];
                case Year::Y2012: [[fallthrough]];
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt1MM; break;
                case Year::Y2015: [[fallthrough]];
                case Year::Y2016: [[fallthrough]];
                case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::Hlt1MM; break;
                case Year::Y2017: [[fallthrough]];
                case Year::Y2018: [[fallthrough]];
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt1MM; break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: [[fallthrough]];
                case Year::Y2012: [[fallthrough]];
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt1EE; break;
                case Year::Y2015: [[fallthrough]];
                case Year::Y2016: [[fallthrough]];
                case Year::Run2p1: _cut = CutDefRX::Trigger::Run2p1::Hlt1EE; break;
                case Year::Y2017: [[fallthrough]];
                case Year::Y2018: [[fallthrough]];
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt1EE; break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_cutOption.Contains("HLTTIS")) {
        if (m_cutOption.Contains("HLTTISTOS")) {
            TString _cutOption = m_cutOption;
            m_cutOption.ReplaceAll("HLTTISTOS", "HLTTIS");
            _cut        = GetHLT1Cut() && _cut;
            m_cutOption = _cutOption;
        } else {
            _cut = ReplaceCut(_cut, "TOS", "TIS");
            if (m_cutOption.Contains("HLTPHYS") || m_cutOption.Contains("HLT1PHYS")) { _cut = CutDefRX::Trigger::Hlt1TIS; }
        }
    }

    if (IsCut(_cut)) m_cuts["cutHLT1"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT1Cut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetHLT2Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT2Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt2MM; break;
                case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2MM; break;
                case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::Y2016::Hlt2MM; break;
                case Year::Run2p1: MessageSvc::Warning("GetHLT2Cut", (TString) "Invalid year", to_string(year)); break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt2MM; break;
                case Year::All: MessageSvc::Warning("GetHLT2Cut", (TString) "Invalid year", to_string(year)); break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2012: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Run1: _cut = CutDefRX::Trigger::Run1::Hlt2EE; break;
                case Year::Y2015: _cut = CutDefRX::Trigger::Run2p1::Y2015::Hlt2EE; break;
                case Year::Y2016: _cut = CutDefRX::Trigger::Run2p1::Y2016::Hlt2EE; break;
                case Year::Run2p1: MessageSvc::Warning("GetHLT2Cut", (TString) "Invalid year", to_string(year)); break;
                case Year::Y2017: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Y2018: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::Run2p2: _cut = CutDefRX::Trigger::Run2p2::Hlt2EE; break;
                case Year::All: MessageSvc::Warning("GetHLT2Cut", (TString) "Invalid year", to_string(year)); break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT FAILURE"); break;
        default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_cutOption.Contains("HLTTIS")) {
        if (m_cutOption.Contains("HLTTISTOS")) {
            TString _cutOption = m_cutOption;
            m_cutOption.ReplaceAll("HLTTISTOS", "HLTTIS");
            _cut        = GetHLT2Cut() && _cut;
            m_cutOption = _cutOption;
        } else {
            if (m_cutOption.Contains("HLT2T4B")) {
                switch (ana) {
                    case Analysis::MM:
                        switch (year) {
                            case Year::Y2011: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BMM; break;
                            case Year::Y2012: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BMM; break;
                            case Year::Run1: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BMM; break;
                            case Year::Y2015: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2015::Hlt2Topo4BMM; break;
                            case Year::Y2016: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2016::Hlt2Topo4BMM; break;
                            case Year::Run2p1: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2015::Hlt2Topo4BMM || CutDefRX::Trigger::Run2p1::Y2016::Hlt2Topo4BMM; break;
                            case Year::Y2017: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BMM; break;
                            case Year::Y2018: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BMM; break;
                            case Year::Run2p2: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BMM; break;
                            default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT FAILURE"); break;
                        }
                        break;
                    case Analysis::EE:
                        switch (year) {
                            case Year::Y2011: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BEE; break;
                            case Year::Y2012: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BEE; break;
                            case Year::Run1: _cut = _cut || CutDefRX::Trigger::Run1::Hlt2Topo4BEE; break;
                            case Year::Y2015: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2015::Hlt2Topo4BEE; break;
                            case Year::Y2016: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2016::Hlt2Topo4BEE; break;
                            case Year::Run2p1: _cut = _cut || CutDefRX::Trigger::Run2p1::Y2015::Hlt2Topo4BEE || CutDefRX::Trigger::Run2p1::Y2016::Hlt2Topo4BEE; break;
                            case Year::Y2017: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BEE; break;
                            case Year::Y2018: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BEE; break;
                            case Year::Run2p2: _cut = _cut || CutDefRX::Trigger::Run2p2::Hlt2Topo4BEE; break;
                            default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT FAILURE"); break;
                        }
                        break;
                    case Analysis::ME: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT FAILURE"); break;
                    default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT FAILURE"); break;
                }
            }
            _cut = ReplaceCut(_cut, "TOS", "TIS");
            if (m_cutOption.Contains("HLTPHYS") || m_cutOption.Contains("HLT2PHYS")) { _cut = CutDefRX::Trigger::Hlt2TIS; }
        }
    }

    if (IsCut(_cut)) m_cuts["cutHLT2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT2Cut", &_cut);
    return _cut;
}

TCut CutHolderRPhi::GetTruthMatchCut() {
    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_configHolder.IsMC()) return _cut;

    _cut = TruthMatching(m_configHolder, m_cutOption, m_debug);

    if (IsCut(_cut)) m_cuts["cutMCT"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", &_cut);
    return _cut;
}

#endif
