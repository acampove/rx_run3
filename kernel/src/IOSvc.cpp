#ifndef IOSVC_CPP
#define IOSVC_CPP

#include "IOSvc.hpp"
#include "ConfigHolder.hpp"
#include "HelperSvc.hpp"
#include "SettingDef.hpp"
#include "TFile.h"
#include "TKey.h"
#include "core.h"
#include <TString.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <glob.h>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/stat.h>
#include <vector>
#include "TSystem.h"
int IOSvc::runCommand( std::string command ){     
    TString _cmd( command);
    int status = system(_cmd.Data());
    return status;
}
int IOSvc::runCommand( TString command){
    return runCommand( std::string( command.Data()));
}



TString IOSvc::GetDataDir(TString _option) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetDataDir", _option);
    TString _dir = SettingDef::IO::dataDir + "/" + _option;   //--> TODO: can-should be substituted by fmt::format("{0}/{1}", SettingDef::IO::dataDir, _option);
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetEfficiencyDir(const ConfigHolder & _configHolder) { return GetTupleDir("eff", _configHolder); };

TString IOSvc::GetEfficiencyDir(TString _project, TString _ana, TString _q2bin, TString _year, TString _trigger) { return GetTupleDir("eff", _project, _ana, _q2bin, _year, _trigger); };

TString IOSvc::GetIODir(TString _option) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetIODir", _option);
    TString _dir = SettingDef::IO::ioDir;
    if (SettingDef::IO::useEOS) {
        _dir = SettingDef::IO::eosDir;
        if (SettingDef::IO::useEOSHome) _dir = SettingDef::IO::eosHome;
    }
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetFitDir(TString _option, const ConfigHolder & _configHolder) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetFitDir", _option);
    TString _dir = GetIODir(_option) + "/fits/v" + SettingDef::Tuple::gngVer;
    if ((SettingDef::Tuple::option == "pro") && (SettingDef::Tuple::proVer != "")) _dir += "/TupleProcess_" + SettingDef::Tuple::proVer;
    if ((SettingDef::Tuple::option == "cre") && (SettingDef::Tuple::creVer != "")) _dir += "/TupleCreate_" + SettingDef::Tuple::creVer;
    if (SettingDef::Efficiency::ver != "") _dir += "/Efficiency_" + SettingDef::Efficiency::ver;
    if (SettingDef::Fit::ver != "") _dir += "/Fit_" + SettingDef::Fit::ver;
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetFlatnessDir(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf, const TString _variable, const TString _fitName, const int _binIDX) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetFlatnessDir", " " + to_string(_prj) + " " + to_string(_year) + " " + to_string(_trigger) + " " + to_string(_triggerConf));
    TString _headDir = GetIODir("");

    TString _directory = _headDir + "/flatness/v" + SettingDef::Tuple::gngVer + "/v" + SettingDef::Efficiency::flatnessVer + "/{PRJ}-{RUN}-{TRG}-{TRGCONF}";
    _directory.ReplaceAll("{PRJ}", to_string(_prj));   // flatness/RK-{RUN}-{TRG}-{TRGCONF};
    // _directory.ReplaceAll("{RUN}", GetRunFromYear(to_string(_year)));   // flatness/RK-{RUN}-{TRG}-{TRGCONF};
    _directory.ReplaceAll("{RUN}", to_string(_year));   // flatness/RK-{RUN}-{TRG}-{TRGCONF}
    _directory.ReplaceAll("{TRG}", to_string(_trigger));
    _directory.ReplaceAll("{TRGCONF}", to_string(_triggerConf));
    // flatness/v7/vrquaglia_6bins/RK-R1-L0I-exclusive/B_PT/KJPS_DTF_Bin0
    // Directory structure is : v{SettingDef::Tuple::ver}/v{SettingDef::Efficiency::flatnessVer}/{PRJ}-{RUN}-{TRG}-{TRGCONF}/variableAlias/{FITNAME}/{BINX}/this_bin_result
    // Directory structure is : vSettingDef::Tuple::ver}/v{SettingDef::Efficiency::flatnessVer}/{PRJ}-{RUN}-{TRG}-{TRGCONF}/SplotFlatness/{FITNAME}/Result and Splot of it.

    if (_variable != "") {
        _directory += "/" + _variable;
        if (_fitName != "" && _binIDX >= 0) { _directory += "/" + _fitName + "_Bin" + to_string(_binIDX); }
    }
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetFlatnessDir", _directory);
    // if (!ExistDir(_directory)) { MessageSvc::Error("GetFlatnessDir, directory not exists run the isobinning first and make directories in", _directory, "EXIT_FAILURE"); }
    return _directory;
}

