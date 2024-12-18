#ifndef CUTHOLDERRKS_CPP
#define CUTHOLDERRKS_CPP

#include "CutHolderRKS.hpp"

#include "CutDefRKS.hpp"
#include "SettingDef.hpp"

#include "TruthMatchingSvc.hpp"

ClassImp(CutHolderRKS)

    CutHolderRKS::CutHolderRKS()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKS", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
}

CutHolderRKS::CutHolderRKS(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKS", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
}

CutHolderRKS::CutHolderRKS(const CutHolderRKS & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolderRKS", (TString) "CutHolderRKS");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
}

ostream & operator<<(ostream & os, const CutHolderRKS & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolderRKS");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void CutHolderRKS::Init() {
    MessageSvc::Info(Color::Cyan, "CutHolderRKS", (TString) "Initialize ...");
    CreateCut();
    return;
}

void CutHolderRKS::CreateCut() {
    if (m_debug) MessageSvc::Debug("CreateCut", m_cutOption);

    m_cut = TCut(NOCUT);
    m_cuts.clear();

    if (!m_cutOption.BeginsWith("no-")) {
        if (!m_cutOption.Contains("noSPD")) m_cut = m_cut && GetSPDCut();
        if (!m_cutOption.Contains("noTRG")) m_cut = m_cut && GetTriggerCut();
        if (!m_cutOption.Contains("noPS")) m_cut = m_cut && GetPreSelectionCut();
        if (!m_cutOption.Contains("noBKG")) m_cut = m_cut && GetBackgroundCut();
        if (!m_cutOption.Contains("noPID")) m_cut = m_cut && GetPIDCut();
        if (!m_cutOption.Contains("noMVA")) m_cut = m_cut && GetMVACut();
        if (!m_cutOption.Contains("noQ2")) m_cut = m_cut && GetQ2Cut();
    }
    if (!m_cutOption.Contains("noTRACK")) m_cut = m_cut && GetTrackCut();

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

TCut CutHolderRKS::GetBackgroundCut() {
    if (m_debug) MessageSvc::Debug("GetBackgroundCut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = TCut(NOCUT); break;   // CutDefRKS::Background::;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetBackgroundCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutBKG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetBackgroundCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetPIDCut() {
    if (m_debug) MessageSvc::Debug("GetPIDCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Track    track = m_configHolder.GetTrack();

    TCut _cut = TCut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRKS::PID::pidMM; break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutPID"] = _cut;

    if (m_debug) MessageSvc::Debug("GetPIDCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetPreSelectionCut() {
    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Track    track = m_configHolder.GetTrack();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRKS::Quality::qualityMM && CutDefRKS::Mass::KS; break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetPreSelectionCut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_debug) MessageSvc::Debug("GetPreSelectionCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetQ2Cut() {
    if (m_debug) MessageSvc::Debug("GetQ2Cut", m_cutOption);

    Analysis ana   = m_configHolder.GetAna();
    Q2Bin    q2bin = m_configHolder.GetQ2bin();

    TCut _cut(NOCUT);
    switch (q2bin) {
        case Q2Bin::All: break;
        case Q2Bin::JPsi: _cut = ana == Analysis::MM ? CutDefRKS::Mass::JPsMM : TCut(NOCUT); break;
        default: MessageSvc::Error("GetQ2Cut", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutQ2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetQ2Cut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetTrackCut() {
    if (m_debug) MessageSvc::Debug("GetTrackCut", m_cutOption);

    Track track = m_configHolder.GetTrack();

    TCut _cut(NOCUT);
    switch (track) {
        case Track::All: _cut = TCut(NOCUT); break;
        case Track::LL: _cut = CutDefRKS::Track::LL; break;
        case Track::DD: _cut = CutDefRKS::Track::DD; break;
        default: MessageSvc::Error("GetTrackCut", (TString) "Invalid track", to_string(track), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutTRACK"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTrackCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetSPDCut() {
    if (m_debug) MessageSvc::Debug("GetSPDCut", m_cutOption);

    Year year = m_configHolder.GetYear();

    TCut _cut(NOCUT);
    switch (year) {
        case Year::Y2011: _cut = CutDefRKS::Trigger::Run1::nSPD; break;
        case Year::Y2012: _cut = CutDefRKS::Trigger::Run1::nSPD; break;
        case Year::Run1: _cut = CutDefRKS::Trigger::Run1::nSPD; break;
        case Year::Y2015: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::Y2016: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::Run2p1: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::Y2017: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::Y2018: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::Run2p2: _cut = CutDefRKS::Trigger::Run2::nSPD; break;
        case Year::All: MessageSvc::Warning("GetSPDCut", (TString) "Invalid year", to_string(year)); break;
        default: MessageSvc::Error("GetSPDCut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutSPD"] = _cut;

    if (m_debug) MessageSvc::Debug("GetSPDCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetTriggerCut() {
    if (m_debug) MessageSvc::Debug("GetTriggerCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_cutOption.Contains("noL0")) _cut = _cut && GetL0Cut();
    if (!m_cutOption.Contains("noHLT1")) _cut = _cut && GetHLT1Cut();
    if (!m_cutOption.Contains("noHLT2")) _cut = _cut && GetHLT2Cut();

    if (IsCut(_cut)) m_cuts["cutTRG"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTriggerCut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetL0Cut() {
    if (m_debug) MessageSvc::Debug("GetL0Cut", m_cutOption);

    Analysis ana = m_configHolder.GetAna();

    TCut _cut(NOCUT);
    switch (ana) {
        case Analysis::MM: _cut = CutDefRKS::Trigger::L0M; break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetL0Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutL0"] = _cut;

    if (m_debug) MessageSvc::Debug("GetL0Cut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetHLT1Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT1Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);

    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRKS::Trigger::Run1::Hlt1MM; break;
                case Year::Y2012: _cut = CutDefRKS::Trigger::Run1::Hlt1MM; break;
                case Year::Run1: _cut = CutDefRKS::Trigger::Run1::Hlt1MM; break;
                case Year::Y2015: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                case Year::Y2016: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                case Year::Run2p1: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                case Year::Y2017: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                case Year::Y2018: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                case Year::Run2p2: _cut = CutDefRKS::Trigger::Run2::Hlt1MM; break;
                default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetHLT1Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutHLT1"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT1Cut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetHLT2Cut() {
    if (m_debug) MessageSvc::Debug("GetHLT2Cut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);

    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRKS::Trigger::Run1::Hlt2MM; break;
                case Year::Y2012: _cut = CutDefRKS::Trigger::Run1::Hlt2MM; break;
                case Year::Run1: _cut = CutDefRKS::Trigger::Run1::Hlt2MM; break;
                case Year::Y2015: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                case Year::Y2016: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                case Year::Run2p1: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                case Year::Y2017: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                case Year::Y2018: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                case Year::Run2p2: _cut = CutDefRKS::Trigger::Run2::Hlt2MM; break;
                default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetHLT2Cut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (IsCut(_cut)) m_cuts["cutHLT2"] = _cut;

    if (m_debug) MessageSvc::Debug("GetHLT2Cut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetMVACut() {
    if (m_debug) MessageSvc::Debug("GetMVACut", m_cutOption);

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TCut _cut(NOCUT);

    switch (ana) {
        case Analysis::MM:
            switch (year) {
                case Year::Y2011: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Y2012: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Run1: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Y2015: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Y2016: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Run2p1: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Y2017: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Y2018: _cut = CutDefRKS::MVA::mvaMM; break;
                case Year::Run2p2: _cut = CutDefRKS::MVA::mvaMM; break;
                default: MessageSvc::Error("GetMVACut", (TString) "Invalid year", to_string(year), "EXIT_FAILURE"); break;
            }
            break;
        case Analysis::EE: break;
        case Analysis::ME: break;
        default: MessageSvc::Error("GetMVACut", (TString) "Invalid ana", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (m_debug) MessageSvc::Debug("GetMVACut", &_cut);
    return _cut;
}

TCut CutHolderRKS::GetTruthMatchCut() {
    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", m_cutOption);

    TCut _cut(NOCUT);
    if (!m_configHolder.IsMC()) return _cut;

    _cut = TruthMatching(m_configHolder, m_cutOption, m_debug);

    if (IsCut(_cut)) m_cuts["cutMCT"] = _cut;

    if (m_debug) MessageSvc::Debug("GetTruthMatchCut", &_cut);
    return _cut;
}

#endif
