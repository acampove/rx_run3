#pragma once

#include "TString.h"

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"

#include "ConfigHolder.hpp"
#include "CutHolder.hpp"
#include "TupleHolder.hpp"
#include "WeightHolder.hpp"

class FitConfiguration;
class TH1;
class TH2;

/**
 * @brief A zipped version of an EventType. A full EventType can be constructed with this Zipped version
 */
struct ZippedEventType 
{
    Prj         project;
    Analysis    ana;
    TString     sample;
    Q2Bin       q2bin;
    Year        year;
    Polarity    polarity;
    Trigger     trigger;
    TriggerConf triggerConf;
    Brem        brem;
    Track       track;
    TString     cutOption;
    TString     weightOption;
    TString     tupleOption;
    TString     tupleName;
    TString     fileName;
};

/**
 * @brief Class describing 1 tuple + 1 cut + 1 weight for the analysis.
 * It knows how to laod the cut, delegating it to the CutHolder
 * It knows how to load the tuple and the versioning of it , delegating it to the TupleHolder
 * It knows how to load the weight and the versioning of it, delegating it to the WeightHolder
 * It knows the basic underlying analysis switches forwarded to CutHolder, WeightHolder, TupleHolder to enable switches also for them
 * EventType can be used a fundamental building block of the analysis since it knows the cut, weight, tuple to use and allow to change versions of Cuts/weight/tuples to use in a flexible way.
 */
class EventType : public ConfigHolder 
{
  public:
    /**
     * \brief Default constructor, swallow configured SettingDef::Config flags known at build-object time
     */
    EventType();

    /**
     * @brief Constructor taking configuration through config holder 
     * @param _cut/wgt/tup_opt Option passed to Config/Weight/TupleHolder object
     */
    EventType(
            const ConfigHolder &_conf,
            const TString      &_cut_opt,
            const TString      &_wgt_opt,
            const TString      &_tup_opt); 

    /**
     * \brief      Constructs the object.
     * @param[in]  _project       The project          [see EnumeratorSvc]
     * @param[in]  _ana           The ana              [see EnumeratorSvc]
     * @param[in]  _sample        The sample           [see EnumeratorSvc]
     * @param[in]  _q2bin         The q2 bin           [see EnumeratorSvc]
     * @param[in]  _year          The year             [see EnumeratorSvc]
     * @param[in]  _polarity      The polarity         [see EnumeratorSvc]
     * @param[in]  _trigger       The trigger category [see EnumeratorSvc]
     * @param[in]  _brem          The brem category    [see EnumeratorSvc]
     * @param[in]  _track         The track category   [see EnumeratorSvc]
     * @param[in]  _cutOption     The cut option       [see SettingDef and CutHolder]
     * @param[in]  _weightOption  The weight option    [see SettingDef and CutHolder]
     * @param[in]  _tupleOption   The tuple option     [see SettingDef and TupleHolder, "cre", "gng", "pro" , i.e. type of the tuple in processing chain]
     * @param[in]  _init          The initialize flag  [initialize it? true by default]
     */
    EventType(TString _project, TString _ana, TString _sample, TString _q2bin, TString _year, TString _polarity, TString _trigger, TString _brem, TString _track, TString _cutOption, TString _weightOption, TString _tupleOption, bool _init = true);

    /**
     * \brief      Constructs the object.
     * @param[in]  _project       The project          [see EnumeratorSvc]
     * @param[in]  _ana           The ana              [see EnumeratorSvc]
     * @param[in]  _sample        The sample           [see EnumeratorSvc]
     * @param[in]  _q2bin         The q2 bin           [see EnumeratorSvc]
     * @param[in]  _year          The year             [see EnumeratorSvc]
     * @param[in]  _polarity      The polarity         [see EnumeratorSvc]
     * @param[in]  _trigger       The trigger category [see EnumeratorSvc]
     * @param[in]  _brem          The brem category    [see EnumeratorSvc]
     * @param[in]  _track         The track category   [see EnumeratorSvc]
     * @param[in]  _cutOption     The cut option       [see SettingDef and CutHolder]
     * @param[in]  _weightOption  The weight option    [see SettingDef and CutHolder]
     * @param[in]  _tupleOption   The tuple option     [see SettingDef and TupleHolder, "cre", "gng", "pro" , i.e. type of the tuple in processing chain]
     * @param[in]  _init          The initialize flag  [initialize it? true by default]
     */
    EventType(Prj _project, Analysis _ana, TString _sample, Q2Bin _q2bin, Year _year, Polarity _polarity, Trigger _trigger, Brem _brem, Track _track, TString _cutOption, TString _weightOption, TString _tupleOption, bool _init = true);

