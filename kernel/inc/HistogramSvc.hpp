#ifndef HISTOGRAMSVC_HPP
#define HISTOGRAMSVC_HPP

#include "MessageSvc.hpp"

//#include "ROOT/RMakeUnique.hxx"
#include <ROOT/RDataFrame.hxx>

// Forward declaration : to enable implicit conversions do the #include of those classes in your .cpp file
class TChain;
class TCut;
class RooRealVar;
class TString;
class TH1D;
class TH1;
class TH2;

constexpr char DUMMYNAME[] = "dummy";

/**
 * \brief      Create a full copy of the Histogram ( can be 1D-2D-3D)
 * @param[in]  _histo  The histo input to copy
 * @param[in]  _empty  The empty, copy histo and fill it empty
 * @return     the new-copied histogram
 */
TH1 * CopyHist(const TH1 * _histo, bool _empty = false) noexcept;

/**
 * \brief      Gets the histogram from tuple, cut, x,y,z vars (optionals)
 * @param      _tuple      The tuple
 * @param[in]  _cut        The cut
 * @param[in]  _extraName  The extra name to attach to the resulting histo
 * @param[in]  _varX       The variable x (RooRealVar as it already knows bounds and bins)
 * @param[in]  _varY       The variable y (RooRealVar as it already knows bounds and bins)
 * @param[in]  _varZ       The variable z (RooRealVar as it already knows bounds and bins)
 * @return     The histogram : usually do TH1D * myhisto = static_cast<TH1D*>(GetHistogram(.....)) to properly cast to the expected type
 */
TH1 * GetHistogram(TChain & _tuple, const TCut & _cut, const TString & _extraName, const RooRealVar & _varX, const RooRealVar & _varY = RooRealVar(DUMMYNAME, DUMMYNAME, 0), const RooRealVar & _varZ = RooRealVar(DUMMYNAME, DUMMYNAME, 0));

TH1 * RandomizeAllEntries(TH1 * _histo, int _seed = -1, TString _option = "eff") noexcept;

TH1 * RoundToIntAllEntries(TH1 * _histo) noexcept;

/**
 * \brief      CHeck whether the histogram (1D) has to be squeezed
 * @param      _histo  The histo
 * @return     true/false
 */
bool AdaptingBounds(TH1D * _histo) noexcept;

/**
 * \brief      When _histo has variable bin-width, we make a copy of it keeping exactly same content,  but with the last bin squeezed in size
 * @param      _histo  The histo
 * @return     a new histo with squeezed boundaries.
 */
TH1D * SquezeBoundaries(TH1D * _histo, bool _delete = false) noexcept;

/**
 * \brief Get tuple entries given a cut
 * @param[in]  _tuple  [description]
 * @param[in]  _cut    [description]
 * @param[in]  _option [description]
 */
double GetEntries(TChain & _tuple, const TCut & _cut);

double GetEntriesDF(TChain & _tuple, const TCut & _cut);

/**
 * \brief Get histogram value at a given X [ assume 1-D histogram here ]
 * @param[in]  _histo  [description]
 * @param[in]  _var    [description]
 * @param[in]  _option [description]
 */
double GetHistogramVal(TH1D * _histo, double _var, TString _option = "");

/**
 * \brief      Gets the histogram value for a 2 D plot
 * @param      _histo   The histo
 * @param[in]  _varX    The value on the x axis
 * @param[in]  _varY    The value on the y axis
 * @param[in]  _option  The option [ "INTERP" for example ]
 * @return     The histogram value for a 2-D plot ( can be TH2Poly or TH2D but interpolating values does not work for TH2Poly (returns un-interpolated value) )
 */
double GetHistogramVal(TH2 * _histo, double _varX, double _varY, TString _option = "");

/**
 * \brief      Gets the histogram value for a 3 D plot
 * @param      _histo   The histo
 * @param[in]  _varX    The value on the x axis
 * @param[in]  _varY    The value on the y axis
 * @param[in]  _varZ    The value on the z axis
 * @param[in]  _option  The option [ "INTERP" for example ]
 * @return     The histogram value for a 3-D plot ( can be TH3D ) )
 */
double GetHistogramVal(TH3 * _histo, double _varX, double _varY, double _varZ, TString _option = "");

/**
 * \brief      Gets the histogram value for a 3 D plot for PID maps
 * @param      _histos  The map histogram pointing to the histograms following the z coordinate
 * @param[in]  _varX    The value on the x axis
 * @param[in]  _varY    The value on the y axis
 * @param[in]  _varZ    The value on the z axis
 * @param[in]  _option  The option [ "INTERP" for example ]
 * @return     The histogram value for a 3-D plot ( can be TH3D ) )
 */
double GetHistogramVal_3D(pair<TH1D*,vector<TH2D*>> & _histos, double _varX, double _varY, double _varZ, TString _option = "");

//================================================================================
//================ CHECKING HISTOS for Ratios and Efficiencies ===================
//================================================================================

/**
 * \brief      CheckHistogram depending on option  , the meethod could "update" the histogram
 * @param      _histo   The histo used as input , can be 1/2 D
 * @param[in]  _option  The option [ "eff", "effr", "ratio" , "ratior", "q"] for efficiency, efficiency + reset (<0 entries -> 0, >1 entries -> 1) , ratio, ratio + reset (<0 entries)
 */
void CheckHistogram(TH1 * _histo, TString _option = "");

/**
 * \brief Check 2 histogram consistency as a function of the option , the method could "update" the histograms
 * @param[in]  _hPas   [numerator histogram before selection]
 * @param[in]  _hTot   [denominator histogram after selection]
 * @param[in]  _option ["eff", "effr" = eff+reset <0,>1 bins , "ratio" = it's a ratio, no negative entries , "ratior" = check as "ratio" but also reset bin content, "q" = quiet]
 */
void CheckHistogram(TH1 * _hPas, TH1 * _hTot, TString _option = "");

void CompareHistogram(TH1 * _hPas, TH1 * _hTot, TString _option = "");

void PrintHistogram(TH1 * _histo, TString _option = "");

/**
 * \brief      Scale a given Histogram ( 1D or 2D)
 * @param      _histo  The histo to scale
 * @param[in]  _norm   The normalization [TODO  check logic]
 * @param[in]  _normE  The normalizetion for the error [TODO check logic]
 */
void ScaleHistogram(TH1 & _histo, double _norm = 0, double _normE = 0);

Color_t GetColorFromGradient(int _idx, int _total);

#endif
