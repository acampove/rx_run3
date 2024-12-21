#include "ConfigHolder.hpp"

#include "ConstDef.hpp"
#include "SettingDef.hpp"

#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "vec_extends.h"

#include "TString.h"

#include "core.h"

#include "fmt_ostream.h"

ClassImp(ConfigHolder)

ConfigHolder::ConfigHolder() 
{
    if (SettingDef::debug.Contains("CO")) 
        SetDebug(true);

    if (m_debug) 
        MessageSvc::Debug("ConfigHolder", (TString) "Default");

    m_project     = hash_project(SettingDef::Config::project);
    m_ana         = hash_analysis(SettingDef::Config::ana);
    m_sample      = SettingDef::Config::sample;
    m_q2bin       = hash_q2bin(SettingDef::Config::q2bin);
    m_year        = hash_year(SettingDef::Config::year);
    m_polarity    = hash_polarity(SettingDef::Config::polarity);
    m_trigger     = hash_trigger(SettingDef::Config::trigger);
    m_brem        = hash_brem(SettingDef::Config::brem);
    m_track       = hash_track(SettingDef::Config::track);
    m_triggerConf = hash_triggerconf(SettingDef::Config::triggerConf);
    Init();
}

ConfigHolder::ConfigHolder(
        const Prj         & _project, 
        const Analysis    & _ana, 
        const TString     & _sample,
        const Q2Bin       & _q2bin, 
        const Year        & _year, 
        const Polarity    & _polarity, 
        const Trigger     & _trigger, 
        const TriggerConf & _triggerConf, 
        const Brem        & _brem, 
        const Track       & _track) 
{
    if (SettingDef::debug.Contains("CO")) 
        SetDebug(true);

    if (m_debug) 
        MessageSvc::Debug("ConfigHolder", (TString) "Enumerator");

    m_project     = _project;
    m_ana         = _ana;
    m_sample      = _sample;
    m_q2bin       = _q2bin;
    m_year        = _year;
    m_polarity    = _polarity;
    m_trigger     = _trigger;
    m_brem        = _brem;
    m_track       = _track;
    m_triggerConf = _triggerConf;
    Init();
}

ConfigHolder::ConfigHolder( 
        const TString & _project, 
        const TString & _ana, 
        const TString & _sample,
        const TString & _q2bin, 
        const TString & _year, 
        const TString & _polarity, 
        const TString & _trigger, 
        const TString & _triggerConf, 
        const TString & _brem, 
        const TString & _track) 
{
    if (SettingDef::debug.Contains("CO")) 
        SetDebug(true);

    if (m_debug) 
        MessageSvc::Debug("ConfigHolder", (TString) "TStrings");

    m_project     = _project     != "global" ? hash_project(_project)         : hash_project(SettingDef::Config::project);
    m_ana         = _ana         != "global" ? hash_analysis(_ana)            : hash_analysis(SettingDef::Config::ana);
    m_q2bin       = _q2bin       != "global" ? hash_q2bin(_q2bin)             : hash_q2bin(SettingDef::Config::q2bin);
    m_year        = _year        != "global" ? hash_year(_year)               : hash_year(SettingDef::Config::year);
    m_polarity    = _polarity    != "global" ? hash_polarity(_polarity)       : hash_polarity(SettingDef::Config::polarity);
    m_trigger     = _trigger     != "global" ? hash_trigger(_trigger)         : hash_trigger(SettingDef::Config::trigger);
    m_triggerConf = _triggerConf != "global" ? hash_triggerconf(_triggerConf) : hash_triggerconf(SettingDef::Config::triggerConf);
    m_brem        = _brem        != "global" ? hash_brem(_brem)               : hash_brem(SettingDef::Config::brem);
    m_track       = _track       != "global" ? hash_track(_track)             : hash_track(SettingDef::Config::track);

    m_sample      = _sample;
    Init();
}

ConfigHolder::ConfigHolder(const ConfigHolder & _configHolder) 
{
    if (SettingDef::debug.Contains("CO")) 
        SetDebug(true);

    if (m_debug) 
        MessageSvc::Debug("ConfigHolder", (TString) "ConfigHolder");

    m_project     = _configHolder.GetProject();
    m_ana         = _configHolder.GetAna();
    m_sample      = _configHolder.GetSample();
    m_q2bin       = _configHolder.GetQ2bin();
    m_year        = _configHolder.GetYear();
    m_polarity    = _configHolder.GetPolarity();
    m_trigger     = _configHolder.GetTrigger();
    m_brem        = _configHolder.GetBrem();
    m_track       = _configHolder.GetTrack();
    m_triggerConf = _configHolder.GetTriggerConf();
    Init();
}

