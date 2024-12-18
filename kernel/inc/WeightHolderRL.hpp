#ifndef WEIGHTHOLDERRL_HPP
#define WEIGHTHOLDERRL_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"

#include "ConfigHolder.hpp"

#include <iostream>

/**
 * \class WeightHolderRL
 * \brief Weight info
 */
class WeightHolderRL : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    WeightHolderRL();

    /**
     * \brief Constructor with ConfigHolder and TString
     */
    WeightHolderRL(const ConfigHolder & _configHolder, TString _weightOption);

    /**
     * \brief Copy constructor
     */
    WeightHolderRL(const WeightHolderRL & _weightHolder);

    /**
     * \brief Init Weight
     */
    void Init();

    /**
     * \brief Get option used to create Weight
     */
    const TString Option() const { return m_weightOption; };

    const TString Config() const { return m_weightConfig; };

    /**
     * \brief Get Weight. NOTE m_weight for "no" must return "(1.)"
     */
    const TString Weight() const { return m_weight; };

    /**
     * \brief Return Weight
     */
    const Str2WeightMap Weights() const { return m_weights; };

    ConfigHolder GetConfigHolder() const { return m_configHolder; };

  private:
    ConfigHolder m_configHolder = ConfigHolder();

    TString m_weightOption = "";
    TString m_weightConfig = "";
    TString m_weight       = TString(NOWEIGHT);

    Str2WeightMap m_weights;

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Create WeightMC
     */
    void CreateWeightMC();

    /**
     * \brief Create WeightCL
     */
    void CreateWeightCL();

    ClassDef(WeightHolderRL, 1);
};

ostream & operator<<(ostream & os, const WeightHolderRL & _weightHolder);

#endif
