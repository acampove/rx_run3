#include "WeightHolder.hpp"
#include "WeightHolderRL.hpp"
#include "WeightHolderRX.hpp"

#include "SettingDef.hpp"

#include "HistogramSvc.hpp"

#include "core.h"
#include "fmt_ostream.h"
#include "vec_extends.h"

#include "TClass.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TKey.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TROOT.h"
#include "TString.h"

ClassImp(WeightHolder)

WeightHolder::WeightHolder() : m_configHolder() 
{
    MessageSvc::Debug("ConfigHolder", "Calling default constructor");

    m_weightOption = SettingDef::Weight::option;
    m_weightConfig = SettingDef::Weight::config;

    //Special routine to make it work with weightOption[weightConfig] or [weightConfig]weightOption bypassing globally configured one
    if ( m_weightOption.BeginsWith("[") || m_weightOption.EndsWith("]"))
    {
        MessageSvc::Warning("WeightHolder, forcing weightConfig from Weight Option using [XXX] begin/end with");
        m_weightConfig = StripStringBetween(m_weightOption, "[","]");
        //Weak check that the weightConfig contains at least one among the allowed ones in SettingDef
        bool valid = false ;
        for( auto & wConfBitsAllow : SettingDef::AllowedConf::WeightConfig)
        {
            if( m_weightConfig.Contains(wConfBitsAllow))
            { 
                valid = true; 
                break;
            }
        }

        if ( ! valid)
            MessageSvc::Fatal("WeightHolder", "WeightHolder with custom wconfig not allowed ", m_weightConfig);
        else
            MessageSvc::Warning("WeightHolder", "WeightHolder bypassing global settings via weightOption for weightConfig ", m_weightConfig);

        //clear the [XXX] from WeightOption
        m_weightOption =  m_weightOption.ReplaceAll(TString::Format("[%s]", m_weightConfig.Data()), "");
    }

    _Check();
}

WeightHolder::WeightHolder(const ConfigHolder & _configHolder, const TString &_weightOption) : m_configHolder(_configHolder) 
{
    MessageSvc::Debug("ConfigHolder", "Calling constructor with ConfigHolder and option:\"", _weightOption, "\"");

    m_weightOption = _weightOption;

    _SetWeightConfig();

    _Check();
}

void WeightHolder::_SetWeightConfig()
{
    //Special routine to make it work with weightOption[weightConfig] or [weightConfig]weightOption bypassing globally configured one
    if( ! m_weightOption.BeginsWith("[") && ! m_weightOption.EndsWith("]"))
    {
        m_weightConfig = SettingDef::Weight::config;
        return;
    }

    m_weightConfig = StripStringBetween(m_weightOption, "[","]");

    //Weak check that the weightConfig contains at least one among the allowed ones in SettingDef
    bool valid = false ;
    for ( auto & wConfBitsAllow : SettingDef::AllowedConf::WeightConfig)
    {
        if( m_weightConfig.Contains(wConfBitsAllow))
        { 
            valid = true; 
            break;
        }
    }

    if ( !valid )
        MessageSvc::Fatal(TString("WeightHolder"), "WeightHolder with custom wconfig not allowe", m_weightConfig);

    MessageSvc::Warning("WeightHolder", "WeightHolder bypassing global settings via weightOption for weightConfig", m_weightConfig);
    //clear the [XXX] from WeightOption
    m_weightOption =  m_weightOption.ReplaceAll(TString::Format("[%s]", m_weightConfig.Data()), "");
}

WeightHolder::WeightHolder(const WeightHolder & _weightHolder)
    : m_configHolder(_weightHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("WeightHolder", (TString) "WeightHolder");
    m_weightOption = _weightHolder.Option();
    m_weightConfig = _weightHolder.Config();
    m_weight       = _weightHolder.Weight();
    m_weights      = _weightHolder.Weights();
    _Check();
}

