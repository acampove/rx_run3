#ifndef TOYSTUDY_CPP
#define TOYSTUDY_CPP

#include "ToyStudy.hpp"
#include "SettingDef.hpp"
#include "ToyIO.hpp"
#include <fmt_ostream.h>

#include "ToyFileHandler.hpp"
#include "FitterSvc.hpp"

ToyStudy::ToyStudy() {
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy");
    MessageSvc::Line();

    Init();
}

void ToyStudy::Init() {
    MessageSvc::Info("ToyStudy", (TString) "Initialize ...");

    ConfigHolder _configHolder = ConfigHolder();
    m_keysChecked              = false;
    m_studyIndex               = SettingDef::Toy::jobIndex;
    m_toyIndex                 = SettingDef::Toy::jobIndex * SettingDef::Toy::nToysPerJob;

    return;
}

void ToyStudy::SetupFitter() {
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "SetupFitter");
    MessageSvc::Line();

    GenerateFitterTool();
    LoadNominalResult();
    LoadConstraintsOverwrite();
    SetupConstraintSmearer();
    m_fitter->SetInitialValuesAndErrors();
    SetupResetterAndLogger();
    ResetCheckedStatus();
    return;
}

void ToyStudy::GenerateFitterTool() {
    auto    _co              = ConfigHolder();
    TString _fitComponentDir = ToyIO::GetToyFitComponentDir(m_studyIndex);   // Outputs FitComponent plots to dedicated folder
    IOSvc::MakeDir(_fitComponentDir, OpenMode::WARNING);
    SettingDef::IO::outDir = _fitComponentDir;
    m_fitGenerator         = FitGenerator(SettingDef::name, SettingDef::Fit::option, "before", IOSvc::GetFitDir(SettingDef::Fit::option, _co));
    if (!m_fitGenerator.IsLoaded()) {
        MessageSvc::Warning("ToyStudy", (TString) "Cannot LoadFromDisk, create fit model from tuples");
        m_fitGenerator = FitGenerator("ToyStudy", SettingDef::Fit::option);
        m_fitGenerator.Init();
    } else {
        m_fitGenerator.SetName("ToyStudy");
    }
    m_fitter = m_fitGenerator.Fitter();
    m_fitter->SetConstraintFlag(SettingDef::Fit::option.Contains("gconst"));
    m_fitter->SetConstraintFlagEffs(SettingDef::Fit::option.Contains("gconsteffs"));
    m_fitter->Init();
    m_managerMap = &(m_fitter->GetFitManagers());
    return;
}

void ToyStudy::SetupResetterAndLogger() {
    for (auto & manager : *m_managerMap) {
        for (auto & fitInfo : manager.second.FitInfo) {
            RooArgSet * someSet = fitInfo.second.fullmodel->getComponents();
            m_resetter.AddVariablesFromModel(*fitInfo.second.fullmodel);
            m_logger.AddVariablesFromModel(*fitInfo.second.fullmodel);
        }
    }
    return;
}

void ToyStudy::LoadNominalResult() {
    TFile * _file = GetNominalResultFile();
    vector <TString> _parNames = GetResultParNames(_file);
    FillNominalResultMap(_file, _parNames);
    _file->Close();
}