ostream & operator<<(ostream & os, const ConfigHolder & _configHolder) 
{
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "ConfigHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "project", to_string(_configHolder.GetProject()));
    MessageSvc::Print((ostream &) os, "ana", to_string(_configHolder.GetAna()));
    MessageSvc::Print((ostream &) os, "sample", _configHolder.GetSample(), _configHolder.GetSample() != "" ? _configHolder.IsMC() ? "(MC) " : "(CL) " : "");
    MessageSvc::Print((ostream &) os, "year", to_string(_configHolder.GetYear()));
    MessageSvc::Print((ostream &) os, "polarity", to_string(_configHolder.GetPolarity()));
    MessageSvc::Print((ostream &) os, "q2bin", to_string(_configHolder.GetQ2bin()));
    MessageSvc::Print((ostream &) os, "trigger", to_string(_configHolder.GetTrigger()));
    MessageSvc::Print((ostream &) os, "triggerConf", to_string(_configHolder.GetTriggerConf()));
    MessageSvc::Print((ostream &) os, "brem", to_string(_configHolder.GetBrem()));
    MessageSvc::Print((ostream &) os, "track", to_string(_configHolder.GetTrack()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

bool ConfigHolder::Check() 
{
    if (!CheckSample({m_sample})) return false;

    // Parameter consistency checks .... to be expanded if some configuration is not fine
    if ((m_ana == Analysis::MM) && (m_brem != Brem::All)) {
        cout << RED << *this << RESET << endl;
        MessageSvc::Error("ConfigHolder", (TString) "Brem::" + to_string(GetBrem()) + " not defined for Analysis::" + to_string(GetAna()), "EXIT_FAILURE");
        return false;
    }

    if ((m_ana == Analysis::ME) && (m_brem == Brem::G2)) {
        cout << RED << *this << RESET << endl;
        MessageSvc::Error("ConfigHolder", (TString) "Brem::" + to_string(GetBrem()) + " not defined for Analysis::" + to_string(GetAna()), "EXIT_FAILURE");
        return false;
    }

    if ((m_project == Prj::RL) || (m_project == Prj::RKS)) {
        if ((m_trigger != Trigger::L0L) || (m_triggerConf != TriggerConf::Inclusive)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("ConfigHolder", (TString) "Trigger::" + to_string(GetTrigger()) + "-TriggerConf::" + to_string(GetTriggerConf()) + " not supported for Prj::" + to_string(GetProject()), "EXIT_FAILURE");
            return false;
        }
    }

    if (m_project == Prj::RKS) {
        if (m_ana != Analysis::MM) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("ConfigHolder", (TString) "Analysis::" + to_string(GetAna()) + " not supported for Prj::" + to_string(GetProject()), "EXIT_FAILURE");
            return false;
        }
        if (m_q2bin != Q2Bin::JPsi) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("ConfigHolder", (TString) "Q2Bin::" + to_string(GetQ2bin()) + " not supported for Prj::" + to_string(GetProject()), "EXIT_FAILURE");
            return false;
        }
    }

    return true;
}

const bool ConfigHolder::CheckSample(const vector< TString > & _samples) const 
{
    for (const auto & _sample : _samples) {
        if (!CheckVectorContains(GetSamples(), _sample) && !(_sample == to_string(Sample::Empty))) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("ConfigHolder", (TString) "Sample::" + _sample + " not in SettingDef::AllowedConf::Samples" + to_string(m_project), "EXIT_FAILURE");
            return false;
        }
    }
    return true;
}

void ConfigHolder::Init() 
{
    if (m_debug) 
    {
        MessageSvc::Debug("ConfigHolder", (TString) "Initialize ...");
        PrintInline();
    }

    auto fail_1 = (m_sample != "") && (!m_sample.Contains(to_string(Sample::Data))) && (!m_sample.Contains("Comb"));
    auto fail_2 = !m_sample.Contains(to_string(m_ana));

    if (fail_1 && fail_2) 
        MessageSvc::Error("Wrong sample", m_sample, "for ana", to_string(m_ana), "EXIT_FAILURE");

    Check();
}

const bool ConfigHolder::IsMC() const 
{ 
    return m_sample.BeginsWith("Bd2") || m_sample.BeginsWith("Bu2") || m_sample.BeginsWith("Bs2") || m_sample.BeginsWith("B2") || m_sample.BeginsWith("Lb2"); 
}

