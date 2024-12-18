#ifndef CUTHOLDERRK_HPP
#define CUTHOLDERRK_HPP

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
 * \class CutHolderRK
 * \brief Cut info
 */
class CutHolderRK : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    CutHolderRK();

    /**
     * \brief Constructor with WeightHolder and TString
     */
    CutHolderRK(const ConfigHolder & _configHolder, TString _cutOption);

    /**
     * \brief Copy constructor
     */
    CutHolderRK(const CutHolderRK & _cutHolder);

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

    TCut CleanRKstCut(TCut _cut);

    /**
     * \brief Get background TCut
     */
    TCut GetBackgroundCut();

    /**
     * \brief Get MVA TCut
     */
    TCut GetMVACut();

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
     * \brief Get the Kst CrossFeedCut 
    */
    TCut GetXFeedKstCut();

    /**
     * \brief Get constraint mass TCut for same sign data
     */
    TCut GetCombBVeto();

    ClassDef(CutHolderRK, 1);
};

ostream & operator<<(ostream & os, const CutHolderRK & _cutHolder);

#endif
