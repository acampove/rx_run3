#ifndef FITCONFIGURATION_HPP
#define FITCONFIGURATION_HPP

#include "EventType.hpp"

#include "RooBinning.h"

using namespace std;
/**
 * \typedef EventTypeAndOption
 * \brief Template tuple for EventType and Options
 */
typedef tuple< EventType, TString, TString > EventTypeAndOption;

/**
 * \typedef FitComponentAndOption
 * \brief Template tuple for PdfType and FitComponentAndOption
 */
typedef tuple< PdfType, TString, TString, TString, TString, double > FitComponentAndOption;

/**
 * \typedef Smp2CompAndOptMap
 * \brief Template map for Sample to FitComponentAndOption
 */
typedef map< Sample, FitComponentAndOption > Smp2CompAndOptMap;

/**
 * \typedef Brm2CompAndOptMap
 * \brief Template map for Brem to FitComponentAndOption
 */
typedef map< Brem, FitComponentAndOption > Brm2CompAndOptMap;

// How to generate a PDF within the framework,
// StringToPDF  : Steering of a String, quite a lot of PDF suppoered CB (CrystalBall, Ipatia2... see the roofit folder for a list). (we actually have to add a check that the PDF parsed is able to be created ! try / catch ?)
//                How to steer them : see the FitterSvc.cpp
// RooKeysPDF   : It's a RooKeyPDF, modify -rho[value] accepted
//
// SignalCopy   : This flag implies the PDF is already built elsewhere and it will just make a copy-clone of it for further modifications
//                (e.g. Bs = Signal shape with a mass shift)
// KeepAdding element, (I can think of a future implementation of : I made a RooAddPdf * by myself, i want the framework to support it, add it here)

/*
    The FitConfiguration class inherits from ConfigHolder and it is capable to generate a FitHolder, i.e  a fit to Data given a list of Shapes.
    Properties  m_constrainedMass : based on this, it can trigger the creation of a RooRealVar with the name expected in the tuple in the case of constrainedMass or not.
                m_nBinsMC         : N of Bins on which you plot (and FIT Binned if also m_binnedMC = true) the fits performed to MC        samples
                m_nBinsCL         : N of Bins on which you plot (and FIT Binned if also m_binnedCL = true) the fits performed to CL (data) samples
                m_hasBrem         : Internal Flags identifying if the Configuration to use has to make a sum over Brem Categories of various shapes (Signal and SignalCopy shapes)
                m_binnedMC        : Is the MC Fit to be performed Binned ?
                m_binnedCL        : Is the CL Fit to be performed Binned ?
                m_signalSample    : Sample (from Enumerator Sample) identifying the Signal Sample type (filled at creation when the KeyWord "Signal" is found)
                m_var*            : Pointer to the RooRealVar (1D Observable) which is used to make the Fits and Plots. No multi-D fits supported INDEED . To extend to Multi-D we would need to play around with RooArgLists of Variables.
                m_componentsAndOptions     : Smp2CompAndOptMap  For this FitConfiguration, you will chain different Samples in the fit : Comb + Bs + Lb for example . Each one has its own space in the map.
                                        The Pair identify the "MODE" of inserting the PDF in the full model, Currently Supported (see EnumeratorSvc), StringToPDF, RooKeysPDF, SignalCopy. The String->Enum Converter is unique.
                                        This Map stores only the things which are not Splitted BY BremCategory
                m_componentsAndOptionsBrem : map <Brem,  pair <PdfType, TString>> For this FitConfiguration, you will ADD for the Signal (if it gets filled) different Brem Category shapes (mainly for EE FitConfigurations)
                                        with a MODE of filling and a TString which specify the extra options.
*/

class FitConfiguration : public ConfigHolder {

  private:
    TString m_key = "";   // <this is the the KEY value which is going to be added to the relevant FitHolder(of This FitConfiguration) = FitManager[KEY] where KEY = 1 Category of the fit.

    TString m_varName = "";

    bool m_binnedMC = false;
    bool m_binnedCL = false;

    int m_nBinsMC = 100;
    int m_nBinsCL = 100;
    
    double m_minMC = 0;
    double m_minCL = 0;
    double m_maxMC = 0;
    double m_maxCL = 0;

    map< TString, tuple< bool, double, double > > m_namedRangeFits;

    vector< TString > m_composition = {};

    bool m_constrainedMass = false;
    bool m_hasBrem         = false;

    //------------------------- IT GETS FILLED from PARSING, if it doesn't code breaks, i.e. ALWAYS "Signal" in the list to parse.
    Sample m_signalSample = Sample::Error;

    vector< Sample > m_backgroundSamples;

    RooRealVar * m_var = nullptr;

    map< TString, EventTypeAndOption > m_eventTypeAndOptions;

    // It maps the set of Components to store as
    // map of the Key   = Sample of the Fit (i.e. which Component to add)
    // value of the Key first  = Modality of adding it inside the fit [ StringToPDF, RooKeysPDF, SignalCopy ]
    // value of the key second = TString to pass as option: StringToPDF quite flexible, RooKeysPDF has only -rho[1.2],
    // SignalCopy for the moment has no property (We can use -mShift[....bla ] or something more fancy to interpret)