ostream & operator<<(ostream & os, const WeightHolder & _weightHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "WeightHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "weightOption", _weightHolder.Option());
    MessageSvc::Print((ostream &) os, "weightConfig", _weightHolder.Config());
    if (IsWeight(_weightHolder.Weight())) MessageSvc::Print((ostream &) os, "weight", _weightHolder.Weight());
    MessageSvc::Print((ostream &) os, "weights", to_string(_weightHolder.Weights().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

bool WeightHolder::_Check() 
{
    for (auto _opt : TokenizeString(m_weightOption, SettingDef::separator)) 
    {
        if (!CheckVectorContains(SettingDef::AllowedConf::WeightOptions, _opt)) 
        {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("WeightHolder", (TString) "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::WeightOptions", "EXIT_FAILURE");
        }
    }

    for (auto _opt : TokenizeString(m_weightConfig, "_")) {
        if (!CheckVectorContains(SettingDef::AllowedConf::WeightConfig, _opt)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("WeightHolder", (TString) "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::WeightConfig", "EXIT_FAILURE");
        }
    }

    vector< TString > _confs = TokenizeString(m_weightConfig, "_");
    switch (m_configHolder.GetProject()) {
        case Prj::RKst:
            if (_confs.size() > 2) MessageSvc::Error("WeightHolder", (TString) "Invalid \"" + m_weightConfig + "\"", "combination", "EXIT_FAILURE");
            break;
        case Prj::RK:
            if (_confs.size() > 2) MessageSvc::Error("WeightHolder", (TString) "Invalid \"" + m_weightConfig + "\"", "combination", "EXIT_FAILURE");
            break;
        case Prj::RPhi:
            if (_confs.size() > 3) MessageSvc::Error("WeightHolder", (TString) "Invalid \"" + m_weightConfig + "\"", "combination", "EXIT_FAILURE");
            break;
        case Prj::RL: break;
        case Prj::RKS: break;
        default: MessageSvc::Warning("WeightHolder", (TString) "Invalid prj", to_string(m_configHolder.GetProject()), "SettingDef::Weight::config", m_weightConfig); break;
    }

    if (m_configHolder.GetProject() != Prj::All) {
        vector< pair< TString, TString > > _heads = GetParticleBranchNames(m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetQ2bin(), "onlyhead");
        m_weightConfig.ReplaceAll("{B}", _heads[0].first).ReplaceAll("{HEAD}", _heads[0].first);
    } else {
        MessageSvc::Warning("WeightHolder", (TString) "Invalid prj", to_string(m_configHolder.GetProject()), "SettingDef::Weight::config", m_weightConfig);
    }

    if (m_weightOption.Contains("MCT")) { SetOptionMCT(); }

    return false;
}

void WeightHolder::Init() {
    MessageSvc::Info(Color::Cyan, "WeightHolder", (TString) "Initialize ...");
    TString _weightOption      = SettingDef::Weight::option;
    TString _weightConfig      = SettingDef::Weight::config;
    SettingDef::Weight::option = m_weightOption;
    SettingDef::Weight::config = m_weightConfig;
    CreateWeight();
    SettingDef::Weight::option = _weightOption;
    SettingDef::Weight::config = _weightConfig;
    PrintWeights();
    return;
}

void WeightHolder::CreateWeight() {
    m_weight = TString(NOWEIGHT);
    m_weights.clear();
    if ((m_weightOption == "") || (m_weightOption == "no") || (m_weightOption == "-no")) {
        m_weight                         = "(" + m_weight + ")";
        m_weights[WeightDefRX::ID::FULL] = m_weight;
    } else if (m_configHolder.IsRapidSim()) {
        m_weightOption                   = "";
        m_weights[WeightDefRX::ID::FULL] = m_weight;
    } else {
        Prj prj = m_configHolder.GetProject();

        switch (prj) {
            case Prj::RKst: {
                WeightHolderRX _weightHolder = WeightHolderRX(m_configHolder, m_weightOption);
                _weightHolder.Init();
                m_weight  = _weightHolder.Weight();
                m_weights = _weightHolder.Weights();
                break;
            }
            case Prj::RK: {
                WeightHolderRX _weightHolder = WeightHolderRX(m_configHolder, m_weightOption);
                _weightHolder.Init();
                m_weight  = _weightHolder.Weight();
                m_weights = _weightHolder.Weights();
                break;
            }
            case Prj::RPhi: {
                WeightHolderRX _weightHolder = WeightHolderRX(m_configHolder, m_weightOption);
                _weightHolder.Init();
                m_weight  = _weightHolder.Weight();
                m_weights = _weightHolder.Weights();
                break;
            }
            case Prj::RL: {
                WeightHolderRL _weightHolder = WeightHolderRL(m_configHolder, m_weightOption);
                _weightHolder.Init();
                m_weight  = _weightHolder.Weight();
                m_weights = _weightHolder.Weights();
                break;
            }
            case Prj::RKS: {
                WeightHolderRL _weightHolder = WeightHolderRL(m_configHolder, m_weightOption);
                _weightHolder.Init();
                m_weight  = _weightHolder.Weight();
                m_weights = _weightHolder.Weights();
                break;
            }
            default: MessageSvc::Error("CreateWeight", (TString) "Invalid prj", to_string(prj), "EXIT_FAILURE"); break;
        }
    }
    return;
}

TString WeightHolder::Weight(TString _name, bool _force) {
    if (IsWeightInMap(_name, m_weights)) return m_weights[_name];
    if (_force) MessageSvc::Error("WeightHolder", _name, "not in map", "EXIT_FAILURE");
    return TString(NOWEIGHT);
}

void WeightHolder::PrintWeights() {
    if (m_weights.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "WeightHolder", (TString) "PrintWeights");
        PrintInline();
        for (const auto & _weight : m_weights) { MessageSvc::Info(_weight.first, _weight.second); }
        MessageSvc::Line();
    }
    return;
}

void WeightHolder::PrintInline() const noexcept {
    m_configHolder.PrintInline();
    TString _toPrint = fmt::format("WeightOption {0} - WeightConfig {1}", Option(), Config());
    MessageSvc::Info(Color::Cyan, "WeightHolder", _toPrint);
}

WeightHolder WeightHolder::Clone(TString _weightConfig, TString _weightOption) {
    MessageSvc::Info(Color::Cyan, "WeightHolder", (TString) "Cloning ...");

    TString _weightConfigTmp   = SettingDef::Weight::config;
    SettingDef::Weight::config = _weightConfig;
    MessageSvc::Info("WeightConfig", _weightConfig);

    if (_weightOption == "") {
        MessageSvc::Warning("Clone", (TString) "Use old weightOption");
        _weightOption = this->Option();
    } else {
        MessageSvc::Warning("Clone", (TString) "Use new weightOption");
    }
    MessageSvc::Info("WeightOption", _weightOption);

    ConfigHolder _configHolder = this->GetConfigHolder();
    WeightHolder _weightHolder(_configHolder, _weightOption);
    _weightHolder.Init();

    SettingDef::Weight::config = _weightConfigTmp;
    return _weightHolder;
}

TH1D * WeightHolder::GetWeightPartReco(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightPartReco", _option);

    TString _weightDir = IOSvc::GetDataDir("partreco");

    TString _fileName = _weightDir + "/" + _option + ".root";

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightPartReco", _fileName, "does not exist", "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightPartReco", _fileName, "does not exist");
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH1D *  _histo = nullptr;
    TIter   _keyList(gDirectory->GetListOfKeys());
    TKey *  _key;
    while ((_key = (TKey *) _keyList())) {
        if (((TString) _key->ReadObj()->GetName()).Contains(_option)) _histo = (TH1D *) _key->ReadObj();
    }

    if (!_histo) {
        MessageSvc::Error("GetWeightPartReco", (TString) "Map", _option, "does no exist");
        _tFile->ls();
        MessageSvc::Error("GetWeightPartReco", (TString) "Map", _option, "does no exist", "EXIT_FAILURE");
        return nullptr;
    }

    TH1D * _map = (TH1D *) CopyHist(_histo);
    MessageSvc::Info("GetWeightPartReco", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightPartReco", _map);
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

TH2 * WeightHolder::GetWeightMapTRK(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapTRK", _option);

    TString _trkVer = SettingDef::Weight::trkVer;

    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::TRK);

    SettingDef::Weight::trkVer = _trkVer;

    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TString _fileName = _weightDir + "/wTracking_";
    if (ana == Analysis::MM)
        _fileName += "Muon_";
    else if (ana == Analysis::EE)
        _fileName += "Electron_";
    _fileName += to_string(year) + ".root";

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapTRK", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapTRK", _fileName, "does not exist", _option);
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH2 *   _histo;
    if (_option.Contains("norf"))
        _histo = (TH2 *) _tFile->Get("heffratio_norf");
    else if (_option.Contains("rf"))
        _histo = (TH2 *) _tFile->Get("heffratio_rf");
    else
        _histo = (TH2 *) _tFile->Get("Ratio");

    if (!_histo) {
        MessageSvc::Warning("GetWeightMapTRK", (TString) "weightMap", "does not exist", _option);
        return nullptr;
    }

    // TH2 * _map = (TH2 *) CopyHist(_histo);
    TH2 * _map = (TH2 *) _histo->Clone((TString) _histo->GetName() + "_copy");
    MessageSvc::Info("GetWeightMapTRK", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightMapTRK", _map);
    CheckHistogram(_map, _option);
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

pair < TH1D *, vector < TH2D * > > WeightHolder::GetWeightMapsPID_fac(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsPID Fit and Count", _option);

    TString _pidVer = SettingDef::Weight::pidVer;
    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::PID);
    Prj      prj      = m_configHolder.GetProject();
    Year     year     = m_configHolder.GetYear();
    Polarity polarity = m_configHolder.GetPolarity();
    TString _fileName;
    _fileName = _weightDir + "/" + to_string(prj) + "/Fit_";

    vector < TH2D * > efficiency_histograms;

    // particle mis ID
    TString _misID;
    if (!_option.Contains("MID"))        _misID = "ID";
    else if (_option.Contains("MID_K"))  _misID = "MISIDEK";
    else if (_option.Contains("MID_Pi")) _misID = "MISIDEPI";

    MessageSvc::Info(Color::Cyan, "Option: ", _option);
    if (_option.Contains("ALTNTRACKS")){    
      if (_option.Contains("brem0"))      _fileName += "PolBoth_Year" + to_string(year) + "_nBrem0_" + _misID + "_ALTNTRACKS" + ".root";
      else if (_option.Contains("brem1")) _fileName += "PolBoth_Year" + to_string(year) + "_nBrem1_" + _misID + "_ALTNTRACKS" + ".root";
    }
    else if (_option.Contains("NoOpt")){    
      if (_option.Contains("brem0"))      _fileName += "PolBoth_Year" + to_string(year) + "_nBrem0_" + _misID + "_NoOpt"      + ".root";
      else if (_option.Contains("brem1")) _fileName += "PolBoth_Year" + to_string(year) + "_nBrem1_" + _misID + "_NoOpt"      + ".root";
    }
    else {
      if (_option.Contains("brem0"))      _fileName += "PolBoth_Year" + to_string(year) + "_nBrem0_" + _misID                 + ".root";
      else if (_option.Contains("brem1")) _fileName += "PolBoth_Year" + to_string(year) + "_nBrem1_" + _misID                 + ".root";
    }

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapsPID_fac", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapsPID_fac", _fileName, "does not exist", _option);
        return make_pair(nullptr,efficiency_histograms);
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);

    TString _mapName = (_option.Contains("WeightMapPID") and !_option.Contains("MID"))? "hWeight_" : "hEff_";
    MessageSvc::Info(Color::Cyan, "GetWeightMapsPID_fac map Name: ", _mapName);
    pair < TH1D*, vector < TH2D * > > maps;
    TH1D * _nTracksHisto;
    TH1D * nTracks_histogram = _tFile->Get<TH1D>("nTracks_edges");
    if (!nTracks_histogram) {
        MessageSvc::Error("GetWeightMapsPID_fac", _fileName, " nTracks map does not exist", _option, "EXIT_FAILURE");
        return make_pair(nullptr,efficiency_histograms);
    }
    else {
        _nTracksHisto = (TH1D*) nTracks_histogram->Clone((TString) nTracks_histogram->GetName() + "_copy");
        _nTracksHisto->SetName(_option);
        _nTracksHisto->SetDirectory(0);
        delete nTracks_histogram;
    }

    int nTracksSlices = _nTracksHisto->GetNbinsX();
    for ( int slice = 0; slice < nTracksSlices; ++slice ) {
        TH2D * efficiency_histogram;

    if (!_option.Contains("BS")) efficiency_histogram = _tFile->Get<TH2D>((TString) _mapName + to_string(slice));
        if (!efficiency_histogram ) {
            MessageSvc::Error("GetWeightMapsPID_fac", _fileName, " slice ", to_string(slice), " does not exist", _option, "EXIT_FAILURE");
            return make_pair(nullptr,efficiency_histograms);
        }
        else {
            TH2D * _map = (TH2D *) efficiency_histogram->Clone((TString) efficiency_histogram->GetName() + "_copy");
            MessageSvc::Info("GetWeightMapsPID_fac", (TString) _map->GetName());
            _map->SetName(_option + "-nTracks" + to_string(slice) );
            _map->SetDirectory(0);
            MessageSvc::Info("GetWeightMapsPID_fac", _map);
            if (!_option.Contains("WeightMapPID") || _option.Contains("MID")) {                
                CheckHistogram(_map, _option); // for weight map dont change values > 1 to be 1!!
            }
            MessageSvc::Line();
            delete efficiency_histogram;
            efficiency_histograms.push_back(_map);
        }
    }

    maps = make_pair(_nTracksHisto, efficiency_histograms);
    IOSvc::CloseFile(_tFile);

    //Check maps carefully
    // cout << "Check maps:" << endl;
    // maps.first->Print();
    // for(auto const& map: maps.second) {
    //  map->Print();
    // }
    CheckPIDMapsForNullPtrs(maps, _option, true);
    
    return maps;
}

pair < TH1D *, vector < TH2D * > > WeightHolder::GetWeightMapsPID(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsPID", _option);

    TString _pidVer = SettingDef::Weight::pidVer;
    if (_option.Contains("KDE")) SettingDef::Weight::pidVer += "_KDE";

    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::PID);
    SettingDef::Weight::pidVer = _pidVer;
    Prj      prj      = m_configHolder.GetProject();
    Year     year     = m_configHolder.GetYear();
    Polarity polarity = m_configHolder.GetPolarity();

    TString _fileName;
    _fileName = _weightDir + "/" + to_string(prj) + "/weightMap_";
    
    vector < TH2D * > efficiency_histograms;

    // particle real ID
    TString _particle;
    if (_option.Contains("PID_K"))
        _particle = "K";
    else if (_option.Contains("PID_Pi"))
        _particle = "Pi";
    else if (_option.Contains("PID_M"))
        _particle = "Mu";
    else if (_option.Contains("PID_E"))
        _particle = "e";
    else if (_option.Contains("PID_P"))
        _particle = "P";

    // particle mis ID
    TString _misID;
    if (!_option.Contains("MID"))
        _misID = _particle;
    else if (_option.Contains("MID_K"))
        _misID = "K";
    else if (_option.Contains("MID_Pi"))
        _misID = "Pi";
    else if (_option.Contains("MID_M"))
        _misID = "Mu";
    else if (_option.Contains("MID_E"))
        _misID = "e";
    else if (_option.Contains("MID_P"))
        _misID = "P";
    
    if (_option.Contains("KDE_ALTNTRACKS")) {_fileName += _particle + "_" + _misID + "_20" + to_string(year) + "_ALTNTRACKS" + ".root";}
    else                                    {_fileName += _particle + "_" + _misID + "_20" + to_string(year) +                 ".root";}

    cout << "Open _fileName in GetWeightMapsPID(): " << _fileName << endl;
    /*if(_option.Contains("IsMuon"))
    {
        _fileName.ReplaceAll(".root","_IsMuon.root");
    }*/

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapsPID", _fileName, "does not exist", _option, "EXIT_FAILURE");
        cout << "GetWeightMapsPID map not existing but not throwing out of routine" << endl;
        MessageSvc::Warning("GetWeightMapsPID", _fileName, "does not exist", _option);
        return make_pair(nullptr,efficiency_histograms);
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);

    pair < TH1D*, vector < TH2D * > > maps;
    TH1D * _nTracksHisto;
    TH1D * nTracks_histogram = _tFile->Get<TH1D>("nTracks_edges");
    if (! nTracks_histogram) {
        MessageSvc::Error("GetWeightMapsPID", _fileName, " nTracks map does not exist", _option, "EXIT_FAILURE");
        return make_pair(nullptr,efficiency_histograms);
    }
    else {
        _nTracksHisto = (TH1D*) nTracks_histogram->Clone((TString) nTracks_histogram->GetName() + "_copy");
        _nTracksHisto->SetName(_option);
        _nTracksHisto->SetDirectory(0);
        delete nTracks_histogram;
    }

    int nTracksSlices = _nTracksHisto->GetNbinsX();
    for ( int slice = 0; slice < nTracksSlices; ++slice ) {
        TH2D * efficiency_histogram;
        if (!_option.Contains("KDE_BS-")) efficiency_histogram = _tFile->Get<TH2D>((TString) "hEff_" + to_string(slice));
        if (_option.Contains("-KDE_ALTKERNEL")) efficiency_histogram = _tFile->Get<TH2D>((TString) "hEff_" + to_string(slice) + "_R");
        if (_option.Contains("KDE_BS-")) {  
            TString _Number = ((std::string)_option.Data()).substr(_option.Length()-2, ((std::string)_option.Data()).find("-"));
            if (_Number.Contains("-")) _Number = (TString)((std::string)_Number.Data()).substr(1,2);
            cout << "Loading BS slice " << _Number << endl;
            efficiency_histogram = _tFile->Get<TH2D>((TString) "hEff_"  + to_string(slice) + "_BS" + _Number);
        }
        if (! efficiency_histogram ) {
                MessageSvc::Error("GetWeightMapsPID", _fileName, " slice ", to_string(slice), " does not exist", _option, "EXIT_FAILURE");
                return make_pair(nullptr,efficiency_histograms);
        }else {
                TH2D * _map = (TH2D *) efficiency_histogram->Clone((TString) efficiency_histogram->GetName() + "_copy");
                MessageSvc::Info("GetWeightMapsPID", (TString) _map->GetName());
                _map->SetName(_option + "-nTracks" + to_string(slice) );
                _map->SetDirectory(0);
                MessageSvc::Info("GetWeightMapsPID", _map);
                CheckHistogram(_map, _option);
                MessageSvc::Line();
                delete efficiency_histogram;
                efficiency_histograms.push_back(_map);
        }
    }

    maps = make_pair(_nTracksHisto, efficiency_histograms);
    IOSvc::CloseFile(_tFile);

    //Check maps carefully
    // cout << "Check maps:" << endl;
    // maps.first->Print();
    // for(auto const& map: maps.second) {
    //  map->Print();
    // }
    CheckPIDMapsForNullPtrs(maps, _option, true);
    
    return maps;
}