const bool ConfigHolder::IsSignalMC() const 
{
    if (!IsMC()) 
        return false;

    vector< TString > _samples = {};

    switch (m_project) 
    {
        case Prj::RKst:
            _samples = {"Bd2KstMM",       "Bd2KstEE",       "Bd2KstJPsMM",       "Bd2KstJPsEE",       "Bd2KstPsiMM",       "Bd2KstPsiEE",         //
                        "Bd2KstMMvNOFLT", "Bd2KstEEvNOFLT", "Bd2KstJPsMMvNOFLT", "Bd2KstJPsEEvNOFLT", "Bd2KstPsiMMvNOFLT", "Bd2KstPsiEEvNOFLT",   //
                        "Bd2KstEEvPS",    "Bd2KstEEvPSQ2",  "Bd2KstJPsEESS",                                                                      //
                        "Bd2KstGEE",      "Bd2KstGEEv08a",  "Bd2KstGEEv08d",     "Bd2KstGEEv08f",     "Bd2KstGEEv08h"};
            break;
        case Prj::RK:
            _samples = {"Bu2KMM",        "Bu2KEE",        "Bu2KJPsMM",     "Bu2KJPsEE", "Bu2KPsiMM", "Bu2KPsiEE",   //
                        "Bu2KJPsMMv09b", "Bu2KJPsMMv09d", "Bu2KJPsEEv09c", "Bu2KJPsEESS",                           //
                        "Bu2KMMvB0",     "Bu2KEEvMS",                                                               //
                        "Bu2KMMvL0",     "Bu2KEEvL0"};
            break;
        case Prj::RPhi: 
            _samples = {"Bs2PhiMM", "Bs2PhiEE", "Bs2PhiJPsMM", "Bs2PhiJPsEE", "Bs2PhiPsiMM", "Bs2PhiPsiEE"}; 
            break;
        case Prj::RL: 
            _samples = {"Lb2LEE", "Lb2LJPsEE", "Lb2LJPsMM", "Lb2LMM", "Lb2LPsiEE", "Lb2LPsiMM"}; 
            break;
        case Prj::RKS: 
            _samples = {"Bd2KSJPsMM"}; 
            break;

        default: 
            MessageSvc::Error("Wrong project", to_string(m_project), "EXIT_FAILURE"); 
            break;
    }

    CheckSample(_samples);    

    bool _return = find(_samples.begin(), _samples.end(), m_sample) != _samples.end();

    MessageSvc::Info("IsSignalMC (prj)", to_string(m_project));

    std::cout<<RED << "For Sample " << m_sample << " , Status = " << _return<< RESET << std::endl;

    return _return;
}

const bool ConfigHolder::IsSignalMCEfficiencySample() const 
{
    if (!IsSignalMC()) return false;
    // else go ahead.... check the name of the tuple/Q2Bin/Analysis and Project switching.
    TString LL = m_ana == Analysis::EE ? "EE" : "MM";
    switch (m_project) {
        case Prj::RKst:
            if (m_q2bin == Q2Bin::JPsi)
                return m_sample == (TString) "Bd2KstJPs" + LL;
            else if (m_q2bin == Q2Bin::Psi)
                return m_sample == (TString) "Bd2KstPsi" + LL;
            else if ((m_q2bin == Q2Bin::Gamma) && (m_ana == Analysis::EE))
                return m_sample.Contains("Bd2KstGEE");
            else
                return m_sample == (TString) "Bd2Kst" + LL;
            return false;
            break;
        case Prj::RK:
            if (m_q2bin == Q2Bin::JPsi)
                return m_sample == (TString) "Bu2KJPs" + LL;
            else if (m_q2bin == Q2Bin::Psi)
                return m_sample == (TString) "Bu2KPsi" + LL;
            else
                return m_sample == (TString) "Bu2K" + LL;
            return false;
            break;
        case Prj::RPhi:
            if (m_q2bin == Q2Bin::JPsi)
                return m_sample == (TString) "Bs2PhiJPs" + LL;
            else if (m_q2bin == Q2Bin::Psi)
                return m_sample == (TString) "Bs2PhiPsi" + LL;
            else
                return m_sample == (TString) "Bs2Phi" + LL;
            return false;
            break;
        case Prj::RL:
            if (m_q2bin == Q2Bin::JPsi)
                return m_sample == (TString) "Lb2LJPs" + LL;
            else if (m_q2bin == Q2Bin::Psi)
                return m_sample == (TString) "Lb2LPsi" + LL;
            else
                return m_sample == (TString) "Lb2L" + LL;
            return false;
            break;
        case Prj::RKS:
            if (m_q2bin == Q2Bin::JPsi) return m_sample == (TString) "Bd2KSJPs" + LL;
            return false;
            break;
        default: MessageSvc::Error("IsSignalMCEfficiencySample, Wrong project", to_string(m_project), "EXIT_FAILURE");
    }
    return false;
}
const bool ConfigHolder::IsCrossFeedSample() const {
    //Special threatment for RK samples
    if( !(m_project == Prj::RK || m_project == Prj::RKst ) ) return false ; 
    if( !IsMC()) return false ;
    //Do the check on RK only samples type [ here only part-reco samples for which we correctly generate an MCDT in ]
    //TODO : @stephan add the Bu2KstEE,Bu2KstMM when done for 
    vector<TString> _samples = { "Bd2KstEE", "Bd2KstMM", "Bu2KPiEE",
				 "Bu2KstEE", "Bu2KstMM", 
				 "Bd2KPiEE", "Bd2KPiMM" }; //CrossFeedSamples are the ones with a Kst (or KPi) for RK.
    if(find(_samples.begin(), _samples.end(), m_sample) != _samples.end()) return true;
    return false; 
}