void ToyStudy::LoadConstraintsOverwrite() {
    if (SettingDef::Toy::constraintOverwriteFile.size()==0){
        MessageSvc::Info("LoadConstraintsOverwrite, no files to load, nothing done");
        return;
    }
    MessageSvc::Line();
    for( auto & _inputFile : SettingDef::Toy::constraintOverwriteFile ) MessageSvc::Info(Color::Cyan, "ToyStudy", (TString) "LoadConstraintsOverwrite from list of files:", _inputFile);    
    MessageSvc::Line();
    for( auto & _inputFile : SettingDef::Toy::constraintOverwriteFile){
        YAML::Node _fileNode = YAML::LoadFile(_inputFile.Data());
        for (YAML::iterator _it = _fileNode.begin(); _it != _fileNode.end(); ++_it) {
            TString _name      = _it->first.as<  TString >();
            TString _options   = _it->second.as< TString >();
            _options.ReplaceAll("[", "");
            _options.ReplaceAll("]", "");
            _options.ReplaceAll(" ", "");
            //Throw an error if you are trying to overwrite an already configured constraint!
            if(m_overwriteConstraints.find(_name) != m_overwriteConstraints.end()) MessageSvc::Error("LoadConstraintsOverwrite", (TString) _name, "parameter present multiple times, fix the yamls for 1 entry only", "EXIT_FAILURE");
            TObjArray * _strCollection = _options.Tokenize(",");
            //Throw an error if you are trying to overwrite a constraint not passing at least [ value, error ]
            if (_strCollection->GetEntries() < 3 ) MessageSvc::Error("LoadConstraintsOverwrite", (TString) _name, "must have at least 1 mean, 1 error", "EXIT_FAILURE");         
            if (_strCollection->GetEntries() >= 3) { 
                if( _strCollection->GetEntries() >3){
                    MessageSvc::Warning("LoadConstraintsOverwrite", (TString)_name, "Only parameter 0,1 in the list is used, do not rely on extra information for constraints overload, range and Identification of Constant flags not used");
                }
                TString _valueString   = TString(((TObjString *) (*_strCollection).At(0))->String());
                TString _errorString   = TString(((TObjString *) (*_strCollection).At(1))->String());
                TString _isConstraint  = TString(((TObjString *) (*_strCollection).At(2))->String());
                //Override constraint only if the parameter is constrained!
                if( _isConstraint.Atoi() == 2 ){                    
                    double _mean  = _valueString.Atof();
                    double _error = _errorString.Atof();
                    m_overwriteConstraints[_name] = make_pair(_mean, _error);
                }
            }
        }
    }
    MessageSvc::Info("LoadConstraintsOverwrite (Number of constraint overloaded = )", (TString) to_string(m_overwriteConstraints.size()));
    MessageSvc::Line();
}

TFile * ToyStudy::GetNominalResultFile(){
    TString _resultDir = IOSvc::GetFitDir("", ConfigHolder());
    TString _resultPath = _resultDir + "/FitResult.root";
    if(not(IOSvc::ExistFile(_resultPath))) {
        MessageSvc::Error("ToyStudy", "Tried reading nominal results from", _resultPath, "but file not found!", "EXIT_FAILURE");
    }
    if( SettingDef::Toy::CopyLocally ){
        IOSvc::CopyFile( _resultPath, "CopyLocally_FitResult.root");
        _resultPath = "CopyLocally_FitResult.root";
    }
    TFile * _file = IOSvc::OpenFile(_resultPath, OpenMode::READ);
    return _file;
}

vector <TString> ToyStudy::GetResultParNames(TFile * _file) const {
    vector <TString> _parNames;
    TString * _name = new TString();
    TTree * _tree = (TTree*)_file->Get(FitResultLogger::parListTreeName.Data());
    _tree->SetBranchAddress(FitResultLogger::parListBranchName, &_name);
    for (int i = 0; i < _tree->GetEntries(); i++){
        _tree->GetEntry(i);
        _parNames.emplace_back(_name->Data());
    }
    delete _name;
    return _parNames;
}

void ToyStudy::FillNominalResultMap(TFile * _file, const vector <TString> & _parNames) {
    double _result;
    TTree * _tree = (TTree*)_file->Get("NominalFit");
    for (const auto& _name : _parNames) {
        _tree->SetBranchAddress(_name, &_result);
        _tree->GetEntry(0);
        m_nominalResult[_name] = _result;
    }
}

