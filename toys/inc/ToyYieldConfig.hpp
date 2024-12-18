#ifndef TOYYIELDCONFIG_HPP
#define TOYYIELDCONFIG_HPP

#include "TString.h"
#include <map>

// TODO: Cleanup logics

/**
 * \class ToyYieldConfig
 * \brief A data object to configure the yields of a single toy component.
 * \brief Contain data members:
 * \li \c Sample to bookkeep the component.
 * \li \c ScalingFactor to scale the nominal yield to get the mean events.
 * \li \c MeanEvents the mean number of events.
 * \note The \c MeanEvents can be configured using \c ScalingFactor and \c NominalYield from an instance of ToyTupleConfig or by configuring \c MeanEvents directly.
 * \note In the case that both is configured, the larger of the two is used.
 */
class ToyYieldConfig {
  public:
    /**
     * \brief Default constructor.
     */
    ToyYieldConfig();
    /**
     * \brief Constructor that configures all the data members.
     */
    ToyYieldConfig(TString _sample, double _scalingFactor, int _meanEvents);

    ToyYieldConfig(TString _composition);

    ToyYieldConfig(TString _composition, std::map< TString, double> _override);
    /**
     * \brief Copy constructor. Only the three relevant data members are copied.
     */
    ToyYieldConfig(const ToyYieldConfig & _other);
    /**
     * \brief Move constructor. Only the three relevant data members are moved.
     */
    ToyYieldConfig(ToyYieldConfig && _other);
    /**
     * \brief Destructor.
     */
    ~ToyYieldConfig(){};

    /**
     * \brief Returns the \c Sample string.
     */
    TString GetSample() const {
        ThrowIfScaleAndMeanNotSet();
        return m_sample;
    };
    /**
     * \brief Returns the internal \c ScalingFactor.
     */
    double GetScalingFactor() const {
        ThrowIfScaleAndMeanNotSet();
        return m_scalingFactor;
    };
    /**
     * \brief Returns the internal \c MeanEvents.
     */
    int GetMeanEvents() const {
        ThrowIfScaleAndMeanNotSet();
        return m_meanEvents;
    };
    /**
     * \brief Returns the name of the component tree.
     */
    TString GetComponentName() const { return m_sample; };
    /**
     * \brief Returns true if the internal \c ScalingFactor or \c MeanEvents is configured.
     */
    bool IsSet() const;
    /**
     * \brief \c ScalingFactor is multiplied by the \c NominalYield.
     * \brief This will be \c MeanEvents if it is not set.
     * \brief If both \c ScalingFactor and \c MeanEvents is configured, the larger of \c MeanEvents or \c ScalingFactor multiplied by \c NominalYield is used.
     */
    void EvaluateGeneratorMean(double nominalYield);
    /**
     * \brief Prints the value of the internal data members.
     */
    void Print() const;
    /**
     * \brief Equal operator. Only the three relevant data member values are copied.
     */
    ToyYieldConfig & operator=(const ToyYieldConfig & _other);
    /**
     * \brief Move equal operator. Only the three relevant data members are moved.
     */
    ToyYieldConfig & operator=(ToyYieldConfig && _other);

  private:
    enum class YieldVariablesStatus { NoneSet, ScaleSet, MeanSet, BothSet };
    void ResetYieldVariables();
    void CheckToyTupleYieldVariables();
    void EvaluateSetFlags();
    void ThrowIfScaleAndMeanNotSet() const;
    void WarnIfBothScaleAndMeanGiven() const;
    void NotifyIfOnlyOneGiven() const;
    void ScaledMean(double nominalYield);
    void UseLargerMean(double nominalYield);
    void GeneratorMeanMessage(int ScaledMean) const;

  private:
    YieldVariablesStatus m_yieldVariableStatus;
    TString              m_sample;
    double               m_scalingFactor;
    int                  m_meanEvents;
    const double         m_noScalingFactorFlag = -1;
    const int            m_noMeanEventsFlag    = -1;
};

#endif