const bool ConfigHolder::IsLeakageSample() const {
    //Tag if it's a leakage decay mode for Project-Q2 setup    
    if( !IsMC()) return false ;
    vector<TString> _samples{};
    map< pair<Prj,Q2Bin> , vector<TString>> _Index{
        { {Prj::RK,Q2Bin::Central}, {"Bu2KJPsEE"}},
        { {Prj::RK,Q2Bin::Psi    }, {"Bu2KJPsEE"}},
        { {Prj::RK,Q2Bin::JPsi   }, {"Bu2KPsiEE"}},
        { {Prj::RK,Q2Bin::High   }, {"Bu2KPsiEE"}},
        // { {Prj::RK,Q2Bin::High    }, {"Bu2KPsiEE"}}, TODO: MAYBE ? 
        { {Prj::RKst,Q2Bin::Central}, {"Bd2KstJPsEE"}},
        { {Prj::RKst,Q2Bin::Psi    }, {"Bd2KstJPsEE"}},
        { {Prj::RKst,Q2Bin::JPsi   }, {"Bd2KstPsiEE"}},
        { {Prj::RKst,Q2Bin::High   }, {"Bd2KstPsiEE"}},
        // { {Prj::RK,Q2Bin::High    }, {"Bd2KstPsiEE"}}, TODO: MAYBE ? 
        { {Prj::RPhi,Q2Bin::Central}, {"Bs2PhiJPsEE"}},
        { {Prj::RPhi,Q2Bin::Psi    }, {"Bs2PhiJPsEE"}},  
        { {Prj::RPhi,Q2Bin::JPsi   }, {"Bs2PhiPsiEE"}},
        { {Prj::RKst,Q2Bin::High   }, {"Bs2PhiPsiEE"}},
    };
    pair<Prj,Q2Bin> _thisPair( m_project,m_q2bin);
    if( _Index.find( _thisPair) != _Index.end()){
        bool found = false ;
        for( auto & el : _Index.at(_thisPair)){ 
            if (el == m_sample){ found = true;break;}
        }
        return found; 
    }
    return false;
}

const bool ConfigHolder::HasBS() const{
    //Expected to have Bootstrapped weights if SignalMC, or Data
    return (!IsMC() || IsSignalMC());
}

const bool ConfigHolder::IsRapidSim() const { return IsMC() && (SettingDef::Tuple::option == "rap"); }

const bool ConfigHolder::IsSB(TString _type) const {
    if (_type.Contains("SB")) return !IsMC() && m_sample.EndsWith(_type);
    return false;
}