void ToyStudy::SetupConstraintSmearer() {
    // Smear the constraints used
    for (auto _constraintMeanPair : m_fitter->GetConstraintToMeanMap()) {
        auto _constraint     = _constraintMeanPair.first;
        auto _constraintMean = _constraintMeanPair.second;
        TString _name = _constraint->GetName();
        if (OverwriteConstraint(_name)){
            double _mean  = m_overwriteConstraints[_name].first;
            double _error = m_overwriteConstraints[_name].second;
            if( SettingDef::Toy::frozenOverwrite){
                MessageSvc::Warning("SetupConstraintSmeared", _name, "No Error for smearing constraint across toys");
                m_variableSmearers.emplace_back(_constraintMean, _mean,     0.);
            }else{
                MessageSvc::Warning("SetupConstraintSmeared", _name, "Constraint smeared according th mean, error provided by the override constraint flag");
                m_variableSmearers.emplace_back(_constraintMean, _mean, _error);
            }
        }
        else{
            double _error = _constraint->getError();
            _name.ReplaceAll("-", "_");
            double _mean = IsInNominal(_name) ? m_nominalResult[_name] : _constraint->getValV();
            m_variableSmearers.emplace_back(_constraintMean, _mean, _error);
        }
    }
    // Smear the correlated parameters
    for (auto& _correlatedVariablesInfo : m_fitter->GetCorrelatedConstraintsInfos()){
        auto& _correlatedVariables = _correlatedVariablesInfo.variables;
        auto& _constraintMeans     = _correlatedVariablesInfo.constraintMeans;
        auto& _covariance          = _correlatedVariablesInfo.covariance;
        vector <double> _means;
        for (auto * _variable : _correlatedVariables){
            TString _name = _variable->GetName();
            _name.ReplaceAll("-", "_");
            double _mean = IsInNominal(_name) ? m_nominalResult[_name] : _variable->getValV();
            _means.push_back(_mean);
        }
        // std::transform(_correlatedVariables.begin(), _correlatedVariables.end(), std::back_inserter(_means),
        //                [this](const RooRealVar * _variable){TString _name = _variable->GetName(); ; return this->IsInNominal(_name) ? this->m_nominalResult[_name] : _variable->getValV();});
        m_correlatedVariablesSmearers.emplace_back(_constraintMeans, _means, _covariance);
    }
    // Setup the seeds
    SetSeed((uint32_t) m_studyIndex + 823453);
    // for (auto _floatFixedPair : RXFitter::GetParameterPool()->GetFloatingToFixedEfficiencyMap()) {
    //     double _mean  = _floatFixedPair.first->getValV();
    //     double _error = _floatFixedPair.first->getError();
    //     auto _fixed   = _floatFixedPair.second;
    //     m_variableSmearers.emplace_back(_fixed, _mean, _error);
    // }
    return;
}

bool ToyStudy::OverwriteConstraint(TString _name){
    return (m_overwriteConstraints.find(_name) != m_overwriteConstraints.end());
}

bool ToyStudy::IsInNominal(TString _name) {
    return (m_nominalResult.find(_name) != m_nominalResult.end());
}

void ToyStudy::ResetCheckedStatus() {
    m_keysChecked = false;
    return;
}

void ToyStudy::SetupReader() {
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "SetupReader");
    MessageSvc::Line();
    //Set a different seed for each reader, if we have to scale yields around or remove because of a cut.
    ToyTupleComponentReader::SetSeed((uint64_t) m_studyIndex + 823453);
    m_reader.GetToyTupleReaders();
    ResetCheckedStatus();
    return;
}

void ToyStudy::CheckKeys() {
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "CheckKeys");
    MessageSvc::Line();

    PrintManagerKeys();
    CheckManagerKeysMatches();
    PrintFitInfoKeys();
    CheckFitInfoKeysMatches();
    m_keysChecked = true;
    return;
}

void ToyStudy::PrintManagerKeys() const {
    vector< TString > _fitterToolManagerKeys = GetFitterManagerKeys();
    vector< TString > _toyReaderManagerKeys  = GetReaderManagerKeys();
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "Print FitterTool keys");
    for (auto & _managerKeys : _fitterToolManagerKeys) { MessageSvc::Info("", _managerKeys); }
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "Print ToyReader keys");
    for (auto & _managerKeys : _toyReaderManagerKeys) { MessageSvc::Info("", _managerKeys); }
    MessageSvc::Line();
    return;
}

vector< TString > ToyStudy::GetFitterManagerKeys() const {
    vector< TString > _managerKeys;
    for (auto & keyManagerPair : *m_managerMap) {
        auto key = keyManagerPair.first;
        _managerKeys.push_back(key);
    }
    return _managerKeys;
}

vector< TString > ToyStudy::GetReaderManagerKeys() const {
    vector< TString > _managerKeys = m_reader.IterateManagerKeys();
    return _managerKeys;
}

void ToyStudy::CheckManagerKeysMatches() const {
    vector< TString > _fitterToolManagerKeys = GetFitterManagerKeys();
    vector< TString > _toyReaderManagerKeys  = GetReaderManagerKeys();
    CheckKeysMatches(_fitterToolManagerKeys, _toyReaderManagerKeys);
    return;
}