    /**
     * \brief      Constructs the object with the basic classes : compose this EventType
     * @param[in]  _configHolder  The configuration holder
     * @param[in]  _cutHolder     The cut holder
     * @param[in]  _weightHolder  The weight holder
     * @param[in]  _tupleHolder   The tuple holder
     * @param[in]  _init          The initialize
     */
    EventType(const ConfigHolder & _configHolder, const CutHolder & _cutHolder, const WeightHolder & _weightHolder, const TupleHolder & _tupleHolder, bool _init = true);

    /**
     * \brief      Constructs the object with the Zipped version of it
     * @param      _zip    The zipped EventType to use
     * @param[in]  _reset  The reset flag
     */
    EventType(ZippedEventType & _zip, bool _init = true, bool _reset = false);

    /**
     * \brief      Copy Constructs the object
     * @param[in]  _eventType  The event type
     */
    EventType(const EventType & _eventType);

    /**
     * \brief      operator equality
     * @param[in]  The event type to compare with
     * @return     are the 2 event types equal
     */
    bool operator==(const EventType & _eventType) const;

    /**
     * \brief      operator inequality
     * @param[in]  _eventType  The event type to compare with
     * @return     are them different ?
     */
    bool operator!=(const EventType & _eventType) const;

    /**
     * \brief      operator <
     * @param[in]  _eventType  The event type to compare with
     * @return     true or false, used to sort EventType list
     */
    // bool operator<(const EventType & _eventType) const;

    /**
     * \brief      operator >
     * @param[in]  _eventType  The event type to compare with
     * @return     true or false, used to sort EventType list
     */
    // bool operator>(const EventType & _eventType) const;

    /**
     * @brief      Initialize This EventType
     * @param[in]  _force  The force initialization re-do the initialization even if the status of init is True
     * @param[in]  _tuple  Initialize the TupleHolder
     */
    void Init(const bool &_force_initialization = false, const bool &_initialize_tuple_holder = true);

    /**
     * \brief      Check consistency of the EventType, flags etc, are they valid?
     */
    void Check();

    /**
     * \brief      Check consistency among ConfigHolder, CutHolder, WeightHolder sparsed objects... Method can be used as EventType::Check( ConfigHolder, CutHolder, WeightHolder )
     * @param      _configHolder  The configuration holder which is maybe shared (can be the EventType itself)
     * @param      _cutHolder     The cut holder object, not strictly the one inside this EventType
     * @param      _weightHolder  The weight holder, not strictly the one einside this EventType
     * The purpose of this is to have 1 TupleHolder, and a list of < CutHolder, WeightHolder > sharing same ConfigHolder , thus no neeed to initialize 10 different nTuples to check 10 different Weights/Cuts
     */
    static void Check(ConfigHolder & _configHolder, CutHolder & _cutHolder, WeightHolder & _weightHolder);

    static void Check(ConfigHolder & _configHolder, CutHolder & _cutHolder, WeightHolder & _weightHolder, TupleHolder & _tupleHolder);

    /**
     * \brief      Close the EventType, clean-up all memory garbage around.
     */
    void Close();

    /**
     * \brief      Produce a zipped info EventType to port and construct other ones
     * @return     The zipped EventType
     */
    ZippedEventType Zip();

    /**
     * \brief      Gets the latex description of this EventType
     * @return     The latex to append to plots
     */
    TString GetLatex();

    /**
     * \brief      Determines if initialized.
     * @return     True if initialized, False otherwise.
     */
    bool IsInitialized() const { return m_isInitialized; };

    /**
     * \brief      Determines if weighted.
     * @return     True if weighted, False otherwise.
     */
    bool IsWeighted() const { return IsWeight(GetWeight()); };

    /**
     * \brief      Determine if HasWeightOption(TString _option)
    */
    bool HasWeightOption(const TString _opt="") const{ return m_weightHolder.Option().Contains(_opt);}
    /**
     * \brief      Determines if cut.
     * @return     True if cut, False otherwise.
     */
    bool IsCutted() const { return IsCut(GetCut()); };

