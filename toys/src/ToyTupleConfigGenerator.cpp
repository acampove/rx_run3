#ifndef TOYTUPLECONFIGGENERATOR_CPP
#define TOYTUPLECONFIGGENERATOR_CPP

#include "ToyTupleConfigGenerator.hpp"

#include "ToyTupleConfigSaver.hpp"

#include "TTimeStamp.h"

ToyTupleConfigGenerator::ToyTupleConfigGenerator()
    : m_configHolder() {
    MessageSvc::Line();
    MessageSvc::Info("ToyTupleConfigGenerator");
    MessageSvc::Line();
}

ToyTupleConfigGenerator::ToyTupleConfigGenerator(const ToyTupleConfigGenerator & _other)
    : m_configHolder(_other.m_configHolder) {
    m_configs = _other.m_configs;
}

ToyTupleConfigGenerator::ToyTupleConfigGenerator(ToyTupleConfigGenerator && _other)
    : m_configHolder(_other.m_configHolder) {
    m_configs = _other.m_configs;
}

void ToyTupleConfigGenerator::UpdateTupleConfig(const FitComponent & _fitComponent, double _nominalYield) {
    MessageSvc::Info("UpdateTupleConfig FitComponent", _fitComponent.Name());
    ExtractConfigHolder(_fitComponent);
    AddComponent(_fitComponent, _fitComponent.GetEventType().GetStringSample(), _nominalYield);
    UpdateComponents();
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::UpdateTupleConfig(const FitHolder & _fitHolder) {
    MessageSvc::Info("UpdateTupleConfig FitHolder", _fitHolder.Name());
    ExtractConfigHolder(_fitHolder.Data());
    ExtractComponents(_fitHolder);
    UpdateComponents(_fitHolder.Name());
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::UpdateTupleConfig(const FitManager & _fitManager) {
    MessageSvc::Info("UpdateTupleConfig FitManager", _fitManager.Name());
    for (auto & _fitHolder : _fitManager.Holders()) { UpdateTupleConfig(_fitHolder.second); }
    return;
}

void ToyTupleConfigGenerator::UpdateTupleConfig(const FitGenerator & _fitGenerator) {
    MessageSvc::Info("UpdateTupleConfig FitGenerator", _fitGenerator.Name(), to_string(_fitGenerator.Managers().size()));
    for (auto & _manager : _fitGenerator.Managers()) { UpdateTupleConfig(_manager.second); }
    return;
}

void ToyTupleConfigGenerator::UpdateTupleConfigWithPDF(const ConfigHolder & _configHolder, const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield) {
    MessageSvc::Info("UpdateTupleConfig with ConfigHolder, RooRealVar and RooAbsPdf");
    ThrowIfObservableNotInPdf(_observable, _pdf);
    m_configHolder = _configHolder;
    AddConfig(_componentKey, _observable, _pdf, _nominalYield, _configHolder.GetSample());
    UpdateComponents(_configHolder.GetKey());
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::OverwriteTupleConfig(const FitComponent & _fitComponent, double _nominalYield) {
    MessageSvc::Info("OverwriteTupleConfig FitComponent", _fitComponent.Name());
    ExtractConfigHolder(_fitComponent);
    AddComponent(_fitComponent, _fitComponent.GetEventType().GetStringSample(), _nominalYield);
    OverwriteComponents();
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::OverwriteTupleConfig(const FitHolder & _fitHolder) {
    MessageSvc::Info("OverwriteTupleConfig FitHolder", _fitHolder.Name());
    ExtractConfigHolder(_fitHolder.Data());
    ExtractComponents(_fitHolder);
    OverwriteComponents(_fitHolder.Name());
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::OverwriteTupleConfig(const FitManager & _fitManager) {
    MessageSvc::Info("OverwriteTupleConfig FitManager", _fitManager.Name());
    for (auto & _fitHolder : _fitManager.Holders()) { OverwriteTupleConfig(_fitHolder.second); }
    return;
}

void ToyTupleConfigGenerator::OverwriteTupleConfig(const FitGenerator & _fitGenerator) {
    MessageSvc::Info("OverwriteTupleConfig FitGenerator", _fitGenerator.Name(), to_string(_fitGenerator.Managers().size()));
    for (auto & _manager : _fitGenerator.Managers()) { OverwriteTupleConfig(_manager.second); }
    return;
}

void ToyTupleConfigGenerator::OverwriteTupleConfig(const ConfigHolder & _configHolder, const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield) {
    MessageSvc::Info("OverwriteTupleConfig with ConfigHolder, RooRealVar and RooAbsPdf");
    ThrowIfObservableNotInPdf(_observable, _pdf);
    m_configHolder = _configHolder;
    AddConfig(_componentKey, _observable, _pdf, _nominalYield, _configHolder.GetSample());
    OverwriteComponents();
    DeleteCurrentComponents();
    return;
}

void ToyTupleConfigGenerator::AddComponent(const FitComponent & _fitComponent, const TString _componentKey, double _nominalYield) {
    MessageSvc::Line();
    MessageSvc::Info("Adding  FitComponent", _fitComponent.Name(), _componentKey, to_string(_nominalYield));
    cout << _fitComponent << endl;
    ;

    TString _description = _fitComponent.GetEventType().GetSample();
    AddConfig(_componentKey, *(_fitComponent.Var()), *(_fitComponent.PDF()), _nominalYield, _description);

    MessageSvc::Line();
    return;
}

void ToyTupleConfigGenerator::AddConfig(const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield, const TString & _description) {
    MessageSvc::Line();
    MessageSvc::Info("Adding a toy tuple configuration", _componentKey, to_string(_nominalYield), _description);

    TString    _pdfKey        = TString(_pdf.GetName());
    TString    _observableKey = TString(_observable.GetName());
    TTimeStamp _timestamp;

    m_configs.emplace_back(_pdfKey, _observableKey, _componentKey, _nominalYield, _timestamp, _pdf);
    MessageSvc::Line();
}

void ToyTupleConfigGenerator::ExtractConfigHolder(const FitComponent & _fitComponent) {
    m_configHolder = ConfigHolder(_fitComponent.GetEventType());
    return;
}

void ToyTupleConfigGenerator::ExtractComponents(const FitHolder & _fitHolder) {
    ExtractSignal(_fitHolder);
    ExtractBackgrounds(_fitHolder);
    return;
}

void ToyTupleConfigGenerator::ExtractSignal(const FitHolder & _fitHolder) {
    TString                      _signalComponentkey = "Signal";
    const FitComponentAndYield & signalComponent     = _fitHolder.SignalComponent();
    ExtractComponent(signalComponent, _signalComponentkey);
    return;
}

void ToyTupleConfigGenerator::ExtractComponent(const FitComponentAndYield & _componentAndYield, const TString _componentKey) {
    RooAbsReal *         _yield        = _componentAndYield.yield;
    double               _nominalYield = _yield->getValV();
    const FitComponent & _fitComponent = _componentAndYield.fitComponent;
    AddComponent(_fitComponent, _componentKey, _nominalYield);
    return;
}

void ToyTupleConfigGenerator::ExtractBackgrounds(const FitHolder & _fitHolder) {
    const Str2ComponentMap & backgroundComponentMap = _fitHolder.BackgroundComponents();
    for (auto & stringComponentPair : backgroundComponentMap) {
        TString _componentKey      = stringComponentPair.first;
        auto &  _componentAndYield = stringComponentPair.second;
        ExtractComponent(_componentAndYield, _componentKey);
    }
    return;
}

void ToyTupleConfigGenerator::ThrowIfObservableNotInPdf(const RooRealVar & _observable, const RooAbsPdf & _pdf) const {
    if (!_pdf.dependsOn(_observable)) MessageSvc::Error("Add FitComponent", (TString) "PDF", _pdf.GetName(), "does not depend on observable", _observable.GetName(), "EXIT_FAILURE");
    return;
}

void ToyTupleConfigGenerator::UpdateComponents(TString _key) {
    MessageSvc::Info("ToyTupleConfigGenerator", (TString) "UpdateComponents");
    ToyTupleConfigSaver _saver = ToyTupleConfigSaver(m_configHolder);
    _saver.AddConfigurations(m_configs);
    if (SettingDef::Toy::mergeConfig)
        _saver.Update(_key);
    else
        _saver.Update();
    return;
}

void ToyTupleConfigGenerator::OverwriteComponents(TString _key) {
    MessageSvc::Info("ToyTupleConfigGenerator", (TString) "OverwriteComponents");
    ToyTupleConfigSaver _saver = ToyTupleConfigSaver(m_configHolder);
    _saver.AddConfigurations(m_configs);
    if (SettingDef::Toy::mergeConfig)
        _saver.Append(_key);
    else
        _saver.Overwrite();
    return;
}

void ToyTupleConfigGenerator::DeleteCurrentComponents() {
    m_configs.clear();
    return;
}

#endif