const bool ConfigHolder::HasMCDecayTuple() const {
    vector< TString > _samples = {};
    switch (m_project) {
        case Prj::RKst: _samples = {"Bu2KPiPiEE", "Bd2KstPi0EEG", "Bd2KstEtaEEG", "Bd2KstEtaEEGvQ2", 
                                    "Bd2KstSwapJPsEE", "Bd2KstSwapJPsMM", "Bd2KstSwapPsiEE", "Bd2KstSwapPsiMM", 
                                    "Lb2pKJPsEE", "Lb2pKJPsMM", 
                                    "Lb2pKPsiEE", "Lb2pKPsiMM", "Bd2KstEtaGEE"
                                    }; break;
        case Prj::RK:   _samples = {"Bd2KstJPsEE", "Bd2KstJPsMM", 
                                    "Bd2KstPsiEE", "Bd2KstPsiMM", 
                                    "Bd2KstEE",    "Bd2KstMM", 
                                    "Bd2KPiEE",    "Bd2KPiMM", 
                                    "Bu2KstEE",    
                                    "Bu2KPiEE",                                 
                                    "Bu2KEtaPrimeGEE", "Bd2KstEtaGEE"
                                    };  break; 
        case Prj::RPhi: _samples = {"Bs2PhiJPsEE", "Bs2PhiJPsMM", 
                                    "Bs2PhiPsiEE", "Bs2PhiPsiMM", 
                                    "Bs2PhiEE",    "Bs2PhiMM"}; break;
        case Prj::RL: break;
        case Prj::RKS: break;
        default: MessageSvc::Error("Wrong project", to_string(m_project), "EXIT_FAILURE"); break;
    }
    CheckSample(_samples);
    bool _inList = false;
    if(find(_samples.begin(), _samples.end(), m_sample) != _samples.end()) _inList = true;
    return IsSignalMC() || _inList ;
}
const bool ConfigHolder::PortingEnabled() const{ 
    if( m_ana != Analysis::EE) return false;  //Only EE samples supports porting.
    if( !IsMC()) return false; 
    vector< TString > _samples = {};
    if( IsSignalMC() ) return true;
    switch (m_project) {
        case Prj::RKst: _samples = {/*signal MC*/
                                    "Bd2KstEE", "Bd2KstJPsEE", "Bd2KstPsiEE", 
                                    /*bkg MC*/
                                    "Bu2KPiPiEE" 
                                    }; break;
        case Prj::RK:   _samples = {/*signal MC*/
                                    "Bu2KEE"  , "Bu2KJPsEE", "Bu2KPsiEE","Bd2KstEE", 
                                    /*bkg MC*/
                                    "Bd2KPiEE",  "Bu2KstEE", "Bu2KPiEE", "Bu2KEtaPrimeGEE", "Bd2KstEtaGEE" 
                                    };  break; 
        case Prj::RPhi: _samples = {"Bs2PhiJPsEE", "Bs2PhiPsiEE", "Bs2PhiEE"/*signal MC*/}; break;
        case Prj::RL: break;
        case Prj::RKS: break;
        default: MessageSvc::Error("Wrong project", to_string(m_project), "EXIT_FAILURE"); break;
    }
    CheckSample(_samples);
    bool _inList = false;
    if(find(_samples.begin(), _samples.end(), m_sample) != _samples.end()) _inList = true;
    if( _inList && !HasMCDecayTuple()) MessageSvc::Error("Invalid configuration, porting enabled only if HasMCDecayTuple == true, fix the code", "","EXIT_FAILURE");
    if( _inList && !IsSignalMC()     ) MessageSvc::Warning("Background sample Porting enabled for this sample", m_sample);
    return _inList;
}
const vector< TString > ConfigHolder::GetSamples() const {
    vector< TString > _samples = {};
    if (m_project != Prj::All)
        _samples = SettingDef::AllowedConf::Samples.at(m_project);
    else
        _samples = {SettingDef::Config::sample};
    return _samples;
}

const TString ConfigHolder::GetTriggerAndConf(TString _option) const {
    TString _name = to_string(m_trigger);
    if (_option == "short") {
        if (m_triggerConf == TriggerConf::Inclusive) _name += "_incl";
        if ((m_triggerConf == TriggerConf::Exclusive) && (m_trigger == Trigger::L0I)) _name += "_incl";
        if ((m_triggerConf == TriggerConf::Exclusive) && (m_trigger == Trigger::L0L)) _name += "_excl";
        if ((m_triggerConf == TriggerConf::Exclusive) && (m_trigger == Trigger::L0H)) _name += "_excl";
        if ((m_triggerConf == TriggerConf::Exclusive2) && (m_trigger == Trigger::L0I)) _name += "_excl";
        if ((m_triggerConf == TriggerConf::Exclusive2) && (m_trigger == Trigger::L0L)) _name += "_incl";
    } else {
        if ((m_triggerConf == TriggerConf::Exclusive) && (m_trigger == Trigger::L0L)) _name += "exclusive";
        if ((m_triggerConf == TriggerConf::Exclusive2) && (m_trigger == Trigger::L0I)) _name += "exclusive";
    }
    return _name;
}

const TString ConfigHolder::GetStep() const {
    TString _step = SettingDef::IO::yaml;
    if (_step != "") _step = SettingDef::separator + _step;
    MessageSvc::Info("ConfigHolder", (TString) "GetStep", _step);
    if (_step == "") MessageSvc::Warning("Empty Step");
    return _step;
}

