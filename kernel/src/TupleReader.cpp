#ifndef TUPLEREADER_CPP
#define TUPLEREADER_CPP

#include "TupleReader.hpp"

#include "SettingDef.hpp"
#include <fstream>

#include "core.h"
#include "vec_extends.h"

TupleReader::TupleReader() {
    if (SettingDef::debug.Contains("TR")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "Default");
    SetFileRecover();
    m_addTuple = SettingDef::Tuple::addTuple;
    m_frac     = SettingDef::Tuple::frac;
}

TupleReader::TupleReader(TString _tupleName, TString _fileName) {
    if (SettingDef::debug.Contains("TR")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "TString");
    SetFileRecover();
    m_tuple = new TChain(_tupleName);
    if (_fileName != "") AddFile(_fileName);
    m_tuple->SetBranchStatus("*",1);
    m_reader   = new TTreeReader(m_tuple);
    m_addTuple = SettingDef::Tuple::addTuple;
    m_frac     = SettingDef::Tuple::frac;
}

TupleReader::TupleReader(TChain * _tuple) {
    if (SettingDef::debug.Contains("TR")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "TChain");
    SetFileRecover();
    m_tuple         = _tuple;
    m_reader        = new TTreeReader(m_tuple);
    m_addTuple      = SettingDef::Tuple::addTuple;
    m_frac          = SettingDef::Tuple::frac;
    m_isInitialized = true;
    m_fileNames.push_back(_tuple->GetCurrentFile()->GetName());
}

TupleReader::TupleReader(const TupleReader & _tupleReader) {
    if (SettingDef::debug.Contains("TR")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "TupleReader");
    SetFileRecover();
    m_tuple         = _tupleReader.Tuple();
    m_reader        = _tupleReader.Reader();
    m_addTuple      = _tupleReader.GetAddTuple();
    m_frac          = _tupleReader.GetFrac();
    m_fileNames     = _tupleReader.GetFileNames();
    m_isInitialized = _tupleReader.IsInitialized();
}

ostream & operator<<(ostream & os, const TupleReader & _tupleReader) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "TupleReader");
    MessageSvc::Line(os);
    if (_tupleReader.GetNFiles() == 0) {
        if (SettingDef::Tuple::datasetCache) { MessageSvc::Print((ostream &) os, "datasetCache"); }
    }
    MessageSvc::Print((ostream &) os, "files", _tupleReader.IsInitialized() ? to_string(_tupleReader.GetNFiles()) : "---");
    MessageSvc::Print((ostream &) os, "entries", _tupleReader.IsInitialized() ? to_string(_tupleReader.Tuple()->GetEntriesFast()) : "---");
    MessageSvc::Print((ostream &) os, "friend files", _tupleReader.IsInitialized() && _tupleReader.Tuple()->GetListOfFriends() != nullptr ? to_string(_tupleReader.Tuple()->GetListOfFriends()->GetSize()) : "---");
    MessageSvc::Print((ostream &) os, "friend entries", _tupleReader.IsInitialized() && _tupleReader.Tuple()->GetListOfFriends() != nullptr ? to_string(_tupleReader.Tuple()->GetEntriesFriend()) : "---");
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void TupleReader::Init() {
    MessageSvc::Info(Color::Cyan, "TupleReader", (TString) "Initialize ...");
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) m_tuple->GetName());
    if (m_debug) PrintListOfFiles();
    if (GetNFiles() == 0) {
        cout << *this;
        if (!SettingDef::Tuple::datasetCache) {
            if (SettingDef::trowLogicError)
                MessageSvc::Error("TupleReader", (TString) "Empty file list", "logic_error");
            else{
                if( SettingDef::Tuple::addTuple == true){
                        MessageSvc::Error("TupleReader", (TString) "Empty file list", "EXIT_FAILURE");
        	    }
    	    }
        } else {
            MessageSvc::Error("TupleReader", (TString) "Empty file list");
        }
    }
    if (m_addTuple) {
        if (m_tuple->GetEntries() == 0) {
            cout << *this;
            if (!SettingDef::Tuple::datasetCache) {
                if (SettingDef::trowLogicError)
                    MessageSvc::Error("TupleReader", (TString) "Empty tuple", "logic_error");
                else
                    MessageSvc::Error("TupleReader", (TString) "Empty tuple", "EXIT_FAILURE");
            } else {
                MessageSvc::Error("TupleReader", (TString) "Empty file list");
            }
        }
        if (m_tuple->GetListOfFriends() != nullptr) {
            if (m_tuple->GetListOfFriends()->GetSize() != 0) MessageSvc::Info("TupleReader", (TString) "Has friends");
            if (m_tuple->GetEntries() != m_tuple->GetEntriesFriend()) MessageSvc::Error("TupleReader", (TString) "Different entries in Tuple and Friend", to_string(m_tuple->GetEntries()), "!=", to_string(m_tuple->GetEntriesFriend()), "EXIT_FAILURE");
        }
        // if (m_debug) Size();
        m_isInitialized = true;
        PrintInline();
    }
    return;
}

