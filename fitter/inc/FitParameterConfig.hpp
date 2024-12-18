#ifndef FITPARAMETERCONFIG_HPP
#define FITPARAMETERCONFIG_HPP

#include "ConfigHolder.hpp"
#include "FitConfiguration.hpp"

/**
 * \class FitParameterConfig
 * \brief
 */
class FitParameterConfig : public ConfigHolder {

  public:
    /**
     * \brief      Constructs the FitParameterConfig object from global flags in SettingDef.
     */
    FitParameterConfig();

    /**
     * \brief      Constructs the FitParameterConfig object from a ConfigHolder and Sample.
     * @param[in]  _configHolder     The ConfigHolder
     * @param[in]  _componentSample  The component's Sample
     */
    FitParameterConfig(const ConfigHolder & _configHolder, const Sample & _componentSample);

    /**
     * \brief      Constructs the object.
     * @param[in]  _project          The project
     * @param[in]  _ana              The ana
     * @param[in]  _decaySample      The decay sample string
     * @param[in]  _q2bin            The q2bin
     * @param[in]  _year             The year
     * @param[in]  _polarity         The polarity
     * @param[in]  _trigger          The trigger
     * @param[in]  _brem             The brem
     * @param[in]  _componentSample  The component sample
     * @param[in]  _componentSample  The forRatio  flag
     */
    FitParameterConfig(const Prj & _project, const Analysis & _ana, TString _decaySample, const Q2Bin & _q2bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track, const Sample & _componentSample, bool forRatio = false);
    /**
     * \brief Copy constructor
     */
    FitParameterConfig(const FitParameterConfig & _fitParameterConfig);

    // void SetDecaySample(TString _decaySample) { m_sample = _decaySample; };
    // void SetComponentSample(const Sample & _componentSample) { m_componentSample = _componentSample; }

    /**
     * \brief      Gets the configHolder.
     * @return     The project.
     */
    ConfigHolder GetConfigHolder() const;

    /**
     * \brief      Gets the decay sample.
     * @return     The decay sample.
     */
    TString GetDecaySample() const { return m_sample; };

    /**
     * \brief      Gets the component sample.
     * @return     The component sample.
     */
    Sample GetComponentSample() const { return m_componentSample; };

    /**
     * \brief      Gets the string key. This class is responsible for bookkeping. So, the string key should be used for printouts and debugging only.
     * @param[in]  _option  The option
     * @return     The key.
     */
    TString GetKey(TString _option = "") const;

    /**
     * \brief      Gets the string for the yield parameter associated to this FitParameterConfig.
     * @param[in]  _option  The option
     * @return     The yield string.
     */
    TString GetYieldString(TString _option = "") const;

    /**
     * \brief      Gets the name string for the ratio parameter associated to this FitParameterConfig.
     * @param[in]  _option  The option
     * @return     The ratio name.
     */
    TString GetRatioName(TString _ratioType = "") const;

    /**
     * \brief      Gets the label string for the ratio parameter associated to this FitParameterConfig.
     * @param[in]  _option  The option
     * @return     The ratio label.
     */
    TString GetRatioLabel(TString _option = "") const;

    /**
     * \brief      Gets the name string for the efficiency parameter associated to this FitParameterConfig.
     * @return     The efficiency name.
     */
    TString GetEfficiencyName() const;

