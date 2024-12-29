#ifndef TOYREADER_CPP
#define TOYREADER_CPP

#include "ToyReader.hpp"
#include "EnumeratorSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"
#include <utility>
#include "yamlcpp.h"

void ToyReader::GetToyTupleReaders() {
    MessageSvc::Line();
    MessageSvc::Info("ToyReader", (TString) "GetToyTupleReaders");
    MessageSvc::Line();

    ParserSvc _parser("quiet");

    vector< FitConfiguration >                          _configurations = SettingDef::Toy::configurations;
    map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamls          = SettingDef::Toy::yamls;

    if (_yamls.size() != 0) {
        ConfigHolder _conf = ConfigHolder();
        for (auto _yaml : _yamls) {
            SettingDef::Config::project = to_string(_yaml.first.first);
            SettingDef::Toy::option     = _yaml.second.second;

            _parser.Init(_yaml.second.first);

            vector< FitConfiguration > _configurationsTmp = SettingDef::Toy::configurations;
            if (_configurationsTmp.size() == 0) MessageSvc::Error("ToyReader", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");

            GetToyTupleReaders(_configurationsTmp);
        }
        ResetSettingDefConfig(_conf);
    } else if (_configurations.size() != 0) {
        GetToyTupleReaders(_configurations);
    } else {
        MessageSvc::Error("ToyReader", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");
    }
    MessageSvc::Line();
    SettingDef::Toy::configurations = _configurations;
    SettingDef::Toy::yamls          = _yamls;
    return;
}

map< ConfigHolder, map<TString, double> > ToyReader::ParseConfigurationOverride( TString _overrideFile ){
    map< ConfigHolder, map<TString, double> > _overrides;

    if ( _overrideFile == "" ) return _overrides;

    auto parserYaml = YAML::LoadFile(ExpandEnvironment(_overrideFile).Data());

    for (YAML::iterator _iter = parserYaml.begin(); _iter != parserYaml.end(); ++_iter) {
        TString _configString = _iter->first.as< TString >();

        auto *  _strCollection = ((TString) _configString).ReplaceAll(" ", "").Tokenize("-");
        Prj _project           = hash_project(TString(((TObjString *) (*_strCollection).At(0))->String()));
        Analysis _ana          = hash_analysis(TString(((TObjString *) (*_strCollection).At(1))->String()));
        Q2Bin _q2bin           = hash_q2bin(TString(((TObjString *) (*_strCollection).At(2))->String()));
        Trigger _trigger       = hash_trigger(TString(((TObjString *) (*_strCollection).At(3))->String()));
        Year _year             = hash_year(TString(((TObjString *) (*_strCollection).At(4))->String()));

        ConfigHolder _configHolder = ConfigHolder(
                _project, 
                _ana, 
                "", 
                _q2bin, 
                _year, 
                hash_polarity(SettingDef::Config::polarity), 
                _trigger,
                hash_triggerconf(SettingDef::Config::triggerConf), 
                hash_brem(SettingDef::Config::brem), 
                hash_track(SettingDef::Config::track));

        std::map< TString, double > _scaleConfigs;

        auto _scalesNode = _iter->second;
        for (YAML::iterator _sampleIter = _scalesNode.begin(); _sampleIter != _scalesNode.end(); ++_sampleIter){
            TString _sampleID = _sampleIter->first.as< TString >();
            double _scale = _sampleIter->second.as< double >();
            if (_scale < 0) MessageSvc::Error("ParseToyConfigurationOverride negative scale", _sampleID, to_string(_scale), "EXIT_FAILURE");
            _scaleConfigs[_sampleID] = _scale;
        }
        _overrides[_configHolder] = _scaleConfigs;
    }

    return _overrides;
}

void ToyReader::GetToyTupleReaders(vector< FitConfiguration > _configurations) {
    MessageSvc::Line();
    MessageSvc::Info("ToyReader", (TString) "GetToyTupleReaders", to_string(_configurations.size()));

    map< ConfigHolder, map<TString, double> > _configurationOverride = ParseConfigurationOverride(SettingDef::Toy::configurationOverrideFile);
    for (auto & _configuration : _configurations) {
        ConfigHolder _configHolder = static_cast< ConfigHolder >(_configuration);
        _configHolder.Print();
        MessageSvc::Info("ToyReader"  , (TString) "Configuration", _configHolder.GetKey());
        MessageSvc::Info("Composition", to_string(_configuration.Composition().size()));
        MessageSvc::Line();
        map< TString, double > _overrideConfigs = _configurationOverride[_configHolder];
        vector< ToyYieldConfig > _yieldConfigs;
        for (auto & _composition : _configuration.Composition()) {
            _yieldConfigs.emplace_back(_composition, _overrideConfigs);
            MessageSvc::Line();
        }
        TString _project         = to_string(_configHolder.GetProject());
        TString _q2Bin           = "q2" + to_string(_configHolder.GetQ2bin());
        TString _managerKey      = _project + SettingDef::separator + _q2Bin;
        TString _configHolderKey = _configHolder.GetKey();
        AddToManagerKeyIterator(_managerKey);
        AddToReaderKeyIterator(_managerKey, _configHolderKey);
        m_keysToToyTupleReaderMap[_managerKey].insert( { _configHolderKey, ToyTupleReader(_configHolder, _yieldConfigs) } );
        // m_keysToToyTupleReaderMap[_managerKey][_configHolderKey] = ToyTupleReader(_configHolder, _yieldConfigs);
    }
    MessageSvc::Info("ToyReader", to_string(m_keysToToyTupleReaderMap.size()));
    MessageSvc::Line();
    return;
}

void ToyReader::AddToManagerKeyIterator(TString _managerKey) {
    auto & map = m_keysToToyTupleReaderMap;
    if (map.find(_managerKey) == map.end()) { m_managerKeys.push_back(_managerKey); }
}

void ToyReader::AddToReaderKeyIterator(TString _managerKey, TString _configHolderKey) {
    auto & map = m_keysToToyTupleReaderMap[_managerKey];
    if (map.find(_managerKey) == map.end()) { m_managerToReaderKeyMap[_managerKey].push_back(_configHolderKey); }
}

ToyTupleReader & ToyReader::GetToyTupleReader(TString _managerKey, TString _configHolderKey) { return m_keysToToyTupleReaderMap[_managerKey][_configHolderKey]; }

vector< TString > ToyReader::IterateManagerKeys() const { return m_managerKeys; }

vector< TString > ToyReader::IterateKeysInManager(TString _managerKey) const {
    vector< TString > _keysInManager = m_managerToReaderKeyMap.at(_managerKey);
    return _keysInManager;
}

#endif
