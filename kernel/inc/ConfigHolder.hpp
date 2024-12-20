#ifndef CONFIGHOLDER_HPP
#define CONFIGHOLDER_HPP

#include "EnumeratorSvc.hpp"

#include <iostream>

using namespace RooFit;

// ROOT::Math::IntegratorOneDimOptions::SetDefaultIntegrator("VEGAS");
// ROOT::Math::IntegratorMultiDimOptions::SetDefaultIntegrator("VEGAS");
// RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

/*
    TODO : all classes are now inherithing from ConfigHolder. Have the main flags being private members doesn't ensure that once you create an Instance of ConfigHolder, the variables cannot be modified anymore.
            I would enforce the fact that : YES you can use directly m_ana, m_project in classes which inherits from this we can then remove all the GetAnalysis(), GetYear() etc.. and directly use m_ana, m_year, in the inherithed classes.)
            To ensure NO-ONE can ever modify the status of them, we need to declare the following private members:
            const Analysis m_ana;
            and for each constructor:
            A(int c) : b(c) {} //const member initialized in initialization list
            This has also to be propagated in all classes inherithing from ConfigHolder
*/

/**
 * \class ConfigHolder
 * \brief Config info
 */
class ConfigHolder : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    ConfigHolder();

    /**
     * \brief Constructor with Enumerator
     */
    ConfigHolder(const Prj & _project, const Analysis & _ana, TString _sample, const Q2Bin & _q2bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track);

    ConfigHolder(const Prj & _project, const Analysis & _ana, TString _sample, const Q2Bin & _q2bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem);   // TO BE DROPPED

    ConfigHolder(const Prj & _project, const Analysis & _ana, TString _sample, const Q2Bin & _q2bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const TriggerConf & _triggerConf, const Brem & _brem, const Track & _track);
    
    ConfigHolder( const TString & _project,
                  const TString & _ana,
                  TString _sample="",
                  const TString & _q2bin="global", 
                  const TString & _year="global", 
                  const TString & _polarity="global",
                  const TString & _trigger="global",
                  const TString & _triggerConf="global",
                  const TString & _brem="global", 
                  const TString & _track="global");
    /**
     * \brief Copy constructor
     */
    ConfigHolder(const ConfigHolder & _configHolder);

    /**
     * \brief Equality checkers
     */
    bool operator==(const ConfigHolder & _configHolder) const {
        return ((GetProject() == _configHolder.GetProject()) &&           //
                (GetAna() == _configHolder.GetAna()) &&                   //
                (GetSample() == _configHolder.GetSample()) &&             //
                (GetQ2bin() == _configHolder.GetQ2bin()) &&               //
                (GetYear() == _configHolder.GetYear()) &&                 //
                (GetPolarity() == _configHolder.GetPolarity()) &&         //
                (GetTrigger() == _configHolder.GetTrigger()) &&           //
                (GetTriggerConf() == _configHolder.GetTriggerConf()) &&   //
                (GetBrem() == _configHolder.GetBrem()) &&                 //
                (GetTrack() == _configHolder.GetTrack()));
    };

    bool operator!=(const ConfigHolder & _configHolder) const { return !((*this) == _configHolder); };

    /**
     * \brief Init Config
     */
    void Init();

    const Prj         GetProject() const { return m_project; };
    const Analysis    GetAna() const { return m_ana; };
    const TString     GetSample() const { return m_sample; };
    const Q2Bin       GetQ2bin() const { return m_q2bin; };
    const Year        GetYear() const { return m_year; };
    const Polarity    GetPolarity() const { return m_polarity; };
    const Trigger     GetTrigger() const { return m_trigger; };
    const TriggerConf GetTriggerConf() const { return m_triggerConf; };
    const Brem        GetBrem() const { return m_brem; };
    const Track       GetTrack() const { return m_track; };

    // void SetProject(const Prj & _project) { m_project = _project; };
    // void SetAna(const Analysis & _ana) { m_ana = _ana; };
    // void SetSample(TString _sample) { m_sample = _sample; };
    // void SetQ2Bin(const Q2Bin & _q2bin) { m_q2bin = _q2bin; };
    // void SetYear(const Year & _year) { m_year = _year; };
    // void SetPolarity(const Polarity & _polarity) { m_polarity = _polarity; };
    // void SetTrigger(const Trigger & _trigger) { m_trigger = _trigger; };
    // void SetBrem(const Brem & _brem) { m_brem = _brem; };
    // void SetTrack(const Track & _track) { m_track = _track; };

    const TString GetStringProject() const { return to_string(m_project); };
    const TString GetStringAna() const { return to_string(m_ana); };
    const TString GetStringSample() const { return m_sample; };
    const TString GetStringQ2bin() const { return to_string(m_q2bin); };
    const TString GetStringYear() const { return to_string(m_year); };
    const TString GetStringPolarity() const { return to_string(m_polarity); };
    const TString GetStringTrigger() const { return to_string(m_trigger); };
    const TString GetStringTriggerConf() const { return to_string(m_triggerConf); };
    const TString GetStringBrem() const { return to_string(m_brem); };
    const TString GetStringTrack() const { return to_string(m_track); };

    const TNamed GetNamedProject() const { return TNamed((TString) "Project", to_string(m_project)); };
    const TNamed GetNamedAna() const { return TNamed((TString) "Analysis", to_string(m_ana)); };
    const TNamed GetNamedSample() const { return TNamed((TString) "Sample", m_sample); };
    const TNamed GetNamedQ2bin() const { return TNamed((TString) "Q2bin", to_string(m_q2bin)); };
    const TNamed GetNamedYear() const { return TNamed((TString) "Year", to_string(m_year)); };
    const TNamed GetNamedPolarity() const { return TNamed((TString) "Polarity", to_string(m_polarity)); };
    const TNamed GetNamedTrigger() const { return TNamed((TString) "Trigger", to_string(m_trigger)); };
    const TNamed GetNamedTriggerConf() const { return TNamed((TString) "TriggerConf", to_string(m_triggerConf)); };
    const TNamed GetNamedBrem() const { return TNamed((TString) "Brem", to_string(m_brem)); };
    const TNamed GetNamedTrack() const { return TNamed((TString) "Track", to_string(m_track)); };

    const bool IsProject(TString _project) const { return m_project == hash_project(_project); };
    const bool IsAna(TString _ana) const { return m_ana == hash_analysis(_ana); };
    const bool IsSample(TString _sample) const { return m_sample == _sample; };
    const bool IsQ2bin(TString _q2bin) const { return m_q2bin == hash_q2bin(_q2bin); };
    const bool IsYear(TString _year) const { return m_year == hash_year(_year); };
    const bool IsPolarity(TString _polarity) const { return m_polarity == hash_polarity(_polarity); };
    const bool IsTrigger(TString _trigger) const { return m_trigger == hash_trigger(_trigger); };
    const bool IsTriggerConf(TString _triggerConf) const { return m_triggerConf == hash_triggerconf(_triggerConf); };
    const bool IsBrem(TString _brem) const { return m_brem == hash_brem(_brem); };
    const bool IsTrack(TString _track) const { return m_track == hash_track(_track); };

    const bool IsMC() const;
    const bool IsSignalMC() const;
    const bool IsSignalMCEfficiencySample() const;
    const bool IsCrossFeedSample() const;
    const bool IsLeakageSample() const;

    const bool HasBS() const;
    const bool IsRapidSim() const;
    const bool IsSB(TString _type) const;

    const bool HasMCDecayTuple() const;

    const bool PortingEnabled() const; 

    const vector< TString > GetSamples() const;

    const TString GetTriggerAndConf(TString _option = "") const;

    const TString GetStep() const;

    /**
     * \brief Get UniqueStringID (KEY) for this Config you basically
     *        have already all the info to identify the DecayType,
     *        made public (i.e EventType.GetKey() is ConfigHolder.GetKey() due to inheritance and usage in EfficiecyHolder
     */
    TString GetKey(TString _option = "") const;

    /**
     * \brief Get Particle Names : HEAD, H1, H2, L1, L2, HH, LL
     */
    map< TString, TString > GetParticleNames() const;

    /**
     * \brief Get Particle Labels : HEAD, H1, H2, L1, L2, HH, LL
     */
    map< TString, TString > GetParticleLabels() const;

    /**
     * \brief Get Particle IDs : HEAD, H1, H2, L1, L2, HH, LL
     */
    map< TString, int > GetParticleIDs() const;

    /**
     * \brief Get tuple name for TupleCreate
     */
    TString GetTupleName(TString _option = "") const;

    int GetNBodies(TString _option = "") const;

    bool operator<(const ConfigHolder & _config) const;

    bool operator>(const ConfigHolder & _config) const;

    void Print() const noexcept;

    void PrintInline() const noexcept;

  protected:                     // Protected : who inherits from ConfigHolder EventType / FitConfiguration / CutHolder /)
    Prj         m_project;       // Project definition [RKst,RK,RPhi]
    Analysis    m_ana;           // Ana definition (MM,EE)
    TString     m_sample;        // Tuple Name in TTree after processing Bd2KstEE, etc...
    Q2Bin       m_q2bin;         // Q2 bin name [low,central,high,jps,psi]
    Year        m_year;          // Data taking year [11,12,R1,15,15,R2]
    Polarity    m_polarity;      // Magnet Polarity [MD,MU]
    Trigger     m_trigger;       // Trigger category [L0I,L0L]
    TriggerConf m_triggerConf;   // Trigger types [Inclusive,Exclusive]
    Brem        m_brem;          // Brem Category [0G,1G,2G]
    Track       m_track;         // Track Type [LL,DD]

  private:
    /**
     * \brief Check allowed arguments
     * @param  _configHolder [description]
     */
    bool Check();

    const bool CheckSample(const vector< TString > & _samples) const;

    bool m_debug = false;   // Debug flag
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    ClassDef(ConfigHolder, 1);
};

ostream & operator<<(ostream & os, const ConfigHolder & _configHolder);

extern void ResetSettingDefConfig(ConfigHolder _config) noexcept;

#endif