void ToyStudy::CheckKeysMatches(const vector< TString > & _fitterKeys, const vector< TString > & _readerKeys) const {
    for (const auto & key : _readerKeys) { ThrowIfKeyNotFound(key, _fitterKeys); }
    for (const auto & key : _fitterKeys) { ThrowIfKeyNotFound(key, _readerKeys); }
    return;
}

void ToyStudy::ThrowIfKeyNotFound(TString key, const vector< TString > & keyContainer) const {
    bool keyNotFound = (find(keyContainer.begin(), keyContainer.end(), key) == keyContainer.end());
    if (keyNotFound) { MessageSvc::Error("ToyStudy", "Key", key, "is missing from either FitterTool or ToyReader"); }
    return;
}

void ToyStudy::PrintFitInfoKeys() const {
    vector< TString > _managerKeys = GetFitterManagerKeys();
    for (auto key : _managerKeys) { PrintKeysInManager(key); }
    return;
}

void ToyStudy::PrintKeysInManager(TString _managerKey) const {
    vector< TString > managerFitInfoKeys = GetFitterFitInfoKeys(_managerKey);
    vector< TString > readerFitInfoKeys  = GetReaderFitInfoKeys(_managerKey);
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "Print FitterTool FitInfo keys", _managerKey);
    for (const auto & fitInfoKey : managerFitInfoKeys) { MessageSvc::Info("", fitInfoKey); }
    MessageSvc::Line();
    MessageSvc::Info("ToyStudy", (TString) "Print ToyReader FitInfo keys", _managerKey);
    for (const auto & fitInfoKey : readerFitInfoKeys) { MessageSvc::Info("", fitInfoKey); }
    MessageSvc::Line();
    return;
}

vector< TString > ToyStudy::GetFitterFitInfoKeys(TString _managerKey) const {
    vector< TString > managerFitInfoKeys;
    auto &            manager = (*m_managerMap)[_managerKey].FitInfo;
    for (auto & keyFitInfoPair : manager) {
        TString key = keyFitInfoPair.first;
        managerFitInfoKeys.push_back(key);
    }
    return managerFitInfoKeys;
}

vector< TString > ToyStudy::GetReaderFitInfoKeys(TString _managerKey) const {
    vector< TString > readerFitInfoKeys = m_reader.IterateKeysInManager(_managerKey);
    return readerFitInfoKeys;
}

void ToyStudy::CheckFitInfoKeysMatches() const {
    vector< TString > _managerKeys = GetFitterManagerKeys();
    for (auto _key : _managerKeys) { CheckKeysInManagerMatches(_key); }
    return;
}

void ToyStudy::CheckKeysInManagerMatches(TString _managerKey) const {
    vector< TString > _managerFitInfoKeys = GetFitterFitInfoKeys(_managerKey);
    vector< TString > _readerFitInfoKeys  = GetReaderFitInfoKeys(_managerKey);
    CheckKeysMatches(_managerFitInfoKeys, _readerFitInfoKeys);
    return;
}

void ToyStudy::SynchroniseObservable(){
    for (auto _managerKey : m_reader.IterateManagerKeys()) {
        for (auto _holderKey : m_reader.IterateKeysInManager(_managerKey)) {
            RooRealVar * _variable = (*m_managerMap)[_managerKey].FitInfo[_holderKey].var;
            RooArgSet * _observable = new RooArgSet(*_variable);
            m_reader.GetToyTupleReader(_managerKey, _holderKey).SetObservable(_observable);
        }
    }
    return;
}

void ToyStudy::SetIndex(unsigned int _newIndex) {
    for (auto _managerKey : m_reader.IterateManagerKeys()) {
        for (auto _holderKey : m_reader.IterateKeysInManager(_managerKey)) { m_reader.GetToyTupleReader(_managerKey, _holderKey).SetIndex(_newIndex); }
    }
    return;
}

void ToyStudy::Fit(uint _index) {
    MessageSvc::Line();
    MessageSvc::Info("ToyFit Loop: ", "Fitting", "Toy", to_string(m_toyIndex), "...");
    MessageSvc::Line();
    if (m_keysChecked) {
        GetNextToyData(_index);
        ResetFitter();
        m_fitter->Fit(false);
        SaveCurrentToyResult();
        m_toyIndex++;
    } else {
        CheckKeyMessage();
    }
    return;
}

