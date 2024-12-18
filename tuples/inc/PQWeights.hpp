#ifndef PQWEIGHTS
#define PQWEIGHTS

#include "TBranch.h"
#include "TDatime.h"
#include "TFile.h"
#include "TList.h"
#include "TLorentzVector.h"
#include "TSystem.h"
#include "RooArgList.h"
#include "EventType.hpp"

namespace PQWeights{ 


    static double xmkp;
    static double xcostheta;
    static double xcostheta1;
    static double xcostheta2;
    static double xphi1;
    static double xphi2;
    static double xcosthetaB;
    static double xcosthetaZ;
    static double xcosthetaPsi;
    static double xphiZ;
    static double xphiPsi;
    static double xphiMu;
    static double xcosthetap;
    static double xalphaMu;
    static double xmjpsip;
    inline bool exists(const std::string & name) {
        return (access(name.c_str(), F_OK) != -1);
        //  struct stat buffer;
        //  return (stat (name.c_str(), &buffer) == 0);
    }
    void helicityJpsiLam(TLorentzVector * Pmu1, TLorentzVector * Pmu2, TLorentzVector * PJ, TLorentzVector * Pproton, TLorentzVector * Pkaon);
    void helicityZK(TLorentzVector * Pmu1, TLorentzVector * Pmu2, TLorentzVector * Pproton, TLorentzVector * Pkaon);
    void helicityTwoFrame(TLorentzVector * Pmu1, TLorentzVector * Pmu2, TLorentzVector * Pproton, TLorentzVector * Pkaon);
    // set how many SL of Lambda to float
    void SetLPar(RooArgList * argli, int maxind, bool fixfirst = false);

    void FloatPar(RooArgList * argli, int ind);

    void ResetPar(RooArgList * argli);

    void FloatZPar(RooArgList * argli);

    int AppendPQWeights( EventType & _cHolder, TString _fileName, bool addMCT = false  );
}
#endif // !PQWEIGHTS