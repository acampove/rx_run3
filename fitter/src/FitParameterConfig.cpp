#ifndef FITPARAMETERCONFIG_CPP
#define FITPARAMETERCONFIG_CPP

#include "FitParameterConfig.hpp"

#include "HelperSvc.hpp"
#include "MessageSvc.hpp"
#include "SettingDef.hpp"

ClassImp(FitParameterConfig)

FitParameterConfig::FitParameterConfig()
    : ConfigHolder() {
    m_componentSample = Sample::Empty;
    m_ForRatio = false;
}

FitParameterConfig::FitParameterConfig(const ConfigHolder & _configHolder, const Sample & _componentSample)
    : ConfigHolder(_configHolder) {
    m_componentSample = _componentSample;
    m_ForRatio = false;
}

FitParameterConfig::FitParameterConfig(const Prj & _project, const Analysis & _ana, TString _decaySample, const Q2Bin & _q2bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track, const Sample & _componentSample, const bool forRatio)
    : ConfigHolder(
            _project, 
            _ana, 
            _decaySample, 
            _q2bin, 
            _year, 
            _polarity, 
            _trigger, 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            _brem, 
            _track) {
    m_componentSample = _componentSample;
    m_ForRatio = forRatio;
}

FitParameterConfig::FitParameterConfig(const FitParameterConfig & _fitParameterConfig)
    : ConfigHolder(_fitParameterConfig) {
    m_componentSample = _fitParameterConfig.GetComponentSample();
    m_ForRatio        = _fitParameterConfig.ForRatio();
}

ostream & operator<<(ostream & os, const FitParameterConfig & _fitParameterConfig) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitParameterConfig");
    // MessageSvc::Line(os);
    os << static_cast< const ConfigHolder & >(_fitParameterConfig);
    os << WHITE;
    MessageSvc::Print((ostream &) os, "decay", _fitParameterConfig.GetDecaySample());
    MessageSvc::Print((ostream &) os, "component", to_string(_fitParameterConfig.GetComponentSample()));
    TString _dummy = _fitParameterConfig.ForRatio()? "true" : "false";
    MessageSvc::Print((ostream &) os, "forRatio", _dummy);
    MessageSvc::Line(os);
    os << "\033[F";
    os << RESET;
    return os;
}

ConfigHolder FitParameterConfig::GetConfigHolder() const {
    ConfigHolder _configHolder = ConfigHolder(
            GetProject(), 
            GetAna(), 
            GetDecaySample(), 
            GetQ2bin(), 
            GetYear(), 
            GetPolarity(), 
            GetTrigger(), 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            GetBrem());

    return _configHolder;
}

TString FitParameterConfig::GetKey(TString _option) const {
    TString _key;
    if (!_option.Contains("noprj")) _key += to_string(m_project) + SettingDef::separator;
    if (!_option.Contains("noana")) _key += to_string(m_ana) + SettingDef::separator;
    if (!_option.Contains("noq2")) _key += to_string(m_q2bin) + SettingDef::separator;
    if (!_option.Contains("nosample")) _key += to_string(m_componentSample) + SettingDef::separator;
    if (!_option.Contains("notrg")) _key += to_string(m_trigger) + SettingDef::separator;
    if (!_option.Contains("noyear")) _key += to_string(m_year) + SettingDef::separator;
    if (!_option.Contains("nomag")) _key += to_string(m_polarity) + SettingDef::separator;
    if (!_option.Contains("nobrem")) _key += to_string(m_brem) + SettingDef::separator;
    if (!_option.Contains("notrack")) _key += to_string(m_track) + SettingDef::separator;

    if( _option == "eff_syst"){
        //Special naming , aligned to yamls of systematics labelled columns.
        //i.e RK_jps_R1_L0L-exclusive (for rJPsi eff systematic), will be RK_low_R1... for RK-low 
        _key = TString::Format( "%s_%s_%s_%s-%s", to_string(m_project).Data() , 
                                               to_string(m_q2bin).Data(), 
                                               to_string(m_year).Data(), 
                                               to_string(m_trigger).Data(), 
                                               to_string(m_triggerConf).Data());
    }
    _key = CleanString(_key);
    if ( m_ForRatio){
        _key +="_FULL";
    }
    return _key;
}

TString FitParameterConfig::GetYieldString(TString _option) const {
    TString _leadingN;
    bool    _isSignal = GetComponentSample() == GetSignalSample(GetQ2bin());
    if (_isSignal)
        _leadingN = "nsig_";
    else
        _leadingN = "nbkg_";
    TString _yieldString = _leadingN + to_string(GetComponentSample()) + SettingDef::separator + GetKey("nosample");
    return _yieldString;
}

TString FitParameterConfig::GetRatioName(TString _ratioType) const {
    TString _configKey = GetKey();
    TString _ratioName = _ratioType + "_" + _configKey;
    return _ratioName;
}

