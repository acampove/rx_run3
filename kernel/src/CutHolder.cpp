#ifndef CUTHOLDER_CPP
#define CUTHOLDER_CPP

#include "CutHolder.hpp"
#include "CutHolderRK.hpp"
#include "CutHolderRKS.hpp"
#include "CutHolderRKst.hpp"
#include "CutHolderRL.hpp"
#include "CutHolderRPhi.hpp"

#include "CutDefRX.hpp"

#include "HelperSvc.hpp"
#include "MessageSvc.hpp"
#include "SettingDef.hpp"

#include "core.h"
#include "fmt_ostream.h"
#include "vec_extends.h"
#include <iostream>

#include "TCut.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TString.h"
#include "TTreeFormula.h"

ClassImp(CutHolder)

    CutHolder::CutHolder()
    : m_configHolder() {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolder", (TString) "Default");
    m_cutOption = SettingDef::Cut::option;
    Check();
}

CutHolder::CutHolder(const ConfigHolder & _configHolder, TString _cutOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolder", (TString) "ConfigHolder");
    m_cutOption = _cutOption;
    Check();
}

CutHolder::CutHolder(const CutHolder & _cutHolder)
    : m_configHolder(_cutHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("CH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("CutHolder", (TString) "CutHolder");
    m_cutOption = _cutHolder.Option();
    m_cut       = _cutHolder.Cut();
    m_cuts      = _cutHolder.Cuts();
    Check();
}

ostream & operator<<(ostream & os, const CutHolder & _cutHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "CutHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "cutOption", _cutHolder.Option());
    if (IsCut(_cutHolder.Cut())) MessageSvc::Print((ostream &) os, "cut", TString(_cutHolder.Cut()));
    MessageSvc::Print((ostream &) os, "cuts", to_string(_cutHolder.Cuts().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

bool CutHolder::Check() {
    for (auto _opt : TokenizeString(m_cutOption, SettingDef::separator)) {
        if (!CheckVectorContains(SettingDef::AllowedConf::CutOptions, _opt)) {
            if (_opt.Contains("BREM")) continue;
            if (_opt.Contains("q2Gamma[")   || m_cutOption.Contains("q2Gamma[-")) continue;
            if (_opt.Contains("q2Psi[")     || m_cutOption.Contains("q2Psi[-")) continue;
            if (_opt.Contains("q2High[")    || m_cutOption.Contains("q2High[-")) continue;
            if (_opt.Contains("q2Low[")     || m_cutOption.Contains("q2Low[-")) continue;
            if (_opt.Contains("q2Central[") || m_cutOption.Contains("q2Central[-")) continue;
            if (_opt.Contains("q2JPsi[")    || m_cutOption.Contains("q2JPsi[-")) continue;
            if (_opt.Contains("q2All[")     || m_cutOption.Contains("q2All[-")) continue;
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("CutHolder", (TString) "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::CutOptions", "EXIT_FAILURE");
        }
    }
    return false;
}

CutHolder CutHolder::Clone(TString _cutOption, TString _cutExtra) {
    MessageSvc::Info(Color::Cyan, "CutHolder", (TString) "Cloning ...");
    /*
    if (_cutOption == "") {
        MessageSvc::Warning("Clone", (TString) "Use old cutOption");
        _cutOption = this->Option();
    } else {
        MessageSvc::Warning("Clone", (TString) "Use new cutOption");
    }
    */
    MessageSvc::Info("CutOption", _cutOption);

    TString _extraCut = SettingDef::Cut::extraCut;
    if (_cutExtra != "") {
        SettingDef::Cut::extraCut = _cutExtra;
    } else {
        MessageSvc::Warning("Clone", (TString) "Ensure you set SettingDef::Cut::extraCut before calling this, cloning doesn't transport the value");
    }
    MessageSvc::Info("ExtraCut", SettingDef::Cut::extraCut);

    CutHolder _cutHolder(this->GetConfigHolder(), _cutOption);
    _cutHolder.Init();

    SettingDef::Cut::extraCut = _extraCut;
    return _cutHolder;
}

void CutHolder::Init() {
    // https://root.cern.ch/doc/master/classROOT_1_1v5_1_1TFormula.html#a93b0de2e86cdcf7250ffa4aa6aae9f6b
    int _maxop    = 2000;   // maximum number of operations
    int _maxpar   = 2000;   // maximum number of parameters
    int _maxconst = 2000;   // maximum number of constants
    TTreeFormula::SetMaxima(_maxop, _maxpar, _maxconst);

    MessageSvc::Info(Color::Cyan, "CutHolder", (TString) "Initialize ...");
    TString _cutOption      = SettingDef::Cut::option;
    SettingDef::Cut::option = m_cutOption;
    CreateCut();
    SettingDef::Cut::option = _cutOption;
    PrintCuts();
    return;
}

void CutHolder::CreateCut() {
    m_cut = TCut(NOCUT);
    m_cuts.clear();
    if ((m_cutOption == "no") || (m_cutOption == "-no")) {
        m_cut = TCut(NOCUT);
    } else if (m_configHolder.IsRapidSim()) {
        m_cutOption = "";
    } else {
        Prj prj = m_configHolder.GetProject();

        switch (prj) {
            case Prj::RKst: {
                CutHolderRKst _cutHolder = CutHolderRKst(m_configHolder, m_cutOption);
                _cutHolder.Init();
                m_cut  = _cutHolder.Cut();
                m_cuts = _cutHolder.Cuts();
                break;
            }
            case Prj::RK: {
                CutHolderRK _cutHolder = CutHolderRK(m_configHolder, m_cutOption);
                _cutHolder.Init();
                m_cut  = _cutHolder.Cut();
                m_cuts = _cutHolder.Cuts();
                break;
            }
            case Prj::RPhi: {
                CutHolderRPhi _cutHolder = CutHolderRPhi(m_configHolder, m_cutOption);
                _cutHolder.Init();
                m_cut  = _cutHolder.Cut();
                m_cuts = _cutHolder.Cuts();
                break;
            }
            case Prj::RL: {
                CutHolderRL _cutHolder = CutHolderRL(m_configHolder, m_cutOption);
                _cutHolder.Init();
                m_cut  = _cutHolder.Cut();
                m_cuts = _cutHolder.Cuts();
                break;
            }
            case Prj::RKS: {
                CutHolderRKS _cutHolder = CutHolderRKS(m_configHolder, m_cutOption);
                _cutHolder.Init();
                m_cut  = _cutHolder.Cut();
                m_cuts = _cutHolder.Cuts();
                break;
            }
            default: MessageSvc::Error("CreateCut", (TString) "Invalid prj", to_string(prj), "EXIT_FAILURE"); break;
        }
    }

    if (!m_cutOption.Contains("noEXTRA")) {
        TCut _extraCut = TCut(SettingDef::Cut::extraCut);
        if((SettingDef::Cut::extraEEOnly && m_configHolder.GetAna() == Analysis::MM)) _extraCut = "1. > 0.";
        if((SettingDef::Cut::extraMMOnly && m_configHolder.GetAna() == Analysis::EE)) _extraCut = "1. > 0.";
        TCut _cut(_extraCut);
    
        if (IsCut(_cut)) {
            _cut                           = UpdateDTFCut(_cut);
            _cut                           = UpdatePIDTune(_cut, to_string(m_configHolder.GetYear()));
            _cut                           = ReplaceProject(_cut, m_configHolder.GetProject());
            map< TString, TString > _names = m_configHolder.GetParticleNames();
            _cut                           = TCut(ReplaceWildcards(TString(_cut), _names));
            _cut                           = CleanCut(_cut);
            m_cuts["cutEXTRA"]             = _cut;
            if (IsCutInMap("cutEXTRA", m_cuts)) m_cut = m_cut && m_cuts["cutEXTRA"];
        }
    }

    if (m_cutOption.Contains("NORM")) GetNormalizationCut();

    m_cut = UpdateDTFCut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = UpdateDTFCut(_cut.second); }

    if (m_configHolder.IsMC() && m_configHolder.IsSignalMC()) {
        m_cut = UpdateHLT1Cut(m_cut, m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetYear(), m_cutOption);
        for (auto _cut : m_cuts) { m_cuts[_cut.first] = UpdateHLT1Cut(_cut.second, m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetYear(), m_cutOption); }
    }

    m_cut = UpdateMVACut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = UpdateMVACut(_cut.second); }

    m_cut = ReplaceProject(m_cut, m_configHolder.GetProject());
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = ReplaceProject(_cut.second, m_configHolder.GetProject()); }

    m_cut = CleanCut(m_cut);
    for (auto _cut : m_cuts) { m_cuts[_cut.first] = CleanCut(_cut.second); }

    m_cuts["cutFULL"] = m_cut;

    return;
}

void CutHolder::GetNormalizationCut() {
    if (m_debug) MessageSvc::Debug("GetNormalizationCut", m_cutOption);

    TString _cutOption = m_cutOption;
    if (m_configHolder.GetAna() == Analysis::EE) _cutOption += "-noMinLPET";
    _cutOption.ReplaceAll("noSPD", "");
    _cutOption.ReplaceAll("noPS", "");
    // _cutOption.ReplaceAll("noPID", "");
    if (_cutOption.Contains("NORM60")) {
        // Keep bkgcat 60, removed by hand later in EfficiencyCreateFast
        _cutOption.ReplaceAll("tmSig", "tmSig");
        _cutOption.ReplaceAll("tmBkg", "tmBkg");
    } else {
        // remove for cutNORM the tmSigNoGhost
        _cutOption.ReplaceAll("tmSig", "tmSigNoGhost");
        _cutOption.ReplaceAll("tmBkg", "tmBkg");   // already without ghosts
    }
    _cutOption.ReplaceAll("NORM60", "").ReplaceAll("NORM", "");

    auto _ch = (*this).Clone(_cutOption);

    TCut _cut(NOCUT);
    if (!_cutOption.Contains("noSPD")) _cut = _cut && _ch.Cuts().at("cutSPD");
    if (!_cutOption.Contains("noPS")) _cut = _cut && _ch.Cuts().at("cutPS");
    // if (!_cutOption.Contains("noPID")) _cut = _cut && _ch.Cuts().at("cutPID");
    if (_cutOption.Contains("tm")) _cut = _ch.Cuts().at("cutMCT") && _cut;

    if (IsCut(_cut)) m_cuts["cutNORM"] = _cut;

    if (m_debug) MessageSvc::Debug("GetNormalizationCut", &_cut);
    return;
}

TCut CutHolder::Cut(TString _name) {
    if (IsCutInMap(_name, m_cuts)) return m_cuts[_name];
    MessageSvc::Error("IsCutInMap", _name, "not in map", "EXIT_FAILURE");
    return TCut(NOCUT);
}

void CutHolder::PrintCuts() {
    if (m_cuts.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "CutHolder", (TString) "PrintCuts");
        PrintInline();
        for (const auto & _cut : m_cuts) { MessageSvc::Info(_cut.first, &_cut.second); }
        MessageSvc::Line();
    }
    return;
}

void CutHolder::PrintInline() const noexcept {
    m_configHolder.PrintInline();
    TString _toPrint = fmt::format("CutOption {0}", Option());
    MessageSvc::Info(Color::Cyan, "CutHolder", _toPrint);
}

#endif
