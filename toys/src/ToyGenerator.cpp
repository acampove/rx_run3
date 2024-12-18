#ifndef TOYGENERATOR_CPP
#define TOYGENERATOR_CPP

#include "ToyGenerator.hpp"

#include "MessageSvc.hpp"
#include "ParserSvc.hpp"
#include "HelperSvc.hpp"
#include "yamlcpp.h"

ToyGenerator::ToyGenerator(const ToyGenerator & _other) { m_tupleGenerators = _other.m_tupleGenerators; }

ToyGenerator::ToyGenerator(const ToyGenerator && _other) { m_tupleGenerators = _other.m_tupleGenerators; }

void ToyGenerator::GetTupleGenerators() {
    MessageSvc::Line();
    MessageSvc::Info("ToyGenerator", (TString) "GetTupleGenerators");
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
            if (_configurationsTmp.size() == 0) MessageSvc::Error("ToyGenerator", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");

            GetTupleGenerators(_configurationsTmp);
        }
        ResetSettingDefConfig(_conf);
    } else if (_configurations.size() != 0) {
        GetTupleGenerators(_configurations);
    } else {
        MessageSvc::Error("ToyGenerator", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");
    }
    MessageSvc::Line();
    SettingDef::Toy::configurations = _configurations;
    SettingDef::Toy::yamls          = _yamls;
    return;
}

map< ConfigHolder, map<TString, double> > ToyGenerator::ParseConfigurationOverride( TString _overrideFile ){
    auto parserYaml = YAML::LoadFile(ExpandEnvironment(_overrideFile).Data());
    map< ConfigHolder, map<TString, double> > _overrides;
    for (YAML::iterator _iter = parserYaml.begin(); _iter != parserYaml.end(); ++_iter) {
        TString _configString = _iter->first.as< TString >();
        auto *  _strCollection = ((TString) _configString).ReplaceAll(" ", "").Tokenize("-");
        Prj _project           = hash_project(TString(((TObjString *) (*_strCollection).At(0))->String()));
        Analysis _ana          = hash_analysis(TString(((TObjString *) (*_strCollection).At(1))->String()));
        Q2Bin _q2bin           = hash_q2bin(TString(((TObjString *) (*_strCollection).At(2))->String()));
        Trigger _trigger       = hash_trigger(TString(((TObjString *) (*_strCollection).At(3))->String()));
        Year _year             = hash_year(TString(((TObjString *) (*_strCollection).At(4))->String()));

        ConfigHolder _configHolder = ConfigHolder(_project, _ana, "", _q2bin, _year, 
                                                  hash_polarity(SettingDef::Config::polarity), 
                                                  _trigger,
                                                  hash_brem(SettingDef::Config::brem), 
                                                  hash_track(SettingDef::Config::track));
        map< TString, double > _scaleConfigs;

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

void ToyGenerator::GetTupleGenerators(vector< FitConfiguration > _configurations) {
    MessageSvc::Line();
    MessageSvc::Info("ToyGenerator", (TString) "GetTupleGenerators", to_string(_configurations.size()));
    map< ConfigHolder, map<TString, double> >  _configurationOverride;
    bool overrideIt = false;
    if( SettingDef::Toy::configurationOverrideFile != ""){
        overrideIt = true;
         _configurationOverride = ParseConfigurationOverride(SettingDef::Toy::configurationOverrideFile);
    }
    for (auto & _configuration : _configurations) {
        ConfigHolder _configHolder = static_cast< ConfigHolder >(_configuration);
        _configHolder.Print();
        MessageSvc::Info("Composition", to_string(_configuration.Composition().size()));
        MessageSvc::Line();
        vector< ToyYieldConfig > _yieldConfigs;
        if( overrideIt){
            map< TString, double > _overrideConfigs = _configurationOverride[_configHolder];
            for (auto & _composition : _configuration.Composition()) {
                _yieldConfigs.emplace_back(_composition, _overrideConfigs);
            }
        }else{
            for (auto & _composition : _configuration.Composition()) {
                        _yieldConfigs.emplace_back(_composition);
            }
        }
        MessageSvc::Line();
        m_tupleGenerators.emplace_back(_configHolder, _yieldConfigs);
    }
    MessageSvc::Info("ToyGenerator", to_string(m_tupleGenerators.size()));
    MessageSvc::Line();
    return;
}

void ToyGenerator::Generate(uint _index) {
    MessageSvc::Info("ToyGenerator", (TString) "Generating", to_string(m_tupleGenerators.size()), "ToyTupleGenerator(s) ...");
    int i = 0;
    for (auto & _tupleGenerator : m_tupleGenerators) {
        MessageSvc::Info("ToyTupleGenerator", (TString) "Generating", to_string(i), "ToyTupleGenerator ...");
        _tupleGenerator.SetToyIndex(_index);
        _tupleGenerator.Generate();
        i++;
    }
    return;
}

void ToyGenerator::SetSeed(unsigned long _seed) {
    ToyTupleComponentGenerator::SetSeed(_seed);
    return;
}

#endif