TString ConfigHolder::GetKey(TString _option) const {
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetKey", _option);
    // NOTE : THIS DEFINES THE KEY MAPPING OF THE FITMANAGER/FITHOLDER
    TString _key("");
    if (!_option.Contains("noprj")) _key += to_string(m_project) + SettingDef::separator;
    if (!_option.Contains("noana")) _key += to_string(m_ana) + SettingDef::separator;
    if (!_option.Contains("noq2")) _key += to_string(m_q2bin) + SettingDef::separator;
    /*
    if (! _option.Contains("notyp")) {
        if (IsMC()) _key += "MC" + SettingDef::separator;
        else        _key += "CL" + SettingDef::separator;
    }
    */
    if (!_option.Contains("notrg")) {
        _key += to_string(m_trigger) + SettingDef::separator;

        if (m_trigger != Trigger::All) {
            if (_option.Contains("addtrgconf")) {
                // Map everything to exclusive/exclusive2 convention, which is used for the Efficiencies
                const std::map< pair< Trigger, TriggerConf >, pair< Trigger, TriggerConf > > _triggerConfs = {
                    {make_pair(Trigger::L0L, TriggerConf::Inclusive), make_pair(Trigger::L0L, TriggerConf::Exclusive2)},    // L0L
                    {make_pair(Trigger::L0L, TriggerConf::Exclusive), make_pair(Trigger::L0L, TriggerConf::Exclusive)},     // L0L
                    {make_pair(Trigger::L0L, TriggerConf::Exclusive2), make_pair(Trigger::L0L, TriggerConf::Exclusive2)},   // L0L
                                                                                                                            //
                    {make_pair(Trigger::L0I, TriggerConf::Inclusive), make_pair(Trigger::L0I, TriggerConf::Exclusive)},     // L0I
                    {make_pair(Trigger::L0I, TriggerConf::Exclusive), make_pair(Trigger::L0I, TriggerConf::Exclusive)},     // L0I
                    {make_pair(Trigger::L0I, TriggerConf::Exclusive2), make_pair(Trigger::L0I, TriggerConf::Exclusive2)},    // L0I
                                                                                                                            //
                    {make_pair(Trigger::L0H, TriggerConf::Inclusive), make_pair(Trigger::L0H, TriggerConf::Exclusive2)},    // L0H
                    {make_pair(Trigger::L0H, TriggerConf::Exclusive), make_pair(Trigger::L0H, TriggerConf::Exclusive)},     // L0H
                    {make_pair(Trigger::L0H, TriggerConf::Exclusive2), make_pair(Trigger::L0H, TriggerConf::Exclusive2)}    // L0H
                };
                auto        _triggerConf = make_pair(m_trigger, m_triggerConf);
                TriggerConf _trgConf     = _triggerConfs.at(_triggerConf).second;
                _key += to_string(_trgConf) + SettingDef::separator;
            }
        }
    }
    if (!_option.Contains("noyear")) _key += to_string(m_year) + SettingDef::separator;
    if (!_option.Contains("nomag")) _key += to_string(m_polarity);
    if (!_option.Contains("nobrem")) _key += to_string(m_brem) + SettingDef::separator;
    if (!_option.Contains("notrack")) _key += to_string(m_track) + SettingDef::separator;

    if (_option.Contains("short")) _key = m_sample + SettingDef::separator + GetTriggerAndConf();
    if (_option.Contains("addsample")) _key = m_sample + SettingDef::separator  + _key;
    _key = CleanString(_key);

    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetKey", _key);
    return _key;
}

map< TString, TString > ConfigHolder::GetParticleNames() const {
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetParticleNames");

    map< TString, TString >            _map;
    vector< pair< TString, TString > > _names;

    _names     = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyleptons");
    _map["L1"] = _names[0].first;
    _map["L2"] = _names[1].first;

    _names     = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyhadrons");
    _map["H1"] = _names[0].first;
    if (m_project != Prj::RK)
        _map["H2"] = _names[1].first;
    else
        _map["H2"] = "K";   // TWEAK FOR TUPLEPROCESS

    _names = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyintermediate");
    if (m_project != Prj::RK) {
        _map["HH"] = _names[0].first;
        _map["LL"] = _names[1].first;
    } else {
        _map["LL"] = _names[0].first;
        _map["HH"] = "K";   // TWEAK FOR TUPLEPROCESS
    }

    _names       = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyhead");
    _map["HEAD"] = _names[0].first;

    if (m_debug) {
        for (const auto & _particle : _map) MessageSvc::Debug("GetParticleNames", _particle.first, "->", _particle.second);
    }
    return _map;
}

