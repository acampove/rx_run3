#ifndef CUTHOLDER_HPP
#define CUTHOLDER_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"

#include "ConfigHolder.hpp"
#include "YamlCutReader.hpp"

/**
 * \class CutHolder
 * \brief Cut info
 */
class CutHolder : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    CutHolder();

    /**
     * \brief Constructor with WeightHolder and TString
     */
    CutHolder(const ConfigHolder & _configHolder, TString _cutOption);

    /**
     * \brief Copy constructor
     */
    CutHolder(const CutHolder & _cutHolder);

    /**
     * \brief Equality checkers
     */
    bool operator==(const CutHolder & _cutHolder) const { return (GetConfigHolder() == _cutHolder.GetConfigHolder() && Option() == _cutHolder.Option() && Cut() == _cutHolder.Cut()); };

    bool operator!=(const CutHolder & _cutHolder) const { return !((*this) == _cutHolder); };

    /**
     * \brief Init TCut
     */
    void Init();

    /**
     * @brief      Clone a CutHolder with a new-CutOption!
     * @param[in]  _cutOption  The cut option to use on the cloned CutHolder
     * @return     A new CutHolder (to be initialized) herediting everything except the CutOption
     */
    CutHolder Clone(TString _cutOption, TString _cutExtra = "");

    /**
     * \brief Return option used to create TCut
     */
    const TString Option() const { return m_cutOption; };

    /**
     * \brief Return TCut
     */
    const TCut Cut() const { return m_cut; };

    /**
     * \brief Return TCut
     */
    const Str2CutMap Cuts() const { return m_cuts; };

    /**
     * \brief Return TCut
     * @param  _name [description]
     */
    TCut Cut(TString _name);

    void PrintCuts();

    void PrintInline() const noexcept;

    /**
     * \brief Set option used to create TCut
     * @param  _cutOption [description]
     */
    void SetOption(TString _cutOption) {
        m_cutOption = _cutOption;
        Check();
        return;
    };

    bool IsOption(TString _cutOption) { return _cutOption == "no" ? m_cutOption == _cutOption : m_cutOption.Contains(_cutOption); };

    /*
    void SetExtraCut(TString _cut) {
        if (IsCut(TCut(_cut))) {
            m_cuts["extraCut"] = TCut(_cut);
            m_cuts["cutFULL"] = m_cuts["cutFULL"] && TCut(_cut);
        }
        return;
    };
    void ResetExtraCut(TString _cut) {
        if (IsCut(TCut(_cut))) {
            m_cuts["extraCut"] = TCut(NOCUT);
            m_cuts["cutFULL"] = ReplaceCut(m_cuts["cutFULL"], _cut, "");
        }
        return;
    };
    */

    ConfigHolder GetConfigHolder() const { return m_configHolder; };

  private:
    ConfigHolder m_configHolder;

    TString m_cutOption = "";            // Cut option driving the cut generation
    TCut    m_cut       = TCut(NOCUT);   // The cut to apply

    Str2CutMap m_cuts;

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

    /**
     * \brief Create TCut
     */
    void CreateCut();

    /**
     * \brief Get normalization TCut
     */
    void GetNormalizationCut();

    ClassDef(CutHolder, 1);
};

ostream & operator<<(ostream & os, const CutHolder & _cutHolder);

#endif