void TupleReader::Close() {
    if ((m_reader != nullptr) || (m_tuple != nullptr)) MessageSvc::Info(Color::Cyan, "TupleReader", (TString) "Close ...");
    if (m_reader != nullptr) {
        MessageSvc::Warning("TupleReader", (TString) "Delete TTreeReader");
        delete m_reader;
        m_reader = nullptr;
    }
    if (m_tuple != nullptr) {
        MessageSvc::Warning("TupleReader", (TString) "Delete TChain");
        m_tuple->Reset(); //https://root-forum.cern.ch/t/close-files-opened-in-tchains-explicitly/14949/3  . If in pyROOT we open all the TChain files in loop, they never get closed if et.Close() is called. We need to reset the TChain to achieve this.
        delete m_tuple;
        m_tuple = nullptr;
    }
    m_fileNames.clear();
    m_isInitialized = false;
    return;
}

void TupleReader::SetFileRecover() {
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "TNetXNGFile::Recover", to_string(m_fileRecover));
    gEnv->SetValue("TFile.Recover", m_fileRecover);
    return;
}

TString TupleReader::GetXrootD(TString _fileName) {
    if (m_useXrootD) {
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "GetXrootD", _fileName);
        if (_fileName.Contains("root://gridproxy@eoslhcb.cern.ch/") ){
            _fileName = _fileName; //trick to make useURLS work, site is CERN, needs double slash! 
        }else if (_fileName.Contains("/eos/lhcb/") && !_fileName.Contains("root://eoslhcb.cern.ch/")) _fileName = "root://eoslhcb.cern.ch/" + _fileName;

        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "GetXrootD", _fileName);
    }
    return _fileName;
}

bool TupleReader::CheckVarInTuple(TString _name) {
    if (m_debug) MessageSvc::Debug("TupleReader", (TString) "CheckVarInTuple", to_string(m_tuple->GetListOfBranches()->GetSize()));
    return IsVarInTuple(m_tuple, _name);
}

bool TupleReader::CheckVarsInChain(const vector<TString> &_columns, const TString _theTupleName){
    //Go file by file in the TChain and check in each that the branch exists!
    if(m_tuple==nullptr){
        MessageSvc::Warning("TupleReader",(TString)"CheckVarsInChain invalid");
    }
    MessageSvc::Debug("TupleReader", (TString)"CheckVarsInChain");
    vector<TString> _aliasesOnTuple{};
    if(m_tuple->GetListOfAliases() != nullptr){
        TObjArray * _aliases = (TObjArray *) m_tuple->GetListOfAliases();
        for (int i = 0; i < _aliases->GetEntries(); ++i) {
            TString _nameVar = _aliases->At(i)->GetName();
            _aliasesOnTuple.push_back(_nameVar);
        }
    }
    vector<TString> _columns_noAliases{};    
    for( auto & el : _columns){
        bool _isAlias = false;
        for( auto & al : _aliasesOnTuple){
            if( el == al) _isAlias = true; break;
        }
        //Only check non-alias branches
        if( el == "wkin_RpK.wkin" || el == "wkin"){
            MessageSvc::Warning("Skipping checking over wkin_RpK.wkin branch");
            continue;
        }
        if( !_isAlias) _columns_noAliases.push_back(el);
    }

    bool ok = true; 
    MessageSvc::Info("CheckVarsInChain (nFiles in Chain)", TString::Format("%i", m_fileNames.size()));
    for( auto & f : m_fileNames){
        if (f.Contains("/eos/lhcb/") && !f.Contains("root://eoslhcb.cern.ch/") && !IsBATCH("CERN")) f = "root://eoslhcb.cern.ch/" + f;
        if(!IOSvc::ExistFile(f)) {
            MessageSvc::Error("TupleReader", TString::Format("CheckListVarsInTuple (file not accessible) %s",f.Data()), "EXIT_FAILURE");
        }
        MessageSvc::Info("CheckVarsInChain", TString::Format("%s::%s",f.Data(), _theTupleName.Data()));
        ROOT::RDataFrame df( _theTupleName, f);
        for( auto & el : _columns_noAliases){     
            ok = ok && df.HasColumn(el);
            if(!df.HasColumn(el)){
                MessageSvc::Warning("Missing column in one element of TChain from passed list (branch)", (TString)el);
                MessageSvc::Warning("Missing column in one element of TChain from passed list (file)  ", (TString)f);                
            }
        }
    }
    return ok;
}