TString FitParameterConfig::GetRatioLabel(TString _option) const {
    TString _leadingR   = FitParameterConfig::GetLeadingR(_option, GetQ2bin());
    TString _configKey  = GetKey("noana");
    TString _ratioLabel = _leadingR + "_{" + _configKey + "}";
    return _ratioLabel;
}

TString FitParameterConfig::GetEfficiencyName() const {
    TString _effName = "eff_" + GetKey();
    // if( m_ForRatio) return _effName+"_FULL";
    return _effName;
}

TString FitParameterConfig::GetEfficiencyLabel() const {
    TString _effLabel = "eff_{" + GetKey() + "}^{" + to_string(GetQ2bin()) + "}";
    // if( m_ForRatio) _effLabel+="_{FULL}";
    return _effLabel;
}

TString FitParameterConfig::GetLeadingR(const TString & _option, const Q2Bin & _q2bin) {
    TString _leadingR;
    if (_q2bin == Q2Bin::JPsi) {
        _leadingR = "r" + to_string(Sample::JPsi);
    } else {
        if (_option.Contains("modyieldsigsingle"))
            _leadingR = "r";
        else if (_option.Contains("modyieldsigdouble"))
            _leadingR = "R";
        _leadingR += to_string(GetSignalSample(_q2bin));
    }
    if( _option.Contains("eratioSyst")){
        _leadingR = TString("csyst_") + _leadingR;
    }
    return _leadingR;
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Prj & _project) const {
    FitParameterConfig _other(_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, m_trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Analysis & _ana) const {
    TString _decaySample = m_sample;
    Brem    _brem        = m_brem;
    if (_ana == Analysis::All) {   
        // Mainly used for ratios, decay sample doesn't matter
        _decaySample = "";
    } else {   // Replaces the ana in the decay sample
        _decaySample.ReplaceAll(to_string(m_ana), to_string(_ana));
    }
    if (_ana == Analysis::MM) { _brem = Brem::All; }
    FitParameterConfig _other(m_project, _ana, _decaySample, m_q2bin, m_year, m_polarity, m_trigger, _brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const TString & _decaySample) const {
    FitParameterConfig _other(m_project, m_ana, _decaySample, m_q2bin, m_year, m_polarity, m_trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Q2Bin & _q2bin) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, _q2bin, m_year, m_polarity, m_trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Year & _year) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, _year, m_polarity, m_trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Polarity & _polarity) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, m_year, _polarity, m_trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Trigger & _trigger) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, _trigger, m_brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Brem & _brem) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, m_trigger, _brem, m_track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Track & _track) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, m_trigger, m_brem, _track, m_componentSample , m_ForRatio);
    return move(_other);
}

FitParameterConfig FitParameterConfig::ReplaceConfig(const Sample & _componentSample) const {
    FitParameterConfig _other(m_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, m_trigger, m_brem, m_track, _componentSample , m_ForRatio);
    return move(_other);
}

bool FitParameterConfig::operator==(const FitParameterConfig & _other) const {
    // Decay sample string not used.
    //@dtou: I removed them since they were causing some errors as they were somehow not consistently persisted during the fitting sequence.
    //@dtou: Also, the enumerators are enough to uniquely identify a yield, efficiency or ratio parameter for now.
    return ((m_project  == _other.GetProject()) &&          //
            (m_ForRatio == _other.ForRatio())  &&           // @rquaglia Needed to decouple efficiency for RRatio usage and efficiency for BKGConstraints
            (m_ana == _other.GetAna()) &&                   //
            //(m_sample == _other.GetSample()) &&           //
            (m_q2bin == _other.GetQ2bin()) &&               //
            (m_year == _other.GetYear()) &&                 //
            (m_polarity == _other.GetPolarity()) &&         //
            (m_trigger == _other.GetTrigger()) &&           //
            (m_triggerConf == _other.GetTriggerConf()) &&   //
            (m_brem == _other.GetBrem()) &&                 //
            (m_track == _other.GetTrack()) &&               //
            (m_componentSample == _other.GetComponentSample()));
}

