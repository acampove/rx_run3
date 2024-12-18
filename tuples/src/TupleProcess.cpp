#ifndef TUPLEPROCESS_CPP
#define TUPLEPROCESS_CPP

#include "TupleProcess.hpp"

#include "SettingDef.hpp"

#include "itertools.hpp"
#include "vec_extends.h"

#include "ROOT/RDataFrame.hxx"
#include "TSystem.h"
using namespace iter;

TupleProcess::TupleProcess(const EventType & _eventType, TString _option) {
    if (SettingDef::debug.Contains("TP")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleProcess", (TString) "EventType");
    m_eventType = _eventType;
    m_option    = _option;
    Check();
    // Init();
}

TupleProcess::TupleProcess(TString _fileName, TString _option) {
    if (SettingDef::debug.Contains("TP")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleProcess", (TString) "TString");
    m_fileName = _fileName;
    m_option   = _option;
    Check();
    Init();
}

TupleProcess::TupleProcess(const TupleProcess & _tupleProcess) {
    if (SettingDef::debug.Contains("TP")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleProcess", (TString) "TupleProcess");
    m_eventType = _tupleProcess.GetEventType();
    m_option    = _tupleProcess.Option();
    m_fileName  = _tupleProcess.FileName();
    Check();
}

ostream & operator<<(ostream & os, const TupleProcess & _tupleProcess) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "TupleProcess");
    MessageSvc::Line(os);
    if (_tupleProcess.FileName() != "") MessageSvc::Print((ostream &) os, "file", _tupleProcess.FileName());
    MessageSvc::Print((ostream &) os, "option", _tupleProcess.Option());
    // MessageSvc::Line(os);
    os << _tupleProcess.GetEventType();
    os << RESET;
    return os;
}

bool TupleProcess::Check() {
    /*
    for (auto _opt : TokenizeString(m_option, SettingDef::separator)) {
        if (!CheckVectorContains(SettingDef::AllowedConf::TupleOptions, _opt)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("TupleProcess", "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::TupleOptions", "EXIT_FAILURE");
        }
    }
    */
    return false;
}

void TupleProcess::Init() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "Initialize ...");
    MessageSvc::Line();

    bool _aliases              = SettingDef::Tuple::aliases;
    SettingDef::Tuple::aliases = false;

    if (m_fileName != "") {
        if (!IOSvc::ExistFile(m_fileName)) MessageSvc::Error("TupleProcess", m_fileName, "does not exist", "EXIT_FAILURE");

        MessageSvc::Info("TupleProcess", (TString) "Initialize ...", m_fileName);

        auto _configHolder = ConfigHolder(hash_project(SettingDef::Config::project), hash_analysis(SettingDef::Config::ana), SettingDef::Config::sample, hash_q2bin(SettingDef::Config::q2bin), hash_year(SettingDef::Config::year), hash_polarity(SettingDef::Config::polarity), hash_trigger(SettingDef::Config::trigger), hash_brem(SettingDef::Config::brem), hash_track(SettingDef::Config::track));

        auto _weightHolder = WeightHolder(_configHolder, SettingDef::Weight::option);

        auto _cutHolder = CutHolder(_configHolder, SettingDef::Cut::option);

        auto _tupleHolder = TupleHolder(_configHolder, m_fileName, SettingDef::Tuple::tupleName, SettingDef::Tuple::option);

        m_eventType = EventType(_configHolder, _cutHolder, _weightHolder, _tupleHolder, false);
    }
    
    SettingDef::Tuple::aliases = _aliases;

    // cout << *this << endl;
    return;
}

void TupleProcess::Process() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "Processing ...");
    MessageSvc::Line();    
    if( SettingDef::debug.Contains("tpCopySingle")){
        /*
            Special Routine, no cuts to apply , just a copy of file, no operations done on MCDT and DT
            Be careful using it.
        */
        if( IOSvc::ExistFile("./TupleProcess.root") ){
            MessageSvc::Info("Input file already copied locally, early quitting, DEBUG MODE ONLY!!!");
            return; 
        }
        m_eventType.Init();
        MessageSvc::Warning("Expected 1 single input file to process, checking nfiles in TChain");
        MessageSvc::Warning(TString::Format("nFiles             : %i", m_eventType.GetTupleReader().GetNFiles()));  
        MessageSvc::Warning(TString::Format("frac               : %i", SettingDef::Tuple::frac));  
        MessageSvc::Warning(TString::Format("FileNames.size()   : %i", m_eventType.GetTupleReader().GetFileNames().size()));  

        if(m_eventType.GetTupleReader().GetNFiles() == 1 && SettingDef::Tuple::frac <0){
            if( m_eventType.GetTupleReader().GetFileNames().size() !=1 ){
                MessageSvc::Error("GetNFiles() misaligned to GetFileNames() vector content");
            }
            bool _aliases              = SettingDef::Tuple::aliases;
            SettingDef::Tuple::aliases = false;
            auto _file =  m_eventType.GetTupleReader().GetFileNames()[0];      
            MessageSvc::Warning("Only 1 file asked for",_file);            
            MessageSvc::Info(TString::Format("IOSvc::CopyFile(%s,%s)", _file.Data(),"TupleProcess.root"));
            IOSvc::RemoveFile("TupleProcess.root");
            IOSvc::CopyFile(_file, "TupleProcess.root" );
            bool copyOk = false;   
            TFile *f = new TFile("TupleProcess.root","READ"); 
            if (f == nullptr || f->IsZombie()){     
                MessageSvc::Error("File copied is nullptr or a Zombie", "","EXIT_FAILURE" );
            }else{
                copyOk = true;
            }
            f->Close(); 
            delete f;
            SettingDef::Tuple::aliases = _aliases;
            if( copyOk ==false  ){
                MessageSvc::Error("Cannot make the copy of the file locally for further processing Status",TString::Format("%i", copyOk) ,"EXIT_FAILURE");
            }else{                
                MessageSvc::Info("Copy done, early quitting");
                m_eventType.Close();
                return;
            }
        }
    }else{
      MessageSvc::Warning("Procedure to apply only for SKIMMING, BE CAREFUL");
    }

    TString _oFileName = "TupleProcess.root";
    Bool_t SSLepton = false;
    if (m_eventType.GetSample().Contains("SS"))   {
	    _oFileName = "tmpTupleProcessLL.root";
	    SSLepton   = true;
    }
    if (m_eventType.GetSample().Contains("SSHH")) {
	    _oFileName = "tmpTupleProcessHH.root";
	    SSLepton   = false;
    }

    TFile _oFile(_oFileName, to_string(OpenMode::RECREATE));

    TFile _tFile("tmpTupleProcess.root", to_string(OpenMode::RECREATE));

    bool _aliases              = SettingDef::Tuple::aliases;
    SettingDef::Tuple::aliases = false;

    ZippedEventType _zip = m_eventType.Zip();
    _zip.tupleName       = "DT";
    EventType _etDT      = EventType(_zip);
    cout << _etDT << endl;

    SettingDef::Tuple::aliases = _aliases;

    MessageSvc::Line();
    m_branches = _etDT.GetTupleHolder().Branches();
    MessageSvc::Info("TupleProcess", (TString) "Input branches", to_string((m_branches.size())));
    m_branchesSKIM = SettingDef::Tuple::branches ? _etDT.GetTupleHolder().GetBranches("SKIM") : m_branches;
    MessageSvc::Info("TupleProcess", (TString) "Output branches", to_string((m_branchesSKIM.size())));
    if (SettingDef::Tuple::aliases) m_aliasesSKIM = _etDT.GetTupleHolder().GetAliases("SKIM");
    MessageSvc::Info("TupleProcess", (TString) "Output aliases", to_string((m_aliasesSKIM.size())));
    MessageSvc::Line();

    _tFile.cd();
    TTree *     _tupleDT   = (TTree *) GetTupleFiltered(_etDT, SettingDef::Tuple::DT);
    Long64_t    _entriesIN = _tupleDT->GetEntries();
    TupleReader _readerDT  = TupleReader((TChain *) _tupleDT);

    _tFile.cd();
    if (_etDT.IsRapidSim()) {
        _tupleDT = GetTupleProcessRST(_etDT, _readerDT, _oFile, SettingDef::Tuple::RST);
    } else {
        _tupleDT = GetTupleProcessDT(_etDT, _readerDT, _oFile, SettingDef::Tuple::DT);
    }
    Long64_t _entriesOUT = _tupleDT->GetEntries();

    MessageSvc::Info("TupleProcess", (TString) "DT entries IN  =", to_string(_entriesIN));
    MessageSvc::Info("TupleProcess", (TString) "DT entries OUT =", to_string(_entriesOUT));
    if (!_etDT.IsCutted()) {
        if (_entriesIN != _entriesOUT) MessageSvc::Error("TupleProcess", (TString) "DT entries IN != DT entries OUT", "EXIT_FAILURE");
    }

    MessageSvc::Info("TupleProcess", (TString) "Writing", SettingDef::Tuple::DT, "...");
    MessageSvc::Line();
    _oFile.cd();
    if (_tupleDT) _tupleDT->Write(SettingDef::Tuple::DT, TObject::kOverwrite);

    SettingDef::Tuple::aliases = false;

    TTree * _tupleMCT = nullptr;
    TTree * _tupleLT  = nullptr;
    if (_etDT.IsMC()) {
        if (_etDT.HasMCDecayTuple() && !_etDT.IsRapidSim()) {
            ZippedEventType _zip2 = m_eventType.Zip();
            _zip2.cutOption       = "no";
            _zip2.weightOption    = "no";
            _zip2.tupleName       = "MCT";
            if( _etDT.IsCrossFeedSample() && _etDT.GetSample().BeginsWith("Bd") ){
                MessageSvc::Warning("Swapping to and RKst project to have proper MCDT handling");
                _zip2.project = Prj::RKst;
            }
            EventType _etMCT     = EventType(_zip2);
            cout << _etMCT << endl;
            _tFile.cd();
            _tupleMCT              = (TTree *) GetTupleFiltered(_etMCT, SettingDef::Tuple::MCT);
            Long64_t    _entriesIN2 = _tupleMCT->GetEntries();
            TupleReader _readerMCT = TupleReader((TChain *) _tupleMCT);
            _tFile.cd();
            _tupleMCT            = GetTupleProcessMCT(_etMCT, _readerMCT, _oFile, SettingDef::Tuple::MCT);
            Long64_t _entriesOUT2 = _tupleMCT->GetEntries();

            MessageSvc::Info("TupleProcess", (TString) "MCT entries IN  =", to_string(_entriesIN2));
            MessageSvc::Info("TupleProcess", (TString) "MCT entries OUT =", to_string(_entriesOUT2));
            if (!_etMCT.IsCutted()) {
                if (_entriesIN2 != _entriesOUT2) MessageSvc::Error("TupleProcess", (TString) "MCT entries IN != MCT entries OUT", "EXIT_FAILURE");
            }
            MessageSvc::Info("TupleProcess", (TString) "Writing", SettingDef::Tuple::MCT, "...");
            MessageSvc::Line();
            _oFile.cd();
            if (_tupleMCT) _tupleMCT->Write(SettingDef::Tuple::MCT, TObject::kOverwrite);
        } else {
            MessageSvc::Info("TupleProcess", (TString) "Skipping", SettingDef::Tuple::MCT, "...");
        }
    } else {
        ZippedEventType _zip2 = m_eventType.Zip();
        _zip2.cutOption       = "no";
        _zip2.weightOption    = "no";
        _zip2.tupleName       = "LT";
        EventType _etLT      = EventType(_zip2);
        cout << _etLT << endl;

        _tupleLT = _etLT.GetTuple();

        MessageSvc::Info("TupleProcess", (TString) "Writing", SettingDef::Tuple::LT, "...");
        MessageSvc::Line();
        _oFile.cd();
        if (_tupleLT) _tupleLT->Write(SettingDef::Tuple::LT, TObject::kOverwrite);
    }

    TTree * _tupleET = nullptr;
    /*
    ZippedEventType _zip = m_eventType.Zip();
    _zip.cutOption    = "no";
    _zip.weightOption = "no";
    _zip.tupleName    = "ET";
    EventType _etET = EventType(_zip);
    cout << _etET << endl;

    _tupleET = _etET.GetTuple();

    MessageSvc::Info("TupleProcess", (TString) "Writing", SettingDef::Tuple::ET, "...");
    MessageSvc::Line();
    _oFile.cd();
    if (_tupleET) _tupleET->Write(SettingDef::Tuple::LT, TObject::kOverwrite);
    */

    MessageSvc::Line();
    _oFile.ls();
    MessageSvc::Line();

    Long64_t _entriesDT  = -1;
    Long64_t _entriesMCT = -1;
    Long64_t _entriesLT  = -1;
    Long64_t _entriesET  = -1;
    if (_tupleDT) {
        _entriesDT = _tupleDT->GetEntries();
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::DT, to_string(_entriesDT), "entries");
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::DT, to_string(_tupleDT->GetNbranches()), "branches");
        if (_tupleDT->GetListOfAliases() != nullptr) MessageSvc::Info("TupleProcess", SettingDef::Tuple::DT, to_string(_tupleDT->GetListOfAliases()->GetSize()), "aliases");
    }
    if (_tupleMCT) {
        _entriesMCT = _tupleMCT->GetEntries();
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::MCT, to_string(_entriesMCT), "entries");
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::MCT, to_string(_tupleMCT->GetNbranches()), "branches");
        if (_tupleMCT->GetListOfAliases() != nullptr) MessageSvc::Info("TupleProcess", SettingDef::Tuple::MCT, to_string(_tupleMCT->GetListOfAliases()->GetSize()), "aliases");
    }
    if (_tupleLT) {
        _entriesLT = _tupleLT->GetEntries();
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::LT, to_string(_entriesLT), "entries");
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::LT, to_string(_tupleLT->GetNbranches()), "branches");
        if (_tupleLT->GetListOfAliases() != nullptr) MessageSvc::Info("TupleProcess", SettingDef::Tuple::LT, to_string(_tupleLT->GetListOfAliases()->GetSize()), "aliases");
    }
    if (_tupleET) {
        _entriesET = _tupleET->GetEntries();
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::ET, to_string(_entriesET), "entries");
        MessageSvc::Info("TupleProcess", SettingDef::Tuple::ET, to_string(_tupleET->GetNbranches()), "branches");
        if (_tupleET->GetListOfAliases() != nullptr) MessageSvc::Info("TupleProcess", SettingDef::Tuple::ET, to_string(_tupleET->GetListOfAliases()->GetSize()), "aliases");
    }

    /*
    if (_tupleDT)  PlotBranches(_tupleDT,  "DT");
    if (_tupleMCT) PlotBranches(_tupleMCT, "MCT");
    if (_tupleLT)  PlotBranches(_tupleLT,  "LT");
    if (_tupleET)  PlotBranches(_tupleET,  "ET");
    */
    /*
    if (m_debug) {
        cout << YELLOW;
        if (_tupleDT) _tupleDT->Print();
        if (_tupleMCT) _tupleMCT->Print();
        if (_tupleLT) _tupleLT->Print();
        if (_tupleET) _tupleET->Print();
        cout << RESET;
    }
    MessageSvc::Line();
    */

    // m_eventType.Write("EventType", TObject::kOverwrite);
    // m_eventType.SaveToNamed(& _oFile, "EventType");

    _oFile.Close();
    _tFile.Close();

    if ((_entriesDT == 0) || (_entriesMCT == 0) || (_entriesLT == 0) || (_entriesET == 0)) MessageSvc::Error("TupleProcess", (TString) "Empty tuple", "EXIT_FAILURE");
    return;
}