void TupleReader::SetEntries(Long64_t _maxEntries) {
    if (_maxEntries == 0) _maxEntries = m_frac;

    Long64_t _entries = m_tuple->GetEntries();
    if ((_maxEntries != -1) && (_maxEntries >= 1)) _entries = (int) floor(_maxEntries);
    if ((_maxEntries >= 0.f) && (_maxEntries < 1.0f)) _entries = (int) floor(_maxEntries * _entries);

    MessageSvc::Info("TupleReader", (TString) "SetEntries", to_string(_entries));
    m_tuple->SetEntries(_entries);
    return;
}

void TupleReader::AddTuple(TChain * _tuple) {
    if (m_tuple != nullptr) {
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddTuple");
        TObjArray * _fileElements = _tuple->GetListOfFiles();
        if (_fileElements) {
            TIter           _next(_fileElements);
            TChainElement * _chainElement = nullptr;
            while ((_chainElement = (TChainElement *) _next())) {
                if (!AddFile(_chainElement->GetTitle())) break;
            }
        }
    }
    return;
}

bool TupleReader::AddFile(TString _fileName) {
    if (m_tuple != nullptr) {
        if ((m_frac != -1) && (m_tuple->GetEntries() > m_frac)) {
            MessageSvc::Info("TupleReader", (TString) "Events loaded", to_string(m_tuple->GetEntries()), "> Events requested", to_string(m_frac), "... Stop AddFile");
            return false;
        }
        if( CheckVectorContains( m_fileNames, _fileName)){
            MessageSvc::Warning("TupleReader", (TString)"AddFile skip, already added");
            return false;
        }
        m_fileNames.push_back(_fileName);
        if (true) MessageSvc::Debug("TupleReader", (TString) "AddFile", _fileName);
        if (m_addTuple) {
            if (IOSvc::ExistFile(GetXrootD(_fileName))) {
                //
                int _status = m_tuple->Add(GetXrootD(_fileName), -1);
                if( _status == 0 ){
                    // MessageSvc::Error("TupleReader::Add(FileName)", TString::Format("TTree not in file(%s)",_fileName.Data()), "EXIT_FAILURE");
                    MessageSvc::Warning("TupleReader::Add(FileName)", TString::Format("TTree not in file(%s)",_fileName.Data()));
                    m_fileNames.pop_back();
                }
            } else {
                MessageSvc::Warning("TupleReader", (TString) "AddFile", _fileName, "does not exist");		
                m_fileNames.pop_back();
                return false;
            }
        }
    }
    return true;
}

bool TupleReader::AddFile(TString _fileName, TString _tupleName) {
    if (_fileName == "") return false;
    if (m_tuple != nullptr) {
        if ((m_frac != -1) && (m_tuple->GetEntries() > m_frac)) {
            MessageSvc::Info("TupleReader", (TString) "Events loaded", to_string(m_tuple->GetEntries()), "> Events requested", to_string(m_frac), "... Stop AddFile");
            return false;
        }
        m_fileNames.push_back(_fileName);
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddFile", _fileName, _tupleName);
        if (m_addTuple) {
            if (IOSvc::ExistFile(GetXrootD(_fileName))) {
                int _status = m_tuple->AddFile(GetXrootD(_fileName), -1, _tupleName);
                if( _status == 0 ){
                    MessageSvc::Error("TupleReader::Add(FileName)", TString::Format("TTree not in file(%s)",_fileName.Data())); //, "EXIT_FAILURE");
                    m_fileNames.pop_back();
                }                
            } else {
                MessageSvc::Warning("TupleReader", (TString) "AddFile", _fileName, "does not exist");
                m_fileNames.pop_back();
            }
        }
    }
    return true;
}

