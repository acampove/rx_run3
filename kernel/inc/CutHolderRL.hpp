#ifndef CUTHOLDERRL_HPP
#define CUTHOLDERRL_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "ConfigHolder.hpp"

#include <iostream>

#include "TCut.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TString.h"

/**
 * \class CutHolderRL
 * \brief Cut info
 */
class CutHolderRL : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    CutHolderRL();

    /**
     * \brief Constructor with WeightHolder and TString
     */
    CutHolderRL(const ConfigHolder & _configHolder, TString _cutOption);

    /**
     * \brief Copy constructor
     */
    CutHolderRL(const CutHolderRL & _cutHolder);

    /**
     * \brief Init TCut
     */
    void Init();

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

    ConfigHolder GetConfigHolder() const { return m_configHolder; };

  private:
    ConfigHolder m_configHolder;

    TString m_cutOption = "";            // Cut option driving the cut generation
    TCut    m_cut       = TCut(NOCUT);   // The cut to apply

    Str2CutMap m_cuts;

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
     * \brief Get background TCut
     */
    TCut GetBackgroundCut();

    /**
     * \brief Get brem TCut
     */
    TCut GetBremCut();

    /**
     * \brief Get HOP TCut
     */
    TCut GetHOPCut();

    /**
     * \brief Get isSingle TCut
     */
    TCut GetIsSingleCut();

    /**
     * \brief Get MVA TCut
     */
    TCut GetMVACut();

    /**
     * \brief Get part-reco TCut
     */
    TCut GetPartRecoCut();

    /**
     * \brief Get PID TCut
     */
    TCut GetPIDCut();

    /**
     * \brief Get pre-selection TCut
     */
    TCut GetPreSelectionCut();

    /**
     * \brief Get q2 TCut
     */
    TCut GetQ2Cut();

    /**
     * \brief Get nSPD TCut
     */
    TCut GetSPDCut();

    /**
     * \brief Get track TCut
     */
    TCut GetTrackCut();

    /**
     * \brief Get trigger TCut
     */
    TCut GetTriggerCut();

    /**
     * \brief Get trigger TCut
     */
    TCut GetL0Cut();

    /**
     * \brief Get trigger TCut
     */
    TCut GetHLT1Cut();

    /**
     * \brief Get trigger TCut
     */
    TCut GetHLT2Cut();

    /**
     * \brief Get TruthMatch TCut
     */
    TCut GetTruthMatchCut();

    ClassDef(CutHolderRL, 1);
};

ostream & operator<<(ostream & os, const CutHolderRL & _cutHolder);

#endif