    /**
     * \brief      Gets the cut holder copy
     * @return     The cut holder.
     */
    CutHolder GetCutHolder() const { return m_cutHolder; };

    /**
     * \brief      Gets the cut holder.
     * @return     The cut holdeder housed in this EventType
     */
    CutHolder & GetCutHolder() { return m_cutHolder; };

    /**
     * \brief      Get the Weight Holder pointing to the stored one
     * @return     The weight holder housed in this EventType
     */
    WeightHolder & GetWeightHolder() { return m_weightHolder; };

    /**
     * \brief      Gets the weight holder copy of this EventType
     * @return     The weight holder copy house in this EventType
     */
    WeightHolder GetWeightHolder() const { return m_weightHolder; };

    /**
     * \brief      Get the tuple holder pointing to the stored one
     * @return     The tuple holder hosted in this EventType.
     */
    TupleHolder & GetTupleHolder() { return m_tupleHolder; };

    /**
     * \brief      Get the tuple holder copy
     * @return     The tuple holder.
     */
    TupleHolder GetTupleHolder() const { return m_tupleHolder; };

    /**
     * \brief      Get the selection cut of this EventType
     * @param[in]  _extraCut  post-pend an extra cut on the returned one, default nothing is added
     * @return     The cut.
     */
    TCut GetCut(const TString _extraCut = "") const;

    /**
     * \brief      Gets the weighted cut.
     * @param[in]  _extraCut  post-pend an extra cut on the returned one, default nothing is added
     * @return     The Weighted-Cut expression to use
     */
    TCut GetWCut(const TString _extraCut = "") const;

    /**
     * \brief      Gets the weight.
     * @return     The weight expression hosted and generated in this EventType
     */
    TString GetWeight() const { 
      TString _weight = m_weightHolder.Weight(); 
      if( _weight.Contains("wkin_RpK.wkin") && m_tupleHolder.Option().Contains("cre")){
        MessageSvc::Warning("Replace(wkin_RpK.wkin,wkin_RpK_wkin) weight string for TupleCreate Lb samples!");
        _weight = _weight.ReplaceAll("wkin_RpK.wkin", "wkin_RpK_wkin");
      };
      return _weight;
    }

    // bool isTupleCreate(){ return }

    // bool isTupleProcess(){ return }
    /**
     * \brief      Gets the tuple reader.
     * @return     The tuple reader hosted by this EventType
     */
    TupleReader & GetTupleReader() { return m_tupleHolder.GetTupleReader(); };

    /**
     * \brief      Gets the tuple as TChain
     * @return     The tuple hosted by this EventType
     */
    TChain * GetTuple() { return m_tupleHolder.GetTuple(); };

    const Long64_t GetTupleEntries(TCut _cut = "");

    const Long64_t GetTupleEntries(TString _cutOption, TString _weightOption, TString _tupleOption, TCut _cut = "");

    /**
     * \brief      Reduce stored tuple with a given fractional part of it
     * @param[in]  _frac  The fracional part : -1 all, [0,1] a 0-100% of it, >1 : the exact max entries to use.
     */
    void ReduceTuple(double _frac = -1);

    void ScanTuple(TString _option = "");

    /**
     * \brief      Sets the branches to switch on in the underlying tuple
     * @param[in]  _branches  The branches to enable
     */
    void SetBranches(vector< TString > _branches = {});

    /**
     * \brief      Gets the branches enabled in the ntuple
     * @return     The vector<TString> of branches enabled in the ntuple
     */
    vector< TString > GetBranches() { return m_tupleHolder.Branches(); };

    void SetAliases();

    /**
     * \brief      Gets the branches from a given Cut
     * @param[in]  _extraCut  The cut to parse
     * @return     The branches contained in the cut
     */
    vector< TString > GetBranches(TCut _extraCut);

    /**
     * \brief      Gets 1D histo given
     * @param[in]  _varX       The variable x to draw : the Name() is used as Branch to plot [note it must be already binned and with a range]
     * @param[in]  _extraCut   The extra cut, on top of the Cut stored in this EventType which extra cut you want to apply ? m_cut && (_extraCut)
     * @param[in]  _extraName  The extraName , to name the returned histogram (varPlotted_extraName) is the histo name
     * @return     The histo (1D)
     */
    TH1D * GetHisto(const RooRealVar & _varX, TCut _extraCut = TCut(NOCUT), const TString & _extraName = "");