template < typename T > bool TupleProcess::AddMap(T & _map, TString _name, TTree * _tuple) {
    bool _flag = false;
    if (_map.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "AddMap", _name);
        MessageSvc::Line();
        for (const auto & _branch : _map) {
            if (!_tuple->GetListOfBranches()->Contains(_branch.second.name)) {
                MessageSvc::Info("Adding", _branch.second.name, _branch.second.size != 1 ? to_string(_branch.second.size) : "");
                if (_branch.second.size != 1) {
                    if (GetTypename(_map).Contains("BranchInfo<int>")) {
                        _tuple->Branch(_branch.second.name, _branch.second.addr, _branch.second.name + "[" + to_string(_branch.second.size) + "]/I");
                        _flag = true;
                    } else if (GetTypename(_map).Contains("BranchInfo<double>")) {
                        _tuple->Branch(_branch.second.name, _branch.second.addr, _branch.second.name + "[" + to_string(_branch.second.size) + "]/D");
                        _flag = true;
                    } else if (GetTypename(_map).Contains("vector<double")) {
                        _tuple->Branch(_branch.second.name, _branch.second.addr);
                        _flag = true;
                    } else if (GetTypename(_map).Contains("vector<int")) {
                        _tuple->Branch(_branch.second.name, _branch.second.addr);
                        _flag = true;
                    } else
                        MessageSvc::Warning("AddMap", (TString) "typename", GetTypename(_map), "not known", "SKIPPING");
                } else {
                    _tuple->Branch(_branch.second.name, _branch.second.addr);
                    _flag = true;
                }
            } else {
                MessageSvc::Warning("Already in tuple", _branch.second.name, "SKIPPING");
            }
        }
    } else {
        MessageSvc::Warning("AddMap", _name, "NOTHING TO ADD");
    }
    return _flag;
}

bool TupleProcess::AddAlias(vector< pair< TString, TString > > & _aliases, TString _name, TTree * _tuple) {
    bool _flag = false;
    if (_aliases.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "AddAlias", _name);
        MessageSvc::Line();
        for (const auto & _branch : _aliases) {
            if ((_tuple->GetListOfAliases() == nullptr) || ((_tuple->GetListOfAliases() != nullptr) && !_tuple->GetListOfAliases()->Contains(_branch.first))) {
                MessageSvc::Info("Adding", _branch.first, "->", _branch.second);
                _tuple->SetAlias(_branch.first, _branch.second);
            } else {
                MessageSvc::Warning(_branch.first, (TString) "Already in tuple", "SKIPPING");
            }
        }
    } else {
        MessageSvc::Info("AddAlias", _name, "NOTHING TO ADD");
    }
    return _flag;
}

TTree * TupleProcess::GetTupleFiltered(EventType & _eventType, TString _tupleName) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "GetTupleFiltered", _tupleName);
    MessageSvc::Line();
    return (TTree *) _eventType.GetTupleReader().CopyTuple(_eventType.GetCut());
}