void TupleReader::AddFiles(TString _fileName) {
    if (m_tuple != nullptr) {
        auto   _start = chrono::high_resolution_clock::now();
        glob_t _globResult;
        glob(_fileName, GLOB_TILDE, nullptr, &_globResult);
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddFiles", _fileName, to_string(_globResult.gl_pathc));
        for (unsigned int i = 0; i < _globResult.gl_pathc; ++i) {
            if (!AddFile(_globResult.gl_pathv[i])) break;
        }
        globfree(&_globResult);
        auto _stop = chrono::high_resolution_clock::now();
        MessageSvc::Warning("TupleReader", (TString) "AddFiles took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    }
    return;
}

void TupleReader::AddFile(TString _fileName, int _iFile) {
    if (m_tuple != nullptr) {
        glob_t _globResult;
        glob(_fileName, GLOB_TILDE, nullptr, &_globResult);
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddFile", _fileName, to_string(_globResult.gl_pathc), to_string(_iFile));
        if (_iFile < _globResult.gl_pathc)
            AddFile(_globResult.gl_pathv[_iFile]);
        else
            MessageSvc::Error("TupleReader", (TString) "AddFile", _fileName, to_string(_iFile), "does not exist");
        globfree(&_globResult);
    }
    return;
}

int TupleReader::GetNFiles(TString _fileName) {
    int _nFiles = 0;
    if (m_tuple != nullptr) {
        glob_t _globResult;
        glob(_fileName, GLOB_TILDE, nullptr, &_globResult);
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "GetNFiles", _fileName, to_string(_globResult.gl_pathc));
        _nFiles = _globResult.gl_pathc;
        globfree(&_globResult);
    }
    return _nFiles;
}

bool TupleReader::AddList(TString _fileName) {
    if (m_tuple != nullptr) {
        if (!IOSvc::ExistFile(_fileName)) {
            MessageSvc::Warning("TupleReader", (TString) "AddList", _fileName, "does not exist");
            return false;
        }
        auto _start = chrono::high_resolution_clock::now();
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddList", _fileName);
        ifstream _list;
        _list.open(_fileName, ifstream::in);
        string _line;
        int    _listSize = 0;
        while (getline(_list, _line)) {
            if (!_line.empty()) ++_listSize;
        }
        _list.close();
        _list.open(_fileName, ifstream::in);
        int _addSize = 0;
        while (getline(_list, _line)) {
            if (!_line.empty()) {
                if (AddFile(_line)) ++_addSize;
            }
        }
        _list.close();
        auto _stop = chrono::high_resolution_clock::now();
        MessageSvc::Warning("TupleReader", (TString) "ToAdd", to_string(_listSize));
        MessageSvc::Warning("TupleReader", (TString) "Added", to_string(_addSize));
        MessageSvc::Warning("TupleReader", (TString) "AddList took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
        if (m_frac == -1) {
            if (_listSize != _addSize) {
                MessageSvc::Warning("TupleReader", (TString) "ToAdd != Added");
                return false;
            }
        }
        return true;
    }
    return false;
}

void TupleReader::AddFriend(TChain * _tuple) {
    if (m_tuple != nullptr) {
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddFriend");
        TObjArray * _fileElements = _tuple->GetListOfFiles();
        if (_fileElements) {
            TIter           _next(_fileElements);
            TChainElement * _chainElement = nullptr;
            while ((_chainElement = (TChainElement *) _next())) AddFriend(_chainElement->GetTitle());
        }
    }
    return;
}

void TupleReader::AddFriend(TString _fileName, TString _tupleName) {
    if (m_tuple != nullptr) {
        if (!IOSvc::ExistFile(GetXrootD(_fileName))) {
            MessageSvc::Warning("TupleReader", (TString) "AddFriend", _fileName, "does not exist");
            return;
        }
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "AddFriend", _fileName, _tupleName);
        m_tuple->AddFriend(_tupleName, GetXrootD(_fileName));
    }
    return;
}

void TupleReader::PrintListOfFiles() const noexcept {
    if (m_tuple != nullptr) {
        MessageSvc::Debug("TupleReader", (TString) "PrintListOfFiles", to_string(m_fileNames.size()));
        for (const auto & _file : m_fileNames) { MessageSvc::Debug(_file); }
    }
    return;
}

TString TupleReader::GetFileName(int _iFile) {
    if (m_tuple != nullptr) {
        if (_iFile < m_fileNames.size())
            return m_fileNames[_iFile];
        else
            MessageSvc::Error("TupleReader", (TString) "GetFileName", to_string(_iFile), "does not exist");
    }
    return TString("");
}

int TupleReader::SetBranches(vector< TString > _branches, bool _status, TString _option) {
    int _nBranches = 0;
    //TODO :: remove the hack once we attach nominal q2 smearing branches. 
    if (m_tuple != nullptr) {
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "SetBranches", to_string(_branches.size()), _status ? "true" : "false", _option);
        if (_option.Contains("all")) {
            m_tuple->SetBranchStatus("*", _status);
            if (m_tuple->GetListOfBranches() == nullptr) return 0;
            return m_tuple->GetListOfBranches()->GetSize();
        }
        if (_branches.size() != 0) {
            vector< TString > _branchesTmp;
            for (auto * _branch : *m_tuple->GetListOfBranches()) {
                TString _name = _branch->GetName();
                if (find(_branchesTmp.begin(), _branchesTmp.end(), _name) == _branchesTmp.end()) {
                    for (const auto & _b : _branches) {
                        if (_b.BeginsWith("_") && _name.EndsWith(_b)) _branchesTmp.push_back(_name);
                        if (!_b.BeginsWith("_") && (_name == _b)) _branchesTmp.push_back(_name);
                    }
                }
            }
            if (m_tuple->GetListOfFriends() != nullptr) {
                for (auto * _branch : *m_tuple->GetFriend(SettingDef::Tuple::SPT)->GetListOfBranches()) {
                    TString _name = _branch->GetName();
                    if (find(_branchesTmp.begin(), _branchesTmp.end(), _name) == _branchesTmp.end()) {
                        for (const auto & _b : _branches) {
                            if (_b.BeginsWith("_") && _name.EndsWith(_b)) _branchesTmp.push_back(_name);
                            if (!_b.BeginsWith("_") && (_name == _b)) _branchesTmp.push_back(_name);
                        }
                    }
                }
            }
            RemoveVectorDuplicates(_branchesTmp);
            if (m_debug) {
                if (_branches.size() != _branchesTmp.size()) MessageSvc::Debug("TupleReader", (TString) "SetBranches", to_string(_branches.size()), _status ? "true" : "false", _option);
            }
            _branches = _branchesTmp;

            for (auto * _branch : *m_tuple->GetListOfBranches()) {
                for (size_t b = 0; b < _branches.size(); ++b) {
                    if (_branches[b] == _branch->GetName()) {
                        m_tuple->SetBranchStatus(_branches[b], _status);
                        _nBranches++;
                        if (m_debug) { MessageSvc::Debug("TupleReader", (TString) "SetBranches", _branches[b], _status ? "true" : "false", _option); }
                    }
                }
            }

            if (m_debug) {
                if (_nBranches != _branches.size()) MessageSvc::Debug("TupleReader", (TString) "SetBranches", to_string(_nBranches), _status ? "true" : "false", _option);
            }

            if (m_tuple->GetListOfFriends() != nullptr) {
                for (auto * _branch : *m_tuple->GetFriend(SettingDef::Tuple::SPT)->GetListOfBranches()) {
                    for (size_t b = 0; b < _branches.size(); ++b) {
                        if (_branches[b] == _branch->GetName()) {
                            m_tuple->GetFriend(SettingDef::Tuple::SPT)->SetBranchStatus(_branches[b], _status);
                            _nBranches++;
                            if (m_debug) { MessageSvc::Debug("TupleReader", (TString) "SetBranchesFriends", _branches[b], _status ? "true" : "false", _option); }
                        }
                    }
                }

                if (m_debug) {
                    if (_nBranches != _branches.size()) MessageSvc::Debug("TupleReader", (TString) "SetBranchesFriends", to_string(_nBranches), _status ? "true" : "false", _option);
                }
            }
        }
    }    
    return _nBranches;
}