    Smp2CompAndOptMap m_componentsAndOptions;
    // Should be empty if ana = MM (always!)

    // It maps the set of BremComponents to build the final Signal shape for EE (only) (Code is forcing this map to always be Empty for MM)
    // map of the Key   = Brem of the Fit (i.e. which Component to add)
    // value of the Key first  = Modality of adding it inside the fit [ StringToPDF, RooKeysPDF, SignalCopy ]
    // value of the key second = TString to pass as option: StringToPDF quite flexible, RooKeysPDF has only -rho[1.2],
    //                           SignalCopy for the moment has no property (We can use -mShift[....bla ] or something more fancy to interpret)

    Brm2CompAndOptMap m_componentsAndOptionsBrem;

    // String for Range option in fit. Empty string means  that it should never have to use the custom range
    TString m_extraRange = "";

    bool m_debug = false;

  public:
    // Default construct
    FitConfiguration() = default;

    // Constructor with options (_composition gets steered by the parser)
    // What are the switch i can use for the FitConfiguration ?
    //(1 for EE/MM of a Given Trigger cateogry.
    //(1 for L0L/L0I for Years)
    FitConfiguration(
        /* Needed for the EventType building : it depends what we want to do with this FitConfiguratio...we can simply tell, a list of Triggers to build 1 FitManager or pass a vector of them */
        const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track,
        /* Extras on top of EventType to be used for the Fit! */
        const bool & _constrainedMass, const tuple< bool, int, double, double > & _configVarMC, const tuple< bool, int, double, double > & _configVarCL, const vector< TString > & _composition);

    FitConfiguration(
        /* Needed for the EventType building : it depends what we want to do with this FitConfiguratio...we can simply tell, a list of Triggers to build 1 FitManager or pass a vector of them */
        const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track,
        /* Extras on top of EventType to be used for the Fit! */
        const TString & _varName, const tuple< bool, int, double, double > & _configVarMC, const tuple< bool, int, double, double > & _configVarCL, const vector< TString > & _composition);

    FitConfiguration(
        /* Needed for the EventType building : it depends what we want to do with this FitConfiguratio...we can simply tell, a list of Triggers to build 1 FitManager or pass a vector of them */
        const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track,
        /* Extras on top of EventType to be used for the Fit! */
        const vector< TString > & _composition);

    /**
     * \brief Copy constructor
     */
    // FitConfiguration(const FitConfiguration & _fitConfiguration);

    /**
     * \brief Equality checkers for storing elements in map, based on comparators of ConfigHolder Object
     */
    bool operator==(const FitConfiguration & _fitConfiguration) const { return (GetProject() == _fitConfiguration.GetProject() && GetAna() == _fitConfiguration.GetAna() && GetQ2bin() == _fitConfiguration.GetQ2bin() && GetYear() == _fitConfiguration.GetYear() && GetPolarity() == _fitConfiguration.GetPolarity() && GetTrigger() == _fitConfiguration.GetTrigger() && HasConstrainedMass() == _fitConfiguration.HasConstrainedMass() && HasBrem() == _fitConfiguration.HasBrem()); };

    bool operator!=(const FitConfiguration & _fitConfiguration) const { return !((*this) == _fitConfiguration); };

    /**
     * \brief Initialize the object
     */
    void Init();
    /**
     * \brief Close the object
    */
    void Close();
    /**
     * \brief Check entries in ntuples associated to this FitConfiguration object
    */
    void CheckTupleEntries(Long64_t _entries = 0, bool _error = false);

    TString ExtraRange() { return m_extraRange; };
    void    Print() const noexcept;
    void    PrintContent() const noexcept;

    void SetBinAndRange(TString _name, bool _binned, int _nBins, double _min, double _max);

    void UseBinAndRange(TString _name);

    // Return pointer to the Var Generated in this ConfigHolder
    RooRealVar * Var() { return m_var; };
    RooRealVar * Var() const { return m_var; };

    // Check wheter the MCFIt is binned or not
    bool IsBinnedMC() const noexcept { return m_binnedMC; };

    // Check wheter the CLFit is binned or not
    bool IsBinnedCL() const noexcept { return m_binnedCL; };

    TString VarName() const noexcept { return m_varName; };

    bool HasConstrainedMass() const noexcept { return m_constrainedMass; };
    bool HasBrem() const noexcept { return m_hasBrem; };

    // GetKey for the FitHolder with extra name of Brem Category, DO THIS ONLY if m_HasBrem!
    TString GetKeyWithBrem(const Brem & _brem) const noexcept;

    // GetKey for the FitHolder with extra name of Brem Category, DO THIS ONLY if you expect that String to contains a Brem KeyWord!
    Brem GetBremFromKey(const TString & _string) const noexcept;

    vector< TString > Composition() const noexcept { return m_composition; };

    map< TString, EventTypeAndOption > EventTypes() const noexcept { return m_eventTypeAndOptions; };

    Smp2CompAndOptMap Components() const noexcept { return m_componentsAndOptions; };

