#pragma once

#include "TString.h"
#include <vector>

using namespace std;

/**
 * @brief Custom class parsed for eficiency slot generations
 *
 */
class EffSlot 
{
  private:
    TString                m_wOpt                 = "";   // to use
    TString                m_cOpt                 = "";   // to append ( to baseline definition of a given sample )
    TString                m_wOptNormN            = "";   // to use for NormN
    TString                m_wOptNormD            = "";   // to use for NormD
    TString                m_wOptMCDecay          = "";   // to use for MCDecayTuple
    TString                m_cutSetNorm           = "";   // the cut-set map to use for NormCUT    
    std::vector< TString > m_weightConfigurations = {};
  public:
    EffSlot() = default;
    EffSlot(
            TString _wOpt, 
            TString _cOpt, 
            TString _wMCDecayOpt, 
            TString _wOptNormN, 
            TString _wOptNormD, 
            TString _cutSetNormalization, 
            vector< TString > _weightConfigurations)
        : m_wOpt(_wOpt)
        , m_cOpt(_cOpt)
        , m_wOptMCDecay(_wMCDecayOpt)
        , m_wOptNormN(_wOptNormN)
        , m_wOptNormD(_wOptNormD)
        , m_cutSetNorm(_cutSetNormalization)
        , m_weightConfigurations(_weightConfigurations){};

    TString                wOpt()          const { return m_wOpt; }
    TString                cOpt()          const { return m_cOpt; }
    TString                wOptNormN()     const { return m_wOptNormN; }
    TString                wOptNormD()     const { return m_wOptNormD; }
    TString                wOptMCDecay()   const { return m_wOptMCDecay; }
    TString                cutSetNorm()    const { return m_cutSetNorm; }
    std::vector< TString > weightConfigs() const { return m_weightConfigurations; }

    void Print() const;
    std::vector< TString > ListAndCutsNorm() const;
};