int TupleReader::SetAliases(vector< pair< TString, TString > > _aliases, TString _option) {
    int _nAliases = 0;
    if (m_tuple != nullptr) {
        if (m_debug) MessageSvc::Debug("TupleReader", (TString) "SetAliases", to_string(_aliases.size()), _option);

        if (_aliases.size() != 0) {
            for (size_t a = 0; a < _aliases.size(); ++a) {
                bool _status = true;
                for (auto * _branch : *m_tuple->GetListOfBranches()) {
                    if (_aliases[a].first == _branch->GetName()) _status = false;
                }
                if (_status) {
                    m_tuple->SetAlias(_aliases[a].first, _aliases[a].second);
                    _nAliases++;
                } else {
                    if (m_debug) MessageSvc::Debug("Cannot add alias", (TString) "Invalid branches for", _aliases[a].second);
                }
            }
            if (m_debug) {
                if (_nAliases != _aliases.size()) MessageSvc::Debug("TupleReader", (TString) "SetAliases", to_string(_nAliases), _option);
            }
        }
    }
    return _nAliases;
}

TTree * TupleReader::CopyTuple(TCut _cut, TString _tupleName, double _frac, bool _replace) {
    TFile * _tFile = nullptr;
    if (_replace) _tFile = new TFile("tmpCopyTuple" + _tupleName + ".root", to_string(OpenMode::RECREATE));   // DO NOT DELETE
    TTree * _tuple = nullptr;
    if (m_tuple != nullptr) {
        if (_cut == "1") _cut = "";
        if (_frac == -1) _frac = m_frac;
        auto _start = chrono::high_resolution_clock::now();
        MessageSvc::Warning("TupleReader", (TString) "CopyTuple", "Frac  =", to_string(_frac));
        Long64_t _nTotal = m_tuple->GetEntries();
        Long64_t _nTot   = m_tuple->GetEntries();
        if ((_nTot != 0) && (_frac != 0) && (_cut != "")) MessageSvc::Info("TupleReader", &_cut);

        if ((_frac == -1) && !IsCut(_cut)) {   // TO BE CHANGED WITH CLONE() ???
            if (_tupleName != "") m_tuple->SetName(_tupleName);
            return (TTree *) m_tuple;
        }

        Long64_t _nTot_ = _nTot;
        if (_frac == 0) {
            _cut  = "";
            _nTot = 0;
        }
        if ((_frac > 0) && (_frac < 1)) _nTot *= _frac;
        if ((_frac > 1) && (_frac < _nTot)) _nTot = _frac;

        if (_frac != 0) {
            MessageSvc::Info("TupleReader", (TString) "CopyTuple", (TString) fmt::format("N Tot {0} out of {1}", _nTot, _nTotal));
            if (_frac >= 0) MessageSvc::Info("TupleReader", (TString) "CopyTuple", "Copying entries ...");
        } else
            MessageSvc::Info("TupleReader", (TString) "CopyTuple", "Cloning structure ...");

        if (!IsCut(_cut))
            _tuple = (TTree *) m_tuple->CloneTree(_nTot);
        else
            _tuple = (TTree *) m_tuple->CopyTree(_cut, "", _nTot);

        if (_tupleName != "") _tuple->SetName(_tupleName);

        if (_nTot != 0) {
            Long64_t _nPas = _tuple->GetEntries();
            MessageSvc::Info("TupleReader", (TString) "CopyTuple", "N Pas =", to_string(_nPas));   //, "(", to_string((double) _nPas / (double) _nTot_ * 100), "% )");
        }

        auto _stop = chrono::high_resolution_clock::now();
        MessageSvc::Warning("TupleReader", (TString) "CopyTuple took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");

        if (_replace) {
            MessageSvc::Info("TupleReader", (TString) "CopyTuple", "Replacing ...");
            if (m_reader != nullptr) {
                delete m_reader;
                m_reader = nullptr;
            }
            if (m_tuple != nullptr) {
                delete m_tuple;
                m_tuple = nullptr;
            }
            m_fileNames = {_tFile->GetName()};
            m_tuple     = (TChain *) _tuple;
            m_reader    = new TTreeReader(m_tuple);
            Init();
            return nullptr;
        }
    }
    return _tuple;
}

