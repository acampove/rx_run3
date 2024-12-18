#ifndef STYLESVC_HPP
#define STYLESVC_HPP

#include "MessageSvc.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "TGaxis.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TText.h"

TStyle * LHCbStyle();

TPaveText * PrintLHCb(TString optLR = "L", TString optPrelim = "P", TString optText = "");

Color_t GetColor(int i);

/**
 * \namespace Tex
 * \brief Used by AllowedConf::TexSample map for SampleToTex conversion
 */
namespace Tex {

    const TString rightarrow = "#it{#rightarrow}";

    const TString B0      = "#it{B}^{0}";
    const TString Bp      = "#it{B}^{+}";
    const TString Bs      = "#bar{#it{B}}_{s}^{0}";
    const TString Lambdab = "#Lambda_{b}";
    const TString Lambda0 = "#Lambda^{0}";

    const TString Muon     = "#mu";
    const TString Electron = "#it{e}";

    const TString MM   = "#mu^{+}#mu^{#minus}";
    const TString EE   = "#it{e}^{+}#it{e}^{#minus}";
    const TString ME   = "#mu^{+}#it{e}^{#minus}";
    const TString MMSS = "#mu^{+}#mu^{+}";
    const TString EESS = "#it{e}^{+}#it{e}^{+}";

    const TString Kaon    = "#it{K}";
    const TString KPlus   = "#it{K}^{+}";
    const TString KMinus  = "#it{K}^{#minus}";
    const TString KShort  = "#it{K}_{S}";
    const TString Pion    = "#pi";
    const TString PiPlus  = "#pi^{+}";
    const TString PiMinus = "#pi^{#minus}";
    const TString Pi0     = "#pi^{0}";
    const TString Eta     = "#eta";
    const TString Photon  = "#gamma";
    const TString Proton  = "#it{p}";

    const TString JPsi = "#it{J}/#psi";
    const TString Psi  = "#psi(#it{2S})";
    const TString Phi  = "#phi";

    const TString Kstar0 = "#it{K}#lower[.3]{^{* 0}}";
    const TString KstarP = "#it{K}#lower[.3]{^{* +}}";
    const TString K1     = "#it{K}_{#it{1}}(#it{1270})^{+}";
    const TString K2     = "#it{K}_{#it{2}}(#it{1430})#lower[.3]{^{* +}}";

    const TString D0     = "#it{D}^{0}";
    const TString Dplus  = "#it{D}^{+}";
    const TString Dminus = "#it{D}^{#minus}";
    const TString nue    = "#nu_{#it{e}}";
    const TString anue   = "#bar{nu}_{#it{e}}";

};   // namespace Tex

#endif
