#ifndef ENUMERATORSVC_HPP
#define ENUMERATORSVC_HPP

#include "MessageSvc.hpp"
#include "StyleSvc.hpp"

class TString;

/**
 * \namespace pyPrj
 */
namespace pyPrj {
    /**
     * \enum Prj
     * \brief Project type
     */
    enum class Prj { All = 0, RK = 1, RKst = 2, RPhi = 3, RL = 4, RKS = 5, Error = 99 };
    const Prj hash_project(const TString & _string);
    TString   to_string(const Prj & _enum);
    TString   to_tex(const Prj & _enum);
};   // namespace pyPrj
using namespace pyPrj;

/**
 * \namespace pyAnalysis
 */
namespace pyAnalysis {
    /**
     * \enum Analysis
     * \brief Analysis type
     */
    enum class Analysis { All = 0, MM = 1, EE = 2, ME = 3, Error = 99 };
    const Analysis hash_analysis(const TString & _string);
    TString        to_string(const Analysis & _enum);
    TString        to_tex(const Analysis & _enum);
};   // namespace pyAnalysis
using namespace pyAnalysis;

/**
 * \namespace pyQ2Bin
 */
namespace pyQ2Bin {
    /**
     * \enum Q2Bin
     * \brief Q2Bin regions
     */
    enum class Q2Bin { All = 0, Low = 1, Central = 2, High = 3, JPsi = 4, Psi = 5, Gamma = 6, Error = 99 };
    const Q2Bin hash_q2bin(const TString & _string);
    TString     to_string(const Q2Bin & _enum);
    TString     to_tex(const Q2Bin & _enum);
};   // namespace pyQ2Bin
using namespace pyQ2Bin;

/**
 * \namespace pySample
 */
namespace pySample {
    /**
     * \enum Sample
     * \brief Sample type (used to identify the TYPE in the Fitter)
     */
    enum class Sample { Empty = 0, 
                        LL = 10, 
                        JPsi = 11, 
                        Psi = 12, 
                        Gamma = 13, 
                        BdBu = 19, 
                        Bd = 20, 
                        Bs = 21, 
                        Lb = 22, 
                        Comb = 23, 
                        PartReco = 24, 
                        PartRecoH = 25, 
                        PartRecoL = 26, 
                        Leakage = 27, 
                        MisID = 28, 
                        Bs2Phi = 29, 
                        HadSwap = 30, 
                        Psi2JPsX = 31, 
                        CombSS = 32, 
                        Bd2Kst = 33, 
                        Bu2Kst = 34, 
                        DSLC = 35, 
                        Psi2JPsPiPi  = 36, 
                        KEtaPrime = 37,
                        PartRecoK1 = 38, 
                        PartRecoK2 = 39, 
                        PartRecoHad = 41, 
                        Data = 40, 
                        DataDrivenEMisID = 42,
                        TemplateMisID_PIDe3 = 43,
                        DoubleMisID_PiPi = 44,
                        DoubleMisID_KK = 45,
                        Custom = 50, 
                        Error = 99 };
    const Sample hash_sample(const TString & _string);
    TString      to_string(const Sample & _enum);
    const Sample GetSignalSample(const Q2Bin & _q2bin);
};   // namespace pySample
using namespace pySample;

/**
 * \namespace pyYear
 */
namespace pyYear {
    /**
     * \enum Year
     * \brief Year of data taking
     */
    enum class Year { All = 0, Y2011 = 1, Y2012 = 2, Run1 = 3, Y2015 = 4, Y2016 = 5, Run2p1 = 6, Y2017 = 7, Y2018 = 8, Run2p2 = 9, Run2 = 10, Error = 99 };
    const Year hash_year(const TString & _string);
    TString    to_string(const Year & _enum);
    TString    to_tex(const Year & _enum);
    int        to_int( const Year & year);
    const Year from_int( int year);
};   // namespace pyYear
using namespace pyYear;

/**
 * \namespace pyPolarity
 */
namespace pyPolarity {
    /**
     * \enum Polarity
     * \brief Magnet polarity of data taking
     */
    enum class Polarity { All = 0, MD = 1, MU = 2, Error = 99 };
    const Polarity hash_polarity(const TString & _string);
    TString        to_string(const Polarity & _enum);
    TString        to_tex(const Polarity & _enum);
};   // namespace pyPolarity
using namespace pyPolarity;

/**
 * \namespace pyTrigger
 */
