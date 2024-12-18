#ifndef CUTHOLDERRKST_HPP
#define CUTHOLDERRKST_HPP

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
 * \class CutHolderRKst
 * \brief Cut info
 */
class CutHolderRKst : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    CutHolderRKst();

    /**
     * \brief Constructor with WeightHolder and TString
     */
    CutHolderRKst(const ConfigHolder & _configHolder, TString _cutOption);

    /**
     * \brief Copy constructor
     */
    CutHolderRKst(const CutHolderRKst & _cutHolder);

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
     * \brief Get bremET TCut
     */
    TCut GetBremETCut();

    /**
     * \brief Get HOP TCut
     */
    TCut GetHOPCut();

    /**
     * \brief Get isSingle TCut
     */
    TCut GetIsSingleCut();

    /**
     * \brief Get invariant mass TCut
     */
    TCut GetBMassCut();
    
    /**
     * \brief Get constraint mass TCut for same sign data
     */
    TCut GetCombBVeto();

    /**
     * \brief Get nTracks cut for BKIN-MULT sPlots
     */
    TCut GetSPlotnTracks();

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
     * \brief Get KstMass cut
     */
    TCut GetKstMassCut();

    /**
     * \brief Get Min lepton PT/ET TCut
     */
    TCut GetMinLPETCut();

    /**
     * \brief Get q2 TCut
     */
    TCut GetQ2Cut();

    /**
     * \brief Get SideBand TCut
     */
    TCut GetSideBandCut();

    /**
     * \brief Get nSPD TCut
     */
    TCut GetSPDCut();

    /**
     * \brief Get TCK TCut
     */
    TCut GetTCKCut();

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
     * @brief      Gets the hlttck cut ( TCK data cut )
     *
     * @return     The hlttck cut.
     */
    TCut GetHLT1TCKCutAlignment();

    /**
     * \brief Get trigger TCut
     */
    TCut GetHLT2Cut();

    /**
     * \brief Get TruthMatch TCut
     */
    TCut GetTruthMatchCut();

    ClassDef(CutHolderRKst, 1);
};

ostream & operator<<(ostream & os, const CutHolderRKst & _cutHolder);

#endif