TTree * TupleProcess::GetTupleProcessDT(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "GetTupleProcessDT", _tupleName);
    MessageSvc::Line();

    bool _addBSevt    = (!_eventType.IsMC() || _eventType.IsSignalMC()) && m_bootstrapEvt;
    bool _addTCK      = _eventType.IsMC() && (_eventType.GetYear() == Year::Y2015 || _eventType.GetYear() == Year::Y2016);
    bool _addWPREC    = _eventType.IsMC() && ( _eventType.GetSample().Contains("2KPi") && !_eventType.GetSample().Contains("Bu2KPiEE") );

    MessageSvc::Info("TupleProcess", (TString) "SetBranches", to_string(m_branches.size()), "->", to_string(m_branchesSKIM.size()));
    _tupleReader.SetBranches({}, false, "all");
    _tupleReader.SetBranches(m_branchesSKIM, true);

    _tFile.cd();
    TTree * _tuple = (TTree *) _tupleReader.CloneTuple(_tupleName);

    MessageSvc::Info("TupleProcess", (TString) "SetBranches", to_string(m_branchesSKIM.size()), "->", to_string(m_branches.size()));
    _tupleReader.SetBranches({}, false, "all");
    _tupleReader.SetBranches(m_branches, true);

    // ==========================================================================================================================================================================

    Prj      prj      = _eventType.GetProject();
    Analysis ana      = _eventType.GetAna();
    Year     year     = _eventType.GetYear();
    Polarity polarity = _eventType.GetPolarity();

    map< TString, TString > _names = _eventType.GetParticleNames();
    map< TString, int >     _ids   = _eventType.GetParticleIDs();
    if (m_debug) {
        for (const auto & _name : _names) { MessageSvc::Debug("GetParticleNames", _name.first, "->", _name.second); }
        for (const auto & _id : _ids) { MessageSvc::Debug("GetParticleIDs", _id.first, "->", to_string(_id.second)); }
    }

    TString _head   = _names["HEAD"];
    uint    _headID = _ids["HEAD"];

    TString _diHad = _names["HH"];

    TString _diHadName;   // TODO: should probably be part of the _names stuff
    switch (prj) {
        case Prj::RKst: _diHadName = "Kst_892_0"; break;
        case Prj::RK: _diHadName = "K"; break;
        case Prj::RPhi: _diHadName = "phi_1020"; break;
        default: MessageSvc::Error("Wrong project", to_string(prj), "EXIT_FAILURE"); break;
    }

    TString _diLep = _names["LL"];

    TString _had1   = _names["H1"];
    uint    _had1ID = _ids["H1"];

    TString _had2   = _names["H2"];
    uint    _had2ID = _ids["H2"];

    TString _had1Name = ((TString) _names["H1"]).ReplaceAll("1", "");   // The "base" particle names are needed for
    TString _had2Name = ((TString) _names["H2"]).ReplaceAll("2", "");   // finding the correct PID weight maps

    TString _lep    = ((TString) _names["L1"]).Remove(1);
    TString _lep1   = ((TString) _names["L1"]);
    TString _lep2   = ((TString) _names["L2"]);
    uint    _lepID  = _ids["L1"];
    uint    _lep1ID = _ids["L1"];
    uint    _lep2ID = _ids["L2"];

    // ==========================================================================================================================================================================

    // Event
    InfoEvent _infoEvent = InfoEvent(_tupleReader);

    // Particle
    InfoParticle _infoParticleHEAD  = InfoParticle(_tupleReader, _head);
    InfoParticle _infoParticleDIHAD = InfoParticle(_tupleReader, _diHad);
    InfoParticle _infoParticleH1    = InfoParticle(_tupleReader, _had1);
    InfoParticle _infoParticleH2    = InfoParticle(_tupleReader, _had2);
    InfoParticle _infoParticleDILEP = InfoParticle(_tupleReader, _diLep);
    InfoParticle _infoParticleL1    = InfoParticle(_tupleReader, _lep1);
    InfoParticle _infoParticleL2    = InfoParticle(_tupleReader, _lep2);

    // Particle momentum
    InfoMomentum _infoMomentumHEAD = _infoParticleHEAD.P;
    InfoMomentum _infoMomentumH1   = _infoParticleH1.P;
    InfoMomentum _infoMomentumH2   = _infoParticleH2.P;
    InfoMomentum _infoMomentumL1   = _infoParticleL1.P;
    InfoMomentum _infoMomentumL2   = _infoParticleL2.P;

    InfoMomentum _infoMomentumHEADTRUE = InfoMomentum();
    InfoMomentum _infoMomentumH1TRUE   = InfoMomentum();
    InfoMomentum _infoMomentumH2TRUE   = InfoMomentum();
    InfoMomentum _infoMomentumL1TRUE   = InfoMomentum();
    InfoMomentum _infoMomentumL2TRUE   = InfoMomentum();
    if (_eventType.IsMC()) {
        _infoMomentumHEADTRUE = InfoMomentum(_tupleReader, _head, "TRUE");
        _infoMomentumH1TRUE   = InfoMomentum(_tupleReader, _had1, "TRUE");
        _infoMomentumH2TRUE   = InfoMomentum(_tupleReader, _had2, "TRUE");
        _infoMomentumL1TRUE   = InfoMomentum(_tupleReader, _lep1, "TRUE");
        _infoMomentumL2TRUE   = InfoMomentum(_tupleReader, _lep2, "TRUE");
    }

    // Track momentum
    InfoMomentum _infoMomentumL1TRACK = InfoMomentum();
    InfoMomentum _infoMomentumL2TRACK = InfoMomentum();
    if (ana == Analysis::EE) {
        _infoMomentumL1TRACK = InfoMomentum(_tupleReader, _lep1, "TRACK");
        _infoMomentumL2TRACK = InfoMomentum(_tupleReader, _lep2, "TRACK");
    }

    // HOP mass
    InfoHOP _infoHOP = InfoHOP(_tupleReader, _head, _diHad, _diLep, _lep1, _lep2);

    // JPs constraint mass fits for K<->e and Pi<->e swaps
    InfoFitCOV _infoFitCOV = InfoFitCOV();
    if (ana == Analysis::EE) _infoFitCOV = InfoFitCOV(_tupleReader, _head, _had1, _had2, _lep1, _lep2);

    // VELO material interaction
    InfoVertex _infoVertexDILEP    = InfoVertex(_tupleReader, _diLep);
    InfoVertex _infoVertexDILEPERR = InfoVertex(_tupleReader, _diLep, "ERR");

    InfoVELO _infoVeloDIHADL1     = InfoVELO();
    InfoVELO _infoVeloDIHADL2     = InfoVELO();
    InfoVELO _infoVeloDIHADL1TRUE = InfoVELO();
    InfoVELO _infoVeloDIHADL2TRUE = InfoVELO();
    InfoVELO _infoVeloDILEPL1TRUE = InfoVELO();
    InfoVELO _infoVeloDILEPL2TRUE = InfoVELO();
    if (_eventType.GetNBodies() > 3) {
        _infoVeloDIHADL1 = InfoVELO(_tupleReader, _diHad, _lep1);
        _infoVeloDIHADL2 = InfoVELO(_tupleReader, _diHad, _lep2);
        if (_eventType.IsMC()) {
            _infoVeloDIHADL1TRUE = InfoVELO(_tupleReader, _diHad, _lep1, "TRUE");
            _infoVeloDIHADL2TRUE = InfoVELO(_tupleReader, _diHad, _lep2, "TRUE");
            _infoVeloDILEPL1TRUE = InfoVELO(_tupleReader, _diLep, _lep1, "TRUE");
            _infoVeloDILEPL2TRUE = InfoVELO(_tupleReader, _diLep, _lep2, "TRUE");
        }
    }

    // ==========================================================================================================================================================================
    // Event info
    Str2BranchMapI  _mapEventI;
    Str2BranchMapVI _mapEventBootstrap;
    Str2BranchMapD  _mapEventD;

    int _year = to_string(year).Atoi();
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "Year", _year, _tuple);

    int _polarity = polarity == Polarity::MD ? -1 : +1;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "Polarity", _polarity, _tuple);

    int _rndGroup;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "RndGroup", _rndGroup, _tuple);
    int _rndType;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "RndType", _rndType, _tuple);

    vector< int > _rndPoisson;
    _rndPoisson.resize(WeightDefRX::nBS);
    if (_addBSevt) SetBranchInfo< Str2BranchMapVI, vector< int > >(_mapEventBootstrap, "RndPoisson", _rndPoisson, _tuple, WeightDefRX::nBS);

    int _TCKCat = -1;
    if (_addTCK) SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "TCKCat", _TCKCat, _tuple);

    int _signalLikeness;
    if (_eventType.IsMC()) SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "SignalLikeness", _signalLikeness, _tuple);

    // ==========================================================================================================================================================================
    // Extra variables
    Str2BranchMapD _mapVar;

    TTreeReaderArray< float > * _massHeadDTFPV = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_M");
    double                      _massHeadDTFPV0;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_M", _massHeadDTFPV0, _tuple);
    TTreeReaderArray< float > * _massHeadDTFJPs = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_JPs_M");
    double                      _massHeadDTFJPs0;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_JPs_M", _massHeadDTFJPs0, _tuple);
    TTreeReaderArray< float > * _massHeadDTFPsi = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_Psi_M");
    double                      _massHeadDTFPsi0;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_Psi_M", _massHeadDTFPsi0, _tuple);

    TTreeReaderArray< float > * _massDiLepDTFB = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_" + _head + "_J_psi_1S_M");
    double                      _massDiLepDTFB0;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_DTF_" + _head + "_M", _massDiLepDTFB0, _tuple);
    TTreeReaderArray< float > * _massDiHadDTFB = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_" + _head + "_" + _diHadName + "_M");
    double                      _massDiHadDTFB0;
    if (_eventType.GetNBodies() > 3) SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diHad + "_DTF_" + _head + "_M", _massDiHadDTFB0, _tuple);

    TTreeReaderArray< float > * _massHeadDTFPVstatus = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_status");
    double                      _massHeadDTFPV0status;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_status", _massHeadDTFPV0status, _tuple);
    TTreeReaderArray< float > * _massHeadDTFJPsstatus = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_JPs_status");
    double                      _massHeadDTFJPs0status;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_JPs_status", _massHeadDTFJPs0status, _tuple);
    TTreeReaderArray< float > * _massHeadDTFPsistatus = _tupleReader.GetArrayPtr< float >(_head + "_DTF_PV_Psi_status");
    double                      _massHeadDTFPsi0status;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_DTF_Psi_status", _massHeadDTFPsi0status, _tuple);

    double _massDiLepTrue, _massDiLepTruePostFSR;
    if (_eventType.IsMC()) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRUEM", _massDiLepTrue, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRUEM_POSTFSR", _massDiLepTruePostFSR, _tuple);
    }

    double _massDiLepTrack, _massDiLepTrack1, _massDiLepTrack2, _massDiLepTrackPi, _massDiLepTrack1Pi, _massDiLepTrack2Pi;
    double _massDiHadTrack1Pi, _massDiHadTrack2Pi, _massDiHadTrack1K, _massDiHadTrack2K;
    double _massHeadTrack, _massHeadTrack1, _massHeadTrack2, _massHeadTrackPi, _massHeadTrack1Pi, _massHeadTrack2Pi;
    double _mass3BodyTrackPi, _mass3BodyTrackK;
    //
    // B0   ==> ^(J/psi(1S) ==> ^l+ ^l-) ^(K*(892)0  ==> ^K+ ^pi-)
    // B_s0 ==> ^(J/psi(1S) ==> ^l+ ^l-) ^(phi(1020) ==> ^K+ ^K-)
    // DecayTreeTuple            L1  L2                   K   Pi
    // TupleToolSubMass          1   0                    2   3
    //
    // B+ ==> ^(J/psi(1S) ==> ^l+ ^l-) ^K+
    // DecayTreeTuple          L1  L2   K
    // TupleToolSubMass        1   0    2
    //
    if (ana == Analysis::EE) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK_M", _massDiLepTrack, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK1_M", _massDiLepTrack1, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK2_M", _massDiLepTrack2, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK_M_Subst_e2pi", _massDiLepTrackPi, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK1_M_Subst_e2pi", _massDiLepTrack1Pi, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK2_M_Subst_e2pi", _massDiLepTrack2Pi, _tuple);

        // WRONG NAME
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M02_Subst0_e2pi", _massDiHadTrack1Pi, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M12_Subst1_e2pi", _massDiHadTrack2Pi, _tuple);
        // SHOULD INSTEAD BE
        // SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M12_Subst0_e2pi", _massDiHadTrack1Pi, _tuple);
        // SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M02_Subst1_e2pi", _massDiHadTrack2Pi, _tuple);

        if (_eventType.GetNBodies() > 3) {
            // WRONG NAME
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M03_Subst0_e2K", _massDiHadTrack1K, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M13_Subst1_e2K", _massDiHadTrack2K, _tuple);
            // SHOULD INSTEAD BE
            // SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M13_Subst0_e2K", _massDiHadTrack1K, _tuple);
            // SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M03_Subst1_e2K", _massDiHadTrack2K, _tuple);
        }

        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M", _massHeadTrack, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK1_M", _massHeadTrack1, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK2_M", _massHeadTrack2, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_e2pi", _massHeadTrackPi, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK1_M_Subst_e2pi", _massHeadTrack1Pi, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK2_M_Subst_e2pi", _massHeadTrack2Pi, _tuple);

        if (_eventType.GetNBodies() > 3) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M012_Subst01_e2pi", _mass3BodyTrackPi, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M013_Subst01_e2K", _mass3BodyTrackK, _tuple);
        }
    }

    // ==========================================================================================================================================================================
    // Opening angles
    double _L1L2_OA, _H1H2_OA, _L1H1_OA, _L1H2_OA, _L2H1_OA, _L2H2_OA;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, "L1L2_OA", _L1L2_OA, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, "L1H1_OA", _L1H1_OA, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, "L2H1_OA", _L2H1_OA, _tuple);
    if (_eventType.GetNBodies() > 3) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, "H1H2_OA", _H1H2_OA, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, "L1H2_OA", _L1H2_OA, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, "L2H2_OA", _L2H2_OA, _tuple);
    }

    // ==========================================================================================================================================================================
    // Helicity angles
    double _thetaL, _thetaK, _phi;
    double _trueThetaL, _trueThetaK, _truePhi;
    if (_eventType.GetNBodies() == 3) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_ThetaL_custom", _thetaL, _tuple);
        if (_eventType.IsMC()) { SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaL_custom", _trueThetaL, _tuple); }
    } else {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_ThetaL_custom", _thetaL, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_ThetaK_custom", _thetaK, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_Phi_custom", _phi, _tuple);
        if (_eventType.IsMC()) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaL_custom", _trueThetaL, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaK_custom", _trueThetaK, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEPhi_custom", _truePhi, _tuple);
        }
    }

    // ==========================================================================================================================================================================
    // CORR mass
    double _massHeadCorr, _massDiHadCorr, _massDiLepCorr;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_CORR_M", _massHeadCorr, _tuple);
    if (_eventType.GetNBodies() > 3) SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diHad + "_CORR_M", _massDiHadCorr, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_CORR_M", _massDiLepCorr, _tuple);

    // ==========================================================================================================================================================================
    // HOP mass
    double _alphaHOP, _massHeadHOP, _massDiLepHOP;
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_HOP_alpha", _alphaHOP, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_HOP_M", _massHeadHOP, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_HOP_M", _massDiLepHOP, _tuple);

    // ==========================================================================================================================================================================
    // JPs constraint mass fits for K<->e and Pi<->e swaps
    double _jpsMassH1L1BConstr, _bMassH1L1JPsConstr;
    double _jpsMassH2L2swapBConstr, _bMassH2L2swapJPsConstr;
    double _bMassH1L1PsiConstr, _bMassH2L2swapPsiConstr;
    if (ana == Analysis::EE) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK_M_Subst_Kl2lK_DTF_" + _head, _jpsMassH1L1BConstr, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_Kl2lK_DTF_" + _diLep, _bMassH1L1JPsConstr, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_Kl2lK_DTF_Psi", _bMassH1L1PsiConstr, _tuple);
        if (prj == Prj::RKst) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK_M_Subst_lpi2pil_DTF_" + _head, _jpsMassH2L2swapBConstr, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_lpi2pil_DTF_" + _diLep, _bMassH2L2swapJPsConstr, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_lpi2pil_DTF_Psi", _bMassH2L2swapPsiConstr, _tuple);
        }
        if (prj == Prj::RPhi) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRACK_M_Subst_lK2Kl_DTF_" + _head, _jpsMassH2L2swapBConstr, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_lK2Kl_DTF_" + _diLep, _bMassH2L2swapJPsConstr, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRACK_M_Subst_lK2Kl_DTF_Psi", _bMassH2L2swapPsiConstr, _tuple);
        }
    }

    // ==========================================================================================================================================================================
    // VELO material interaction
    TString _veloPars   = "1_pars";
    int     _veloHalf   = 0;
    int     _veloMethod = 0;
    int     _veloDir    = 0;

    ModuleMaterial _moduleMaterial(_eventType.GetWeightHolder().GetVeloPars(_veloPars).Data());
    FoilMaterial   _foilMaterial(_eventType.GetWeightHolder().GetVeloPars(_veloPars).Data());
    VeloMaterial   _veloMaterial(_eventType.GetWeightHolder().GetVeloPars(_veloPars).Data());

    double _diLepMaterialX, _diLepMaterialY, _diLepMaterialZ, _diLepMaterialD;
    double _diLepFoilX, _diLepFoilY, _diLepFoilZ, _diLepFoilD;
    double _diLepVelo;
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_MMI_X", _diLepMaterialX, _tuple);
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_MMI_Y", _diLepMaterialY, _tuple);
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_MMI_Z", _diLepMaterialZ, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_MMI_D", _diLepMaterialD, _tuple);
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_FMI_X", _diLepFoilX, _tuple);
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_FMI_Y", _diLepFoilY, _tuple);
    // SetBranchInfo <Str2BranchMapD, double>(_mapVar, _diLep + "_FMI_Z", _diLepFoilZ, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_FMI_D", _diLepFoilD, _tuple);
    SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_VMI_D", _diLepVelo, _tuple);

    double _fheL1diHad, _fheL2diHad;
    double _fheL1diHadTrue, _fheL2diHadTrue;
    double _fheL1diLepTrue, _fheL2diLepTrue;
    if ((ana == Analysis::EE) || (ana == Analysis::ME)) {
        if (_eventType.GetNBodies() > 3) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep1 + "_" + _diHad + "_FEH", _fheL1diHad, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep2 + "_" + _diHad + "_FEH", _fheL2diHad, _tuple);
            if (_eventType.IsMC()) {
                SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep1 + "_" + _diHad + "_TRUEFEH", _fheL1diHadTrue, _tuple);
                SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep2 + "_" + _diHad + "_TRUEFEH", _fheL2diHadTrue, _tuple);
                SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep1 + "_" + _diLep + "_TRUEFEH", _fheL1diLepTrue, _tuple);
                SetBranchInfo< Str2BranchMapD, double >(_mapVar, _lep2 + "_" + _diLep + "_TRUEFEH", _fheL2diLepTrue, _tuple);
            }
        }
    }

    

    
    // ==========================================================================================================================================================================
    // MC weights
    Str2BranchMapD _mapMC;

    // Part-Reco sWeights
    double  _partReco;
    TString _partRecoMass;
    TH1D *  _partRecoMap = nullptr;
    map< pair< int, int >, double > _partRecoMCDT;
    if (_addWPREC && !_eventType.GetSample().Contains("2KPiPiPsi") && _eventType.HasMCDecayTuple() ) {
        //We start to have a lot of 2KPi samples 
        ZippedEventType _zip = _eventType.Zip();
        _zip.cutOption       = "no";
        _zip.tupleName       = "MCT";
        EventType _etMCT     = EventType(_zip);

        TString _partRecoSample;
        TString _partRecoID;
        if (_eventType.GetSample().Contains("2KPiPi")) {
            _partRecoSample = "Bu2KPiPiMM";
            _partRecoID     = "K_1_1270";
            _partRecoMass   = "TMath::Sqrt(TMath::Sq(" + _partRecoID + "_TRUEP_E)-(TMath::Sq(" + _partRecoID + "_TRUEP_X)+TMath::Sq(" + _partRecoID + "_TRUEP_Y)+TMath::Sq(" + _partRecoID + "_TRUEP_Z)))";
        } else if (_eventType.GetSample().Contains("2KPi")) {
            _partRecoSample = "Bu2KPiMM";
            _partRecoID     = "KPi";
            _partRecoMass   = "TMath::Sqrt(TMath::Sq(K_TRUEP_E+Pi_TRUEP_E)-(TMath::Sq(K_TRUEP_X+Pi_TRUEP_X)+TMath::Sq(K_TRUEP_Y+Pi_TRUEP_Y)+TMath::Sq(K_TRUEP_Z+Pi_TRUEP_Z)))";
        } else {
            MessageSvc::Error("AddPartReco", _eventType.GetSample(), "not implemented", "EXIT_FAILURE");
        }
        MessageSvc::Info("AddPartReco", "Use sWeights from", _partRecoSample, "for", _partRecoID, "hadronic system");
        _partRecoMap = (TH1D *) _eventType.GetWeightHolder().GetWeightPartReco(_partRecoSample);

        double _min = 0;
        for (int i = 0; i <= _partRecoMap->GetNcells() + 1; ++i) { _min = _partRecoMap->GetBinContent(i) < _min ? _partRecoMap->GetBinContent(i) : _min; }
        if (_min < 0) {
            for (int i = 0; i <= _partRecoMap->GetNcells() + 1; ++i) { _partRecoMap->SetBinContent(i, _partRecoMap->GetBinContent(i) - _min); }
        }

        RooRealVar _mass = RooRealVar(_partRecoMass, "", _partRecoMap->GetXaxis()->GetXmin(), _partRecoMap->GetXaxis()->GetXmax());
        _mass.setBins(_partRecoMap->GetNbinsX());

        TH1D * _partRecoNorm = (TH1D *) GetHistogram(*_etMCT.GetTuple(), TCut(NOCUT), "", _mass);
        MessageSvc::Info("AddPartReco", _partRecoNorm);

        _partRecoMap->Divide(_partRecoNorm);
        ScaleHistogram(*_partRecoMap);
        MessageSvc::Info("AddPartReco", _partRecoMap);

        EnableMultiThreads();
        auto _df      = ROOT::RDataFrame(*_etMCT.GetTuple());
        auto _dd      = _df.Define("RUNNUMBER", "(int) runNumber").Define("EVENTNUMBER", "(int) eventNumber").Define("PARTRECOMASS", ("(double) " + _partRecoMass).Data());
        auto _rnumber = _dd.Take< int >("RUNNUMBER");
        auto _enumber = _dd.Take< int >("EVENTNUMBER");
        auto _prmass  = _dd.Take< double >("PARTRECOMASS");
        MessageSvc::Info("GetTupleProcessDT", (TString) "Extracting (RunNumber,EventNumber,PartRecoMass) from MCDecayTuple - START");
        *_dd.Count();
        for (int j = 0; j < _enumber->size(); ++j) {
            auto _pair = make_pair(_rnumber->at(j), _enumber->at(j));
            if (_partRecoMCDT.find(_pair) != _partRecoMCDT.end()) {
                MessageSvc::Warning("GetTupleProcessDT", (TString) "Multiple Candidate found at(" + to_string(j) + "), keeping last entry found for partRecoMass in this event");
                _partRecoMCDT[_pair] = _prmass->at(j);
            } else {
                _partRecoMCDT[_pair] = _prmass->at(j);
            }
        }
        MessageSvc::Info("GetTupleProcessDT", (TString) "Extracting (RunNumber,EventNumber,PartRecoMass) from MCDecayTuple - STOP");
        DisableMultiThreads();

        SetBranchInfo< Str2BranchMapD, double >(_mapMC, "w" + WeightDefRX::ID::PTRECO, _partReco, _tuple);
    }

    // ==========================================================================================================================================================================
    // Add new branches
    if (!AddMap(_mapEventI, "Extra event info", _tuple)) { _mapEventI.clear(); }
    if (!AddMap(_mapEventBootstrap, "Extra event info", _tuple)) { _mapEventBootstrap.clear(); }
    if (!AddMap(_mapEventD, "Extra event info", _tuple)) { _mapEventD.clear(); }
    if (!AddMap(_mapVar, "Extra variables", _tuple)) { _mapVar.clear(); }   
    if (!AddMap(_mapMC,  "MC corrections", _tuple)) { _addWPREC = false; }

    if (_tuple->GetListOfBranches() != nullptr) {
        MessageSvc::Line();
        MessageSvc::Info("TupleProcess", (TString) "Branches", to_string(_tuple->GetNbranches()));
        MessageSvc::Line();
    }

    // ==========================================================================================================================================================================
    // Aliases
    vector< pair< TString, TString > > _aliasHlt1 = {};
    vector< pair< TString, TString > > _aliasHlt2 = {};
    if (SettingDef::Tuple::aliases) {
        _aliasHlt1 = GetAliasHLT1(to_string(prj), to_string(ana), to_string(year));
        _aliasHlt2 = GetAliasHLT2(to_string(prj), to_string(ana), to_string(year));
    }

    AddAlias(m_aliasesSKIM, "Event", _tuple);
    AddAlias(_aliasHlt1, "HLT1", _tuple);
    AddAlias(_aliasHlt2, "HLT2", _tuple);

    if (_tuple->GetListOfAliases() != nullptr) {
        MessageSvc::Line();
        MessageSvc::Info("TupleProcess", (TString) "Aliases", to_string(_tuple->GetListOfAliases()->GetSize()));
        MessageSvc::Line();
    }

    // ==========================================================================================================================================================================
    // loop over events and fill new branches
    Long64_t _nEntries = _tupleReader.GetEntries();
    _tupleReader.Tuple()->LoadTree(0);
    MessageSvc::Line();
    MessageSvc::Info("TupleProcess", (TString) "Looping ...", to_string(_nEntries));

    Long64_t i = 0;
    while (_tupleReader.Reader()->Next()) {
        if (!m_debug) MessageSvc::ShowPercentage(i, _nEntries);

        _tupleReader.GetEntry(i);
        _tupleReader.Reader()->SetEntry(i);

        // ==========================================================================================================================================================================
        // Event info
        if (_mapEventI.size() != 0) {
            if (m_debug && (i == 0)) MessageSvc::Debug("TupleProcess", (TString) "Add mapEvent");

            _rndGroup = (int) floor(m_rnd.Rndm() * m_kFolds);
            _rndType  = (int) floor(m_rnd.Rndm() * m_kFolds);
        }
        if (_mapEventBootstrap.size() != 0) {
            if (_addBSevt) {
                _rndPoisson.clear();
                m_rndPoisson.SetSeed(**_infoEvent.RN * **_infoEvent.EN);
                for (int j = 0; j < WeightDefRX::nBS; ++j) { _rndPoisson.push_back((int) m_rndPoisson.PoissonD(1)); }
            }
        }
        if (_addTCK) {
            if (year == Year::Y2015) {
                _TCKCat = 0;
            } else if (year == Year::Y2016) {
                //////////////////////////////////////////////
                // 2016
                //
                // TCKCat 0, b = 1.1 (default on MC)
                // TCKCat 1, b = 1.6
                // TCKCat 2, b = 2.3
                //
                // MD : 89.8% (1.1) :  0.0% (1.6) : 10.2% (2.3)
                // MU : 30.0% (1.1) : 15.5% (1.6) : 54.5% (2.3)
                //////////////////////////////////////////////
                float _frac_TCKCat1, _frac_TCKCat2;
                switch (polarity) {
                    case Polarity::MD:
                        _frac_TCKCat1 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMD_TCKcat1;   // 0.000
                        _frac_TCKCat2 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMD_TCKcat2;   // 0.102
                        break;
                    case Polarity::MU:
                        _frac_TCKCat1 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMU_TCKcat1;   // 0.155
                        _frac_TCKCat2 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMU_TCKcat2;   // 0.545
                        break;
                    default: MessageSvc::Error("TupleProcess", (TString) "Invalid polarity", to_string(polarity), "EXIT_FAILURE"); break;
                }

                m_rndTCK.SetSeed(**_infoEvent.RN * **_infoEvent.EN);
                double _rndTCK = m_rndTCK.Rndm();

                _TCKCat = 0;
                if (_rndTCK < _frac_TCKCat2) {
                    _TCKCat = 2;
                } else if (_rndTCK < (_frac_TCKCat2 + _frac_TCKCat1)) {
                    _TCKCat = 1;
                }
            }
        }
        
        if (_eventType.IsMC()) {
            _signalLikeness = 0;
            if (_eventType.GetNBodies() == 3) {
                _signalLikeness = GetSignalLikeness(**_infoParticleH1.TID, 0, **_infoParticleL1.TID, **_infoParticleL2.TID);
            } else {
                _signalLikeness = GetSignalLikeness(**_infoParticleH1.TID, **_infoParticleH2.TID, **_infoParticleL1.TID, **_infoParticleL2.TID);
            }
        }

        // ==========================================================================================================================================================================
        // Extra variables
        if (_mapVar.size() != 0) {
            if (m_debug && (i == 0)) MessageSvc::Debug("TupleProcess", (TString) "Add mapVar");

            _massHeadDTFPV0  = (*_massHeadDTFPV)[0];
            _massHeadDTFJPs0 = (*_massHeadDTFJPs)[0];
            _massHeadDTFPsi0 = (*_massHeadDTFPsi)[0];

            _massDiLepDTFB0 = (*_massDiLepDTFB)[0];
            if (_massDiHadDTFB != nullptr) _massDiHadDTFB0 = (*_massDiHadDTFB)[0];

            _massHeadDTFPV0status  = (*_massHeadDTFPVstatus)[0];
            _massHeadDTFJPs0status = (*_massHeadDTFJPsstatus)[0];
            _massHeadDTFPsi0status = (*_massHeadDTFPsistatus)[0];

            if (_eventType.IsMC()) {
                TLorentzVector _headT = _infoMomentumHEADTRUE.LV();
                TLorentzVector _H1T   = _infoMomentumH1TRUE.LV();
                TLorentzVector _H2T   = _infoMomentumH2TRUE.LV();
                TLorentzVector _L1T   = _infoMomentumL1TRUE.LV();
                TLorentzVector _L2T   = _infoMomentumL2TRUE.LV();

                TLorentzVector LV_diLep  = _L1T + _L2T;
                TLorentzVector LV_diLepD = _headT - _H1T;
                if (_eventType.GetNBodies() > 3) { LV_diLepD -= _H2T; }

                _massDiLepTruePostFSR = LV_diLep.M();
                _massDiLepTrue        = LV_diLepD.M();
            }

            // calculate masses using electron track momenta
            if (ana == Analysis::EE) {
                TLorentzVector LV_H1    = _infoMomentumH1.LV();
                TLorentzVector LV_H2    = _infoMomentumH2.LV();
                TLorentzVector LV_E1    = _infoMomentumL1.LV();
                TLorentzVector LV_E2    = _infoMomentumL2.LV();
                TLorentzVector LV_E1T   = _infoMomentumL1TRACK.LV("E");
                TLorentzVector LV_E2T   = _infoMomentumL2TRACK.LV("E");
                TLorentzVector LV_E1TPi = _infoMomentumL1TRACK.LV("Pi");
                TLorentzVector LV_E2TPi = _infoMomentumL2TRACK.LV("Pi");
                TLorentzVector LV_E1TK  = _infoMomentumL1TRACK.LV("K");
                TLorentzVector LV_E2TK  = _infoMomentumL2TRACK.LV("K");

                TLorentzVector LV_diLepT    = LV_E1T + LV_E2T;
                TLorentzVector LV_diLepT1   = LV_E1T + LV_E2;
                TLorentzVector LV_diLepT2   = LV_E1 + LV_E2T;
                TLorentzVector LV_diLepTPi  = LV_E1TPi + LV_E2TPi;
                TLorentzVector LV_diLepT1Pi = LV_E1TPi + LV_E2;
                TLorentzVector LV_diLepT2Pi = LV_E1 + LV_E2TPi;

                TLorentzVector LV_diHadT1Pi = LV_E1TPi + LV_H1;
                TLorentzVector LV_diHadT2Pi = LV_E2TPi + LV_H1;
                TLorentzVector LV_diHadT1K  = LV_E1TK;
                TLorentzVector LV_diHadT2K  = LV_E2TK;
                if (_eventType.GetNBodies() > 3) {
                    LV_diHadT1K += LV_H2;
                    LV_diHadT2K += LV_H2;
                }

                TLorentzVector LV_headT    = LV_diLepT + LV_H1;
                TLorentzVector LV_headT1   = LV_diLepT1 + LV_H1;
                TLorentzVector LV_headT2   = LV_diLepT2 + LV_H1;
                TLorentzVector LV_headTPi  = LV_diLepTPi + LV_H1;
                TLorentzVector LV_headT1Pi = LV_diLepT1Pi + LV_H1;
                TLorentzVector LV_headT2Pi = LV_diLepT2Pi + LV_H1;
                if (_eventType.GetNBodies() > 3) {
                    LV_headT += LV_H2;
                    LV_headT1 += LV_H2;
                    LV_headT2 += LV_H2;
                    LV_headTPi += LV_H2;
                    LV_headT1Pi += LV_H2;
                    LV_headT2Pi += LV_H2;
                }

                TLorentzVector LV_3BodyTPi = LV_E1TPi + LV_E2TPi + LV_H1;
                TLorentzVector LV_3BodyTK  = LV_E1TPi + LV_E2TPi + LV_H2;

                _massDiLepTrack    = LV_diLepT.M();
                _massDiLepTrack1   = LV_diLepT1.M();
                _massDiLepTrack2   = LV_diLepT2.M();
                _massDiLepTrackPi  = LV_diLepTPi.M();
                _massDiLepTrack1Pi = LV_diLepT1Pi.M();
                _massDiLepTrack2Pi = LV_diLepT2Pi.M();

                _massDiHadTrack1Pi = LV_diHadT1Pi.M();
                _massDiHadTrack2Pi = LV_diHadT2Pi.M();
                if (_eventType.GetNBodies() > 3) {
                    _massDiHadTrack1K = LV_diHadT1K.M();
                    _massDiHadTrack2K = LV_diHadT2K.M();
                }

                _massHeadTrack    = LV_headT.M();
                _massHeadTrack1   = LV_headT1.M();
                _massHeadTrack2   = LV_headT2.M();
                _massHeadTrack2   = LV_headT2.M();
                _massHeadTrackPi  = LV_headTPi.M();
                _massHeadTrack1Pi = LV_headT1Pi.M();
                _massHeadTrack2Pi = LV_headT2Pi.M();

                if (_eventType.GetNBodies() > 3) {
                    _mass3BodyTrackPi = LV_3BodyTPi.M();
                    _mass3BodyTrackK  = LV_3BodyTK.M();
                }

                if (ana == Analysis::EE) {
                    if (SettingDef::Tuple::option == "gng") {
                        if (_infoFitCOV.HEAD_ID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD ID is nullptr", "EXIT_FAILURE");
                        if ((_infoFitCOV.L1_COV.PCOV00 == nullptr) || (_infoFitCOV.L2_COV.PCOV00 == nullptr)) MessageSvc::Error("TupleProcess", (TString) "PCOV is nullptr", "EXIT_FAILURE");
                        if ((_infoFitCOV.H1_COV.TCOV00 == nullptr) || (_infoFitCOV.H2_COV.TCOV00 == nullptr)) MessageSvc::Error("TupleProcess", (TString) "TCOV is nullptr", "EXIT_FAILURE");
                        ConstraintMassFit(_infoFitCOV, _jpsMassH1L1BConstr, _bMassH1L1JPsConstr, _bMassH1L1PsiConstr, _jpsMassH2L2swapBConstr, _bMassH2L2swapJPsConstr, _bMassH2L2swapPsiConstr, to_string(prj));
                    }
                }
            }

            // ==========================================================================================================================================================================
            // Opening angles
            _L1L2_OA = _infoMomentumL1.LV().Angle(_infoMomentumL2.LV().Vect());
            _L1H1_OA = _infoMomentumL1.LV().Angle(_infoMomentumH1.LV().Vect());
            _L2H1_OA = _infoMomentumL2.LV().Angle(_infoMomentumH1.LV().Vect());
            if (_eventType.GetNBodies() > 3) {
                _H1H2_OA = _infoMomentumH1.LV().Angle(_infoMomentumH2.LV().Vect());
                _L1H2_OA = _infoMomentumL1.LV().Angle(_infoMomentumH2.LV().Vect());
                _L2H2_OA = _infoMomentumL2.LV().Angle(_infoMomentumH2.LV().Vect());
            }

            // ==========================================================================================================================================================================
            // Helicity angles for h1 l1 l2
            if (_eventType.GetNBodies() == 3) {
                if (_infoParticleHEAD.ID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD ID is nullptr", "EXIT_FAILURE");
                bool _h1Charge = (**_infoParticleH1.ID > 0) ? true : false;   // true for b, false for bbar
                // if L1 = Lp
                if (**_infoParticleL1.ID < 0) {
                    HelicityAngles3Bodies(_h1Charge, _infoMomentumL1, _infoMomentumL2, _infoMomentumH1, _thetaL);
                } else {
                    HelicityAngles3Bodies(_h1Charge, _infoMomentumL2, _infoMomentumL1, _infoMomentumH1, _thetaL);
                }

                if (_eventType.IsMC()) {
                    if (_infoParticleHEAD.TID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD TrueID is nullptr", "EXIT_FAILURE");
                    _trueThetaL = 10000.;
                    if (TMath::Abs(**_infoParticleHEAD.TID) == _headID) {
                        bool _h1ChargeMC = (**_infoParticleH1.TID > 0) ? true : false;   // true for b, false for bbar
                        // if L1 = Lp
                        if (**_infoParticleL1.TID < 0) {
                            HelicityAngles3Bodies(_h1ChargeMC, _infoMomentumL1TRUE, _infoMomentumL2TRUE, _infoMomentumH1TRUE, _trueThetaL);
                        } else {
                            HelicityAngles3Bodies(_h1ChargeMC, _infoMomentumL2TRUE, _infoMomentumL1TRUE, _infoMomentumH1TRUE, _trueThetaL);
                        }
                    }
                }
            }
            // Helicity angles for h1 h2 l1 l2
            else {
                if (_infoParticleHEAD.ID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD ID is nullptr", "EXIT_FAILURE");
                bool _h1Charge = (**_infoParticleH1.ID > 0) ? true : false;   // true for b, false for bbar
                // If L1 = Lp
                if (**_infoParticleL1.ID < 0) {
                    HelicityAngles4Bodies(_h1Charge, _infoMomentumL1, _infoMomentumL2, _infoMomentumH1, _infoMomentumH2, _thetaL, _thetaK, _phi);
                } else {
                    HelicityAngles4Bodies(_h1Charge, _infoMomentumL2, _infoMomentumL1, _infoMomentumH1, _infoMomentumH2, _thetaL, _thetaK, _phi);
                }

                if (_eventType.IsMC()) {
                    if (_infoParticleHEAD.TID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD TrueID is nullptr", "EXIT_FAILURE");

                    // Helicity angles from true momenta
                    _trueThetaL = 10000.;
                    _trueThetaK = 10000.;
                    _truePhi    = 10000.;
                    if (TMath::Abs(**_infoParticleHEAD.TID) == _headID) {
                        bool _h1ChargeMC = (**_infoParticleH1.TID > 0) ? true : false;   // true for b, false for bbar
                        // if L1 = Lp
                        if (**_infoParticleL1.TID < 0) {
                            HelicityAngles4Bodies(_h1ChargeMC, _infoMomentumL1TRUE, _infoMomentumL2TRUE, _infoMomentumH1TRUE, _infoMomentumH2TRUE, _trueThetaL, _trueThetaK, _truePhi);
                        } else {
                            HelicityAngles4Bodies(_h1ChargeMC, _infoMomentumL2TRUE, _infoMomentumL1TRUE, _infoMomentumH1TRUE, _infoMomentumH2TRUE, _trueThetaL, _trueThetaK, _truePhi);
                        }
                    }
                }
            }

            // ==========================================================================================================================================================================
            // calculate momentum/dira corrected mass
            _massHeadCorr = GetMCORR(**_infoParticleHEAD.M, _infoParticleHEAD.PTOT(), **_infoParticleHEAD.DIRA);
            if (_eventType.GetNBodies() > 3) _massDiHadCorr = GetMCORR(**_infoParticleDIHAD.M, _infoParticleDIHAD.PTOT(), **_infoParticleDIHAD.DIRA);
            _massDiLepCorr = GetMCORR(**_infoParticleDILEP.M, _infoParticleDILEP.PTOT(), **_infoParticleDILEP.DIRA);

            // ==========================================================================================================================================================================
            // calculate HOP mass
            GetHOP(_infoHOP, _alphaHOP, _massHeadHOP, _massDiLepHOP);

            // ==========================================================================================================================================================================
            // calculate VELO material interaction
            TLorentzVector _distanceModule = _moduleMaterial.distance(**_infoVertexDILEP.X, **_infoVertexDILEP.Y, **_infoVertexDILEP.Z, **_infoVertexDILEPERR.X, **_infoVertexDILEPERR.Y, **_infoVertexDILEPERR.Z, _veloHalf);
            _diLepMaterialX                = _distanceModule.X();
            _diLepMaterialY                = _distanceModule.Y();
            _diLepMaterialZ                = _distanceModule.Z();
            _diLepMaterialD                = _distanceModule.T();

            TLorentzVector _distanceFoil = _foilMaterial.distance(**_infoVertexDILEP.X, **_infoVertexDILEP.Y, **_infoVertexDILEP.Z, **_infoVertexDILEPERR.X, **_infoVertexDILEPERR.Y, **_infoVertexDILEPERR.Z, _veloHalf, _veloMethod, _veloDir);
            _diLepFoilX                  = _distanceFoil.X();
            _diLepFoilY                  = _distanceFoil.Y();
            _diLepFoilZ                  = _distanceFoil.Z();
            _diLepFoilD                  = _distanceFoil.T();

            _diLepVelo = _veloMaterial.distance(**_infoVertexDILEP.X, **_infoVertexDILEP.Y, **_infoVertexDILEP.Z, **_infoVertexDILEPERR.X, **_infoVertexDILEPERR.Y, **_infoVertexDILEPERR.Z);

            if ((ana == Analysis::EE) || (ana == Analysis::ME)) {
                if (_eventType.GetNBodies() > 3) {
                    _fheL1diHad = GetFEH(_infoVeloDIHADL1, _veloMaterial);
                    _fheL2diHad = GetFEH(_infoVeloDIHADL2, _veloMaterial);
                    if (_eventType.IsMC()) {
                        _fheL1diHadTrue = GetFEH(_infoVeloDIHADL1TRUE, _veloMaterial);
                        _fheL2diHadTrue = GetFEH(_infoVeloDIHADL2TRUE, _veloMaterial);
                        _fheL1diLepTrue = GetFEH(_infoVeloDILEPL1TRUE, _veloMaterial);
                        _fheL2diLepTrue = GetFEH(_infoVeloDILEPL2TRUE, _veloMaterial);
                    }
                }
            }
        }

        if (_eventType.IsMC()) {            
            // ==========================================================================================================================================================================
            // Part-Reco sWeights
            // ==========================================================================================================================================================================
            if (_addWPREC) {
                if (m_debug && (i == 0)) MessageSvc::Debug("TupleProcess", (TString) "Add wPREC");

                pair< int, int > _ID = make_pair((int) **_infoEvent.RN, (int) **_infoEvent.EN);
                // Eventually can be skipped as the map will always contrains this...( evtNumber,runNumber)  DecayTreee fully contained in MCDecayTree.
                if (_partRecoMCDT.find(_ID) != _partRecoMCDT.end()) {
                    double _massHad = _partRecoMCDT.at(_ID);
                    _partReco       = GetHistogramVal(_partRecoMap, _massHad);
                } else {
                    MessageSvc::Warning("AddPartReco", (TString) "No true events with runNumber =", to_string(**_infoEvent.RN), "&& eventNumber =", to_string(**_infoEvent.EN));
                    _partReco = 1;
                }
            }
        }
        _tuple->Fill();

        ++i;
    }
    MessageSvc::Line();

    return _tuple;
}

TTree * TupleProcess::GetTupleProcessMCT(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "GetTupleProcessMCT", _tupleName);
    MessageSvc::Line();

    bool _addBSevt    = _eventType.HasBS() && m_bootstrapEvt;
    bool _addTCK      = _eventType.IsMC() && (_eventType.GetYear() == Year::Y2015 || _eventType.GetYear() == Year::Y2016);
    bool _addWPREC    = _eventType.IsMC() && ( _eventType.GetSample().Contains("2KPi") && !_eventType.GetSample().Contains("Bu2KPiEE") && _eventType.HasMCDecayTuple() );

    _tFile.cd();
    TTree * _tuple = (TTree *) _tupleReader.CloneTuple(_tupleName);

    // ==========================================================================================================================================================================

    Prj      prj      = _eventType.GetProject();
    Analysis ana      = _eventType.GetAna();
    Year     year     = _eventType.GetYear();
    Polarity polarity = _eventType.GetPolarity();
    TString  sample   = _eventType.GetSample();
    
    map< TString, TString > _names = _eventType.GetParticleNames();
    map< TString, int >     _ids   = _eventType.GetParticleIDs();
    if (m_debug) {
        for (const auto & _name : _names) { MessageSvc::Debug("GetParticleNames", _name.first, "->", _name.second); }
        for (const auto & _id : _ids) { MessageSvc::Debug("GetParticleIDs", _id.first, "->", to_string(_id.second)); }
    }
    TString _head   = _names["HEAD"];
    uint    _headID = _ids["HEAD"];
    if (sample.Contains("Bd")) {
        _head   = "B0";
        _headID = PDG::ID::Bd;
    } else if (sample.Contains("Bu")) {
        _head   = "Bp";
        _headID = PDG::ID::Bu;
    } else if (sample.Contains("Bs")) {
        _head   = "Bs";
        _headID = PDG::ID::Bs;
    }else if ( sample.Contains("Lb2pK")){
        _head = "Lb";
        _headID = PDG::ID::Lb;
    } else{
        MessageSvc::Error("Wrong sample", sample, "EXIT_FAILURE");
    }


    TString _diHad = _names["HH"];
    if (sample.Contains("Bd")) {
        _diHad = "Kst";
    } else if (sample.Contains("KPiPi")) {
        _diHad = "K_1_1270";
    } else if (sample.Contains("Bu")) {
        _diHad = "K";
    } else if (sample.Contains("Bs")) {
        _diHad = "Phi";
    } else if (sample.Contains("Lb")){
        _diHad = "<NOTEXISTING>";
    }else {
        MessageSvc::Error("Wrong sample", sample, "EXIT_FAILURE");
    }
    TString _diLep = _names["LL"];

    TString _had1   = _names["H1"];
    uint    _had1ID = _ids["H1"];
    
    TString _had2   = _names["H2"];
    uint    _had2ID = _ids["H2"];

    TString _had3;
    if (sample.Contains("Bd")) {
        _had1 = "K";
        _had2 = "Pi";
        _had3 = "Pi";
    } else if (sample.Contains("KPiPi")) {
        _had1 = "K";
        _had2 = "Pi1";
        _had3 = "Pi2";
    } else if (sample.Contains("Bu")) {
        _had1 = "K";
        _had2 = "K";
        _had3 = "K";
    } else if (sample.Contains("Bs")) {
        _had1 = "K1";
        _had2 = "K2";
        _had3 = "K2";
    }else if ( sample.Contains("Lb2pK")){
        _had1 = "K";
        _had2 = "p";
    } else{
        MessageSvc::Error("Wrong sample", sample, "EXIT_FAILURE");
    }

    TString _lep   = ((TString) _names["L1"]).Remove(1);
    uint    _lepID = _ids["L1"];

    TString _lep1 = _lep + "1";
    TString _lep2 = _lep + "2";
    
    // In case leptons do not exist, _lep is initialized to the head. It will never be used later, except to initialize _infoMomentumL1 and _infoMomentumL2
    if (sample.Contains("Bd2KstGEE")) {
        _lep1 = _head;
        _lep2 = _head;
        MessageSvc::Warning("TupleProcess", (TString) "Since leptons are not defined _infoMomentumL1 and _infoMomentumL2 have been initialized to _infoMomentumHEAD");
    }
    
    // ==========================================================================================================================================================================

    // Event
    InfoEvent _infoEvent = InfoEvent(_tupleReader, "TRUE");

    // Particle
    InfoParticle _infoParticleHEAD = InfoParticle(_tupleReader, _head, "TRUE");
    InfoParticle _infoParticleH1   = InfoParticle(_tupleReader, _had1, "TRUE");
    InfoParticle _infoParticleL1   = InfoParticle(_tupleReader, _lep1, "TRUE");
    InfoParticle _infoParticleL2   = InfoParticle(_tupleReader, _lep2, "TRUE");

    // Particle momentum
    InfoMomentum _infoMomentumHEAD = _infoParticleHEAD.P;
    InfoMomentum _infoMomentumH1   = InfoMomentum(_tupleReader, _had1, "TRUE");
    InfoMomentum _infoMomentumH2   = InfoMomentum(_tupleReader, _had2, "TRUE");
    InfoMomentum _infoMomentumL1   = InfoMomentum(_tupleReader, _lep1, "TRUE");
    InfoMomentum _infoMomentumL2   = InfoMomentum(_tupleReader, _lep2, "TRUE");

    // Part-Reco sWeights
    InfoMomentum _infoMomentumPRECO = InfoMomentum();

    // ==========================================================================================================================================================================
    // Event info
    Str2BranchMapI  _mapEventI;
    Str2BranchMapVI _mapEventBootstrap;
    Str2BranchMapD  _mapEventD;

    int _year = to_string(year).Atoi();
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "Year", _year, _tuple);

    int _polarity = polarity == Polarity::MD ? -1 : +1;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "Polarity", _polarity, _tuple);

    int _rndGroup;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "RndGroup", _rndGroup, _tuple);
    int _rndType;
    SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "RndType", _rndType, _tuple);

    vector< int > _rndPoisson;
    _rndPoisson.resize(WeightDefRX::nBS);
    if (_addBSevt) SetBranchInfo< Str2BranchMapVI, vector< int > >(_mapEventBootstrap, "RndPoisson", _rndPoisson, _tuple, WeightDefRX::nBS);

    int _TCKCat = -1;
    if (_addTCK) { SetBranchInfo< Str2BranchMapI, int >(_mapEventI, "TCKCat", _TCKCat, _tuple); }

    // ==========================================================================================================================================================================
    // Extra variables
    Str2BranchMapD _mapVar;

    double _massDiLep, _massDiLepPostFSR;
    if (!sample.Contains("Bd2KstGEE")) {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRUEM", _massDiLep, _tuple);
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _diLep + "_TRUEM_POSTFSR", _massDiLepPostFSR, _tuple);
    }
    
    // ==========================================================================================================================================================================
    // helicity angles
    double _thetaL, _thetaK, _phi;
    if (_eventType.GetNBodies() > 3){
        if (!sample.Contains("Bd2KstGEE")) {
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaL_custom", _thetaL, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaK_custom", _thetaK, _tuple);
            SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEPhi_custom", _phi, _tuple);
        }
    } else {
        SetBranchInfo< Str2BranchMapD, double >(_mapVar, _head + "_TRUEThetaL_custom", _thetaL, _tuple);
    }

    // ==========================================================================================================================================================================
    // MC weights
    Str2BranchMapD _mapMC;

    // Part-Reco sWeights
    double _partReco;
    TH1D * _partRecoMap = nullptr;
    if (_addWPREC) {
        TString _partRecoSample;
        TString _partRecoID;
        TString _partRecoMass;
        if (_eventType.GetSample().Contains("2KPiPi")) {
            _partRecoSample = "Bu2KPiPiMM";
            _partRecoID     = "K_1_1270";
            _partRecoMass   = "TMath::Sqrt(TMath::Sq(" + _partRecoID + "_TRUEP_E)-(TMath::Sq(" + _partRecoID + "_TRUEP_X)+TMath::Sq(" + _partRecoID + "_TRUEP_Y)+TMath::Sq(" + _partRecoID + "_TRUEP_Z)))";
        } else if (_eventType.GetSample().Contains("2KPi")) {
            _partRecoSample = "Bu2KPiMM";
            _partRecoID     = "KPi";
            _partRecoMass   = "TMath::Sqrt(TMath::Sq(K_TRUEP_E+Pi_TRUEP_E)-(TMath::Sq(K_TRUEP_X+Pi_TRUEP_X)+TMath::Sq(K_TRUEP_Y+Pi_TRUEP_Y)+TMath::Sq(K_TRUEP_Z+Pi_TRUEP_Z)))";
        } else
            MessageSvc::Error("AddPartReco", _eventType.GetSample(), "not implemented", "EXIT_FAILURE");

        MessageSvc::Info("AddPartReco", "Use sWeights from", _partRecoSample, "for", _partRecoID, "hadronic system");
        _partRecoMap = (TH1D *) _eventType.GetWeightHolder().GetWeightPartReco(_partRecoSample);

        double _min = 0;
        for (int i = 0; i <= _partRecoMap->GetNcells() + 1; ++i) { _min = _partRecoMap->GetBinContent(i) < _min ? _partRecoMap->GetBinContent(i) : _min; }
        if (_min < 0) {
            for (int i = 0; i <= _partRecoMap->GetNcells() + 1; ++i) { _partRecoMap->SetBinContent(i, _partRecoMap->GetBinContent(i) - _min); }
        }

        RooRealVar _mass = RooRealVar(_partRecoMass, "", _partRecoMap->GetXaxis()->GetXmin(), _partRecoMap->GetXaxis()->GetXmax());
        _mass.setBins(_partRecoMap->GetNbinsX());

        TH1D * _partRecoNorm = (TH1D *) GetHistogram(*_eventType.GetTuple(), TCut(NOCUT), "", _mass);

        MessageSvc::Info("AddPartReco", _partRecoNorm);

        _partRecoMap->Divide(_partRecoNorm);
        ScaleHistogram(*_partRecoMap);
        MessageSvc::Info("AddPartReco", _partRecoMap);

        SetBranchInfo< Str2BranchMapD, double >(_mapMC, "w" + WeightDefRX::ID::PTRECO, _partReco, _tuple);

        _infoMomentumPRECO = InfoMomentum(_tupleReader, _partRecoID, "TRUE");
    }

    // ==========================================================================================================================================================================
    // add new branches
    AddMap(_mapEventI, "Extra event info", _tuple);
    AddMap(_mapEventBootstrap, "Extra event info", _tuple);
    AddMap(_mapEventD, "Extra event info", _tuple);
    AddMap(_mapVar, "Extra variables", _tuple);
    if (!AddMap(_mapMC, "MC corrections", _tuple)) { _addWPREC = false; }

    if (_tuple->GetListOfBranches() != nullptr) {
        MessageSvc::Line();
        MessageSvc::Info("TupleProcess", (TString) "Branches", to_string(_tuple->GetNbranches()));
        MessageSvc::Line();
    }

    // ==========================================================================================================================================================================
    // loop over events and fill new branches
    Long64_t _nEntries = _tupleReader.GetEntries();
    _tupleReader.Tuple()->LoadTree(0);
    MessageSvc::Line();
    MessageSvc::Info("TupleProcess", (TString) "Looping ...", to_string(_nEntries));

    Long64_t i = 0;
    while (_tupleReader.Reader()->Next()) {
        if (!m_debug) MessageSvc::ShowPercentage(i, _nEntries);

        _tupleReader.GetEntry(i);

        // ==========================================================================================================================================================================
        // Event info
        if (_mapEventI.size() != 0) {
            _rndGroup = (int) floor(m_rnd.Rndm() * m_kFolds);
            _rndType  = (int) floor(m_rnd.Rndm() * m_kFolds);
        }
        if (_mapEventBootstrap.size() != 0) {
            if (_addBSevt) {
                _rndPoisson.clear();
                m_rndPoisson.SetSeed(**_infoEvent.RN * **_infoEvent.EN);
                for (int j = 0; j < WeightDefRX::nBS; ++j) { _rndPoisson.push_back((int) m_rndPoisson.PoissonD(1)); }
            }
        }

        if (_addTCK) {
            if (year == Year::Y2015) {
                _TCKCat = 0;
            } else if (year == Year::Y2016) {
                //////////////////////////////////////////////
                // 2016
                //
                // TCKCat 0, b = 1.1 (default on MC)
                // TCKCat 1, b = 1.6
                // TCKCat 2, b = 2.3
                //
                // MD : 89.8% (1.1) :  0.0% (1.6) : 10.2% (2.3)
                // MU : 30.0% (1.1) : 15.5% (1.6) : 54.5% (2.3)
                //////////////////////////////////////////////
                float _frac_TCKCat1, _frac_TCKCat2;
                switch (polarity) {
                    case Polarity::MD:
                        _frac_TCKCat1 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMD_TCKcat1;
                        _frac_TCKCat2 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMD_TCKcat2;
                        break;
                    case Polarity::MU:
                        _frac_TCKCat1 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMU_TCKcat1;
                        _frac_TCKCat2 = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_fracMU_TCKcat2;
                        break;
                    default: MessageSvc::Error("TupleProcess", (TString) "Invalid polarity", to_string(polarity), "EXIT_FAILURE"); break;
                }

                m_rndTCK.SetSeed(**_infoEvent.RN * **_infoEvent.EN);
                double _rndTCK = m_rndTCK.Rndm();

                _TCKCat = 0;
                if (_rndTCK < _frac_TCKCat2) {
                    _TCKCat = 2;
                } else if (_rndTCK < (_frac_TCKCat2 + _frac_TCKCat1)) {
                    _TCKCat = 1;
                }
            }
        }

        // ==========================================================================================================================================================================
        // Extra variables
        if (_mapVar.size() != 0) {
            TLorentzVector LV_head = _infoMomentumHEAD.LV();
            TLorentzVector LV_H1   = _infoMomentumH1.LV();
            TLorentzVector LV_H2   = _infoMomentumH2.LV();
            TLorentzVector LV_L1, LV_L2;
            if (!sample.Contains("Bd2KstGEE")) {
                LV_L1 = _infoMomentumL1.LV();
                LV_L2 = _infoMomentumL2.LV();

                TLorentzVector LV_diLep  = LV_L1 + LV_L2;
                TLorentzVector LV_diLepD = LV_head - LV_H1;
                if (_eventType.GetNBodies() > 3){
		            LV_diLepD -= LV_H2; 
        		}		
                //For x-feed samples the EventType GetNBodies() is not correct ! Should use the postFSR one which is from E1 + E2 True Kinematic, should be more correct as true q2 than the nominal one                
                _massDiLepPostFSR = LV_diLep.M();
                _massDiLep        = LV_diLepD.M();
            }

            // ==========================================================================================================================================================================
            // Helicity angles for h1 l1 l2
            if (_eventType.GetNBodies() == 3) {
                if (_infoParticleHEAD.ID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD ID is nullptr", "EXIT_FAILURE");
                _thetaL = 10000.;
                if (TMath::Abs(**_infoParticleHEAD.ID) == _headID) {
                    bool _h1Charge = (**_infoParticleH1.ID > 0) ? true : false;   // true for b, false for bbar
                    // if L1 = Lp
                    if (**_infoParticleL1.ID < 0) {
                        HelicityAngles3Bodies(_h1Charge, _infoMomentumL1, _infoMomentumL2, _infoMomentumH1, _thetaL);
                    } else {
                        HelicityAngles3Bodies(_h1Charge, _infoMomentumL2, _infoMomentumL1, _infoMomentumH1, _thetaL);
                    }
                }
                // Helicity angles for h1 h2 l1 l2
            } else {
                if (!sample.Contains("Bd2KstGEE")) {
                    _thetaL = 10000.;
                    _thetaK = 10000.;
                    _phi    = 10000.;
                    if (_infoParticleHEAD.ID == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD ID is nullptr", "EXIT_FAILURE");
                    if (TMath::Abs(**_infoParticleHEAD.ID) == _headID) {
                        bool _h1Charge = (**_infoParticleH1.ID > 0) ? true : false;   // true for b, false for bbar
                        // If L1 = Lp
                        if (**_infoParticleL1.ID < 0) {
                            HelicityAngles4Bodies(_h1Charge, _infoMomentumL1, _infoMomentumL2, _infoMomentumH1, _infoMomentumH2, _thetaL, _thetaK, _phi);
                        } else {
                            HelicityAngles4Bodies(_h1Charge, _infoMomentumL2, _infoMomentumL1, _infoMomentumH1, _infoMomentumH2, _thetaL, _thetaK, _phi);
                        }
                    }
                }
            }
        }

        // ==========================================================================================================================================================================
        // Part-Reco sWeights
        if (_addWPREC) {
            TLorentzVector _had = _infoMomentumPRECO.LV();
            _partReco           = GetHistogramVal(_partRecoMap, _had.M());
        }
        _tuple->Fill();
        ++i;
    }
    MessageSvc::Line();

    return _tuple;
}

TTree * TupleProcess::GetTupleProcessRST(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "TupleProcess", (TString) "GetTupleProcessRST", _tupleName);
    MessageSvc::Line();

    _tFile.cd();
    TTree * _tuple = (TTree *) _tupleReader.CloneTuple(_tupleName);

    // ==========================================================================================================================================================================

    Prj      prj      = _eventType.GetProject();
    Analysis ana      = _eventType.GetAna();
    Year     year     = _eventType.GetYear();
    Polarity polarity = _eventType.GetPolarity();

    map< TString, TString > _names = _eventType.GetParticleNames();
    map< TString, int >     _ids   = _eventType.GetParticleIDs();
    if (m_debug) {
        for (const auto & _name : _names) { MessageSvc::Debug("GetParticleNames", _name.first, "->", _name.second); }
        for (const auto & _id : _ids) { MessageSvc::Debug("GetParticleIDs", _id.first, "->", to_string(_id.second)); }
    }

    TString _head   = _names["HEAD"];
    uint    _headID = _ids["HEAD"];

    TString _diHad = _names["HH"];

    TString _diLep = _names["LL"];

    TString _had1   = _names["H1"];
    uint    _had1ID = _ids["H1"];

    TString _had2   = _names["H2"];
    uint    _had2ID = _ids["H2"];

    TString _had1Name = ((TString) _names["H1"]).ReplaceAll("1", "");
    TString _had2Name = ((TString) _names["H2"]).ReplaceAll("2", "");

    TString _lep    = ((TString) _names["L1"]).Remove(1);
    TString _lep1   = ((TString) _names["L1"]);
    TString _lep2   = ((TString) _names["L2"]);
    uint    _lepID  = _ids["L1"];
    uint    _lep1ID = _ids["L1"];
    uint    _lep2ID = _ids["L2"];

    // ==========================================================================================================================================================================
    // MC weights
    Str2BranchMapD               _mapMC;
    TTreeReaderValue< double > * _headTruePT  = _tupleReader.GetValuePtr< double >(_head + "_PT_TRUE");
    TTreeReaderValue< double > * _headTrueETA = _tupleReader.GetValuePtr< double >(_head + "_eta_TRUE");
    // ...

    // ==========================================================================================================================================================================
    // Add new branches
    // ...

    if (_tuple->GetListOfBranches() != nullptr) {
        MessageSvc::Line();
        MessageSvc::Info("TupleProcess", (TString) "Branches", to_string(_tuple->GetNbranches()));
        MessageSvc::Line();
    }

    // loop over events and fill new branches
    Long64_t _nEntries = _tupleReader.GetEntries();
    _tupleReader.Tuple()->LoadTree(0);
    MessageSvc::Line();

    MessageSvc::Info("TupleProcess", (TString) "Looping ...", to_string(_nEntries));

    Long64_t i = 0;
    while (_tupleReader.Reader()->Next()) {
        if (!m_debug) MessageSvc::ShowPercentage(i, _nEntries);

        _tupleReader.GetEntry(i);
        _tupleReader.Reader()->SetEntry(i);
        // MC
        if (_eventType.IsMC()) {
            // ...
            // Protect against nullptr deferencing here for MC !
            if (_headTruePT == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD TruePT is nullptr", "EXIT_FAILURE");
            if (_headTrueETA == nullptr) MessageSvc::Error("TupleProcess", (TString) "HEAD TrueETA is nullptr", "EXIT_FAILURE");

            // ...
        }
        _tuple->Fill();

        ++i;
    }
    MessageSvc::Line();

    return _tuple;
}

#endif