namespace pyTrigger {
    /**
     * \enum Trigger
     * \brief L0 trigger category
     */
    enum class Trigger {
        All   = 0,   // No trigger split
        L0I   = 1,
        L0L   = 2,
        L0H   = 3,
        Error = 99
    };
    const Trigger hash_trigger(const TString & _string);
    TString       to_string(const Trigger & _enum);
    TString       to_tex(const Trigger & _enum);
};   // namespace pyTrigger
using namespace pyTrigger;

/**
 * \namespace pyTriggerConf
 */
namespace pyTriggerConf {
    /**
     * \enum TriggerConf
     * \brief L0 trigger configuration
     */
    enum class TriggerConf { Inclusive = 0, Exclusive = 10, Exclusive2 = 11, Error = 99 };
    const TriggerConf hash_triggerconf(const TString & _string);
    TString           to_string(const TriggerConf & _enum);
    TString           to_tex(const TriggerConf & _enum);
};   // namespace pyTriggerConf
using namespace pyTriggerConf;

/**
 * \namespace pyBrem
 */
namespace pyBrem {
    /**
     * \enum Brem
     * \brief Brem category
     */
    enum class Brem {
        All   = 0,   // No brem split
        G0    = 1,
        G1    = 2,
        G2    = 3,
        Error = 99
    };
    const Brem hash_brem(const TString & _string);
    TString    to_string(const Brem & _enum);
    TString    to_tex(const Brem & _enum);
};   // namespace pyBrem
using namespace pyBrem;

/**
 * \namespace pyTrack
 */
namespace pyTrack {
    /**
     * \enum Track
     * \brief Track category
     */
    enum class Track {
        All   = 0,   // No track split
        LL    = 1,
        DD    = 2,
        TAG   = 10,
        PRB   = 11,
        Error = 99
    };
    const Track hash_track(const TString & _string);
    TString     to_string(const Track & _enum);
    TString     to_tex(const Track & _enum);
};   // namespace pyTrack
using namespace pyTrack;

/**
 * \namespace pyPdfType
 */
namespace pyPdfType {
    /**
     * \enum PdfType
     * \brief PDF types
     */
    enum class PdfType {
        Empty       = 0,
        StringToPDF = 1,    // PDF generated via TString
        StringToFit = 2,    // PDF generated via TString and Fit to MC
        RooAbsPDF   = 3,    // PDF generated with a RooAbsPDF
        RooHistPDF  = 4,    // PDF generated with a RooHistPDF
        RooKeysPDF  = 5,    // PDF generated with a RooKeysPDF
        SignalCopy  = 6,    // PDF copy of the signal PDF
        Template    = 7, // PDF loaded externally from a RooWorkspace
        ToyPDF      = 10,   // PDF for toy
        Error       = 99
    };
    const PdfType hash_pdftype(const TString & _string);
    TString       to_string(const PdfType & _enum);
};   // namespace pyPdfType
using namespace pyPdfType;

/**
 * \namespace pyRatioType
 */
namespace pyRatioType {
    /**
     * \enum RatioType
     * \brief Ratio types
     */
    enum class RatioType {
        SingleRatio                 = 0,    //
        DoubleRatio                 = 1,    //
        BranchingFraction           = 10,   //
        FsOverFd                    = 20,   //
        FLbOverFd                   = 21,   //
        YieldRatio                  = 30,   //
        EfficiencyRatio             = 31,    //
        EfficiencyRatioSystematic   = 50     //
    };
    // const PdfType hash_ratiotype(const TString & _string);
    TString       to_string(const RatioType & _enum);
};   // namespace pyRatioType
using namespace pyRatioType;

/**
 * \namespace pyBlindMode
 */
namespace pyBlindMode {
    /**
     * \enum BlindMode
     * \brief BlindMode
     */
    enum class BlindMode { Empty = 0, OffsetScale = 1, Error = 99 };
    const BlindMode hash_blindmode(const TString & _string);
    TString         to_string(const BlindMode & _enum);
};   // namespace pyBlindMode
using namespace pyBlindMode;

/**
 * \namespace pyOpenMode
 */
namespace pyOpenMode {
    /**
     * \enum OpenMode
     * \brief OpenMode
     */
    enum class OpenMode { NONE = 0, READ = 1, RECREATE = 2, UPDATE = 3, WARNING = 4, ERROR = 5 };
    const OpenMode hash_openmode(const TString & _string);
    TString        to_string(const OpenMode & _enum);
};   // namespace pyOpenMode
using namespace pyOpenMode;

#endif