pair< TString, TString > WeightHolder::GetStrMapL0(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetStrMapL0", _option);

    TString _optionDir = _option;   // avoid problems with GetWeightDir
    TString _weightDir = IOSvc::GetWeightDir(_optionDir.ReplaceAll("MC", ""));

    Analysis ana    = m_configHolder.GetAna();
    Year     year   = m_configHolder.GetYear();
    TString  sample = m_configHolder.GetSample();

    TString _channel     = "";
    TString _triggerCat  = "";
    TString _triggerConf = "";
    TString _BINopt = "coarse";

    if (_option.Contains(to_string(Trigger::L0I)))      _triggerCat = to_string(Trigger::L0I);
    else if (_option.Contains(to_string(Trigger::L0L))) _triggerCat = to_string(Trigger::L0L);
    else if (_option.Contains(to_string(Trigger::L0H))) _triggerCat = to_string(Trigger::L0H);
    else
        MessageSvc::Error("GetStrMapL0", (TString) "Invalid trigger", _option, "EXIT_FAILURE");        
    if (_triggerCat == "L0I"){
        if( SettingDef::Weight::L0I_EToE && ana == Analysis::EE){
            MessageSvc::Warning("Maps for electrons wL0I Taken from EE!!!");
            ana = Analysis::EE;
        }else{
            MessageSvc::Warning("Maps for electrons wL0I Taken from Muons!!!");
            ana = Analysis::MM; //Grab always muon maps 
        }
    }
    if (_triggerCat == "L0H")
        ana = Analysis::MM;
    if (_option.Contains("B0"))
        _channel = "Bd2KstJPs" + to_string(ana);
    else if (_option.Contains("Bp"))
        _channel = "Bu2KJPs" + to_string(ana);
    else if (_option.Contains("Bs"))
        _channel = "Bs2PhiJPs" + to_string(ana);

    if (sample.Contains("vL0")) _channel += "vL0";

    if (_option.Contains("exclOverIncl"))
        _triggerConf = "ExclOverIncl";
    else if (_option.Contains("incl"))
        _triggerConf = "Incl";
    else if (_option.Contains("excl"))
        _triggerConf = "Excl";
    else if (_option.Contains("comb"))
        _triggerConf = "Incl";
    else
        MessageSvc::Error("GetStrMapL0", (TString) "Invalid trigger configuration", _option, "EXIT_FAILURE");

    // if (_option.Contains("interp") || _option.Contains("fit")) _THopt = "no";
    if (_option.Contains("fine")) _BINopt = "fine";
    if (_option.Contains("fit"))  _BINopt = "fitEff";

    // TString _L0opt = _TOSopt + "_" + _TISopt + "_" + _MC1opt + "_" + _THopt + "_" + _L0LVARopt + "_" + _L0IVARopt + "_" + _BINopt;
    TString _L0opt = _BINopt;
    if( _option.Contains("Brem0")) _L0opt+= "_Brem0";
    if( _option.Contains("Brem1")) _L0opt+= "_Brem1";

    TString _subDir = _channel + "_" + to_string(year) + "_" + _triggerCat;

    TString _fileName = "wL0" + _triggerConf + "_" + _subDir;
    _fileName         = _weightDir + "/" + _subDir + "_" + _L0opt + "/" + _fileName + ".root";

    TString _histoName = "h_wL0_" + _subDir;

    if (_option.Contains("effCL")) _histoName += "_effCL";
    if (_option.Contains("effMC")) _histoName += "_effMC";

    pair< TString, TString > _names = {_fileName, _histoName};

    return _names;
}