map< TString, TString > ConfigHolder::GetParticleLabels() const {
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetParticleLabels");

    map< TString, TString >            _map;
    vector< pair< TString, TString > > _labels;

    _labels    = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyleptons");
    _map["L1"] = _labels[0].second;
    _map["L2"] = _labels[1].second;

    _labels    = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyhadrons");
    _map["H1"] = _labels[0].second;
    if (m_project != Prj::RK)
        _map["H2"] = _labels[1].second;
    else
        _map["H2"] = "K";   // TWEAK FOR TUPLEPROCESS

    _labels = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyintermediate");
    if (m_project != Prj::RK) {
        _map["HH"] = _labels[0].second;
        _map["LL"] = _labels[1].second;
    } else {
        _map["LL"] = _labels[0].second;
        _map["HH"] = "K";   // TWEAK FOR TUPLEPROCESS
    }

    _labels      = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyhead");
    _map["HEAD"] = _labels[0].second;

    if (m_debug) {
        for (const auto & _particle : _map) MessageSvc::Debug("GetParticleLabels", _particle.first, "->", _particle.second);
    }
    return _map;
}

map< TString, int > ConfigHolder::GetParticleIDs() const {
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetParticleIDs");

    map< TString, TString > _names = GetParticleNames();

    map< TString, int > _map;
    for (const auto & _name : _names) {
        if (_name.second == "B0") _map[_name.first] = PDG::ID::Bd;
        if (_name.second == "Bp") _map[_name.first] = PDG::ID::Bu;
        if (_name.second == "Bs") _map[_name.first] = PDG::ID::Bs;

        if (_name.second == "Kst") _map[_name.first] = PDG::ID::Kst;
        if (_name.second == "Phi") _map[_name.first] = PDG::ID::Phi;
        if (_name.second == "JPs") _map[_name.first] = PDG::ID::JPs;
        if (_name.second == "Psi") _map[_name.first] = PDG::ID::Psi;

        if (_name.second == "K" || _name.second == "K1" || _name.second == "K2") _map[_name.first] = PDG::ID::K;   // TODO: this has to be more generic.
        if (_name.second == "Pi") _map[_name.first] = PDG::ID::Pi;                                                 //      Don't check for particle names but types
        if ((_name.second == "E1") || (_name.second == "E2")) _map[_name.first] = PDG::ID::E;                      //      (e.g. "K" for "K1" and "K2")
        if ((_name.second == "M1") || (_name.second == "M2")) _map[_name.first] = PDG::ID::M;
    }

    if (m_debug) {
        for (const auto & _particle : _map) MessageSvc::Debug("GetParticleIDs", _particle.first, "->", to_string(_particle.second));
    }
    return _map;
}

TString ConfigHolder::GetTupleName(TString _option) const {
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetTupleName", _option);

    TString _name = m_sample;
    _name.ReplaceAll("JPs_", "JPs");
    _name.ReplaceAll("Psi_", "Psi");
    _name.ReplaceAll("Eta_", "Eta");
    _name.ReplaceAll("Pi0_", "Pi0");
    /*
    if (! IsMC()) {
        _name.ReplaceAll("Bd2", "");
        _name.ReplaceAll("Bu2", "");
        _name.ReplaceAll("Bs2", "");
    }
    */
    // Trigger and Brem

    // TString _separator = SettingDef::separator;
    TString _separator = "_";   // TO AVOID ISSUES IN INTERACTIVE ROOT

    if (SettingDef::IO::exe.Contains("tupleCreate") || SettingDef::IO::exe.Contains("efficiencyCreate")) {
        // MessageSvc::Warning("ConfigHolder::GetTupleName", (TString) "Allowing sample name to have extra bit for Trigger and Brem (to split any tuple/efficiency Create) ");
        if (m_trigger != Trigger::All) _name += _separator + GetTriggerAndConf();
        if (m_brem != Brem::All) _name += _separator + to_string(m_brem);
    } else {
        if (m_trigger != Trigger::All && !SettingDef::IO::exe.Contains("optimise.py")) { _name += _separator + GetTriggerAndConf(); }
        if (!IsMC() || IsSignalMC()) {
            if (m_brem != Brem::All) {
                if (IsMC()) _name += _separator + to_string(m_brem);
            }
        }
    }

    // SideBands
    if (_option.Contains("SB") || IsSB("SB")) {
        if (_option.Contains("SBU") || IsSB("SBU")) _name += _separator + "SBU";
        if (_option.Contains("SBL") || IsSB("SBL")) _name += _separator + "SBL";
    }

    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "GetTupleName", _name);
    return _name;
}

int ConfigHolder::GetNBodies(TString _option) const 
{
    if( _option == "MCDT")
    {
        MessageSvc::Warning("GetNBodies, Option MCDT not implemented, will be useful in the future since MCDT has different structure of final states than DT");
        return 0;
    }

    switch (m_project) 
    {
        case Prj::RKst: return 4; break;
        case Prj::RK:   return 3; break;
        case Prj::RPhi: return 4; break;
        case Prj::RL:   return 4; break;
        case Prj::RKS:  return 4; break;
        default: 
                       MessageSvc::Error("Wrong project", to_string(m_project), "EXIT_FAILURE");
                       return 0;
    }
}

