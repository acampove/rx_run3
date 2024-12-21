#include "TupleHolder.hpp"

#include "SettingDef.hpp"

#include "core.h"
#include "TKey.h"
#include "fmt_ostream.h"
#include "vec_extends.h"

ClassImp(TupleHolder)

    TupleHolder::TupleHolder()
    : m_configHolder() {
    if (SettingDef::debug.Contains("TH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleHolder", (TString) "Default");
    m_tupleOption = SettingDef::Tuple::option;
    m_tupleName   = SettingDef::Tuple::tupleName;
    m_fileName    = SettingDef::Tuple::fileName;
    Check();
}

TupleHolder::TupleHolder(const ConfigHolder & _configHolder, TString _tupleOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("TH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleHolder", (TString) "ConfigHolder");
    m_tupleOption = _tupleOption;
    m_tupleName   = SettingDef::Tuple::tupleName;
    m_fileName    = SettingDef::Tuple::fileName;
    Check();
}

TupleHolder::TupleHolder(const TupleHolder & _tupleHolder)
    : m_configHolder(_tupleHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("TH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleHolder", (TString) "TupleHolder");
    m_tupleOption   = _tupleHolder.Option();
    m_tupleDir      = _tupleHolder.TupleDir();
    m_tupleName     = _tupleHolder.TupleName();
    m_fileName      = SettingDef::Tuple::fileName;
    m_tupleReader   = _tupleHolder.GetTupleReader();
    m_branches      = _tupleHolder.Branches();
    m_aliases       = _tupleHolder.Aliases();
    m_isInitialized = _tupleHolder.IsInitialized();
    Check();
}

TupleHolder::TupleHolder(const ConfigHolder & _configHolder, TString _fileName, TString _tupleName, TString _tupleOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("TH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleHolder", (TString) "TString");
    m_tupleOption = _tupleOption;
    m_tupleName   = SettingDef::Tuple::tupleName;
    m_fileName    = SettingDef::Tuple::fileName;
    Check();    
    Init(true, _fileName, _tupleName);
}

ostream & operator<<(ostream & os, const TupleHolder & _tupleHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "TupleHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "tupleOption", _tupleHolder.Option());
    MessageSvc::Print((ostream &) os, "tupleDir", _tupleHolder.TupleDir());
    MessageSvc::Print((ostream &) os, "tupleName", _tupleHolder.TupleName());
    MessageSvc::Print((ostream &) os, "tupleBranches", to_string(_tupleHolder.Branches().size()));
    MessageSvc::Print((ostream &) os, "tupleAliases", to_string(_tupleHolder.Aliases().size()));
    os << _tupleHolder.GetTupleReader();
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

TupleHolder TupleHolder::GetMCDecayTupleHolder() {
    MessageSvc::Info("TupleHolder", (TString) "GetMCDecayTupleHolder");
    TString _tupleName           = SettingDef::Tuple::tupleName;
    SettingDef::Tuple::tupleName = "MCDT";
    TupleHolder _tupleHolder     = TupleHolder(this->GetConfigHolder(), this->Option());
    SettingDef::Tuple::tupleName = _tupleName;
    return _tupleHolder;
}

bool TupleHolder::Check() {
    if( m_tupleOption.Contains("pro[")){
        MessageSvc::Warning("TupleHolder check, skipped, pro[XXX] specificed");
        return false; 
    }
    if( m_tupleOption.Contains("cre[")){
        MessageSvc::Warning("TupleHolder check, skipped, cre[XXX] specificed");
        return false; 
    }    
    for (auto _opt : TokenizeString(m_tupleOption, SettingDef::separator)) {        
        if (!CheckVectorContains(SettingDef::AllowedConf::TupleOptions, _opt)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("TupleHolder", (TString) "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::TupleOptions", "EXIT_FAILURE");
        }
    }
    return false;
}

void TupleHolder::Init(bool _force, TString _fileName, TString _tupleName) {
    if( SettingDef::Tuple::useURLS ){
      MessageSvc::Warning("Change eos.list to urls.list, prototyping of updates to ganga module");
      m_fileList = "urls.list";
    }
  
    if( SettingDef::Tuple::noInit == true){
        MessageSvc::Warning( "TupleHolder", (TString)"Initialize", "Tuple::noInit==True, skipping Tuple loading");
        m_isInitialized = true;
        return; //Lazy initialization
    }
    if (_force || !IsInitialized()) {
        MessageSvc::Info(Color::Cyan, "TupleHolder", (TString) "Initialize ...");

        PrintInline();

        if (SettingDef::Tuple::branches) m_branches = GetBranches("INC");
        if (SettingDef::Tuple::aliases) m_aliases = GetAliases("INC");

        if (m_tupleOption.Contains("tmp")) {
            TString _sample = "JPs";
            TString fileName = "tmp" + _sample + SettingDef::separator + m_configHolder.GetKey() + ".root";
            if(  SettingDef::IO::outDir != ""){
                _fileName       = SettingDef::IO::outDir.EndsWith("/")?  SettingDef::IO::outDir+ fileName :  SettingDef::IO::outDir +"/"+ _fileName;
            }
            m_fileName      = _fileName;
            m_tupleName     = ((TString) m_configHolder.GetKey("short")).ReplaceAll(SettingDef::separator, "_");
        }

        if (_fileName != "") {
            m_fileName = _fileName;
            MessageSvc::Info("TupleHolder", (TString) "FileName  =", m_fileName);
        }
        if (_tupleName != "") {
            m_tupleName = _tupleName;
            MessageSvc::Info("TupleHolder", (TString) "TupleName =", m_tupleName);
        }

        CreateTupleDir(m_fileName);
        CreateTupleName();
        CreateTupleReader(m_fileName, m_tupleName);

        if (m_branches.size() == 0) {
            if (GetTuple()->GetListOfBranches() != nullptr) {
                for (auto * _branch : *GetTuple()->GetListOfBranches()) { m_branches.push_back(_branch->GetName()); }
            }
        }
    }
    return;
}

void TupleHolder::Close() {
    MessageSvc::Info(Color::Cyan, "TupleHolder", (TString) "Close ...");
    m_tupleReader.Close();
    m_isInitialized = false;
    return;
}

void TupleHolder::Reset() {
    MessageSvc::Info(Color::Cyan, "TupleHolder", (TString) "Reset ...");
    m_tupleDir = "";
    m_fileName = "";
    Close();
    return;
}

void TupleHolder::CreateTupleName() {
    if (m_debug) MessageSvc::Debug("CreateTupleName", m_tupleName);

    // TupleGanga
    if (m_tupleOption.Contains("gng")) {
        if (m_tupleName == "DT") m_tupleName = "DecayTreeTuple/DecayTree";
        if (m_tupleName == "MCT") m_tupleName = "MCDecayTreeTuple/MCDecayTree";
        if (m_tupleName == "ET") m_tupleName = "EventTuple/EventTuple";
        m_tupleName.ReplaceAll("/", to_string(m_configHolder.GetAna()) + "/");
	if (m_configHolder.GetSample().Contains("SSHH")) m_tupleName.ReplaceAll("/", "_SSHH/"); // SS Hadrons location
        if (m_tupleName == "LT") m_tupleName = "GetIntegratedLuminosity/LumiTuple";
    }
    // TupleProcess
    if (m_tupleOption.Contains("pro")) {
        if (m_tupleDir.Contains("_PQ") && (m_configHolder.GetSample().Contains("Lb2pKJPs") || m_configHolder.GetSample().Contains("Lb2pKPsi"))) {	
            if (m_tupleName == "DT") m_tupleName = "DecayTreeTuple/DecayTree";
            if (m_tupleName == "MCT") m_tupleName = "MCDecayTreeTuple/MCDecayTree";
            if (m_tupleName == "ET") m_tupleName = "EventTuple/EventTuple";
            m_tupleName.ReplaceAll("/", to_string(m_configHolder.GetAna()) + "/");
        } else {
            if (m_tupleName == "DT") m_tupleName = SettingDef::Tuple::DT;
            if (m_tupleName == "MCT") m_tupleName = SettingDef::Tuple::MCT;
            if (m_tupleName == "ET") m_tupleName = SettingDef::Tuple::ET;
            if (m_tupleName == "LT") m_tupleName = SettingDef::Tuple::LT;
        }      
        if (m_configHolder.GetProject() == Prj::RL) {
            switch (m_configHolder.GetAna()) {
                case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple_CutTree"; break;
                case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple_CutTree"; break;
                case Analysis::ME:
                            m_tupleName = "Lb2LemuTuple_CutTree";
                            if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) m_tupleName = "Lb2JpsiL_mmTuple_CutTree";
                            break;
                default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
            }
        }
    }
    // TupleCreate
    if (m_tupleOption.Contains("cre")) { m_tupleName = m_configHolder.GetTupleName(m_tupleOption); }
    // TupleSPlot
    if (m_tupleOption.Contains("spl")) {
        if (m_tupleOption.Contains("splp")) {
            // SPLOT FROM TUPLEPROCESS
            m_tupleName = SettingDef::Tuple::DT;
        } else {
            // SPLOT FROM TUPLECREATE
            m_tupleName = m_configHolder.GetTupleName(m_tupleOption);
        }
    }
    // TupleRapidSim
    if (m_tupleOption.Contains("rap")) { m_tupleName = SettingDef::Tuple::RST; }
    
    // RL & RKS
    if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) {
      // WG PRODUCTION
        if (m_tupleOption.Contains("gng")) {
            if (m_configHolder.GetProject() == Prj::RL) {
                switch (m_configHolder.GetAna()) {
                case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple/DecayTree"; break;
                case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple/DecayTree"; break;
                case Analysis::ME:
                m_tupleName = "Lb2LemuTuple/DecayTree";
                                if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) m_tupleName = "Lb2JpsiL_mmTuple/DecayTree";
                                break;
                default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
                }
            }
            if (m_configHolder.GetProject() == Prj::RKS) {
                switch (m_configHolder.GetAna()) {
                    case Analysis::MM: m_tupleName = "Bd2JpsiKs_mmTuple/DecayTree"; break;
                    default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
                }
            }
        }
        // PROC TUPLES
        if (m_tupleOption.Contains("pro")) {            
            if (m_configHolder.GetProject() == Prj::RL) {
                switch (m_configHolder.GetAna()) {
                    case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple_CutTree"; break;
                    case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple_CutTree"; break;
                    case Analysis::ME:
                        m_tupleName = "Lb2LemuTuple_CutTree";
                        if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) m_tupleName = "Lb2JpsiL_mmTuple_CutTree";
                        break;
                    default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
                }
            }
            if (m_configHolder.GetProject() == Prj::RKS) {
                switch (m_configHolder.GetAna()) {
                    case Analysis::MM: m_tupleName = "Bd2JpsiKs_mmTuple_CutTree"; break;
                    default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
                }
            }
        }
    }

    if (m_debug) MessageSvc::Debug("CreateTupleName", m_tupleName);
    return;
}