TString WeightHolder::GetStrFileL0(TString _option) {
    TString _l0Ver = SettingDef::Weight::l0Ver;
    if (_option.Contains("fit")) SettingDef::Weight::l0Ver += "_FIT";
    pair< TString, TString > _names = GetStrMapL0(_option);
    SettingDef::Weight::l0Ver       = _l0Ver;
    return _names.first;
}

TH1D * WeightHolder::GetWeightMapL01D(TString _option) {
    pair< TString, TString > _names     = GetStrMapL0(_option);
    TString                  _fileName  = _names.first;
    TString                  _histoName = _names.second;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapL01D", _histoName);

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapL01D", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapL01D", _fileName, "does not exist", _option);
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH1D *  _histo = nullptr;
    TIter   _keyList(gDirectory->GetListOfKeys());
    TKey *  _key;
    while ((_key = (TKey *) _keyList())) {
        if (((TString) _key->ReadObj()->GetName()).Contains(_histoName)) _histo = (TH1D *) _key->ReadObj();
    }

    if (!_histo) {
        MessageSvc::Error("GetWeightMapL01D", (TString) "Map", _histoName, "does no exist");
        _tFile->ls();
        MessageSvc::Error("GetWeightMapL01D", (TString) "Map", _histoName, "does no exist", "EXIT_FAILURE");
        return nullptr;
    }

    TH1D * _map = (TH1D *) CopyHist(_histo);
    MessageSvc::Info("GetWeightMapL01D", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightMapL01D", _map);
    CheckHistogram(_map, _option);
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

vector<TH2D>  WeightHolder::GetWeightMapL02DBS(TString _option) {
    TString _l0Ver = SettingDef::Weight::l0Ver;
    if (_option.Contains("fit")) SettingDef::Weight::l0Ver += "_FIT";
    pair< TString, TString > _names     = GetStrMapL0(_option);
    TString                  _fileName  = _names.first;
    TString                  _histoName = _names.second;
    SettingDef::Weight::l0Ver = _l0Ver;
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapL02DBS", _histoName);
    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)){
        MessageSvc::Error("GetWeightMapL02DBS", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapL02DBS", _fileName, "does not exist", _option);
        return {};
    }
    vector<TH2D> _bsHistos; 
    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    for( int bsIdx =0 ; bsIdx < WeightDefRX::nBS; ++bsIdx){
        TH2D * _histo = nullptr;
        TString _toGet =  Form("bs%i/%s", bsIdx, _histoName.Data());
        _histo = (TH2D*)_tFile->Get(_toGet);
        if(_histo==nullptr){ MessageSvc::Error("INVALID GET BOOTSTRAPPED HISTO", _fileName+":"+_toGet,"EXIT_FAILURE");}
        auto * _map = (TH2D*)CopyHist(_histo);
        _map->SetName(Form("%s_BS%i", _option.Data(), bsIdx));
        _map->SetDirectory(0);
        CheckHistogram(_map, _option);
        _bsHistos.push_back( *_map);
        delete _histo;  
    }
    MessageSvc::Info("GetWeightMapL02DBS nLoaded", _bsHistos.size());
    IOSvc::CloseFile(_tFile);
    return _bsHistos;
}

