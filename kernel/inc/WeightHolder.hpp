#ifndef WEIGHTHOLDER_HPP
#define WEIGHTHOLDER_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"

#include "WeightDefRX.hpp"

#include "ConfigHolder.hpp"

#include <iostream>

class TH1;
class TH2;
class TH2Poly;

/**
 * \class WeightHolder
 * \brief Weight info
 */
class WeightHolder : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    WeightHolder();

    /**
     * \brief Constructor with ConfigHolder and TString
     */
    WeightHolder(const ConfigHolder & _configHolder, TString _weightOption);

    /**
     * \brief Copy constructor
     */
    WeightHolder(const WeightHolder & _weightHolder);

    /**
     * \brief Equality checkers
     */
    bool operator==(const WeightHolder & _weightHolder) const { return (GetConfigHolder() == _weightHolder.GetConfigHolder() && Option() == _weightHolder.Option() && Config() == _weightHolder.Config() && Weight() == _weightHolder.Weight()); };

    bool operator!=(const WeightHolder & _weightHolder) const { return !((*this) == _weightHolder); };

    /**
     * \brief Init Weight
     */
    void Init();

    /**
     * \brief      Creates a new instance of the object with same properties than original.
     * @param[in]  _weightConfig  The weight configuration change weight version from one to another using B0_interp/....Bp_interp...
     */
    WeightHolder Clone(TString _weightConfig, TString _weightOption = "");

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

    /**
     * \brief      Retrieve the Weight String for the mapped weights
     * @param[in]  _name   The ID of the weights
     * @param[in]  _force  Throw error if not found
     * @return     The Weight Expression for the Weight ID
     */
    TString Weight(TString _name, bool _force = true);

    void PrintWeights();

    void PrintInline() const noexcept;

    /**
     * \brief Set option used to create Weight
     * @param  _weightOption [description]
     */
    void SetOption(TString _weightOption) {
        m_weightOption = _weightOption;
        Check();
        return;
    };

    void SetOptionMCT() {
        m_weightOption.ReplaceAll(WeightDefRX::ID::TRK, "");
        m_weightOption.ReplaceAll(WeightDefRX::ID::PID, "");
        m_weightOption.ReplaceAll(WeightDefRX::ID::L0, "");
        m_weightOption.ReplaceAll(WeightDefRX::ID::HLT, "");
        m_weightOption.ReplaceAll(WeightDefRX::ID::RECO, "");
        m_weightOption = CleanString(m_weightOption);
        return;
    };

    void SetWeightConfig(const TString & _weightConfig) {
        m_weightConfig = _weightConfig;
        return;
    };

    bool IsOption(TString _weightOption) { return _weightOption == "no" ? m_weightOption == _weightOption : m_weightOption.Contains(_weightOption); };

    bool IsWeight(TString _weight) { return m_weight.Contains(_weight); };

    ConfigHolder GetConfigHolder() const { return m_configHolder; };

    TH1D *  GetWeightPartReco(TString _option);
    TH2 *   GetWeightMapTRK(  TString _option);
    pair < TH1D *, vector < TH2D * > > GetWeightMapsPID(    TString _option);
    pair < TH1D *, vector < TH2D * > > GetWeightMapsPID_fac(TString _option);
    TString GetWeightMapsPID_Name(    TString _option);
    TString GetWeightMapsPID_fac_Name(TString _option);
    TH1D *  GetWeightMapL01D(TString _option);
    
    TH2D *  GetWeightMapL02D(TString _option);
    vector<TH2D>  GetWeightMapL02DBS(TString _option);

    TH1D * GetWeightMapRW1D(TString _option);

    TH1D *  GetWeightMapHLT(TString _option);
    TString GetVeloPars(TString _option);

    vector< pair< TH1D *, vector < TH2D * > > > GetWeightMaps(     TString _type, TString _option, int _nCopies);
    // brief: get weight Maps for Electron BS
    vector< pair< TH1D *, vector < TH2D * > > > GetWeightMapsBS(   TString _type, TString _option, int _nCopies);
    // brief: get weight Maps for BS kde 
    vector< pair< TH1D *, vector < TH2D * > > > GetWeightMapsBSKDE(TString _type, TString _option, int _nCopies);

    TString GetStrFileL0(TString _option);

  private:
    ConfigHolder m_configHolder = ConfigHolder();

    TString m_weightOption = "";
    TString m_weightConfig = "";
    TString m_weight       = TString(NOWEIGHT);

    Str2WeightMap m_weights;

    /**
     * \brief Check allowed arguments
     */
    bool Check();

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    void CreateWeight();
    public : 
    pair< TString, TString > GetStrMapL0(TString _option);
    pair< TString, TString > GetStrMapHLT(TString _option);
    pair< TString, TString > GetStrMapRW1D(TString _option);
    ClassDef(WeightHolder, 1);
};

ostream & operator<<(ostream & os, const WeightHolder & _weightHolder);

#endif