bool FitParameterConfig::operator<(const FitParameterConfig & _other) const {
    // Decay sample string not used.
    //@dtou: I removed them since they were causing some errors as they were somehow not consistently persisted during the fitting sequence.
    //@dtou: Also, the enumerators are enough to uniquely identify a yield, efficiency or ratio parameter for now.
    if (m_project != _other.GetProject())   return m_project < _other.GetProject();
    if (m_ana     != _other.GetAna())           return m_ana < _other.GetAna();
    // if (m_sample != _other.GetSample()) return m_sample < _other.GetSample();
    if (m_q2bin != _other.GetQ2bin())       return m_q2bin < _other.GetQ2bin();
    if (m_year  != _other.GetYear())         return m_year < _other.GetYear();
    if (m_polarity != _other.GetPolarity()) return m_polarity < _other.GetPolarity();
    if (m_trigger != _other.GetTrigger())   return m_trigger < _other.GetTrigger();
    if (m_triggerConf != _other.GetTriggerConf()) return m_triggerConf < _other.GetTriggerConf();
    if (m_brem != _other.GetBrem())         return m_brem < _other.GetBrem();
    if (m_track != _other.GetTrack())       return m_track < _other.GetTrack();
    if (m_componentSample != _other.GetComponentSample()) return m_componentSample < _other.GetComponentSample();
    //@rquaglia those comparators are tricky for boolean flags...be aware of this logic on the map for ParameterPool filling and map.find() callee;
    // return m_ForRatio <= _other.ForRatio() ; 
    if( GetKey() != _other.GetKey()) return GetKey() < _other.GetKey();
    return false;
}

bool FitParameterConfig::operator>(const FitParameterConfig & _other) const { return not((*this) < _other); }
bool FitParameterConfig::IsSignalComponent()const{
    Sample       _signalSample       = GetSignalSample(this->GetQ2bin());
    return m_componentSample == _signalSample; 
};
bool FitParameterConfig::IsLeakageComponent()const{
    return m_componentSample == Sample::Leakage; 
};
bool FitParameterConfig::IsKEtaPrimeComponent()const{
    return m_componentSample == Sample::KEtaPrime; 
};
bool FitParameterConfig::IsCrossFeedComponent()const{
    //Cross-feeds are Bd2Kst, Bu2Kst, BdBu
    if( m_componentSample == Sample::Bd2Kst || m_componentSample == Sample::BdBu || m_componentSample == Sample::Bu2Kst) return true;
    return false; 
}
FitParameterConfig FitParameterConfig::GetSignalConfig(const FitConfiguration & _fitConfiguration) {
    Brem         _brem               = _fitConfiguration.GetBrem();
    ConfigHolder _configHolder       = get< 0 >(_fitConfiguration.GetSignal(_brem));
    Sample       _signalSample       = GetSignalSample(_configHolder.GetQ2bin());
    auto         _fitParameterConfig = FitParameterConfig(_configHolder, _signalSample).ReplaceConfig(_fitConfiguration.GetSampleName(_signalSample));
    return move(_fitParameterConfig);
}
FitParameterConfig FitParameterConfig::GetRatioConfigSyst(const FitConfiguration & _fitConfiguration) {
    auto _signalConfig        = GetSignalConfig( _fitConfiguration);
    auto _fitParameterConfig  = _signalConfig.ReplaceConfig( Analysis::All).ReplaceConfig("").ReplaceConfig( Sample::Empty);
    return move(_fitParameterConfig);
}

FitParameterConfig FitParameterConfig::GetBackgroundConfig(const FitConfiguration & _fitConfiguration, const Sample & _componentSample) {
    ConfigHolder _configHolder       = get< 0 >(_fitConfiguration.GetBackground(_componentSample));
    auto         _fitParameterConfig = FitParameterConfig(_fitConfiguration, _componentSample).ReplaceConfig(_fitConfiguration.GetSampleName(_componentSample));
    return move(_fitParameterConfig);
}

FitParameterConfig GetConfigForRatio(const RatioType & _ratioType, const FitParameterConfig & _configKey) {
    Year               _year        = _configKey.GetYear();
    FitParameterConfig _emptyConfig = FitParameterConfig(Prj::All, Analysis::All, to_string(Sample::Empty), Q2Bin::All, Year::All, Polarity::All, Trigger::All, Brem::All, Track::All, Sample::Empty);

    // branching ratios are independent of configs
    if (_ratioType == RatioType::BranchingFraction) {
        auto _returnConfig = _emptyConfig.ReplaceConfig(_configKey.GetProject()).ReplaceConfig(_configKey.GetQ2bin()).ReplaceConfig(_configKey.GetComponentSample());
        return _returnConfig;
    } else if (_ratioType == RatioType::FLbOverFd) {
        return _emptyConfig;
    }
    // CoM dependence of hadronisation fraction
    else if (_ratioType == RatioType::FsOverFd) {
        if (_year == Year::Y2011 || _year == Year::Y2012 || _year == Year::Run1) {
            auto _returnConfig = _emptyConfig.ReplaceConfig(Year::Run1);
            return _returnConfig;
        }
        if (_year == Year::Y2015 || _year == Year::Y2016 || _year == Year::Y2017 || _year == Year::Y2018 || _year == Year::Run2p1 || _year == Year::Run2p2 || _year == Year::Run2) {
            auto _returnConfig = _emptyConfig.ReplaceConfig(Year::Run2);
            return _returnConfig;
        }
    }
    // Everything else is irrelevant
    else {
        return _configKey;
    }
}

#endif