TTree * TupleReader::SnapshotTuple(TCut _cut, TString _tupleName, double _frac, bool _replace, vector< TString > _branchesToKeep, vector< TString > _aliasesToKeep) {
    TTree * _tuple = nullptr;
    if (m_tuple != nullptr) {
        if (_tupleName == "") _tupleName = m_tuple->GetName();
        map< TString, SplitInfo > _mapSplit = {{_tupleName, SplitInfo("tmpSnapshotTuple" + _tupleName + ".root", _tupleName, _cut, _tupleName)}};
        map< TString, TTree * >   _mapTuple = SplitTuple(_mapSplit, _frac, false, _branchesToKeep, _aliasesToKeep, false);
        _tuple                              = _mapTuple[_tupleName];

        if (_replace) {
            MessageSvc::Info("TupleReader", (TString) "SnapshotTuple", "Replacing ...");
            if (m_reader != nullptr) {
                delete m_reader;
                m_reader = nullptr;
            }
            if (m_tuple != nullptr) {
                delete m_tuple;
                m_tuple = nullptr;
            }
            m_fileNames = {"tmpSnapshotTuple" + _tupleName + ".root"};
            m_tuple     = (TChain *) _tuple;
            m_reader    = new TTreeReader(m_tuple);
            Init();
            return nullptr;
        }
    }
    return _tuple;
}

TTree * TupleReader::CloneTuple(TString _tupleName) { return (TTree *) CopyTuple("", _tupleName, 0); }

map< TString, TTree * > TupleReader::SplitTuple(const map< TString, SplitInfo > & _selections, double _frac, bool _hadd, vector< TString > _branchesToKeep, vector< TString > _aliasesToKeep, bool _replace, TString _selection) {
    if (m_tuple->GetListOfFriends() != nullptr) {
        if (m_tuple->GetListOfFriends()->GetEntries() == 1) {
            MessageSvc::Warning("TupleReader", (TString) "SplitTuple with Friends, add all branches from Friend as regular branches in the Snapshot...");
            auto _friendName     = m_tuple->GetListOfFriends()->At(0)->GetName();
            auto _friendBranches = m_tuple->GetFriend(_friendName)->GetListOfBranches();
            for (int i = 0; i < _friendBranches->GetEntries(); ++i) {
                MessageSvc::Info("Adding ", TString(_friendBranches->At(i)->GetName()));
                _aliasesToKeep.push_back(_friendBranches->At(i)->GetName());
            }
        }
    }
    if (_frac == -1) _frac = m_frac;
    map< TString, TTree * > _mapTuple = GetSplitTuples(m_tuple, _selections, _frac, _hadd, _branchesToKeep, _aliasesToKeep);

    if (_replace) {
        MessageSvc::Info("TupleReader", (TString) "SplitTuple", "Replacing ...", _selection);
        if (_selection == "") MessageSvc::Error("SplitTuple", (TString) "Wrong selection", "EXIT_FAILURE");
        if (m_reader != nullptr) {
            delete m_reader;
            m_reader = nullptr;
        }
        if (m_tuple != nullptr) {
            delete m_tuple;
            m_tuple = nullptr;
        }
        m_fileNames = {_selections.at(_selection).FileName};
        m_tuple     = (TChain *) _mapTuple[_selection];
        m_reader    = new TTreeReader(m_tuple);
        Init();
    }
    return _mapTuple;
}