TH2D * WeightHolder::GetWeightMapL02D(TString _option) {
    TString _l0Ver = SettingDef::Weight::l0Ver;
    if (_option.Contains("fit")) SettingDef::Weight::l0Ver += "_FIT";

    pair< TString, TString > _names     = GetStrMapL0(_option);
    TString                  _fileName  = _names.first;
    TString                  _histoName = _names.second;

    SettingDef::Weight::l0Ver = _l0Ver;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapL02D", _histoName);

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)){
        MessageSvc::Error("GetWeightMapL02D", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapL02D", _fileName, "does not exist", _option);
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH2D *  _histo = nullptr;
    TIter   _keyList(gDirectory->GetListOfKeys());
    TKey *  _key;
    while ((_key = (TKey *) _keyList())) {
        if (((TString) _key->ReadObj()->GetName()) == _histoName) _histo = (TH2D *) _key->ReadObj();
    }

    if (!_histo) {
        MessageSvc::Error("GetWeightMapL02D", (TString) "Map", _histoName, "does no exist");
        _tFile->ls();
        MessageSvc::Error("GetWeightMapL02D", (TString) "Map", _histoName, "does no exist", "EXIT_FAILURE");
        return nullptr;
    }

    TH2D * _map = (TH2D *) CopyHist(_histo);
    MessageSvc::Info("GetWeightMapL02D", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightMapL02D", _map);
    CheckHistogram(_map, _option);
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

pair< TString, TString > WeightHolder::GetStrMapRW1D(TString _option) {
    MessageSvc::Info(Color::Cyan, "GetStrMapRW1D", _option);
    // Init maps
    TString _weightDir = IOSvc::GetWeightDir("MC");
    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();

    TString _fileName = _weightDir + "/weights1D_{PRJ}_{ANA}_{YEAR}_L0L.root";
    
    TString _head = "";
    if (_option.Contains("B0")) {
        _head = "B0";
        _fileName = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RKst));
    } else if (_option.Contains("Bp")) {
        _head = "Bp";
        _fileName = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RK));
    } else if (_option.Contains("Bs")) { 
        _head = "Bs";
        _fileName   = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RPhi));
    } else {
        MessageSvc::Error("GetStrMapRW1D", "invalid project", "EXIT_FAILURE");
    }
    
    _fileName = _fileName.ReplaceAll("{ANA}", to_string(ana));
    _fileName = _fileName.ReplaceAll("{YEAR}", to_string(year));

    TString _mapKeyName = "{branch}_histW";
    if( _option.Contains("nTracks")) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", "nTracksD");
    if( _option.Contains(TString::Format("%s_PT", _head.Data()))) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", TString::Format("%s_PT", _head.Data()));
    if( _option.Contains(TString::Format("%s_ENDVERTEX_CHI2_NDOF", _head.Data()))) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", TString::Format("%s_ENDVERTEX_CHI2_NDOF", _head.Data()));
    if( _option.Contains("K_PT")) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", "K_PT");
    if( _option.Contains("M1_PT")) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", "M1_PT");
    if( _option.Contains("M2_PT")) _mapKeyName = _mapKeyName.ReplaceAll("{branch}", "M2_PT");
    _mapKeyName = _mapKeyName.ReplaceAll("{ANA}", to_string(ana));
    
    pair< TString, TString > _names = {_fileName, _mapKeyName};
    return _names;
}


