#ifndef CUTHOLDERRL_CPP
#define CUTHOLDERRL_CPP

#include "CutHolderRL.hpp"

#include "CutDefRL.hpp"
#include "SettingDef.hpp"

#include "TruthMatchingSvc.hpp"

ClassImp(CutHolderRL)

    CutHolderRL::CutHolderRL()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRL", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
}

CutHolderRL::CutHolderRL(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRL", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
}

CutHolderRL::CutHolderRL(const CutHolderRL & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRL", (TString) "CutHolderRL");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
}

ostream & operator<<(ostream & os, const CutHolderRL & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolderRL");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void CutHolderRL::Init() {
    MessageSvc::Info(Color::Cyan, "CutHolderRL", (TString) "Initialize ...");
    CreateCut();
    return;
}

void CutHolderRL::CreateCut() {
    if (m_debug) MessageSvc::Debug("CreateCut", m_cutOption);

    m_cut = TCut(NOCUT);
    m_cuts.clear();

    if (!m_cutOption.BeginsWith("no-")) {
        if (!m_cutOption.Contains("noSPD")) m_cut = m_cut && GetSPDCut();
        if (!m_cutOption.Contains("noTRG")) m_cut = m_cut && GetTriggerCut();
        if (!m_cutOption.Contains("noPS")) m_cut = m_cut && GetPreSelectionCut();
        if (!m_cutOption.Contains("noBKG")) m_cut = m_cut && GetBackgroundCut();
        if (!m_cutOption.Contains("noPID")) m_cut = m_cut && GetPIDCut();
        // if (!m_cutOption.Contains("noPR")) m_cut = m_cut && GetPartRecoCut();
        if (!m_cutOption.Contains("noMVA")) m_cut = m_cut && GetMVACut();
        if (!m_cutOption.Contains("noQ2")) m_cut = m_cut && GetQ2Cut();
    }
    if (!m_cutOption.Contains("noBREM")) m_cut = m_cut && GetBremCut();
    if (!m_cutOption.Contains("noTRACK")) m_cut = m_cut && GetTrackCut();

    if (m_cutOption.Contains("HOP")) m_cut = m_cut && GetHOPCut();

    if (m_cutOption.Contains("tm")) m_cut = GetTruthMatchCut() && m_cut;
    // if (m_cutOption.Contains("isSingle")) m_cut = GetIsSingleCut() && m_cut;

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

TCut CutHolderRL::GetBackgroundCut() {
    if (m_debug) MessageSvc::Debug("GetBackgroundCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    if ((q2bin == Q2Bin::Low) || (q2bin == Q2Bin::Central)) {
        switch (ana) {
            case Analysis::MM: _cut = CutDefRL::Background::Lc; break;
            case Analysis::EE: _cut = CutDefRL::Background::Lc && CutDefRL::Background::vtx; break;
            case Analysis::ME: _cut = CutDefRL::Background::JPs && CutDefRL::Background::vtx; break;
            default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutBKG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBackgroundCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetBremCut() {
    if (m_debug) MessageSvc::Debug("GetBremCut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Brem     brem = m_configHolder.GetBrem();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::ME:
            switch (brem) {
                case Brem::All: _cut = TCut(NOCUT); break;
                case Brem::G0: _cut = CutDefRL::Brem::E0; break;
                case Brem::G1: _cut = CutDefRL::Brem::E1; break;
                default: MessageSvc::Error("GetBremCut", (TString) "Invalid brem", to_string(brem), "EXIT_FAILURE"); break;
            }
            break;
        default:
            switch (brem) {
                case Brem::All: _cut = TCut(NOCUT); break;
                case Brem::G0: _cut = CutDefRL::Brem::G0; break;
                case Brem::G1: _cut = CutDefRL::Brem::G1; break;
                case Brem::G2: _cut = CutDefRL::Brem::G2; break;
                default: MessageSvc::Error("GetBremCut", (TString) "Invalid brem", to_string(brem), "EXIT_FAILURE"); break;
            }
            break;
    }

    if (IsCut(_cut)) m_cuts["cutBREM"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBremCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetHOPCut() {
    if (m_debug) MessageSvc::Debug("GetHOPCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: break;
        case Analysis::EE:
            if ((q2bin == Q2Bin::Low) || (q2bin == Q2Bin::Central)) _cut = CutDefRL::Mass::HOPEE;
            break;
        case Analysis::ME:
            if (q2bin != Q2Bin::JPsi) _cut = CutDefRL::Mass::HOPME;
            break;
        default: MessageSvc::Error("GetHOPCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutHOP"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHOPCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetPIDCut() {
    if (m_debug) MessageSvc::Debug("GetPIDCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Track    track = m_configHolder.GetTrack();

    TCut _cut = TCut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRL::PID::pidMM; break;
        case Analysis::EE: _cut = CutDefRL::PID::pidEE; break;
        case Analysis::ME: _cut = CutDefRL::PID::pidME; break;
        default: MessageSvc::Error("GetPIDCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    if (track == Track::LL) {
        _cut = _cut && CutDefRL::PID::pidP;
        _cut = _cut && CutDefRL::PID::pidPi;
    }

    if (IsCut(_cut)) m_cuts["cutPID"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPIDCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetPreSelectionCut() {
    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Track    track = m_configHolder.GetTrack();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRL::Quality::qualityMM && CutDefRL::Mass::L0; break;
        case Analysis::EE: _cut = CutDefRL::Quality::qualityEE && CutDefRL::Mass::L0; break;
        case Analysis::ME: _cut = CutDefRL::Quality::qualityME && CutDefRL::Mass::L0; break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }
    if (m_cutOption.Contains("BRMM")) _cut = _cut && CutDefRL::Quality::fiducialLb;
    if (track == Track::LL) {
        _cut = _cut && CutDefRL::Quality::hasDetHH;
        _cut = _cut && CutDefRL::Quality::pidHH;   // Will use for now; to check if it should be included for protons
    }

    if (IsCut(_cut)) m_cuts["cutPS"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetQ2Cut() {
    if (m_debug) MessageSvc::Debug("GetQ2Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    if (ana == Analysis::ME) {
        _cut = CutDefRL::Mass::Q2ME;
        if (q2bin == Q2Bin::JPsi) _cut = CutDefRL::Mass::JPsMM;
    } else {
        switch (q2bin) {
            case Q2Bin::All: break;
            case Q2Bin::Low: _cut = CutDefRL::Mass::Q2Low; break;
            // case Q2Bin::Central: _cut = CutDefRL::Mass::Q2Central; break;
            case Q2Bin::High: _cut = ana == Analysis::MM ? CutDefRL::Mass::Q2HighMM : CutDefRL::Mass::Q2HighEE; break;
            case Q2Bin::JPsi: _cut = ana == Analysis::MM ? CutDefRL::Mass::JPsMM : CutDefRL::Mass::JPsEE; break;
            case Q2Bin::Psi: _cut = ana == Analysis::MM ? CutDefRL::Mass::PsiMM : CutDefRL::Mass::PsiEE; break;
            default: MessageSvc::Error("GetQ2Cut", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutQ2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetQ2Cut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetTrackCut() {
    if (m_debug) MessageSvc::Debug("GetTrackCut", m_cutOption);

    Track track = m_configHolder.GetTrack();

    TCut _cut(NOCUT);
    switch (track) {
        case Track::All: _cut = TCut(NOCUT); break;
        case Track::LL: _cut = CutDefRL::Track::LL; break;
        case Track::DD: _cut = CutDefRL::Track::DD; break;
        default: MessageSvc::Error("GetTrackCut", (TString) "Invalid track", to_string(track), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutTRACK"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTrackCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetSPDCut() {
    if (m_debug) MessageSvc::Debug("GetSPDCut", m_cutOption);

    Year year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (year) {
        case Year::Y2011: _cut = CutDefRL::Trigger::Run1::nSPD; break;
        case Year::Y2012: _cut = CutDefRL::Trigger::Run1::nSPD; break;
        case Year::Run1: _cut = CutDefRL::Trigger::Run1::nSPD; break;
        case Year::Y2015: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::Y2016: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::Y2017: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::Y2018: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::nSPD; break;
        case Year::All: MessageSvc::Warning("GetSPDCut", (TString) "Invalid year", to_string(year)); break;
        default: MessageSvc::Error("GetSPDCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutSPD"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSPDCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetTriggerCut() {
    if (m_debug) MessageSvc::Debug("GetTriggerCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_cutOption.Contains("noL0")) _cut = _cut && GetL0Cut();
    if (!m_cutOption.Contains("noHLT1")) _cut = _cut && GetHLT1Cut();
    if (!m_cutOption.Contains("noHLT2")) _cut = _cut && GetHLT2Cut();

    if (IsCut(_cut)) m_cuts["cutTRG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTriggerCut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetL0Cut() {
    if (m_debug) MessageSvc::Debug("GetL0Cut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);
    if (m_cutOption.Contains("BRMM")) {
        _cut = CutDefRL::Trigger::L0BRMM;
    } else {
        switch (ana) {
            case Analysis::MM: _cut = CutDefRL::Trigger::L0M; break;
            case Analysis::EE: _cut = CutDefRL::Trigger::L0E; break;
            case Analysis::ME: _cut = CutDefRL::Trigger::L0M2; break;
            default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutL0"] = _cut;

    if (m_debug) MessageSvc::Debug("GetL0Cut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetHLT1Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT1Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();
    Year     year  = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    if (m_cutOption.Contains("BRMM")) {
        switch (year) {
            case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt1BRMM; break;
            case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt1BRMM; break;
            case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt1BRMM; break;
            case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt1BRMM; break;
            case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt1BRMM; break;
            case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt1BRMM; break;
            case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt1BRMM; break;
            case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt1BRMM; break;
            case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt1BRMM; break;
            default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
        }
    } else {
        switch (ana) {
            case Analysis::MM:
                switch (year) {
                    case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt1MM; break;
                    case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt1MM; break;
                    case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt1MM; break;
                    case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt1MM; break;
                    case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt1MM; break;
                    case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt1MM; break;
                    case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt1MM; break;
                    case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt1MM; break;
                    case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt1MM; break;
                    default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            case Analysis::EE:
                switch (year) {
                    case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt1EE; break;
                    case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt1EE; break;
                    case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt1EE; break;
                    case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt1EE; break;
                    case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt1EE; break;
                    case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt1EE; break;
                    case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt1EE; break;
                    case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt1EE; break;
                    case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt1EE; break;
                    default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            case Analysis::ME:
                if (q2bin == Q2Bin::JPsi) {
                    switch (year) {
                        case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt1JPsME; break;
                        case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt1JPsME; break;
                        case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt1JPsME; break;
                        case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt1JPsME; break;
                        case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt1JPsME; break;
                        case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt1JPsME; break;
                        case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt1JPsME; break;
                        case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt1JPsME; break;
                        case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt1JPsME; break;
                        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                    }
                } else {
                    switch (year) {
                        case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt1ME; break;
                        case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt1ME; break;
                        case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt1ME; break;
                        case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt1ME; break;
                        case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt1ME; break;
                        case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt1ME; break;
                        case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt1ME; break;
                        case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt1ME; break;
                        case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt1ME; break;
                        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                    }
                }
                break;
            default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutHLT1"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT1Cut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetHLT2Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT2Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();
    Year     year  = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    if (m_cutOption.Contains("BRMM")) {
        switch (year) {
            case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt2BRMM; break;
            case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt2BRMM; break;
            case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt2BRMM; break;
            case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt2BRMM; break;
            case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt2BRMM; break;
            case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt2BRMM; break;
            case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt2BRMM; break;
            case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt2BRMM; break;
            case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt2BRMM; break;
            default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
        }
    } else {
        switch (ana) {
            case Analysis::MM:
                switch (year) {
                    case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt2MM; break;
                    case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt2MM; break;
                    case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt2MM; break;
                    case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt2MM; break;
                    case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt2MM; break;
                    case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt2MM; break;
                    case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt2MM; break;
                    case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt2MM; break;
                    case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt2MM; break;
                    default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            case Analysis::EE:
                switch (year) {
                    case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt2EE; break;
                    case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt2EE; break;
                    case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt2EE; break;
                    case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt2EE; break;
                    case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt2EE; break;
                    case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt2EE; break;
                    case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt2EE; break;
                    case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt2EE; break;
                    case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt2EE; break;
                    default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                }
                break;
            case Analysis::ME:
                if (q2bin == Q2Bin::JPsi) {
                    switch (year) {
                        case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt2JPsME; break;
                        case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt2JPsME; break;
                        case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt2JPsME; break;
                        case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt2JPsME; break;
                        case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt2JPsME; break;
                        case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt2JPsME; break;
                        case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt2JPsME; break;
                        case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt2JPsME; break;
                        case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt2JPsME; break;
                        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                    }
                } else {
                    switch (year) {
                        case Year::Y2011: _cut = CutDefRL::Trigger::Run1::Hlt2ME; break;
                        case Year::Y2012: _cut = CutDefRL::Trigger::Run1::Hlt2ME; break;
                        case Year::Run1: _cut = CutDefRL::Trigger::Run1::Hlt2ME; break;
                        case Year::Y2015: _cut = CutDefRL::Trigger::Run2::Y2015::Hlt2ME; break;
                        case Year::Y2016: _cut = CutDefRL::Trigger::Run2::Hlt2ME; break;
                        case Year::Run2p1: _cut = CutDefRL::Trigger::Run2::Hlt2ME; break;
                        case Year::Y2017: _cut = CutDefRL::Trigger::Run2::Hlt2ME; break;
                        case Year::Y2018: _cut = CutDefRL::Trigger::Run2::Hlt2ME; break;
                        case Year::Run2p2: _cut = CutDefRL::Trigger::Run2::Hlt2ME; break;
                        default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
                    }
                }
                break;
            default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
        }
    }

    if (IsCut(_cut)) m_cuts["cutHLT2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT2Cut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetMVACut() {
    if (m_debug) MessageSvc::Debug("GetMVACut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Y2012: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Run1: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Y2015: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Y2016: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Run2p1: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Y2017: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Y2018: _cut = CutDefRL::MVA::mvaMM; break;
                case Year::Run2p2: _cut = CutDefRL::MVA::mvaMM; break;
                default: MessageSvc::Error("GetMVACut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE:
            switch (year) {
                case Year::Y2011: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Y2012: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Run1: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Y2015: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Y2016: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Run2p1: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Y2017: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Y2018: _cut = CutDefRL::MVA::mvaEE; break;
                case Year::Run2p2: _cut = CutDefRL::MVA::mvaEE; break;
                default: MessageSvc::Error("GetMVACut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::ME:
            switch (year) {
                case Year::Y2011: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Y2012: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Run1: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Y2015: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Y2016: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Run2p1: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Y2017: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Y2018: _cut = CutDefRL::MVA::mvaME; break;
                case Year::Run2p2: _cut = CutDefRL::MVA::mvaME; break;
                default: MessageSvc::Error("GetMVACut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        default: MessageSvc::Error("GetMVACut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutMVA"] = _cut;

    if (m_debug) MessageSvc::Debug("GetMVACut", &_cut);
    return _cut;
}

TCut CutHolderRL::GetTruthMatchCut() {
    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_configHolder.IsMC()) return _cut;

    _cut = TruthMatching(m_configHolder, m_cutOption, m_debug);

    if (IsCut(_cut)) m_cuts["cutMCT"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", &_cut);
    return _cut;
}

#endif