    /**
     * \brief      Gets the label string for the ratio efficiency associated to this FitParameterConfig.
     * @return     The efficiency label.
     */
    TString GetEfficiencyLabel() const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Prj.
     * @param[in]  _project  The new project.
     * @return     Copy of this FitParameterConfig with a different Prj.
     */
    FitParameterConfig ReplaceConfig(const Prj & _project) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Analysis.
     * \brief      Also changes the decay sample string to mirror the change in Analysis.
     * @param[in]  _ana  The new Analysis.
     * @return     Copy of this FitParameterConfig with a different Analysis.
     */
    FitParameterConfig ReplaceConfig(const Analysis & _ana) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the decay sample string.
     * @param[in]  _decaySample  The new decay sample string.
     * @return     Copy of this FitParameterConfig with a different decay sample string.
     */
    FitParameterConfig ReplaceConfig(const TString & _decaySample) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Q2Bin.
     * @param[in]  _q2bin  The new Q2Bin.
     * @return     Copy of this FitParameterConfig with a different Q2Bin.
     */
    FitParameterConfig ReplaceConfig(const Q2Bin & _q2bin) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Year.
     * @param[in]  _year  The new Year.
     * @return     Copy of this FitParameterConfig with a different Year.
     */
    FitParameterConfig ReplaceConfig(const Year & _year) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Polarity.
     * @param[in]  _polarity  The new Polarity
     * @return     Copy of this FitParameterConfig with a different Polarity.
     */
    FitParameterConfig ReplaceConfig(const Polarity & _polarity) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Trigger.
     * @param[in]  _trigger  The new Trigger
     * @return     Copy of this FitParameterConfig with a different Trigger.
     */
    FitParameterConfig ReplaceConfig(const Trigger & _trigger) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Brem.
     * @param[in]  _brem  The new Brem.
     * @return     Copy of this FitParameterConfig with a different Brem.
     */
    FitParameterConfig ReplaceConfig(const Brem & _brem) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the Track.
     * @param[in]  _track  The new Track.
     * @return     Copy of this FitParameterConfig with a different Track.
     */
    FitParameterConfig ReplaceConfig(const Track & _track) const;

    /**
     * \brief      Create a copy of this FitParameterConfig and replace the component Sample.
     * @param[in]  _componentSample  The new component Sample
     * @return     Copy of this FitParameterConfig with a different component Sample.
     */
    FitParameterConfig ReplaceConfig(const Sample & _componentSample) const;

    /**
     * \brief      Equality operator. Needed to bookkeep things in c++ maps.
     * \note       Decay sample string is not used for the equality check.
     * @param[in]  _other  The other FitParameterConfig.
     * @return     Boolean of equality.
     */
    bool operator==(const FitParameterConfig & _other) const;

    bool operator!=(const FitParameterConfig & _other) const { return !((*this) == _other); };

    /**
     * \brief      Comparison operator. Needed to bookkeep things in c++ maps.
     * \note       Decay sample string is not used for the comparison.
     * @param[in]  _other  The other FitParameterConfig.
     * @return     Boolean result of comparison.
     */
    bool operator<(const FitParameterConfig & _other) const;

    /**
     * \brief      Comparison operator. Needed to bookkeep things in c++ maps.
     * \note       Decay sample string is not used for the comparison.
     * @param[in]  _other  The other FitParameterConfig.
     * @return     Boolean result of comparison.
     */
    bool operator>(const FitParameterConfig & _other) const;

    /**
     * \brief      Gets the signal FitParameterConfig from an instance of FitConfiguration.
     * @param[in]  _fitConfiguration  The FitConfiguration.
     * @return     The signal FitParameterConfig.
     */
    static FitParameterConfig GetSignalConfig(const FitConfiguration & _fitConfiguration);
    static FitParameterConfig GetRatioConfigSyst(const FitConfiguration & _fitConfiguration);
    /**
     * \brief      Gets a background FitParameterConfig from an instance of FitConfiguration.
     * @param[in]  _fitConfiguration  The FitConfiguration
     * @param[in]  _componentSample   The component Sample of the background to retrieve.
     * @return     The background FitParameterConfig retrieve using Sample passed.
     */
    static FitParameterConfig GetBackgroundConfig(const FitConfiguration & _fitConfiguration, const Sample & _componentSample);

    static TString GetLeadingR(const TString & _option, const Q2Bin & _q2bin);

    void SetForRatio( bool forRRatio ){ m_ForRatio=  forRRatio;};
    bool ForRatio() const{ return m_ForRatio;}
    bool IsSignalComponent() const;
    bool IsLeakageComponent() const;
    bool IsKEtaPrimeComponent()const;
    bool IsCrossFeedComponent() const;
  private:
    bool m_ForRatio;
    Sample m_componentSample;
    ClassDef(FitParameterConfig, 1);
};

/**
 * \brief      Printout operator for FitParameterConfig.
 *
 * @param      os                  The operating system
 * @param[in]  _fitParameterConfig FitParameterConfig
 *
 * @return     Prints the Enumerators and configs held by the instance of FitParameterConfig passed.
 */
ostream & operator<<(ostream & os, const FitParameterConfig & _fitParameterConfig);

FitParameterConfig GetConfigForRatio(const RatioType & _ratioType, const FitParameterConfig & _configKey);

#endif