TH1D * WeightHolder::GetWeightMapRW1D(TString _option) {
    pair< TString, TString > _names     = GetStrMapRW1D(_option);
    TString                  _fileName  = _names.first;
    TString                  _histoName = _names.second;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapRW1D", _histoName);

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapRW1D", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapRW1D", _fileName, "does not exist", _option);
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH1D *  _histo = (TH1D *) _tFile->Get(_histoName);
    if (!_histo) {
        MessageSvc::Error("GetWeightMapRW1D", (TString) "Map", _histoName, "does no exist");
        _tFile->ls();
        MessageSvc::Error("GetWeightMapRW1D", (TString) "Map", _histoName, "does no exist", "EXIT_FAILURE");
        return nullptr;
    }

    TH1D * _map = (TH1D *) CopyHist(_histo);
    MessageSvc::Info("GetWeightMapRW1D", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightMapRW1D", _map);
    CheckHistogram(_map, _option+"-q");
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

pair< TString, TString > WeightHolder::GetStrMapHLT(TString _option) {
    //_option + various BITS...HLT_{B0/Bp}_{L0I_inc/L0L_exc/L0L_inc}_{i}
    MessageSvc::Info(Color::Cyan, "GetStrMapHLT", _option);
    // Init maps
    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::HLT);
    // should be returning /eos/..../weights/v8/hlt/vXXX/
    // Tuples EE goes to EE , MM to MM
    // If "Bp" pick HLT from RK fits
    // If "B0" pick HLT from RKst fits
    // Triggers possibilities are {L0I_inc, L0L_exc, L0L_inc}
    // Year of this Weight Holder maps to  correct one.
    Analysis ana  = m_configHolder.GetAna();
    Year     year = m_configHolder.GetYear();
    TString _triggerBit = "";
    if (_option.Contains(to_string(Trigger::L0I))) {
        _triggerBit = to_string(Trigger::L0I);
    } else if (_option.Contains(to_string(Trigger::L0L))) {
        _triggerBit = to_string(Trigger::L0L);
    } else {
        MessageSvc::Error("Trigger from option not contained", "", "EXIT_FAILURE");
    }
    if (_option.Contains("incl")) {
        _triggerBit += "_incl";
    } else if (_option.Contains("excl")) {
        _triggerBit += "_excl";
    } else {
        MessageSvc::Error("Trigger from option not contained", "", "EXIT_FAILURE");
    }

    map< TString, TString > _triggerMaps = {{to_string(Trigger::L0I) + "_incl", to_string(Trigger::L0I) + "_exclusive"},    //
                                            {to_string(Trigger::L0L) + "_incl", to_string(Trigger::L0L) + "_exclusive2"},   //
                                            {to_string(Trigger::L0L) + "_excl", to_string(Trigger::L0L) + "_exclusive"}};
    
    _weightDir = _weightDir + "/{PRJ}_{ANA}_{YEAR}_{TRG}";
    if(_option.Contains("-nTracks-1D")){
        _weightDir = _weightDir+"/nTracks/";
    }else if(_option.Contains("-B_PT-1D")){
        _weightDir = _weightDir+"/B_PT/";
    }else if(_option.Contains("-B_PT_nTracks-2D")){
        _weightDir = _weightDir+"/B_PT_nTracks/";
    }else if( _option.Contains("-B_ETA-1D")){
        _weightDir = _weightDir+"/B_ETA/";
    }

    TString _fileName = _weightDir + "/wHLT_{PRJ}_{ANA}_{YEAR}_{TRG}.root";
    // maps are produced by makeMaps.py script in hltcorrections module ( fix naming there to change this)
    TString _mapKeyName = "HLTmap_{ANA}";

    if( _option.Contains("effMC")) _mapKeyName = "MCRatio";
    if( _option.Contains("effCL")) _mapKeyName = "CLRatio";
    
    // Default is to use B0_fit ( prior L0 maps to make maps on RKst ), Bp_fit ( prior L0 maps to make maps on RK ), Bs_fit ( prior L0 maps to make maps on RK )
    // make sure to not replace names when project == RPhi
    if (_option.Contains("B0") && m_configHolder.GetProject() != Prj::RPhi) {
        _fileName = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RKst));
    } else if (_option.Contains("Bp") && m_configHolder.GetProject() != Prj::RPhi) {
        _fileName = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RK));
    } else if (_option.Contains("Bs") || m_configHolder.GetProject() == Prj::RPhi) {            // this is the case RPhi should go into 
        _fileName   = _fileName.ReplaceAll("{PRJ}", to_string(Prj::RPhi));
    }
    _fileName   = _fileName.ReplaceAll("{ANA}", to_string(ana));
    _mapKeyName = _mapKeyName.ReplaceAll("{ANA}", to_string(ana));
    _fileName   = _fileName.ReplaceAll("{YEAR}", to_string(year));
    _fileName   = _fileName.ReplaceAll("{TRG}", _triggerMaps.at(_triggerBit));

    pair< TString, TString > _names = {_fileName, _mapKeyName};
    return _names;
}


