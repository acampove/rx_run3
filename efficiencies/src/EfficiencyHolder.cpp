#ifndef EFFICIENCYHOLDER_CPP
#define EFFICIENCYHOLDER_CPP

#include "EfficiencyHolder.hpp"

#include "SettingDef.hpp"

ClassImp(EfficiencyHolder)

    EfficiencyHolder::EfficiencyHolder() {
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EfficiencyHolder", (TString) "Default");
    m_option = SettingDef::Efficiency::option;
    Check(m_option);
}

EfficiencyHolder::EfficiencyHolder(EventType & _eventType, TString _option) {
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    m_option = _option;
    Check(m_option);

    ZippedEventType _zip = _eventType.Zip();

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "Pas");
    m_pas = EventType(_zip);
    cout << m_pas << endl;

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "Tot");
    _zip.cutOption = "no";   // SHOULD INCLUDE nSPD
    _zip.weightOption += "-MCT";
    _zip.tupleOption = "pro";
    if (!_option.Contains("RECO")) _zip.tupleName = "MCT";
    m_tot = EventType(_zip);
    cout << m_tot << endl;

    Init();
}

EfficiencyHolder::EfficiencyHolder(EventType & _eventTypePas, EventType & _eventTypeTot, TString _option) {
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    m_option = _option;
    Check(m_option);

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "Pas");
    m_pas = EventType(_eventTypePas);
    cout << m_pas << endl;

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "Tot");
    m_tot = EventType(_eventTypeTot);
    cout << m_tot << endl;

    Init();
}

EfficiencyHolder::EfficiencyHolder(const EfficiencyHolder & _efficiencyHolder) {
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    m_option       = _efficiencyHolder.Option();
    m_pas          = _efficiencyHolder.EventPas();
    m_tot          = _efficiencyHolder.EventTot();
    m_efficiencies = _efficiencyHolder.Efficiencies();
    MessageSvc::Info("EfficiencyHolder", (TString) "EfficiencyHolder");
}

ostream & operator<<(ostream & os, const EfficiencyHolder & _efficiencyHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "EfficiencyHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "option", _efficiencyHolder.Option());
    MessageSvc::Print((ostream &) os, "efficiencies", to_string(_efficiencyHolder.Efficiencies().size()));
    MessageSvc::Line(os);
    os << RESET;
    os << "\033[F";
    return os;
}

bool EfficiencyHolder::Check(TString _option) {
    for (auto _opt : *(_option.Tokenize("-"))) {
        if (!CheckVectorContains(SettingDef::AllowedConf::EfficiencyOptions, ((TObjString *) _opt)->String())) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("EfficiencyHolder", "\"" + ((TObjString *) _opt)->String() + "\"", "option not in SettingDef::AllowedConf::EfficiencyOptions", "EXIT_FAILURE");
        }
    }
    return false;
}

void EfficiencyHolder::Init() {
    MessageSvc::Info("EfficiencyHolder", (TString) "Initialize Pas ...");
    m_pas.Init(true);
    cout << WHITE << m_pas << RESET << endl;
    MessageSvc::Info("EfficiencyHolder", (TString) "Initialize Tot ...");
    m_tot.Init(true);
    cout << WHITE << m_tot << RESET << endl;
    return;
}

void EfficiencyHolder::SetVariables(RooRealVar _varPas, RooRealVar _varTot, int nBins) {
    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "SetVariables");
    if (nBins != 0) {
        _varPas.setBins(nBins);
        _varTot.setBins(nBins);
    }
    m_varPas.push_back(_varPas);
    m_varTot.push_back(_varTot);
    MessageSvc::Info("Pas", &_varPas);
    MessageSvc::Info("Tot", &_varTot);
    MessageSvc::Line();
    return;
}