TString IOSvc::GetFitCacheDir(TString _option, const ConfigHolder & _configHolder, TString _cutHash, TString _weightHash, TString _tupleOption, TString _cacheType) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetFitCacheDir", _option);
    TString _dir              = GetIODir(_option) + "/fits/v" + SettingDef::Tuple::gngVer;
    
    if (_tupleOption.Contains("pro[")){
      TString  _proVer = StripStringBetween(_tupleOption, "pro[","]");
      _dir += "/TupleProcess_" + _proVer;
    }   
    if( _tupleOption.Contains("cre[")){
      TString _creVer =  StripStringBetween(_tupleOption, "cre[","]");
      _dir += "/TupleCreate_" + _creVer;
    }
    if((_tupleOption == "pro") && (SettingDef::Tuple::proVer != "")) _dir += "/TupleProcess_" + SettingDef::Tuple::proVer;
    if((_tupleOption == "cre") && (SettingDef::Tuple::creVer != "")) _dir += "/TupleCreate_" + SettingDef::Tuple::creVer;
    _dir += "/" + _cacheType + "/" + _configHolder.GetKey() + SettingDef::separator + _cutHash + SettingDef::separator + _weightHash;
    return _dir;
}

TString IOSvc::GetToyDir(TString _option, const ConfigHolder & _configHolder) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetToyDir", _option);
    TString _dir = GetIODir(_option) + "/toys/v" + SettingDef::Tuple::gngVer;
    if (_option.Contains("study")) _dir += "/Study_" + SettingDef::Toy::studyVer;
    if (_option.Contains("generator")) _dir += "/Generator";
    if (_option.Contains("toyFit")) _dir += "/ToyFit";
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetTupleDirHead(TString _option) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetTupleDirHead", _option);
    TString _dir = GetIODir(_option);
    if (_option.Contains("flatness")) { _dir += "/flatness/v" + SettingDef::Tuple::gngVer; }
    if (_option.Contains("eff")) {
        _dir += "/efficiencies/v" + SettingDef::Tuple::gngVer;
    } else {
        _dir += "/tuples/v" + SettingDef::Tuple::gngVer;
    }
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetTupleDir(TString _option, const ConfigHolder & _configHolder) {
    TString _project = to_string(_configHolder.GetProject());
    TString _ana     = to_string(_configHolder.GetAna());
    TString _q2bin   = to_string(_configHolder.GetQ2bin());
    TString _trigger = to_string(_configHolder.GetTrigger());
    TString _year    = to_string(_configHolder.GetYear());

    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetTupleDir", _option, _project, _ana, _q2bin, _trigger, _year);

    TString _dir;
    if (_option.Contains("gng")) {
        _dir = SettingDef::IO::gangaDir + "/lists/v" + GetBaseVer(SettingDef::Tuple::gngVer);
        if ((hash_project(_project) != Prj::RL) && (hash_project(_project) != Prj::RKS)) {
            switch (hash_project(_project)) {
                case Prj::RKst: _dir += "/Bd2Kst"; break;
                case Prj::RK: _dir += "/Bu2K"; break;
                case Prj::RPhi: _dir += "/Bs2Phi"; break;
                // case Prj::RL: _dir += "/Lb2L"; break;
                // case Prj::RKS: _dir += "/Bd2KS"; break;
                default: MessageSvc::Error("CreateTupleReader", (TString) "Invalid prj", _project, "EXIT_FAILURE"); break;
            }

            if (_configHolder.IsMC()) {
                _dir += _ana;
            } else {
                _dir += "LL";
            }
        }
    } else {
        _dir = GetTupleDirHead(_option);
        if (_project != "") _dir += "/" + _project;
        if (_option.Contains("pro")){
            TString _proVer = SettingDef::Tuple::proVer;
            if(_option.Contains("pro[")){
                _proVer = StripStringBetween(_option, "pro[","]");
            }
            _dir += "/TupleProcess_" + _ana + "_" + _proVer;
        }
        if (_option.Contains("cre")){
            TString _creVer = SettingDef::Tuple::creVer;
            if(_option.Contains("cre[")){
                _creVer = StripStringBetween(_option, "cre[", "]");
                if (_configHolder.GetTriggerConf() == TriggerConf::Exclusive2 && _creVer.Contains("exc") && !_creVer.Contains("exc2")) {
                    _creVer.ReplaceAll("exc", "exc2");
                }
            }
            _dir += "/TupleCreate_" + _ana + "_" + _creVer + SettingDef::separator + "q2" + _q2bin;
        }
        if (_option.Contains("spl")) _dir += "/TupleSPlot_" + SettingDef::Tuple::splVer + SettingDef::separator + _configHolder.GetTriggerAndConf();
        if (_option.Contains("toy")) {
            if (SettingDef::Toy::mergeConfig) {
                _dir = TString(fmt::format("{0}/Toy/TupleToy_{1}", GetTupleDirHead(_option), SettingDef::Toy::tupleVer));
            } else {
                _dir += "/TupleToy_";
                if (_ana != "") _dir += _ana + "_";
                _dir += SettingDef::Toy::tupleVer;
                if (_q2bin != "") _dir += SettingDef::separator + "q2" + _q2bin;
                _dir += "/" + _configHolder.GetKey("-noprj-noana-noq2-addtrgconf");
            }
        }
        if (_option.Contains("rap")) {
            _dir.ReplaceAll("/" + _project, "");
            _dir.ReplaceAll("/v" + SettingDef::Tuple::gngVer, "");
            _dir += "/TupleRapidSim";
        }
        // EfficiencyDir through the TupleDir
        if (_option.Contains("eff")) _dir += "/Efficiency_" + _ana + "_" + SettingDef::Efficiency::ver + SettingDef::separator + "q2" + _q2bin;
    }

    if (_option.Contains("out") && (SettingDef::Tuple::outVer != "")) {
        _dir.ReplaceAll(SettingDef::Tuple::proVer, SettingDef::Tuple::outVer);
        _dir.ReplaceAll(SettingDef::Tuple::creVer, SettingDef::Tuple::outVer);
    }

    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

TString IOSvc::GetTupleDir(TString _option, TString _project, TString _ana, TString _q2bin, TString _year, TString _trigger) 
{ 
    ConfigHolder ch(
            hash_project(_project), 
            hash_analysis(_ana), 
            "", 
            hash_q2bin(_q2bin), 
            hash_year(_year), 
            hash_polarity(SettingDef::Config::polarity), 
            hash_trigger(_trigger), 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            hash_brem(SettingDef::Config::brem), 
            hash_track(SettingDef::Config::track));

    return GetTupleDir(_option, ch); 
}

TString IOSvc::GetWeightDir(TString _option) {
    if (m_debug) MessageSvc::Debug("IOSvc", (TString) "GetWeightDir", _option);
    TString _dir = GetIODir(_option) + "/weights/v" + SettingDef::Tuple::gngVer + "/";
    if (_option.Contains("TRK")) {
        _dir += "trk";
        if (SettingDef::Weight::trkVer != "") _dir += "/v" + SettingDef::Weight::trkVer;
    } else if (_option.Contains("PID")) {
        _dir += "pid";
        if (SettingDef::Weight::pidVer != "") _dir += "/v" + SettingDef::Weight::pidVer;
    } else if (_option.Contains("L0")) {
        _dir += "l0";
        if (SettingDef::Weight::l0Ver != "") _dir += "/v" + SettingDef::Weight::l0Ver;
    } else if (_option.Contains("HLT")) {
        _dir += "hlt";
        if (SettingDef::Weight::hltVer != "") _dir += "/v" + SettingDef::Weight::hltVer;
    } else if (_option.Contains("MC")) {
        _dir += "mc";
        if (SettingDef::Weight::mcVer != "") _dir += "/v" + SettingDef::Weight::mcVer;
    }
    if (m_debug) MessageSvc::Debug("IOSvc", _dir);
    return _dir;
}

bool IOSvc::IsFile(TString _filePath) {
    if (_filePath.Contains("/eos/")) {
        return ExistFile(_filePath);
    } else {
        if (!ExistFile(_filePath)) {
            return false;
        } else {
            // the _filePath exists , check if IsDir type!
            struct stat buf;
            stat(_filePath.Data(), &buf);
            return S_ISREG(buf.st_mode);
        }
    }
    return false;
}

bool IOSvc::ExistFile(TString _name) {
  /*
  if (_name.Contains("/eos/") && !IsBATCH("CERN")) {
    //if you are not at CERN , the xrd stat is called
    cout << WHITE;
    //MessageSvc::Line();
    // int _status = runCommand(EOS + " stat -f " + _name);
    int _status = runCommand(XRD + " stat " + _name);
    //MessageSvc::Line();
    cout << RESET;
    if (_status == 0) return true;
    cout<<RED<< XRD << " stat " << _name << "returned status!=0"<< RESET<< endl;
    return false;
  }
  ifstream _file(_name.Data());
  return _file.good();
  /*
    Returns FALSE if one can access a file using the specified access mode.
    The file name must not contain any special shell characters line ~ or $, in those cases first call ExpandPathName(). 
    Attention, bizarre convention of return value!!
    Reimplemented in TWinNTSystem, TUnixSystem, TAlienSystem, TXNetSystem, TWebSystem, TNetSystem, TGFALSystem, and TDCacheSystem.
    See https://root.cern.ch/doc/master/classTSystem.html#a849c28ea0dd3b3aa3310a4d447c7b21a
  */
  return !gSystem->AccessPathName(_name, kFileExists);
}

void IOSvc::CopyFile(TString _input, TString _output) {
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Info(Color::White, "CopyFile", _input, "->", _output);
    if (_input.Contains("/eos/") || _output.Contains("/eos/")) {
        if (_input.Contains("/eos/")  && !_input.Contains(XRDSVR)) _input  = XRDSVR + "//" + _input;
        //copy to eos from outside cern enabled with this!
        if (_output.Contains("/eos/") && !_input.Contains(XRDSVR)) _output = XRDSVR + "//" + _output;
        MessageSvc::Info(Color::White, "CopyFile", XRDCP + " " + _input + " " + _output);
        runCommand(XRDCP + " " + _input + " " + _output);
    } else {
        MessageSvc::Info(Color::White, "CopyFile", "cp -fv " + _input + " " + _output);
        runCommand("cp -fv " + _input + " " + _output);
    }
    MessageSvc::Line();
    cout << RESET;
    return;
}

void IOSvc::RemoveFile(TString _name) {
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Info(Color::White, "RemoveFile", _name);
    if (_name.Contains("/eos/")) {
        // runCommand(EOS + " rm -f " + _name);
        runCommand(XRD + " rm -f " + _name);
    } else {
        runCommand("rm -fv " + _name);
    }
    MessageSvc::Line();
    cout << RESET;
    return;
}

TFile * IOSvc::OpenFile(TString _nameIN, OpenMode _mode) {
    MessageSvc::Info(Color::White, "OpenFile " + to_string(_mode), _nameIN);
    TString _name = XRootDFileName(_nameIN);
    switch (_mode) {
        case OpenMode::READ:
            if (!ExistFile(_name)) MessageSvc::Error(to_string(_mode), _name, "does not exist", "EXIT_FAILURE");
            break;
        case OpenMode::UPDATE: break;
        case OpenMode::RECREATE: break;
        default: MessageSvc::Error("OpenFile", (TString) "Invalid OpenMode", to_string(_mode), "EXIT_FAILURE"); break;
    }
    // TFile * _file = TFile::Open(_name.Data(), to_string(_mode));
    // TFile * _file = new TFile(_name.Data(), to_string(_mode));   // TO BE CHECKED FOR MEMORY LEAKS
    auto _file = unique_ptr< TFile >{TFile::Open(_name.Data(), to_string(_mode))};
    if (_file->IsZombie()) {
        MessageSvc::Warning("OpenFile", _name, "is Zombie");
        _file->Close();
        delete _file.release();
        _file = nullptr;
    }
    return _file.release();
}

void IOSvc::CloseFile(TFile * _file) {
    if ((_file != nullptr) && _file->IsOpen()) {
        MessageSvc::Info(Color::White, "CloseFile", (TString) _file->GetName());
        _file->Close();
    }
    return;
}

vector< TString > IOSvc::ParseFile(const TString _name) {
    vector< TString > _lines;
    TString           _fileName = _name;
    if (_fileName.Contains("/eos") && !_name.Contains("root://eoslhcb.cern.ch/")) { _fileName = TString(fmt::format("root://eoslhcb.cern.ch/{0}", _fileName.Data())); }
    string buffer;
    // Use ROOT's TFile to load a remote filee (small one) into memory istream
    unique_ptr< TFile > ifile(TFile::Open(_fileName + "?filetype=raw"));
    if (ifile && !ifile->IsZombie()) {
        buffer.resize(ifile->GetSize());
        ifile->ReadBuffer(&buffer[0], buffer.size());
    } else {
        MessageSvc::Error("Cannot open TFile as Raw", _fileName, "EXIT_FAILURE");
    }
    TString _result(buffer);
    return TokenizeString(_result, "\n");
}

vector< vector< TString > > IOSvc::ParseFile(TString _name, TString _delimeter) {
    vector< vector< TString > > _data;
    if (_name.Contains("/eos")) {
        vector< TString > _allLines = ParseFile(_name);
        for (auto & _myline : _allLines) {
            string _line = _myline.Data();
            if (_line == "") continue;
            if (((TString) _line).Contains("//")) continue;
            vector< string > _vec;
            boost::algorithm::split(_vec, _line, boost::is_any_of(_delimeter.Data()));
            if (_vec.size() == 0) continue;
            vector< TString > _svec;
            for (auto & _v : _vec) _svec.push_back((TString) _v);
            _data.push_back(_svec);
        }
        return _data;
    }
    if (IOSvc::ExistFile(_name)) {
        ifstream _file(_name);
        if (_file.is_open()) {
            string _line = "";
            while (getline(_file, _line)) {
                if (_line == "") continue;
                if (((TString) _line).Contains("//")) continue;
                vector< string > _vec;
                boost::algorithm::split(_vec, _line, boost::is_any_of(_delimeter.Data()));
                if (_vec.size() == 0) continue;
                vector< TString > _svec;
                for (auto & _v : _vec) _svec.push_back((TString) _v);
                _data.push_back(_svec);
            }
            _file.close();
        } else
            MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    } else
        MessageSvc::Error("Not existing file", _name, "EXIT_FAILURE");
    return _data;
}

bool IOSvc::IsDir(TString _path) {
    if (_path.Contains("/eos/")) {
        return ExistDir(_path);
    } else {
        if (!ExistDir(_path)) {
            return false;
        } else {
            // the _name exists , check if IsDir type!
            struct stat buf;
            stat(_path.Data(), &buf);
            return S_ISDIR(buf.st_mode);
        }
    }
    return false;
}

bool IOSvc::ExistDir(TString _name) {
    if (_name.Contains("/eos/")) {
        cout << WHITE;
        MessageSvc::Line();
        // int _status = runCommand(EOS + " stat -d " + _name);
        int _status = runCommand(XRD + " stat " + _name);
        MessageSvc::Line();
        cout << RESET;
        if (_status == 0) return true;
        return false;
    }
    struct stat _buffer;
    return (stat(_name.Data(), &_buffer) == 0);
}

void IOSvc::MakeDir(TString _name, OpenMode _mode) {
    if (ExistDir(_name)) {
        switch (_mode) {
            case OpenMode::ERROR:
                MessageSvc::Error("MakeDir", _name, "already exist", "EXIT_FAILURE");
                return;
                break;
            case OpenMode::WARNING: MessageSvc::Warning("MakeDir", _name, "already exist"); break;
            default: break;
        }
    }
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Info(Color::White, "MakeDir " + to_string(_mode), _name);
    if (_name.Contains("/eos/")) {
        // runCommand(EOS + " mkdir -p " + _name);
        runCommand(XRD + " mkdir -p " + _name);
    } else {
        runCommand("mkdir -p -v " + _name);
    }
    MessageSvc::Line();
    cout << RESET;
    return;
}

vector< TString > IOSvc::Glob(const string & _pattern) {
    // glob struct resides on the stack
    glob_t _globResult;
    memset(&_globResult, 0, sizeof(_globResult));

    // do the glob operation
    int _returnValue = glob(_pattern.c_str(), GLOB_TILDE, nullptr, &_globResult);
    if (_returnValue != 0) {
        globfree(&_globResult);
        stringstream ss;
        ss << "glob() failed with return  value " << _returnValue << endl;
        throw runtime_error(ss.str());
    }

    // collect all the _names into a list<string>
    vector< TString > _names;
    for (size_t i = 0; i < _globResult.gl_pathc; ++i) { _names.emplace_back(TString(_globResult.gl_pathv[i])); }

    // cleanup
    globfree(&_globResult);

    // done
    return _names;
}

vector< TString > IOSvc::ListOfDirectoryNamesInFile(TFile & file, int depth) {
    file.cd();
    if (depth > 3 && depth == 0) { MessageSvc::Error("ListOfDirectoryNamesInFile CANNOT GO depth dir >1 , implement it in case", "", "EXIT_FAILURE"); }
    vector< TString > list;
    if (depth == 1) {
        TIter  keyList(file.GetListOfKeys());
        TKey * key;
        while ((key = (TKey *) keyList())) {
            if (TString(key->GetClassName()) == "TDirectoryFile") { list.push_back(key->GetName()); }
        }
    }
    if (depth == 2) {
        // recursive call...
        vector< TString > list_dept1 = ListOfDirectoryNamesInFile(file, 1);
        file.cd();
        for (auto & dir : list_dept1) {
            file.cd();
            gDirectory->cd(dir);
            TIter  keyList(gDirectory->GetListOfKeys());
            TKey * key;
            while ((key = (TKey *) keyList())) {
                if (TString(key->GetClassName()) == "TDirectoryFile") { list.push_back(dir + "/" + key->GetName()); }
            }
        }
    }
    if (depth == 3) {
        // recursive call...
        vector< TString > list_dept2 = ListOfDirectoryNamesInFile(file, 2);
        file.cd();
        for (auto & dir : list_dept2) {
            file.cd();
            gDirectory->cd(dir);
            TIter  keyList(gDirectory->GetListOfKeys());
            TKey * key;
            while ((key = (TKey *) keyList())) {
                if (TString(key->GetClassName()) == "TDirectoryFile") { list.push_back(dir + "/" + key->GetName()); }
            }
        }
    }
    file.cd();
    return list;
}


TString IOSvc::XRootDFileName(TString _fileName){
    if( _fileName.Contains("/eos/lhcb") && !_fileName.Contains("root://eoslhcb.cern.ch/") && !IsBATCH("CERN")){
        TString _newName = "";
        if(_fileName.BeginsWith( "/eos")){
            _newName+= "root://eoslhcb.cern.ch/";
        }else{
            _newName = "root://eoslhcb.cern.ch//";
        }
        return _newName+ _fileName;
    }
    return _fileName;
};

#endif