TH1D * WeightHolder::GetWeightMapHLT(TString _option) {
    pair< TString, TString > _names     = GetStrMapHLT(_option);
    TString                  _fileName  = _names.first;
    TString                  _histoName = _names.second;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapHLT", _histoName);

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapHLT", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetWeightMapHLT", _fileName, "does not exist", _option);
        return nullptr;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TH1D *  _histo = (TH1D *) _tFile->Get(_histoName);
    if (!_histo) {
        MessageSvc::Error("GetWeightMapHLT", (TString) "Map", _histoName, "does no exist");
        _tFile->ls();
        MessageSvc::Error("GetWeightMapHLT", (TString) "Map", _histoName, "does no exist", "EXIT_FAILURE");
        return nullptr;
    }

    TH1D * _map = (TH1D *) CopyHist(_histo);
    MessageSvc::Info("GetWeightMapHLT", (TString) _map->GetName());
    _map->SetName(_option);
    _map->SetDirectory(0);

    MessageSvc::Info("GetWeightMapHLT", _map);
    CheckHistogram(_map, _option+"-q");
    MessageSvc::Line();

    delete _histo;
    IOSvc::CloseFile(_tFile);

    return _map;
}

TString WeightHolder::GetVeloPars(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetVeloPars", _option);

    TString _weightDir = IOSvc::GetDataDir("velo");

    Year year = m_configHolder.GetYear();

    TString _fileName = _weightDir + "/";
    if ((year == Year::Y2011) || (year == Year::Y2012) || (year == Year::Run1)) _fileName += "CL12";
    if ((year == Year::Y2015) || (year == Year::Y2016) || (year == Year::Run2p1)) _fileName += "HE16";
    if ((year == Year::Y2017) || (year == Year::Y2018) || (year == Year::Run2p2)) _fileName += "HE16";
    _fileName += "." + _option;
    _fileName += ".root";

    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetVeloPars", _fileName, "does not exist", _option, "EXIT_FAILURE");
        MessageSvc::Warning("GetVeloPars", _fileName, "does not exist", _option);
        return TString("");
    }

    MessageSvc::Info("GetVeloPars", _fileName);
    MessageSvc::Line();
    return _fileName;
}

vector< pair< TH1D *, vector < TH2D * > > > WeightHolder::GetWeightMaps(TString _type, TString _option, int _nCopies) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMaps", _type, _option, to_string(_nCopies));

    vector< pair< TH1D *, vector < TH2D * > > > _maps_BS; //Bootstrapped sets of maps with nTracks slices
    pair< TH1D *, vector < TH2D * > >           _map; //One single set of maps with nTracks slices
    if (_type == "PID") {
        if(_option.Contains("brem"))
            _map = GetWeightMapsPID_fac(_option);
        else
            _map = GetWeightMapsPID(_option);
    }

    for (int i = 0; i < _nCopies; ++i) {
        pair<TH1D *, vector < TH2D * >> _map_BS; //One single set of bootstrapped map with nTracks slices
        _map_BS.first = _map.first;
        int nTracksSlices = -1;
        if(_option.Contains("brem")) {
            nTracksSlices = WeightDefRX::N_ntracks_fac;
        }
        else {
            nTracksSlices = WeightDefRX::N_ntracks;
        }

        for(double ntrack_slice = 0; ntrack_slice < nTracksSlices; ++ntrack_slice) {
            TH2D * _weightMap = _map.second.at(ntrack_slice);
            _map_BS.second.push_back((TH2D *) RandomizeAllEntries((TH1D *) _weightMap, ((TString) _weightMap->GetName()).Hash() + i + 1));
            _map_BS.second.at(ntrack_slice)->SetName(((TString) _map_BS.second.at(ntrack_slice)->GetName()) + "_" + to_string(i));
            CheckHistogram(_map_BS.second.at(ntrack_slice), _option + "-q");
        }
        _maps_BS.push_back(_map_BS);
    }
    return _maps_BS;
}

vector< pair< TH1D *, vector < TH2D * > > > WeightHolder::GetWeightMapsBSKDE(TString _type, TString _option, int _nBS) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMaps", _type, _option, to_string(_nBS));

    auto saveOption = _option;

    vector< pair< TH1D *, vector < TH2D * > > > _maps_BS;  // Bootstrapped sets of maps with nTracks slices
    TString _fileName;
    // Get FileName
    if (_type == "PID") {
        if(_option.Contains("brem")) { _fileName = GetWeightMapsPID_fac_Name(_option); }
        else {                         _fileName = GetWeightMapsPID_Name(_option); }
    }
    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)) {
        MessageSvc::Error("GetWeightMapsBSKDE", _fileName, "does not exist", _option, "EXIT_FAILURE");
        return _maps_BS;
    }
    
    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);

    TH1D *nTracksSlice = _tFile->Get<TH1D>((TString) "nTracks_edges");
    if (! nTracksSlice ) {
        MessageSvc::Error("GetWeightMapsBSKDE", _fileName, " nTracks map does not exist", _option, "EXIT_FAILURE");
        return _maps_BS;
    }
    else {
        nTracksSlice->SetDirectory(0);
    }
    // more adaptive as alex implementation as it allows to vary nTracks scheme freely
    int nTracksSlices = nTracksSlice->GetNbinsX();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsBSKDE nTracks Bins: ", to_string(nTracksSlices));

    for (int i = 0; i < _nBS; ++i) { 
        _option = saveOption;
        MessageSvc::Info(Color::Cyan, "Loading BS index ", to_string(i));
        //Holder for BS index 
        pair<TH1D *, vector < TH2D * >> _map_BS; 
        _map_BS.first = nTracksSlice;
        for(int ntrack_slice = 0; ntrack_slice < nTracksSlices; ++ntrack_slice) {
            // load all nTracks regions
            _map_BS.second.push_back(_tFile->Get<TH2D>((TString) "hEff_"  + to_string(ntrack_slice) + "_BS" + to_string(i)));
            if (! _map_BS.second.at(ntrack_slice) ) {
                MessageSvc::Error("GetWeightMapsPID", _fileName, " nTracks slices incomplete", _option+"_BS"+to_string(i)+"_nTracks"+to_string(ntrack_slice), "EXIT_FAILURE");
                return _maps_BS;
            }
            else {
                _map_BS.second.at(ntrack_slice)->SetDirectory(0);
            }
            _map_BS.second.at(ntrack_slice)->SetName(((TString) _map_BS.second.at(ntrack_slice)->GetName()) + "_" + to_string(i));
            CheckHistogram(_map_BS.second.at(ntrack_slice), _option + "-q");
        }
        _maps_BS.push_back(_map_BS);
    }
    IOSvc::CloseFile(_tFile);
    return _maps_BS;
}