void EfficiencyHolder::ComputeEfficiency() {
    bool _flag = false;
    for (const auto & _var : m_varPas) {
        if (_var.GetName() == m_var) _flag = true;
    }
    if (!_flag) {
        RooRealVar _var = RooRealVar(m_var, m_var, 0, 1e9);
        _var.setBins(1);
        SetVariables(_var, _var);
    }

    MessageSvc::Line();
    MessageSvc::Info("ComputeEfficiency");

    if (m_varPas.size() != m_varTot.size()) MessageSvc::Error("ComputeEfficiency", (TString) "Incompatible Pas and Tot variable size", "EXIT_FAILURE");

    for (unsigned i = 0; i < m_varPas.size(); i++) {
        MessageSvc::Info("Pas", &m_varPas[i]);
        MessageSvc::Info("Tot", &m_varTot[i]);

        TString _name = "eff_" + m_pas.GetKey();
        if (m_varPas[i].GetName() != m_var) _name += (TString) "_" + m_varPas[i].GetName();

        m_hPas.push_back(*static_cast< TH1D * >(GetHistogram(*m_pas.GetTuple(), m_pas.GetWCut(), "pas", m_varPas[i])));
        m_hTot.push_back(*static_cast< TH1D * >(GetHistogram(*m_tot.GetTuple(), m_tot.GetWCut(), "tot", m_varTot[i])));

        TEfficiency _tEff = TEfficiency();
        _tEff.SetName((TString) "t" + _name);

        TH1D _hEff = m_hPas.back();
        _hEff.Reset();
        _hEff.SetName(((TString) _hEff.GetName()).ReplaceAll("pas", "eff"));
        _hEff.SetMinimum(0);
        _hEff.SetMaximum(1.1);

        RooRealVar * _eff = (RooRealVar *) GetEfficiency(m_hPas.back(), m_hTot.back(), _tEff, _hEff);
        m_tEff.push_back(_tEff);
        m_hEff.push_back(_hEff);
        if (_eff != nullptr) {
            _eff->SetName(_name);
            _eff->SetTitle(_name);
            _eff->setRange(0, 1);

            // if (SettingDef::Efficiency::blind)
            //    m_efficiencies[_name.Data()] = BlindParameter(_eff);
            // else
            m_efficiencies[_name.Data()] = _eff;
        }
        MessageSvc::Line();
    }

    // if (m_option.Contains("gen")) ComputeGeneratorEfficiency();
    // if (m_option.Contains("flt")) ComputeFilteringEfficiency();

    PrintEfficiencies();

    return;
}

RooRealVar * EfficiencyHolder::GetEfficiency(TH1D _hPas, TH1D _hTot, TEfficiency & _tEff, TH1D & _hEff, TString _option) {
    double _nPas = _hPas.GetEntries();
    double _wPas = _hPas.Integral();

    double _nTot = _hTot.GetEntries();
    double _wTot = _hTot.Integral();

    cout << endl;
    MessageSvc::Info("Events");
    MessageSvc::Info("Pas", to_string(_nPas));
    MessageSvc::Info("Tot", to_string(_nTot));
    cout << endl;
    MessageSvc::Info("Weighted Events");
    MessageSvc::Info("Pas", to_string(_wPas));
    MessageSvc::Info("Tot", to_string(_wTot));
    cout << endl;

    for (int i = 1; i <= m_hTot.back().GetNbinsX(); ++i) m_hTot.back().SetBinContent(i, _wTot);

    double _iPas = _wPas;
    double _statPas[10];
    _hPas.GetStats(_statPas);
    if (TMath::Abs(_statPas[0] - _statPas[1]) > 1e-5) {
        MessageSvc::Warning("Pas histogram filled with weights");
        _hPas = *static_cast< TH1D * >(RoundToIntAllEntries(&_hPas));
        _iPas = _hPas.Integral();
    }

    double _iTot = _wTot;
    double _statTot[10];
    _hTot.GetStats(_statTot);
    if (TMath::Abs(_statTot[0] - _statTot[1]) > 1e-5) {
        MessageSvc::Warning("Tot histogram filled with weights");
        _hTot = *static_cast< TH1D * >(RoundToIntAllEntries(&_hTot));
        _iTot = _hTot.Integral();
    }

    CheckHistogram(&_hPas, &_hTot, "effr");

    _tEff = TEfficiency(_hPas, _hTot);

    for (int i = 1; i <= _hEff.GetNbinsX(); ++i) {
        if ((_hPas.GetBinContent(i) != 0) && (_hTot.GetBinContent(i) != 0)) {
            _hEff.SetBinContent(i, _tEff.GetEfficiency(_hEff.GetBin(i)));
            _hEff.SetBinError(i, (_tEff.GetEfficiencyErrorUp(_hEff.GetBin(i)) + _tEff.GetEfficiencyErrorLow(_hEff.GetBin(i))) / 2.);
        }
    }

    MessageSvc::Info("int(Weighted Events)");
    MessageSvc::Info("Pas", to_string(_iPas));
    MessageSvc::Info("Tot", to_string(_iTot));

    return (RooRealVar *) GetEfficiency(_iPas, _iTot);
}