TTree * TupleReader::GetMultCandTuple(vector< FUNC_PTR > & _function, vector< TString > & _name) {
    TTree * _tuple = nullptr;
    if (m_tuple != nullptr) {
        MessageSvc::Info("TupleReader", (TString) "GetMultCandTree");
        for (size_t f = 0; f < _function.size(); f++) MessageSvc::Info("GetMultCandTree", _name[f]);

        _tuple = (TTree *) (this)->CloneTuple((TString) m_tuple->GetName());

        bool        _hasMultiplicity = false;
        TObjArray * _branches        = _tuple->GetListOfBranches();
        for (int i = 0; i < _branches->GetEntries(); ++i)
            if (((TString)((TBranch *) _branches->At(i))->GetName()) == "Multiplicity") _hasMultiplicity = true;

        int _multiplicity = -1;
        if (!_hasMultiplicity) _tuple->Branch("Multiplicity", &_multiplicity, "Multiplicity/I");

        const int _size = 100;
        if (_size < _function.size()) MessageSvc::Error("GetMultCandTree", "Wrong size", "EXIT_FAILURE");

        TString _nameSingle[_size];
        int     _isSingle[_size];
        for (size_t f = 0; f < _function.size(); f++) {
            _nameSingle[f] = "isSingle_" + _name[f];
            _isSingle[f]   = 1;
            _tuple->Branch(_nameSingle[f], &_isSingle[f], _nameSingle[f] + "/I");
        }

        int    _nTot = m_tuple->GetEntries();
        double _frac = SettingDef::Tuple::frac;
        if (_nTot < _frac) _frac = -1;
        if (_frac >= 1) _nTot = _frac;

        MessageSvc::Line();
        MessageSvc::Info("GetMultCandTree", (TString) "Looping ...", to_string(_nTot));

        m_tuple->LoadTree(-1);
        TupleReader _tupleReader = TupleReader(m_tuple);

        TTreeReaderValue< ULong64_t > _evtNumber = _tupleReader.GetValue< ULong64_t >("eventNumber");
        TTreeReaderValue< UInt_t >    _runNumber = _tupleReader.GetValue< UInt_t >("runNumber");

        Long64_t i = 0;
        _tupleReader.Reader()->Next();
        ULong64_t          _lastEvtNumber = *_evtNumber;
        UInt_t             _lastRunNumber = *_runNumber;
        vector< Long64_t > _entry;
        _entry.emplace_back(i);

        while (_tupleReader.Reader()->Next()) {
            // if (! m_debug) MessageSvc::ShowPercentage(i, _nTot - 1);

            i++;
            _tupleReader.GetEntry(i);

            ULong64_t _currEvtNumber = *_evtNumber;
            UInt_t    _currRunNumber = *_runNumber;

            bool _changed = (_currEvtNumber != _lastEvtNumber) || (_currRunNumber != _lastRunNumber);
            bool _last    = (i == (_nTot - 1));

            if (_changed || _last) {
                if (_changed && _last) {
                    _multiplicity = 1;
                    for (size_t f = 0; f < _function.size(); f++) _isSingle[f] = 1;
                    _tuple->Fill();
                }

                if (!_changed) _entry.emplace_back(i);
                _multiplicity = _entry.size();

                vector< Long64_t > _keep;
                for (size_t f = 0; f < _function.size(); f++) _keep.emplace_back(_function[f](m_tuple, _entry, m_rnd));

                for (size_t m = 0; m < _entry.size(); m++) {
                    _tupleReader.GetEntry(_entry[m]);

                    for (size_t f = 0; f < _function.size(); f++) {
                        _isSingle[f] = 0;
                        if (_entry[m] == _keep[f]) _isSingle[f] = 1;
                    }

                    _tuple->Fill();
                }
                _entry.clear();

                _lastEvtNumber = _currEvtNumber;
                _lastRunNumber = _currRunNumber;
            }
            _entry.emplace_back(i);

            if (i >= _nTot - 1) break;
        }
        MessageSvc::Line();
    }
    return _tuple;
}