void TupleHolder::CreateTupleDir(TString _fileName) {
    if (m_debug) MessageSvc::Debug("CreateTupleDir", _fileName, m_tupleOption);
    if (_fileName == "") {
        m_tupleDir = IOSvc::GetTupleDir(m_tupleOption, m_configHolder);
    } else {
        m_tupleDir = ((TString) _fileName).Remove(_fileName.Last('/'), _fileName.Length());
    }
    if (m_debug) MessageSvc::Debug("CreateTupleDir", m_tupleDir);
    return;
}

void TupleHolder::CreateTupleReader(TString _fileName, TString _tupleName) {
    if ((_fileName != "") && (_tupleName != "")) {
        MessageSvc::Info("CreateTupleReader", _fileName, _tupleName);
        MessageSvc::Info("CreateTupleReader", (TString) "AddTuple =", SettingDef::Tuple::addTuple ? "true" : "false");
        m_tupleReader = TupleReader(_tupleName, _fileName);
        m_tupleReader.Init();
    } else {
        Prj      project  = m_configHolder.GetProject();
        Analysis ana      = m_configHolder.GetAna();
        TString  _sample  = m_configHolder.GetSample();
        Q2Bin    q2bin    = m_configHolder.GetQ2bin();
        Year     year     = m_configHolder.GetYear();
        Polarity polarity = m_configHolder.GetPolarity();
        Trigger  trigger  = m_configHolder.GetTrigger();

        vector< TString > _years      = GetYears(to_string(year));
        vector< TString > _polarities = GetPolarities(to_string(polarity));

        MessageSvc::Info("CreateTupleReader", _years);
        MessageSvc::Info("CreateTupleReader", _polarities);

        TString _type = m_configHolder.IsMC() ? "MC" : "CL";

        _tupleName = m_tupleName;
        if (m_tupleOption.Contains("cre") && SettingDef::Tuple::chainexctrg) {
            MessageSvc::Warning("TupleHolder, chain TTree in TupleCreate, force disabling implicit MT. Root is bugged for MT with TChains having different TTreeNames , see https://root-forum.cern.ch/t/really-solved-rdataframe-for-tchain-loaded-with-ttrees-with-different-names/41756");
            MessageSvc::Warning("TupleHolder, Disabling ImplicitMT now, please check if L0I, L0L separately and L0I+L0L together gives you same nentries counting!");
            ROOT::DisableImplicitMT();
            if (m_configHolder.GetTrigger() == Trigger::All) {
                // TupleCreate with TriggerConf::Exclusive
                MessageSvc::Info("CreateTupleReader", (TString) "Chaining TriggerConf::Exclusive tuples for Trigger::All");
                // m_tupleName.ReplaceAll(_sample, _sample + SettingDef::separator + to_string(Trigger::L0I));
                m_tupleName.ReplaceAll(_sample, _sample + "_" + to_string(Trigger::L0I)); //TupleCreate sets names with underscore now...
                _tupleName = m_tupleName;
                _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");
            } else {
                // TupleCreate with TriggerConf::Exclusive
                MessageSvc::Info("CreateTupleReader", (TString) "Chaining TriggerConf::" + SettingDef::Config::triggerConf + " tuples for Trigger::" + to_string(trigger));
                _tupleName = m_tupleName;
                if (hash_triggerconf(SettingDef::Config::triggerConf) == TriggerConf::Exclusive) {
                    if (trigger == Trigger::L0I) _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");
                    if (trigger == Trigger::L0L) _tupleName.ReplaceAll(to_string(Trigger::L0L) + "exclusive", to_string(Trigger::L0I));
                } else if (hash_triggerconf(SettingDef::Config::triggerConf) == TriggerConf::Exclusive2) {
                    if (trigger == Trigger::L0I) m_tupleName.ReplaceAll(to_string(Trigger::L0I) + "exclusive", to_string(Trigger::L0I));
                    if (trigger == Trigger::L0L) m_tupleName.ReplaceAll(to_string(Trigger::L0L), to_string(Trigger::L0L) + "exclusive");
                    _tupleName = m_tupleName;
                    if (trigger == Trigger::L0I) _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");
                    if (trigger == Trigger::L0L) _tupleName.ReplaceAll(to_string(Trigger::L0L) + "exclusive", to_string(Trigger::L0I));
                } else
                    MessageSvc::Info("CreateTupleReader", (TString) "Incompatible TriggerConf to merge", SettingDef::Config::triggerConf, "EXIT_FAILURE");
            }
        }

        MessageSvc::Info("CreateTupleReader", m_configHolder.GetSample(), to_string(year), to_string(polarity), m_tupleName, m_tupleDir);
        if (_tupleName != m_tupleName) MessageSvc::Info("CreateTupleReader", m_configHolder.GetSample(), to_string(year), to_string(polarity), _tupleName, m_tupleDir);
        MessageSvc::Info("CreateTupleReader", (TString) "AddTuple =", SettingDef::Tuple::addTuple ? "true" : "false");

        m_tupleReader = TupleReader(m_tupleName);
        // bool _addList = true;
        bool _addList = true ;
        if( _sample == "Bs2KsKstJPsEE") _sample = "Bs2XJPsEE";// HACK for this , use the inclusive and tmCustom to filter.
        // TupleGanga  
        if (!m_tupleOption.Contains("cre")){
            _sample.ReplaceAll("KstSwap", "Kst");
        }
        if (m_tupleOption.Contains("gng")) {
            if (m_configHolder.IsMC()) {
                _sample.ReplaceAll("JPs", "JPs_");
                _sample.ReplaceAll("Psi", "Psi_");
                if( !_sample.Contains("EtaPrime") && !_sample.Contains("EtaG") ) _sample.ReplaceAll("Eta", "Eta_"); //hack to do due to naming EtaPrimeGEE samples, no splitting ! 
                _sample.ReplaceAll("Pi0", "Pi0_");
                _sample.ReplaceAll("KstG", "KstG_");
                _sample.ReplaceAll("EESS", "EE_SS");
                _sample.ReplaceAll("MMSS", "MM_SS");
            } else {
                _sample.ReplaceAll("LPTSS", "LPT_SS");
                _sample.ReplaceAll("LPT_SSHH", "LPT_SS"); // SS Hadrons saved in same DVNtuple
            }
            for (const auto & _year : _years) {
                for (const auto & _polarity : _polarities) {
                    // if( SettingDef::Tuple::addTuple == false){ MessageSvc::Warning("Break, no add List"); break; /*TO FAST BS FITS (gng tuples never used in fits, so no need here) */}
                    if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) {
                        _addList = m_tupleReader.AddList(m_tupleDir + "/" + _type + _year + "_" + _sample + _polarity + ".list");
                    } else {
                        _addList = m_tupleReader.AddList(m_tupleDir + "/" + _type + _year + "_" + _sample + _polarity + "/" + m_fileList);
                    }
                }
            }
        }
        // TupleProcess
        if (m_tupleOption.Contains("pro")) {   
            if( m_tupleOption.Contains("pro[")){
                MessageSvc::Warning("CreateTupleReader, proVer not global via Option ", m_tupleOption);                
            }
            int _mustEXIST = _years.size() * _polarities.size();          
            for (const auto & _year : _years) {
                for (const auto & _polarity : _polarities) {
                    if( SettingDef::Tuple::addTuple == false && 
                        SettingDef::Weight::useBS && 
                        SettingDef::Weight::iBS >=0 ){ 
                            MessageSvc::Warning("Break AddList TupleProcess pro, no add List (for BS and iBS >0 flag, if you see this in non-BS fits mode, this is a bug)"); 
                            break; /*TO FAST BS FITS from TupleProcess...*/
                    }
                    TString _list = m_tupleDir + "/" + _sample + "/" + _type + _year + _polarity + "/" + m_fileList;
                    _list = IOSvc::XRootDFileName(_list);
                    if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) { _list = m_tupleDir + "/" + _sample + "/" + _type + _year + ".list"; }   // DIFFERENT NAME
                    TString _listBkp = _list;
                    if (IOSvc::ExistFile(_list)) {
                        TString _listTmp = "";
                        if (_list.Contains("/eos/")) {
                            _listTmp = SettingDef::IO::outDir +"/" + m_fileList;
                            _listTmp.ReplaceAll(".//", "./");
                            if (m_debug) MessageSvc::Debug("CreateTupleReader", (TString) "Copying", _list, _listTmp);
                            IOSvc::CopyFile(_list, _listTmp);
                            _list = _listTmp;
                        }
                        MessageSvc::Info("CreateTupleReader", (TString) "Adding", _listBkp);
                        _addList = m_tupleReader.AddList(_list);
                        if (_listTmp != "") IOSvc::RemoveFile(_listTmp);
                    } else {
                        MessageSvc::Warning("CreateTupleReader", _listBkp, "does not exist, trying glob");
                        if (m_tupleDir.Contains("/eos/") && !IsBATCH("CERN")) MessageSvc::Warning("CreateTupleReader", (TString) "Cannot glob eos outside CERN");
                        if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) {   // DIFFERENT NAME
                            m_tupleReader.AddFiles(m_tupleDir + "/" + _sample + "/" + _type + _year + "*.root");
                        } else {
                            m_tupleReader.AddFiles(m_tupleDir + "/" + _sample + "/" + _type + _year + _polarity + "/*/*.root");
                        }
                    }
                        if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) break;   // NO SPLIT BY POLARITY
                }    
            }
            /*
            Protection for eos. If you deal with TupleCreate / LPT and SignalMC, the whole set of data must be loaded ( 11MD + 11MU + 12MU + 12MD ). 
            I.e avoid this to silently load only a part of what is needed. 
            */
            if( (m_configHolder.GetProject()==Prj::RK || m_configHolder.GetProject()==Prj::RKst) && SettingDef::Tuple::gngVer >=10 && SettingDef::Tuple::proVer != "SKIM"){        
                if( m_configHolder.IsSignalMC() && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0){
                    if( SettingDef::Tuple::addTuple){
                        if( m_configHolder.GetSample().Contains("Bd2KstGEE")){
                            MessageSvc::Warning(Form("HACK for Conv-Gamma,CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
                        }else{
                            MessageSvc::Error(Form("CreateTupleReader %s, proVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()),"","EXIT_FAILURE");
                        }
                    }else{
                        MessageSvc::Warning(Form("CreateTupleReader %s, proVer, must have loaded the exact nFiles for polarities and years asked for [addTuple=False, no Error]",m_configHolder.GetSample().Data()));
                    }
                }
            }
        }
        // TupleCreate
        if (m_tupleOption.Contains("cre")) {
            if( m_tupleOption.Contains("cre[")){
                MessageSvc::Warning("CreateTupleReader, creVer not global via Option ", m_tupleOption);                
            }            
            int _mustEXIST = _years.size() * _polarities.size();
            if(SettingDef::Tuple::chainexctrg) _mustEXIST *= 2; //If both exclusive categories loaded, factor 2 is needed.
            for (const auto & _year : _years) {
                for (const auto & _polarity : _polarities) {
                    m_tupleReader.AddFile(m_tupleDir + "/" + _year + _polarity + "/TupleCreate.root");
                    if (_tupleName != m_tupleName) m_tupleReader.AddFile(m_tupleDir + "/" + _year + _polarity + "/TupleCreate.root", _tupleName);
                }
            }
            /*
                Protection for eos. If you deal with TupleCreate / LPT and SignalMC, the whole set of data must be loaded ( 11MD + 11MU + 12MU + 12MD ). 
                I.e avoid this to silently load only a part of what is needed. 
            */
            if( m_configHolder.GetSample() == "LPT" && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0){
                if( SettingDef::Tuple::addTuple){                
                    MessageSvc::Error("CreateTupleReader LPT, creVer, must have loaded the exact nFiles for polarities and years asked for","","EXIT_FAILURE");
                }else{
                    MessageSvc::Warning(Form("CreateTupleReader LPT, creVer, must have loaded the exact nFiles for polarities and years asked for [addTuple=False, no Error]"));
                }
            }
            if( m_configHolder.IsSignalMC() && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0){
                if( SettingDef::Tuple::addTuple){          
                    if( m_configHolder.GetSample().Contains("Bd2KstGEE") && m_configHolder.GetProject() == Prj::RKst){      
                        MessageSvc::Warning(Form("HACK for Conv-Gamma,CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
                    }else{
                        MessageSvc::Error(Form("CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()),"","EXIT_FAILURE");
                    }
                }else{
                    MessageSvc::Warning(Form("CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
                }
            }
        }
        // TupleSPlot
        if (m_tupleOption.Contains("spl")) {
            for (const auto & _year : _years) {
                if (m_configHolder.GetTrigger() != Trigger::All) {
                    _sample = "FitHolder_" + m_configHolder.GetKey() + "_SPlot";
                    m_tupleReader.AddFile(m_tupleDir + "/" + _year + "/" + _sample + ".root");
                    if (m_tupleOption.Contains("splp")) {
                        // SPLOT FROM TUPLEPROCESS
                        m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/" + _sample + ".root", SettingDef::Tuple::SPT);
                    } else {
                        // SPLOT FROM TUPLECREATE
                        // m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root", SettingDef::Tuple::SPT);
                        m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root", SettingDef::Tuple::SPT);
                    }
                } else {
                    TupleReader _tupleReader = TupleReader(SettingDef::Tuple::SPT);
                    for (auto _trigger : GetTriggers(to_string(m_configHolder.GetTrigger()), true)) {
                        SettingDef::Config::trigger = _trigger;
                        ConfigHolder _co            = ConfigHolder();
                        TString      _tupleDir      = IOSvc::GetTupleDir(m_tupleOption, _co);
                        _sample                     = "FitHolder_" + _co.GetKey() + "_SPlot";
                        if (m_tupleOption.Contains("splp")) {
                            // SPLOT FROM TUPLEPROCESS
                            m_tupleReader.AddFile(_tupleDir + "/" + _year + "/" + _sample + ".root");
                            _tupleReader.AddFile(_tupleDir + "/" + _year + "/" + _sample + ".root");
                        } else {
                            // SPLOT FROM TUPLECREATE
                            // m_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root");
                            //_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root");
                            m_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root");
                            _tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root");
                        }
                    }
                    m_tupleReader.AddFriend(_tupleReader.Tuple());
                    SettingDef::Config::trigger = to_string(Trigger::All);
                }
            }
        }
        // TupleRapidSim
        if (m_tupleOption.Contains("rap")) { m_tupleReader.AddFile(m_tupleDir + "/" + _sample + "_tree.root"); }
	
        if (!_addList && !SettingDef::Tuple::datasetCache) {
            if (!m_configHolder.IsMC() || (m_configHolder.IsMC() && m_configHolder.IsSignalMC()))
                MessageSvc::Error("CreateTupleReader", (TString) "AddList incomplete", "EXIT_FAILURE");
            else
                MessageSvc::Warning("CreateTupleReader", (TString) "AddList incomplete");
        }

        // Init TupleReader
        if (m_tupleOption.Contains("cre")) {
            bool _trowLogicError       = SettingDef::trowLogicError;
            bool _useEOS               = SettingDef::IO::useEOS;
            SettingDef::trowLogicError = true;
            SettingDef::IO::useEOS     = true;
            try {
                m_tupleReader.Init();
            } catch (const exception & e) {
                MessageSvc::Line();
                MessageSvc::Warning("CreateTupleReader", (TString) "Trying TupleProcess", SettingDef::Tuple::proVer, "on EOS");
                MessageSvc::Line();
                if (SettingDef::Tuple::proVer == "") MessageSvc::Error("CreateTupleReader", (TString) "proVer not defined", "EXIT_FAILURE");
                SettingDef::trowLogicError = _trowLogicError;
                m_tupleOption              = "pro";
                m_tupleName                = "DT";
                Init();
            }
            SettingDef::trowLogicError = _trowLogicError;
            SettingDef::IO::useEOS     = _useEOS;
        } else if (m_tupleOption != "") {
            m_tupleReader.Init();
        }
    }

    if (m_tupleReader.IsInitialized()) {
        SetBranches();
        SetAliases();
        m_isInitialized = true;
    }
    return;
}

void TupleHolder::CreateSubTupleReader(int _iFile) {
    if (_iFile < m_tupleReader.GetNFiles()) {
        m_fileName = m_tupleReader.GetFileName(_iFile);

        MessageSvc::Info("TupleHolder", (TString) "CreateSubTupleReader for", m_fileName);

        m_tupleReader = TupleReader(m_tupleName);
        m_tupleReader.AddFile(m_fileName);
        m_tupleReader.Init();

        if (m_tupleReader.IsInitialized()) {
            SetBranches();
            SetAliases();
            m_isInitialized = true;
        }

        SettingDef::Tuple::fileName = m_fileName;
    } else
        MessageSvc::Error("TupleHolder", (TString) "CreateSubTupleReader", to_string(_iFile) + "th", "file does not exist (NFiles =", to_string(m_tupleReader.GetNFiles()) + ")", "EXIT_FAILURE");
    return;
}

vector< TString > TupleHolder::GetBranches(TString _option) {
    if (m_debug) MessageSvc::Debug("GetBranches", _option);

    vector< TString > _branches;

    TString _card = IOSvc::GetDataDir("cards");
    if (_option.Contains("EXC"))
        _card += "/branches" + to_string(m_configHolder.GetProject()) + ".txt";
    else if (_option.Contains("MVA"))
        _card += "/branches" + to_string(m_configHolder.GetProject()) + "MVA.txt";
    else if (_option.Contains("SKIM"))
        _card += "/branchesSKIM.txt";
    else if (_option.Contains("INC"))
        _card += "/branchesINC.txt";

    vector< vector< TString > > _lines = IOSvc::ParseFile(_card, "");
    for (auto & _line : _lines) { _branches.push_back(_line[0]); }
    MessageSvc::Info("GetBranches", _card, to_string(_branches.size()));

    if (_option.Contains("EXC")) {
        _card.ReplaceAll(to_string(m_configHolder.GetProject()), "TRUE");
        vector< vector< TString > > _lines_EXC = IOSvc::ParseFile(_card, "");
        for (auto & _line : _lines_EXC) { _branches.push_back(_line[0]); }
        MessageSvc::Info("GetBranches", _card, to_string(_branches.size()));
    }

    if (m_tupleReader.IsInitialized()) {
        if (_branches.size() != 0) {
            vector< TString > _branchesTmp;
            for (auto * _branch : *GetTuple()->GetListOfBranches()) {
                TString _name = _branch->GetName();
                if (find(_branchesTmp.begin(), _branchesTmp.end(), _name) == _branchesTmp.end()) {
                    for (const auto & _b : _branches) {
                        if (_b.BeginsWith("_") && _name.EndsWith(_b)) _branchesTmp.push_back(_name);
                        if (!_b.BeginsWith("_") && (_name == _b)) _branchesTmp.push_back(_name);
                    }
                }
            }
            RemoveVectorDuplicates(_branchesTmp);
            MessageSvc::Info("GetBranches", GetTuple()->GetName(), to_string(_branches.size()), "->", to_string(_branchesTmp.size()));
            _branches = _branchesTmp;
        }
    }

    return _branches;
}

void TupleHolder::SetBranches(vector< TString > _branches) {
    if (_branches.size() == 0) _branches = m_branches;
    int _nBranches = _branches.size();
    if (_branches.size() != 0) {
        MessageSvc::Info("SetBranches", (TString) GetTuple()->GetName(), to_string(GetTuple()->GetNbranches()), "->", to_string(_branches.size()));
        m_tupleReader.SetBranches({}, false, "all");
        _nBranches = m_tupleReader.SetBranches(_branches, true);
        m_branches = _branches;
    } else {
        _nBranches = m_tupleReader.SetBranches({}, true, "all");
    }
    if (_nBranches != _branches.size()) MessageSvc::Info("SetBranches", (TString) GetTuple()->GetName(), to_string(GetTuple()->GetNbranches()), "->", to_string(_nBranches));    
    return;
}

void TupleHolder::UpdateBranches() {
    if (GetTuple()->GetListOfBranches() != nullptr) {
        if (m_debug) MessageSvc::Debug("UpdateBranches", to_string(GetTuple()->GetListOfBranches()->GetSize()), to_string(GetTuple()->GetNbranches()), to_string(m_branches.size()));
        m_branches.clear();
        for (auto * _branch : *GetTuple()->GetListOfBranches()) {
            if (GetTuple()->GetBranchStatus(_branch->GetName())) m_branches.push_back(_branch->GetName());
        }
        if (m_debug) MessageSvc::Debug("UpdateBranches", to_string(GetTuple()->GetListOfBranches()->GetSize()), to_string(GetTuple()->GetNbranches()), to_string(m_branches.size()));
    }
    return;
}

void TupleHolder::CheckBranches(vector< TString > _branches, bool deep ) {
    if( !deep){
        if (_branches.size() == 0) _branches = m_branches;
        if (_branches.size() != 0) {
            MessageSvc::Info("CheckBranches", (TString) GetTuple()->GetName(), to_string(_branches.size()), "out of", to_string(GetTuple()->GetNbranches()));
            int _count = 0;
            for (auto _branch : _branches) {
                if (!CheckVarInTuple(_branch)) {
                    MessageSvc::Warning("TupleHolder", (TString) "Branch", _branch, "does not exist");
                    _count++;
                }
            }
            MessageSvc::Line();
            if (_count != 0)
                MessageSvc::Warning(GetTuple()->GetName(), to_string(_count), "missing branches");
            else
                MessageSvc::Info(GetTuple()->GetName(), (TString) "All requested branches present");
            MessageSvc::Line();
        }
    }else{
        if( !m_tupleReader.CheckVarsInChain( _branches, m_tupleName)){
            MessageSvc::Error("INCOMPATIBLE BRANCHES ON TCHAINS, CANNOT CUT SAFELY, ABORT AND PLEASE REPORT/FIX IT. THIS IS A DANGEROUS A BUG!", "","EXIT_FAILURE");
        };
    }
        
    return;
}


const bool TupleHolder::IsSampleInCreVer(TString _creVer, TString _prj, TString _ana, TString _q2bin, TString _year , TString _trigger, TString _triggerConf, TString _sample) {
    SettingDef::Tuple::creVer = _creVer;
    TString _tupleDir = IOSvc::GetTupleDir("cre", _prj, _ana, _q2bin, _year, _trigger);
    MessageSvc::Info("IsSampleInCreVer: check for", _sample, "in tuple ", _tupleDir + "/" + _year + "MD/TupleCreate.root");
    TFile _file(_tupleDir + "/" + _year + "MD/TupleCreate.root", to_string(OpenMode::READ));
    _file.GetListOfKeys()->Print();
    TIter   _keyList(_file.GetListOfKeys());
    TKey *  _key;
    while ((_key = (TKey *) _keyList())) {
        if (((TString) _key->ReadObj()->GetName()).Contains(_sample)) return true;
    }
    return false;
};
    
vector< pair< TString, TString > > TupleHolder::GetAliases(TString _option) {
    if (m_debug) MessageSvc::Debug("GetAliases");

    vector< pair< TString, TString > > _aliases;

    TString _card = IOSvc::GetDataDir("cards");
    if (_option.Contains("EXC"))
        _card += "/aliases" + to_string(m_configHolder.GetProject()) + ".txt";
    else if (_option.Contains("SKIM"))
        _card += "/aliasesSKIM.txt";
    else if (_option.Contains("INC"))
        _card += "/aliasesINC.txt";

    map< TString, TString > _names = m_configHolder.GetParticleNames();

    vector< vector< TString > > _lines = IOSvc::ParseFile(_card, " ");
    for (auto & _line : _lines) {
        bool _skip = false;

        TString _alias = _line[0];
        TString _expr  = _line[1];

        if (_alias.Contains("_wMVA_") && (SettingDef::Cut::mvaVer == "")) continue;
        if (_alias.Contains("_wMVA_") && _alias.Contains("_wMVA_PR_") && (m_configHolder.GetAna() == Analysis::MM)) continue;

        Prj project = m_configHolder.GetProject();
        switch (project) {
            case Prj::RKst: break;
            case Prj::RK:
                if (_alias.Contains("{HH}") || _alias.Contains("_HH") || _alias.Contains("HH_") || _alias.Contains("H2_") || _alias.Contains("{H2}")) _skip = true;
                if (_expr.Contains("{HH}") || _expr.Contains("_HH") || _expr.Contains("HH_") || _expr.Contains("H2_") || _expr.Contains("{H2}")) _skip = true;
                break;
            case Prj::RPhi: break;
            case Prj::RL: break;
            case Prj::RKS: break;
            default: MessageSvc::Error("Wrong project", to_string(project), "EXIT_FAILURE"); break;
        }
        if (_skip) {
            if (m_debug) MessageSvc::Debug("GetAliases", (TString) "Invalid Alias definition for", to_string(project), "project", _alias, "->", _expr, "SKIPPING");
            continue;
        }

        _alias = ReplaceWildcards(_alias, _names);

        if (_expr.Contains("{TUNE}")) {
            TString _tuneH = GetPIDTune(to_string(m_configHolder.GetYear()), "H");
            TString _tuneL = GetPIDTune(to_string(m_configHolder.GetYear()), "L");
            if (_expr.Contains("{H")) _expr.ReplaceAll("{TUNE}", _tuneH);
            if (_expr.Contains("{L")) _expr.ReplaceAll("{TUNE}", _tuneL);
        }

        _expr = ReplaceWildcards(_expr, _names);

        Analysis ana = m_configHolder.GetAna();
        switch (ana) {
            case Analysis::MM:
                if (_expr.Contains("M1_TRACK") || _expr.Contains("M2_TRACK")) _skip = true;
                if (_alias.Contains("E1_") || _alias.Contains("E2_")) _skip = true;
                if (_expr.Contains("E1_") || _expr.Contains("E2_")) _skip = true;
                break;
            case Analysis::EE:
                if (_alias.Contains("M1_") || _alias.Contains("M2_")) _skip = true;
                if (_expr.Contains("M1_") || _expr.Contains("M2_")) _skip = true;
                break;
            case Analysis::ME:
                if (_alias.Contains("E1_") && _alias.Contains("E2_")) _skip = true;
                if (_expr.Contains("E1_") && _expr.Contains("E2_")) _skip = true;
                if (_alias.Contains("M1_") && _alias.Contains("M2_")) _skip = true;
                if (_expr.Contains("M1_") && _expr.Contains("M2_")) _skip = true;
                break;
            default: MessageSvc::Error("Wrong analysis", to_string(ana), "EXIT_FAILURE"); break;
        }
        if (_skip) {
            if (m_debug) MessageSvc::Debug("GetAliases", (TString) "Invalid Alias definition for", to_string(ana), "analysis", _alias, "->", _expr, "SKIPPING");
            continue;
        }

        if (m_debug) MessageSvc::Debug("GetAliases", _alias, "->", _expr);
        _aliases.push_back(make_pair(_alias, _expr));
    }
    MessageSvc::Info("GetAliases", _card, to_string(_aliases.size()));

    return _aliases;
}

void TupleHolder::SetAliases(vector< pair< TString, TString > > _aliases) {
    if (_aliases.size() == 0) _aliases = m_aliases;
    int _nAliases = _aliases.size();
    if (_aliases.size() != 0) {
        MessageSvc::Info("SetAliases", (TString) GetTuple()->GetName(), to_string(_aliases.size()));
        _nAliases = m_tupleReader.SetAliases(_aliases);
        m_aliases = _aliases;
    }
    if ((_nAliases != _aliases.size()) || (_nAliases == 0)) MessageSvc::Info("SetAliases", (TString) GetTuple()->GetName(), to_string(_nAliases));
    return;
}

void TupleHolder::UpdateAliases() {
    if (GetTuple()->GetListOfAliases() != nullptr) {
        if (m_debug) MessageSvc::Debug("UpdateAliases", to_string(GetTuple()->GetListOfAliases()->GetSize()), to_string(m_aliases.size()));
        m_aliases.clear();
        for (auto * _alias : *GetTuple()->GetListOfAliases()) { m_aliases.push_back(make_pair(_alias->GetName(), _alias->GetTitle())); }
        if (m_debug) MessageSvc::Debug("UpdateAliases", to_string(GetTuple()->GetListOfAliases()->GetSize()), to_string(m_aliases.size()));
    }
    return;
}

TTree * TupleHolder::GetTuple(TCut _cut, TString _tupleName) {
    TTree * _tuple = nullptr;
    if (_tupleName == "") _tupleName = m_tupleName;
    if (SettingDef::Tuple::dataFrame) {
        _tuple = m_tupleReader.SnapshotTuple(_cut, _tupleName, SettingDef::Tuple::frac, false, m_branches, GetVectorFirst(m_aliases));
    } else {
        _tuple = m_tupleReader.CopyTuple(_cut, _tupleName, SettingDef::Tuple::frac, false);
    }
    return _tuple;
}

const Long64_t TupleHolder::TupleEntries(TCut _cut) {
    Long64_t _entries = 0;
    if (IsCut(_cut)) {
        if (HasWeight(_cut)) {
            _entries = round(GetEntries(*GetTuple(), _cut));
        } else {
            // if (SettingDef::Tuple::dataFrame) {
            //    _entries = GetEntriesDF(*GetTuple(), _cut);
            //} else {
            _entries = GetTuple()->GetEntries(_cut);
            //}
        }
    } else {
        _entries = GetTuple()->GetEntriesFast();
    }
    return _entries;
}

pair< double, double > TupleHolder::GetLuminosity() {
    if (!m_configHolder.IsMC() && ((m_tupleName == "LT") || m_tupleName.Contains("Lumi"))) {
        // For errors on luminosity look why we sum it up :
        // https://indico.cern.ch/event/100756/contributions/1296401/attachments/3262/4947/Lumi_Yas.pdf#search=yasmine%20amhis%20luminosity
        // In 2016-2017-2018 data the error is the same as the value, as not calibrated yet....
        MessageSvc::Line();
        MessageSvc::Info("TupleHolder", (TString) "Luminosity");
        MessageSvc::Line();

        double _lumiT  = 0;
        double _lumiTE = 0;

        TTreeReaderValue< double > _lumi  = m_tupleReader.GetValue< double >("IntegratedLuminosity");
        TTreeReaderValue< double > _lumiE = m_tupleReader.GetValue< double >("IntegratedLuminosityErr");

        Long64_t _nEntries = m_tupleReader.GetEntries();
        MessageSvc::Line();
        MessageSvc::Info("TupleHolder", (TString) "Looping ...", to_string(_nEntries));
        Long64_t i = 0;
        while (m_tupleReader.Reader()->Next()) {
            if (!m_debug) MessageSvc::ShowPercentage(i, _nEntries);
            // m_tupleReader.GetEntry(i);
            _lumiT += *_lumi;
            _lumiTE += *_lumiE;
            i++;
        }

        RooRealVar _iLumi = RooRealVar("IntegratedLuminosity", "IntegratedLuminosity", _lumiT);
        _iLumi.setError(_lumiTE);

        MessageSvc::Line();
        MessageSvc::Info("Luminosity", &_iLumi);
        MessageSvc::Line();

        return make_pair(_iLumi.getValV(), _iLumi.getError());
    }
    return make_pair(0, 0);
}

void TupleHolder::PrintInline() const noexcept {
    m_configHolder.PrintInline();
    TString _toPrint = fmt::format("TupleOption {0}", Option());
    MessageSvc::Info(Color::Cyan, "TupleHolder", _toPrint);
    return;
}

vector<TString> TupleHolder::GetFileNames() const { 
    return m_tupleReader.GetFileNames();
}; 