    Brm2CompAndOptMap ComponentsBrem() const noexcept { return m_componentsAndOptionsBrem; };

    // Return the Sample indicated as "Signal" from the parser
    Sample SignalSample() const { return m_signalSample; };

    vector< Sample > BackgroundSamples() const { return m_backgroundSamples; };

    TString GetSampleName(const Sample & _sample) const noexcept;

    // Generate the Signal EventType
    EventTypeAndOption GetSignal(const Brem & _brem) const noexcept;

    // Generate the Background EventType
    EventTypeAndOption GetBackground(const Sample & _sample) const noexcept;

    // Generate the Data EventType
    EventTypeAndOption GetData() const noexcept;

    ConfigHolder GetSignalConfigHolder() const noexcept;

    /* TODO : add the wrapper given a FitConfiguration of the ConfigHolders associated to a given Background (mapped from Sample), useful to later call GetEfficiencyFor this background */
    // vector<ConfigHolder> GetBackgroundsConfigHolder( const Sample _sample) const noexcept;
    bool HasSample(const Sample & _sample) const noexcept {
        bool _found = false;
        for (const auto & _component : m_componentsAndOptions) {
            if (_sample == _component.first) { _found = true; }
        }
        return _found;
    };

    bool HasPdfType(const PdfType & _pdfType) const noexcept {
        bool _found = false;
        for (const auto & _component : m_componentsAndOptions) {
            if (_pdfType == get< 0 >(_component.second)) { _found = true; }
        }
        return _found;
    };

    vector< Sample > GetSignalCopies() const noexcept {
        vector< Sample > _samples;
        for (const auto & _component : m_componentsAndOptions) {
            if (get< 0 >(_component.second) == PdfType::SignalCopy) { _samples.push_back(_component.first); }
        }
        return _samples;
    };

    vector< Sample > GetSamplesWithType(const PdfType & _pdfType) const noexcept;

    PdfType GetTypeFromSample(const Sample & _sample) const noexcept;

    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };
    /**
     * \brief Add the custom ranges for the fit
     * @param _extraRanges [ vector holding a tuple with named range, min, max]
     */
    void AddExtraRanges(const vector< tuple< TString, double, double > > & _extraRanges) noexcept;

    bool HasInitialFraction(Sample _sample);

    double GetInitialFraction(Sample _sample);


    //Labelling stuff
    void SetLabels( const map< Sample, TString> & _labels);
    bool HasLabels() const; 
    TString GetLabel( Sample & _sample) const;
    map<Sample,TString> GetLabels() const ;
    map<TString,TString> GetLabelsNamed() const ;
    //Coloring stuff
    void SetColors( const map< Sample, TString> & _colors);
    bool HasColors() const; 
    Int_t GetColor( Sample & _sample) const;
    map<Sample, Int_t> GetColors() const ;
    map<TString,Int_t> GetColorsNamed() const ;

    pair<double,double> RangeDataFit() const;
  private:
    // Steering  of the vector<TString> from parser to map in internal container.
    // That's the most hacky code i have ever writted, but it works
    // Basically :
    /*
    Composition      : [ "Signal-0G | StringToPDF | Ipatia2-Xz[.005]-Xb[0]-s[10,1,20]-a[1,.01,5]-a2[1,.01,10]-n[1,.01,5]-n2[2,.01,10]-l[-5,-25,-1]",
                         "Signal-1G | StringToPDF | Ipatia2-Xz[.005]-Xb[0]-s[10,1,20]-a[1,.01,5]-a2[1,.01,10]-n[1,.01,5]-n2[2,.01,10]-l[-5,-25,-1]",
                         "Signal-2G | StringToPDF | Ipatia2-Xz[.005]-Xb[0]-s[10,1,20]-a[1,.01,5]-a2[1,.01,10]-n[1,.01,5]-n2[2,.01,10]-l[-5,-25,-1]",
                         "Bs        | SignalCopy  |            ",
                         "Lb        | RooKeysPDF  | -rho[1.2]   ",
                         "Comb      | StringToPDF | Exp-b[-1e-3,-1e-2,-1e-5]" ]
    //The Signal Keyword identify if the shape is related to the Signal shape (from the m_q2bin + m_ana you know already which kind of signal you are looking for)
    //Signal-0G instruct the method to fill the Brem map
    //Bs/Lb/Comb/PartReco/Leakage are associated to the Sample enumerator TString accepted
    //The second column is the Method to be used to build the PDF (it will be parsed as PDF)
    //The third string is up to the user : define whatever can make sense for the fit as a shape to use. (note the code has no guard at the moment that the shape you parse will be built correctly... (TO BE DONE...in the future))
    */
    void ParseComposition();

    void SetHasBrem();

    void BuildFitVar();

    void GenerateSignal(const Brem & _brem);
    void GenerateBackground(const Sample & _sample);
    void GenerateData();

    map<Sample, TString> m_labels;
    map<Sample, Int_t>  m_colors;
    ClassDef(FitConfiguration, 1);

    static constexpr const double m_noYieldFraction = -1;
};

ostream & operator<<(ostream & os, const FitConfiguration & _configHolder);

#endif