void ToyStudy::GetNextToyData(uint _index) {
    for (auto _managerKey : m_reader.IterateManagerKeys()) {
        for (auto _holderKey : m_reader.IterateKeysInManager(_managerKey)) {
            (*m_managerMap)[_managerKey].FitInfo[_holderKey].dataset  = m_reader.GetToyTupleReader(_managerKey, _holderKey).NextToyData(_index);
            (*m_managerMap)[_managerKey].FitInfo[_holderKey].datahist = m_reader.GetToyTupleReader(_managerKey, _holderKey).BinCurrentToy();
        }
    }
    return;
}

void ToyStudy::ResetFitter() {
    ResetPDFs();
    SmearVariables();
    return;
}

void ToyStudy::ResetPDFs() {
    m_resetter.ResetAllVariables();
    return;
}

void ToyStudy::SmearVariables() {
    for (auto & _variableSmearer : m_variableSmearers) { _variableSmearer.SmearVariable(); }
    for (auto & _correlatedVariablesSmearer : m_correlatedVariablesSmearers) { _correlatedVariablesSmearer.SmearVariables(); }
    return;
}

void ToyStudy::SaveCurrentToyResult() {
    RooFitResult & _fitResults = *(m_fitter->Results());
    m_logger.LogFit(_fitResults, m_fitter->LLOffset() );
    if (ShouldWriteFullOutput(_fitResults)) {
        ConfigurePlotsDirectory();
        m_fitter->SaveResults();
        m_fitter->PlotResults();
        ofstream _outFile(SettingDef::IO::outDir + "/ToyStudy_MinimizerStatus.log");
        m_fitter->PrintResults(_outFile);
        _outFile.close();
        SaveRooFitResults(_fitResults);
    }
}

bool ToyStudy::ShouldWriteFullOutput(const RooFitResult & _fitResults) const {
    int _covarianceQuality = _fitResults.covQual();
    int _fitStatus = _fitResults.status();
    bool _fitFailed = not((_covarianceQuality == 3) && (_fitStatus == 0));
    bool _hitLogFrequency = (((m_toyIndex + 1) % m_logFrequency) == 0);
    // return _fitFailed || _hitLogFrequency;
    return true;
}

void ToyStudy::ConfigurePlotsDirectory() {
    TString _nextToyPlotDir = ToyIO::GetToyFitPlotsDir(m_toyIndex);
    SettingDef::IO::outDir  = _nextToyPlotDir;
    IOSvc::MakeDir(_nextToyPlotDir);
}

void ToyStudy::SaveRooFitResults(RooFitResult & _fitResults) const {
    RooFitResult & fitResults = *(m_fitter->Results());
    TString _rooFitResultDir = ToyIO::GetToyFitRooFitResultDir(m_toyIndex);
    IOSvc::MakeDir(_rooFitResultDir, OpenMode::WARNING);
    TString _outPath = _rooFitResultDir + "/RooFitResult.root";
    auto    _file    = new TFile(_outPath, to_string(OpenMode::RECREATE));    
    _file->cd();
    _fitResults.Write();    
    _file->Close();
}

void ToyStudy::CheckKeyMessage() const {
    MessageSvc::Line();
    MessageSvc::Warning((TString) "ToyStudy", (TString) "Keys for fitter and reader are not checked");
    MessageSvc::Warning((TString) "ToyStudy", (TString) "Please call CheckKeys()");
    MessageSvc::Line();
    return;
}

void ToyStudy::SaveResults(TString _name) {
    TString _resultDir = ToyIO::GetToyFitResultDir();
    IOSvc::MakeDir(_resultDir, OpenMode::WARNING);
    TString _resultPath = _resultDir + TString(fmt::format("/ToyStudy_Job{0}.root", m_studyIndex));
    m_logger.SaveResults(_resultPath, "ToyStudyTuple");
    return;
}

void ToyStudy::SetSeed(uint32_t _seed) {
    for (auto& _smearer : m_variableSmearers){
        _smearer.ResetSeed(_seed);
    }
    for (auto& _smearer : m_correlatedVariablesSmearers){
        _smearer.ResetSeed(_seed);
    }
}

#endif