TString WeightHolder::GetWeightMapsPID_Name(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsPID", _option);

    TString _pidVer = SettingDef::Weight::pidVer;
    if (_option.Contains("KDE")) SettingDef::Weight::pidVer += "_KDE";

    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::PID);
    SettingDef::Weight::pidVer = _pidVer;
    Prj      prj      = m_configHolder.GetProject();
    Year     year     = m_configHolder.GetYear();
    Polarity polarity = m_configHolder.GetPolarity();

    TString _fileName;
    _fileName = _weightDir + "/" + to_string(prj) + "/weightMap_";

    // particle real ID
    TString _particle;
    if (_option.Contains("PID_K"))       _particle = "K";
    else if (_option.Contains("PID_Pi")) _particle = "Pi";
    else if (_option.Contains("PID_M"))  _particle = "Mu";
    else if (_option.Contains("PID_E"))  _particle = "e";
    else if (_option.Contains("PID_P"))  _particle = "P";

    // particle mis ID
    TString _misID;
    if (!_option.Contains("MID"))        _misID = _particle;
    else if (_option.Contains("MID_K"))  _misID = "K";
    else if (_option.Contains("MID_Pi")) _misID = "Pi";
    else if (_option.Contains("MID_M"))  _misID = "Mu";
    else if (_option.Contains("MID_E"))  _misID = "e";
    else if (_option.Contains("MID_P"))  _misID = "P";
   
    _fileName += _particle + "_" + _misID + "_20" + to_string(year) + ".root";
    
    return _fileName;
}
TString WeightHolder::GetWeightMapsPID_fac_Name(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsPID Fit and Count", _option);

    TString _pidVer = SettingDef::Weight::pidVer;
    TString _weightDir = IOSvc::GetWeightDir(WeightDefRX::ID::PID);
    Prj      prj      = m_configHolder.GetProject();
    Year     year     = m_configHolder.GetYear();
    Polarity polarity = m_configHolder.GetPolarity();
    TString _fileName;
    _fileName = _weightDir + "/" + to_string(prj) + "/Fit_";

    // particle mis ID
    TString _misID;
    if (!_option.Contains("MID"))        _misID = "ID";
    else if (_option.Contains("MID_K"))  _misID = "MISIDEK";
    else if (_option.Contains("MID_Pi")) _misID = "MISIDEPI";

    MessageSvc::Info(Color::Cyan, "Option: ", _option);
    if (_option.Contains("brem0"))      _fileName += "PolBoth_Year" + to_string(year) + "_nBrem0_" + _misID + ".root";
    else if (_option.Contains("brem1")) _fileName += "PolBoth_Year" + to_string(year) + "_nBrem1_" + _misID + ".root";

    return _fileName;
}

vector< pair< TH1D *, vector < TH2D * > > > WeightHolder::GetWeightMapsBS(TString _type, TString _option, int _nBS) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetWeightMaps", _type, _option, to_string(_nBS));
    auto saveOption = _option;

    vector< pair< TH1D *, vector < TH2D * > > > _maps_BS;  // Bootstrapped sets of maps with nTracks slices
    TH2D *                                      _mapSlice; // empty holder for the bootstrap index
    TString _fileName;
    // Get FileName
    if (_type == "PID") {
        if(_option.Contains("brem")) {
            _fileName = GetWeightMapsPID_fac_Name(_option);
        }else {
            _fileName = GetWeightMapsPID_Name(_option);
        }
    }
    _fileName = IOSvc::XRootDFileName(_fileName);
    if (!IOSvc::ExistFile(_fileName)){
        MessageSvc::Error("GetWeightMapsBS", _fileName, "does not exist", _option, "EXIT_FAILURE");
        return _maps_BS;
    }

    TFile * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
    TString _mapName = (_option.Contains("WeightMapPID") && !_option.Contains("MID"))? "hWeight_" : "hEff_";
    MessageSvc::Info(Color::Cyan, "GetWeightMapsBS map Name: ", _mapName);

    TH1D *nTracksSlice = _tFile->Get<TH1D>((TString) "nTracks_edges");
    if (! nTracksSlice ) {
        MessageSvc::Error("GetWeightMapsBS", _fileName, " nTracks map does not exist", _option, "EXIT_FAILURE");
        return _maps_BS;
    }else {
        nTracksSlice->SetDirectory(0);
    }
    // more adaptive than alex method, as it allows to vary nTracks scheme!
    int nTracksSlices = nTracksSlice->GetNbinsX();
    MessageSvc::Info(Color::Cyan, "GetWeightMapsBS nTracks Bins: ", to_string(nTracksSlices));

    for (int i = 0; i < _nBS; ++i) { 
        _option = saveOption;
        MessageSvc::Info(Color::Cyan, "Loading BS index ", to_string(i));
        //Holder for BS index 
        pair<TH1D *, vector < TH2D * >> _map_BS; 
        _map_BS.first = nTracksSlice;
        for(int ntrack_slice = 0; ntrack_slice < nTracksSlices; ++ntrack_slice) {
            // load all hEff histograms 
            _map_BS.second.push_back(_tFile->Get<TH2D>((TString) _mapName + to_string(ntrack_slice) + "_BS" + to_string(i)));
            if (! _map_BS.second.at(ntrack_slice) ) {
                MessageSvc::Error("GetWeightMapsPID", _fileName, " nTracks slices incomplete", _option+"_BS"+to_string(i)+"_nTracks"+to_string(ntrack_slice), "EXIT_FAILURE");
                return _maps_BS;
            }
            else {
                _map_BS.second.at(ntrack_slice)->SetDirectory(0);
            }
            _map_BS.second.at(ntrack_slice)->SetName(((TString) _map_BS.second.at(ntrack_slice)->GetName()) + "_" + to_string(i));
	    // dont set vals > 1 to 1 for WeightMap!
            if (!_option.Contains("WeightMapPID") || _option.Contains("MID")) {
		CheckHistogram(_map_BS.second.at(ntrack_slice), _option + "-q");
	    }
        }
        _maps_BS.push_back(_map_BS);
    }
    IOSvc::CloseFile(_tFile);

    return _maps_BS;
}