    /**
     * \brief      Gets the histo 2D
     * @param[in]  _varX       The variable x to plot [note it must be already binned and with a range]
     * @param[in]  _varY       The variable y to plot [note it must be already binned and with a range]
     * @param[in]  _extraCut   The extra cut , on top of the Cut stored in this EventType which extra cut you want to apply ? m_cut && (_extraCut)
     * @param[in]  _extraName  The extra name , to name the returned histogram (varPlottedX_varPlottedY_extraName)
     * @return     The histo (2D)
     */
    TH2D * GetHisto(const RooRealVar & _varX, const RooRealVar & _varY, TCut _extraCut = TCut(NOCUT), const TString & _extraName = "");

    /**
     * \brief      Gets the data set from This EventType
     * @param[in]  _name      The name of dataset
     * @param[in]  _varList   The variable list which will contains all necessary Variable names for the cut
     * @param[in]  _extraCut  The extra cut to apply on top of the Cut from CutHolder
     * @param[in]  _frac      The frac
     * @return     The data set.
     */
    RooDataSet * GetDataSet(TString _name, RooArgList _varList, TCut _extraCut = TCut(NOCUT), double _frac = -1, TString _option = "");

    /**
     * \brief      Saves to disk using the ROOT I/O mechanism of TObject
     * @param[in]  _name     The name of the file (.log) where to store it. Path used SettingDef::IO::outDir + EventType {name}.root
     * @param[in]  _verbose  The verbose flag, more messaging
     */

    void SaveToDisk(TString _name = "", bool _verbose = true);

    /**
     * \brief      Load from disk using the ROOT I/O mechanism of TObject
     * @param[in]  _name     The name of the file (.log) to load .
     * @param[in]  _dir      The directory where the EventType is stored.
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    /**
     * \brief      Saves to named objects, fragment EventType to keywords which can be reused to construct it from scratch
     * @param      _tFile  The TFile where to store it
     * @param[in]  _name   The name of the Object to store
     */
    void SaveToNamed(TFile * _tFile, TString _name = "");

    /**
     * \brief      Loads a from named Use the SavedToName output to recreate an EventType
     * @param[in]  _name  The name of the file where the object is saved
     * @param[in]  _dir   The directory where the file is found
     */
    void LoadFromNamed(TString _name = "", TString _dir = "");

    /**
     * \brief      Saves to log the full string describing properties.
     * @param[in]  _name  The name of the file.log to save
     */
    void SaveToLog(TString _name = "");

    /**
     * \brief      Saves to yaml file the Current full setting known , used to generate YAML files in the submitter.
     * @param[in]  _name    The name
     * @param[in]  _option  The option
     * @return    the returned name (which gets updated inside the method)
     */
    TString SaveToYAML(TString _name = "", TString _option = "");

    /**
     * \brief      Gets the named cut option.
     * @return     The named cut option.
     */
    const TNamed GetNamedCutOption() const;

    /**
     * \brief      Gets the named weight option.
     * @return     The named weight option.
     */
    const TNamed GetNamedWeightOption() const;

    /**
     * \brief      Gets the named tuple option.
     * @return     The named tuple option.
     */
    const TNamed GetNamedTupleOption() const;

    /**
     * \brief      Gets the named weight.
     * @return     The named weight.
     */
    const TNamed GetNamedWeight() const;

  private:
    /**
     * \brief The object housing and generating the cuts
     */
    CutHolder m_cutHolder;   // Underlying CutHolder
    /**
     * \brief The object housing and generating the weight expression
     */
    WeightHolder m_weightHolder;   // Underlying WeigthHolder
    /**
     * \brief The objext housing and reading the Input ntuples
     */
    TupleHolder m_tupleHolder;   // Underlying TupleHolder

    bool m_isInitialized = false;   //! do not serialize (we want to do Init() once stored)

    bool m_debug = false;

    /**
     * \brief      Sets the debug of it, more messaging
     * @param[in]  _debug  The debug
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    ClassDef(EventType, 1);
};

ostream & operator<<(ostream & os, const EventType & _eventType);