RooRealVar * EfficiencyHolder::GetEfficiency(double _nPas, double _nTot) {
    TH1D _hPas = TH1D("", "", 1, 0, 1);
    _hPas.SetBinContent(1, _nPas);
    _hPas.SetBinError(1, TMath::Sqrt(_nPas));

    TH1D _hTot = TH1D("", "", 1, 0, 1);
    _hTot.SetBinContent(1, _nTot);
    _hTot.SetBinError(1, TMath::Sqrt(_nTot));

    TEfficiency _tEff = TEfficiency(_hPas, _hTot);

    RooRealVar * _eff = new RooRealVar("eff", "eff", 0, 1);
    _eff->setVal(_tEff.GetEfficiency(1));
    _eff->setError((_tEff.GetEfficiencyErrorLow(1) + _tEff.GetEfficiencyErrorUp(1)) / 2.);
    _eff->setAsymError(_tEff.GetEfficiencyErrorLow(1), _tEff.GetEfficiencyErrorUp(1));
    MessageSvc::Info("Efficiency", _eff);
    return _eff;
}

RooRealVar * EfficiencyHolder::GetEfficiency(TString _name) {
    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "GetEfficiency", _name);

    if (!IsVarInMap(_name.Data(), m_efficiencies)) {
        cout << endl;
        cout << RED;
        PrintPars(m_efficiencies);
        cout << RESET;
        MessageSvc::Error("IsVarInMap", _name, "not in map", "EXIT_FAILURE");
    }
    if (m_efficiencies[_name.Data()] == nullptr) MessageSvc::Error("GetEfficiency", _name, "is nullptr", "EXIT_FAILURE");

    MessageSvc::Info("Efficiency", (RooRealVar *) m_efficiencies[_name.Data()]);
    MessageSvc::Line();
    return (RooRealVar *) m_efficiencies[_name.Data()];
}

void EfficiencyHolder::PrintEfficiencies() {
    MessageSvc::Line();
    MessageSvc::Info("PrintEfficiencies", (TString) "nEfficiencies =", to_string(m_efficiencies.size()));
    cout << GREEN;
    PrintPars(m_efficiencies);
    cout << RESET;
    MessageSvc::Line();
    return;
}

void EfficiencyHolder::PlotEfficiencies(TString _name) {
    MessageSvc::Line();
    MessageSvc::Info("PlotEfficiencies", (TString) "nEfficiencies =", to_string(m_efficiencies.size()));

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "EfficiencyHolder_" + m_pas.GetKey() + _name + ".pdf";

    TCanvas _canvas("canvas");
    _canvas.SaveAs(_name + "[");
    for (auto & _hEff : m_hEff) {
        _hEff.Draw("e1");
        _canvas.SaveAs(_name);
    }
    _canvas.SaveAs(_name + "]");

    MessageSvc::Line();
    return;
}

void EfficiencyHolder::SaveToDisk(TString _name) {
    // SaveToLog(_name);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = "EfficiencyHolder" + _name;
    _name          = SettingDef::IO::outDir + "EfficiencyHolder" + _name + ".root";

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "SaveToDisk", _name);

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);
    _tFile.Close();
    MessageSvc::Line();
    cout << WHITE << *this << RESET << endl;
    return;
}

void EfficiencyHolder::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "LoadFromDisk", _name, _dir);
    MessageSvc::Line();

    if (_name != "") _name = "_" + _name;
    _name = "EfficiencyHolder" + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("EfficiencyHolder", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    _tFile.ls();

    EfficiencyHolder * _eh = (EfficiencyHolder *) _tFile.Get(_name);
    *this                  = *_eh;

    _tFile.Close();
    MessageSvc::Line();
    cout << WHITE << *this << RESET << endl;
    return;
}

void EfficiencyHolder::SaveToLog(TString _name) {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "EfficiencyHolder" + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info("EfficiencyHolder", (TString) "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}

#endif
