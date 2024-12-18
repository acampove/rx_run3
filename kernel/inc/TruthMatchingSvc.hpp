#ifndef TRUTHMATCHSVC_HPP
#define TRUTHMATCHSVC_HPP

#include "SettingDef.hpp"

#include "ConfigHolder.hpp"

#include "EnumeratorSvc.hpp"

#include "TCut.h"
#include "TString.h"

/**
 * \namespace TRUTHMATCHING
 * \brief TruthMatching MC cut definitions
 **/
namespace TRUTHMATCHING {

    const TCut decChain = "{HEAD}_TRUEID != 0";

    // https://twiki.cern.ch/twiki/bin/view/LHCb/TupleToolMCBackgroundInfo
    //----- single BKGCAT
    //  0: Signal !G && !K && !L && A && B && C && D && E
    // 10: Quasi-signal !G && !K && !L && A && B && C && D && !E
    // 20: Phys. back. (full rec.) !G && !K && !L && A && B && C && !D
    // 30: Reflection (mis-ID) !G && !K && !L && A && B && !C
    // 40: Phys. back. (part. rec.) !G && !K && !L && A && !B && !(C && F)
    // 50: Low-mass background !G && !K && !L && A && !B && C && F
    // 60: Ghost G
    // 63: Clone !G && K
    // 66: Hierarchy !G && !K && L
    // 70: FromPV !G && !K && !L && M
    // 80: AllFromSamePV !G && !K && !L && N
    // 100: Pileup/FromDifferentPV !G && !K && !L && !A && H
    // 110: bb event !G && !K && !L && !A && !H && I
    // 120: cc event !G && !K && !L && !A && !H && !I && J
    // 130: light-flavour event !G && !K && !L && !A && !H && !I &&
    const map< TString, int > BKGCAT = {{"Signal", 0}, {"Quasi-Signal", 10}, {"PhyBkg(fullRec)", 20}, {"Reflection(misID)", 30}, {"PartReco", 40}, {"LowMass", 50}, {"Ghost", 60}, {"Clone", 63}, {"Hierarchy", 66}, {"FromPV", 70}, {"AllSamePV", 80}, {"PileUp", 100}, {"bbEvent", 110}, {"ccEvent", 120}, {"lightFlavour", 130}};

    const TCut bkgCatSignal   = "{HEAD}_BKGCAT == 0 || {HEAD}_BKGCAT == 10";
    const TCut bkgCatFullReco = "{HEAD}_BKGCAT == 20";
    const TCut bkgCatSwap     = "{HEAD}_BKGCAT == 30";
    const TCut bkgCatPartReco = "{HEAD}_BKGCAT == 40";
    const TCut bkgCatLowMass  = "{HEAD}_BKGCAT == 50";
    const TCut bkgCatGhost    = "{HEAD}_BKGCAT == 60";
    const TCut bkgCatPhysics  = "{HEAD}_BKGCAT < 70";

    const TCut bkgCatSig1 = bkgCatSignal || bkgCatLowMass;                  // BKGCAT == 0 || 10 || 50
    const TCut bkgCatSig2 = bkgCatSignal || bkgCatLowMass || bkgCatGhost;   // BKGCAT == 0 || 10 || 50 || 60

    const TCut bkgCatBkg = bkgCatFullReco || bkgCatSwap || bkgCatPartReco || bkgCatLowMass;   // BKGCAT == 20 || 30 || 40 || 50
  const TCut bkgCatBkgGhost = bkgCatFullReco || bkgCatSwap || bkgCatPartReco || bkgCatLowMass || bkgCatGhost;    
}   // namespace TRUTHMATCHING

// Set of Extra String to post-pend to the name
const TString TRUEID     = "TRUEID";
const TString MOTHERID   = "MC_MOTHER_ID";
const TString GMOTHERID  = "MC_GD_MOTHER_ID";
const TString GGMOTHERID = "MC_GD_GD_MOTHER_ID";

TCut MatchID(TString _particleLabel, int ID);

TCut MatchIDWithGhost(TString _particleLabel, int ID);

TCut MatchMotherID(TString _particleLabel, int ID);

TCut MatchMotherIDWithGhost(TString _particleLabel, int ID);

TCut MatchGrandMotherID(TString _particleLabel, int ID);

TCut MatchGrandMotherIDWithGhost(TString _particleLabel, int ID);

TCut MatchGrandGrandMotherID(TString _particleLabel, int ID);

TCut MatchGrandGrandMotherIDWithGhost(TString _particleLabel, int ID);


TCut TruthMatching(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2bin, const TString & _sample, TString _option = "", bool _debug = false, bool _swap = false);

TCut TruthMatching(const ConfigHolder & _configHolder, TString _option, bool _debug = false);

#endif