void TupleReader::Size() {
    Long64_t _size = 0;
    for (const auto & _fileName : m_fileNames) {
        TFile _tFile(GetXrootD(_fileName));
        _size += _tFile.GetSize();
        _tFile.Close();
    }
    MessageSvc::Debug("TupleReader", (TString) "Size", to_string(_size / 1024 / 1024), "MB");
    return;
}

vector< vector< double > > TupleReader::GetVariableVector(vector< TString > & _variableNames, TCut _cut, int _nCores) {
    vector< vector< double > > _variablesContainer;
    // Always disable it, if the call is done on the same tuple multiple times, the "order" of the output vector is preserved.
    // If the MT is enabled, for 2-D vector extraction it becomes impossible to preserve order, thus shuffling and destroying all correlations.
    if (_nCores != 1) {
        EnableMultiThreads(_nCores);
    } else {
        DisableMultiThreads();
    }
    vector< pair< TString, TString > > _unpackedAliases;
    if (m_tuple->GetListOfAliases() != nullptr) {
        TObjArray * _aliases = (TObjArray *) m_tuple->GetListOfAliases();
        for (int i = 0; i < _aliases->GetEntries(); ++i) {
            TString _nameVar = _aliases->At(i)->GetName();
            TString _expr    = _aliases->At(i)->GetTitle();
            _unpackedAliases.push_back(make_pair(_nameVar, _expr));
            if (m_debug) { MessageSvc::Warning("Unpacked Alias ", _nameVar, _expr); }
        }
    }
    ROOT::RDataFrame df(*m_tuple);
    ROOT::RDF::RNode latestDF(df);
    for (auto & _alias : _unpackedAliases) {
        for (auto & _variableName : _variableNames) {
            if (TString(_alias.first) == TString(_variableName)) {
                MessageSvc::Info("TupleReader::GetVariableVector", (TString) "Defining alias column on DataFrame:", _alias.first, ":", _alias.second);
                TString _expression = TString("(double)") + TString(_alias.second.ReplaceAll("<Double_t>", ""));
                latestDF            =  latestDF.Define(_alias.first.Data(), _expression.Data());
            }
        }
    }
    int iDX = 0;
    for (auto & _variableName : _variableNames) {
        TString _varToExtract = TString("(double)") + _variableName;
        TString vv            = TString(fmt::format("VAR{0}", iDX));
        MessageSvc::Info("TupleReader::Define(", vv, _varToExtract);
        latestDF = latestDF.Define(vv.Data(), _varToExtract.Data());
        iDX++;
    }
    // can be done before
    TString filter = TString(_cut);
    MessageSvc::Info("GetVariableVector Prefiltering is", TString(_cut), "");

    auto finalDF = latestDF.Filter(filter.Data());
    // Templated unproteected Take.....quite bad, internal DataFrame casting may fail at run time...

    using TakeDoubleCols = ROOT::RDF::RResultPtr< std::vector<double> >;
    std::map< int,  TakeDoubleCols  > vars;
    for (int iLoop = 0; iLoop < _variableNames.size(); ++iLoop) {
        TString vv = TString(fmt::format("VAR{0}", iLoop));
        vars[iLoop]    = finalDF.Take< double >(vv.Data());
    }
    MessageSvc::Info("TupleReader::GetVariableVector NVars = ", to_string(_variableNames.size()), "process", to_string(vars[0]->size()), "entries");

    MessageSvc::Info("TupleReader::GetVariableVector", (TString) "Before = " + to_string(m_tuple->GetEntries()), (TString) "After = " + to_string((vars[0]->size())));
    for (auto & _varToExtract : _variableNames) { 
        MessageSvc::Info("TupleReader::GetVariableVector", _varToExtract, "process", to_string(vars[0]->size()), "entries"); 
    }
    _variablesContainer.resize(vars.size());

    for (int iLoop = 0; iLoop < vars.size(); ++iLoop) { _variablesContainer[iLoop] = std::move(*vars[iLoop]); }

    DisableMultiThreads();

    return _variablesContainer;
}

void TupleReader::PrintInline() const noexcept {
    TString _nfiles   = IsInitialized() ? to_string(GetNFiles()) : "---";
    TString _nentries = IsInitialized() ? to_string(Tuple()->GetEntriesFast()) : "---";
    TString _toPrint  = fmt::format("NFiles {0}, NEntries {1}", _nfiles, _nentries);
    MessageSvc::Info(Color::Cyan, "TupleReader", _toPrint);
    return;
}

#endif