/**
 * \biref usable to sort a vector of EventType (and map[XXX] get sorted with this)
 */
bool ConfigHolder::operator<(const ConfigHolder & _config) const {
    /*  
        Depends on enumerator values , 
        - First Sort By Project, 
        - Then Sort By Analysis
        - Then Sort by Sample
        - Then Sort by Q2Bin
        - Then Sort by Year
        - Then Sort by Trigger
        - Then Sort by Polarity
        - Then Sort by Brem 
        - Then Sort by Track 
    */
    if (GetProject() != _config.GetProject()) return to_string(GetProject()) < to_string(_config.GetProject());
    if (GetAna()     != _config.GetAna()) return to_string(GetAna()) < to_string(_config.GetAna());
    if (GetSample()  != _config.GetSample()) return GetSample() < _config.GetSample();
    if (GetQ2bin()   != _config.GetQ2bin()) return to_string(GetQ2bin()) < to_string(_config.GetQ2bin());
    if (GetYear()    != _config.GetYear()) return to_string(GetYear()) < to_string(_config.GetYear());
    if (GetTrigger() != _config.GetTrigger()) return to_string(GetTrigger()) < to_string(_config.GetTrigger());
    if (GetPolarity() != _config.GetPolarity()) return to_string(GetPolarity()) < to_string(_config.GetPolarity());
    if (GetBrem() != _config.GetBrem()) return to_string(GetBrem()) < to_string(_config.GetBrem());
    if (GetTrack() != _config.GetTrack()) return to_string(GetTrack()) < to_string(_config.GetTrack());
    return false;
}

bool ConfigHolder::operator>(const ConfigHolder & _config) const { return !((*this) < _config); }

void ConfigHolder::Print() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info("Project", to_string(GetProject()));
    MessageSvc::Info("Ana", to_string(GetAna()));
    MessageSvc::Info("Sample", GetSample());
    MessageSvc::Info("Year", to_string(GetYear()));
    MessageSvc::Info("Polarity", to_string(GetPolarity()));
    MessageSvc::Info("Q2bin", to_string(GetQ2bin()));
    MessageSvc::Info("Trigger", to_string(GetTrigger()));
    MessageSvc::Info("TriggerConf", to_string(GetTriggerConf()));
    MessageSvc::Info("Brem", to_string(GetBrem()));
    MessageSvc::Info("Track", to_string(GetTrack()));
    MessageSvc::Line();
    return;
}

void ConfigHolder::PrintInline() const noexcept {
    TString _toPrint = fmt::format("Project {0} - Ana {1} - Sample {2} - Year {3} - Polarity {4} - Q2Bin {5} - Trigger {6}-{7} - Brem {8} - Track {9}", to_string(GetProject()), to_string(GetAna()), GetSample(), to_string(GetYear()), to_string(GetPolarity()), to_string(GetQ2bin()), to_string(GetTrigger()), to_string(GetTriggerConf()), to_string(GetBrem()), to_string(GetTrack()));
    MessageSvc::Info(Color::Cyan, "ConfigHolder", _toPrint);
    return;
}

void ResetSettingDefConfig(ConfigHolder _configHolder) noexcept {
    MessageSvc::Warning("ConfigHolder", (TString) "Reset SettingDef::Config");
    SettingDef::Config::project     = to_string(_configHolder.GetProject());
    SettingDef::Config::ana         = to_string(_configHolder.GetAna());
    SettingDef::Config::sample      = _configHolder.GetSample();
    SettingDef::Config::q2bin       = to_string(_configHolder.GetQ2bin());
    SettingDef::Config::year        = to_string(_configHolder.GetYear());
    SettingDef::Config::polarity    = to_string(_configHolder.GetPolarity());
    SettingDef::Config::trigger     = to_string(_configHolder.GetTrigger());
    SettingDef::Config::brem        = to_string(_configHolder.GetBrem());
    SettingDef::Config::track       = to_string(_configHolder.GetTrack());
    SettingDef::Config::triggerConf = to_string(_configHolder.GetTriggerConf());
    TString _toPrint                = fmt::format("Project {0} - Ana {1} - Sample {2} - Year {3} - Polarity {4} - Q2Bin {5} - Trigger {6}-{7} - Brem {8} - Track {9}", SettingDef::Config::project, SettingDef::Config::ana, SettingDef::Config::sample, SettingDef::Config::q2bin, SettingDef::Config::year, SettingDef::Config::polarity, SettingDef::Config::trigger, SettingDef::Config::triggerConf, SettingDef::Config::brem, SettingDef::Config::track);
    MessageSvc::Warning("ConfigHolder", _toPrint);
    return;
}
