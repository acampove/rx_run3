#ifndef WEIGHTHOLDERRL_CPP
#define WEIGHTHOLDERRL_CPP

#include "WeightHolderRL.hpp"
#include "CutHolder.hpp"

#include "SettingDef.hpp"
#include "WeightDefRX.hpp"

#include "HistogramSvc.hpp"

#include "core.h"
#include "fmt_ostream.h"
#include "vec_extends.h"

#include "TString.h"

ClassImp(WeightHolderRL)

    WeightHolderRL::WeightHolderRL()
    : m_configHolder() {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "Default");
    m_weightOption = SettingDef::Weight::option;
    m_weightConfig = SettingDef::Weight::config;
}

WeightHolderRL::WeightHolderRL(const ConfigHolder & _configHolder, TString _weightOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "ConfigHolder");
    m_weightOption = _weightOption;
    m_weightConfig = SettingDef::Weight::config;
}

WeightHolderRL::WeightHolderRL(const WeightHolderRL & _weightHolder)
    : m_configHolder(_weightHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("WeightHolderRL", (TString) "WeightHolderRL");
    m_weightOption = _weightHolder.Option();
    m_weightConfig = _weightHolder.Config();
    m_weight       = _weightHolder.Weight();
    m_weights      = _weightHolder.Weights();
}

ostream & operator<<(ostream & os, const WeightHolderRL & _weightHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "WeightHolderRL");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "weightOption", _weightHolder.Option());
    MessageSvc::Print((ostream &) os, "weightConfig", _weightHolder.Config());
    if (IsWeight(_weightHolder.Weight())) MessageSvc::Print((ostream &) os, "weight", _weightHolder.Weight());
    MessageSvc::Print((ostream &) os, "weights", to_string(_weightHolder.Weights().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void WeightHolderRL::Init() {
    MessageSvc::Info(Color::Cyan, "WeightHolderRL", (TString) "Initialize ...");
    if (m_configHolder.IsMC())
        CreateWeightMC();
    else
        CreateWeightCL();
    return;
}

void WeightHolderRL::CreateWeightMC() {
    if (m_debug) MessageSvc::Debug("CreateWeightMC", m_weightOption);

    m_weight = TString(NOWEIGHT);

    // ADD HERE
    //

    for (auto _weight : m_weights) {
        if (_weight.second == "") MessageSvc::Error("CreateWeightMC", (TString) "Invalid", _weight.first, "weight", "EXIT_FAILURE");
    }

    for (auto _weight : m_weights) { m_weights[_weight.first] = TString(ReplaceProject(TCut(_weight.second), m_configHolder.GetProject())); }
    for (auto _weight : m_weights) { m_weights[_weight.first] = CleanWeight(_weight.second); }

    // ADD HERE
    // if (IsWeightInMap(WeightDefRX::ID::BS, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::BS];
    // if (IsWeightInMap(WeightDefRX::ID::PID, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::PID];

    m_weight = CleanWeight(m_weight);
    m_weight = "(" + m_weight + ")";

    m_weights[WeightDefRX::ID::FULL] = m_weight;

    if (m_debug) MessageSvc::Debug("CreateWeightMC", m_weight);
    return;
}

void WeightHolderRL::CreateWeightCL() {
    if (m_debug) MessageSvc::Debug("CreateWeightCL", m_weightOption);

    m_weight = TString(NOWEIGHT);

    if (m_weightOption.Contains(WeightDefRX::ID::SP)) {
        Q2Bin q2bin = m_configHolder.GetQ2bin();

        TString _weight = "nsig_";
        switch (q2bin) {
            case Q2Bin::JPsi: _weight += "JPs"; break;
            default: MessageSvc::Error("CreateWeightCL", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
        }
        _weight += SettingDef::separator;
        _weight += m_configHolder.GetKey();
        _weight += "_sw";
        _weight.ReplaceAll("-", "M");   // WEIRD FEATURE OF GetClonedTree
        _weight = CleanWeight(_weight);

        m_weights[WeightDefRX::ID::SP] = _weight;
    }

    if (m_weightOption.Contains(WeightDefRX::ID::BS)) { m_weights[WeightDefRX::ID::BS] = TString(WeightDefRX::BS) + "[" + to_string(SettingDef::Weight::iBS) + "]"; }

    if (IsWeightInMap(WeightDefRX::ID::BS, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::BS];
    if (IsWeightInMap(WeightDefRX::ID::SP, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::SP];

    m_weight = CleanWeight(m_weight);
    m_weight = "(" + m_weight + ")";

    m_weights[WeightDefRX::ID::FULL] = m_weight;

    if (m_debug) MessageSvc::Debug("CreateWeightCL", m_weight);
    return;
}

#endif
