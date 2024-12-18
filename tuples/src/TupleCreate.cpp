#ifndef TUPLECREATE_CPP
#define TUPLECREATE_CPP

#include "TupleCreate.hpp"

#include "SettingDef.hpp"

#include "vec_extends.h"
#include <fmt_ostream.h>
#include "HelperProcessing.hpp"
TupleCreate::TupleCreate(const EventType & _eventType, TString _yaml, TString _option) {
    if (SettingDef::debug.Contains("TC")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleCreate", (TString) "EventType");
    m_eventType                = _eventType;
    SettingDef::trowLogicError = true;
    m_samples                  = ParserSvc::GetListOfSamples(_yaml, m_eventType.GetQ2bin(), m_eventType.GetAna());
    SettingDef::trowLogicError = false;
    m_option                   = _option;
    Check(m_option);
    Init();
}

TupleCreate::TupleCreate(const TupleCreate & _tupleCreate) {
    if (SettingDef::debug.Contains("TC")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("TupleCreate", (TString) "TupleCreate");
    m_eventType = _tupleCreate.GetEventType();
    m_samples   = _tupleCreate.Samples();
    m_option    = _tupleCreate.Option();
    Check(m_option);
}

ostream & operator<<(ostream & os, const TupleCreate & _tupleCreate) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "TupleCreate");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "samples", to_string(_tupleCreate.Samples().size()));
    MessageSvc::Print((ostream &) os, "option", _tupleCreate.Option());
    MessageSvc::Line(os);
    // os << _tupleCreate.GetEventType();
    for (const auto & _sample : _tupleCreate.Samples()) { MessageSvc::Print((ostream &) os, _sample.first); }
    MessageSvc::Line(os);
    os << RESET;
    return os;
}

bool TupleCreate::Check(TString _option) {
    for (auto _opt : *(_option.Tokenize("-"))) {
        if (!CheckVectorContains(SettingDef::AllowedConf::TupleOptions, ((TObjString *) _opt)->String())) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("TupleCreate", "\"" + ((TObjString *) _opt)->String() + "\"", "option not in SettingDef::AllowedConf::TupleOptions", "EXIT_FAILURE");
        }
    }

    if (m_eventType.GetProject() == Prj::All) MessageSvc::Error("TupleCreate", (TString) "Invalid Project", "EXIT_FAILURE");
    if (m_eventType.GetAna() == Analysis::All) MessageSvc::Error("TupleCreate", (TString) "Invalid Analysis", "EXIT_FAILURE");
    // if (m_eventType.GetQ2bin() == Q2Bin::All) MessageSvc::Error("TupleCreate", (TString) "Invalid Q2Bin", "EXIT_FAILURE");
    if (m_eventType.GetYear() == Year::All) MessageSvc::Error("TupleCreate", (TString) "Invalid Year", "EXIT_FAILURE");
    if (m_eventType.GetPolarity() == Polarity::All) MessageSvc::Error("TupleCreate", (TString) "Invalid Polarity", "EXIT_FAILURE");

    return false;
}

void TupleCreate::Init() {
    MessageSvc::Line();
    MessageSvc::Info("TupleCreate", (TString) "Initialize", to_string(m_samples.size()), "sample(s) for", to_string(m_eventType.GetAna()), to_string(m_eventType.GetQ2bin()), "...");

    vector< TString > _samples = {};
    for (auto && _sample : m_samples) {
        MessageSvc::Line();
        MessageSvc::Info("Sample", _sample.first, TString(fmt::format("(nEntries = {0}, nSplits = {1})", _sample.second.first.GetTuple()->GetEntries(), _sample.second.second.size())));
        _sample.second.first.PrintInline();

        vector< TString > _names = {};
        for (auto & _info : _sample.second.second) {
            TString _name = get< 0 >(_info).GetTupleName(get< 1 >(_info).Option() + "-" + get< 2 >(_info).Option() + "-" + _sample.second.first.Option());
            MessageSvc::Warning("Booked", _name);
            get< 0 >(_info).PrintInline();
            get< 1 >(_info).PrintInline();
            get< 2 >(_info).PrintInline();
            if (find(_names.begin(), _names.end(), _name) != _names.end()) MessageSvc::Error("Initialize", _name, "already booked", "EXIT_FAILURE");
            _names.push_back(_name);
        }
        _samples.insert(_samples.end(), _names.begin(), _names.end());
    }

    MessageSvc::Line();
    MessageSvc::Info("Samples to process");
    for (auto & _sample : _samples) { MessageSvc::Info("Sample", _sample); }
    MessageSvc::Line();

    cout << *this << endl;
    return;
}

tuple< TString, TString, TString, bool > TupleCreate::GetSelections(tuple< ConfigHolder, CutHolder, WeightHolder > & _info, TupleHolder & _tuple) {
    auto _configHolder = get< 0 >(_info);
    auto _cutHolder    = get< 1 >(_info);
    auto _weightHolder = get< 2 >(_info);
    auto _cut          = TString(_cutHolder.Cut());
    auto _weight       = _weightHolder.Weight();
    // we make a new branch for this selection being _weight * _cut != 0 ? if true assign to this branch +2.5, else -2.5. I.e
    // If the weighted expression return something different from 0, then this selection is passed.
    // We use after a double check on filtering, if the value on this selection is > 0 then we keep the event.
    // Else reject . s.second.second IS NOT SHUFFLED IN THE LOOPS SO DOING IT TWICE IS FINE.    
    TString _expressionWC = TString(fmt::format("(({0}) != 0) ? 2.5 : -2.5", _cut));
    /* 
       if(IsWeight(_weight) && ! SettingDef::Efficiency::option.Contains("OnTheFly")){ 
         //Compute a weight * selection ONLY IF TupleCreate is run in non-attaching weights mode.
	 _expressionWC = TString(fmt::format("(({0}) * ({1}) != 0) ? 2.5 : -2.5", _weight, _cut)); 
       }
    */
    TString _name          = _configHolder.GetTupleName(_cutHolder.Option() + _weightHolder.Option() + _tuple.Option());
    TString _selectionName = ((TString) _name).ReplaceAll(" ", "_").ReplaceAll("-", "_");
    TString _selection     = TString("SELECTION_") + _selectionName;
    
    MessageSvc::Line();
    MessageSvc::Info("TupleCreate", (TString) "GetSelections", _selectionName);
    _configHolder.PrintInline();
    _cutHolder.PrintInline();
    _weightHolder.PrintInline();
    MessageSvc::Info(_selectionName, _expressionWC);
    MessageSvc::Line();
    return make_tuple(_selection, _expressionWC, _name, _configHolder.IsMC());
}

void TupleCreate::Create() {
    MessageSvc::Line();
    MessageSvc::Info("TupleCreate", (TString) "Creating", to_string(m_samples.size()), "sample(s) for", to_string(m_eventType.GetAna()), to_string(m_eventType.GetQ2bin()), "...");
    MessageSvc::Line();

    EnableMultiThreads();
    // ROOT::DisableImplicitMT();//Much slower, but MUCH SAFER! 
    using SnapRet_t = ROOT::RDF::RResultPtr< ROOT::RDF::RInterface< ROOT::Detail::RDF::RLoopManager > >;   // the RDataFrame snapshot return type

    vector< tuple< TString, TString, bool > > _rootFiles = {};   // collector of root file names and tuple names which gets finally merged ...

    //--------------------------------------------------------------------------------------------------------
    //   Run the single q2-year etc Yaml file
    //   Retrieve from Yaml file on a given q2-slice the list of samples to cut and split,  Year, Polarity, Projects is picked from the Setting::Config fields parsed beforehand
    //   For more info see ParserSvc::GetListOfSamples method
    //   Loop over all Sample(_sample) defined in the list of dictionary from Yaml, init a single tuple (uncutted) and define the selections on that
    //--------------------------------------------------------------------------------------------------------
    // SettingDef::IO::isTupling = false; // Now we should initialize from List the tuples TMPHACK
    if( IOSvc::ExistFile("TupleCreate.root")){
        MessageSvc::Warning("Removing locally created TupleCreate");
        IOSvc::runCommand( TString("rm TupleCreate.root"));
    }
    for (auto && _sample : m_samples) {
        // Body loop of a Given SAMPLE to Process
        // Get the TupleHolder which contains the Tuple over which one wants to apply the selections.
        TupleHolder _tuple = _sample.second.first;
        ConfigHolder _chTuple =  _tuple.GetConfigHolder();
        _tuple.Init(true);

        MessageSvc::Line();
        MessageSvc::Info("Proceessing & Splitting Sample", _sample.first, "nEntries =", to_string(_tuple.GetTuple()->GetEntries()));

        // Make the DataFrame with This TChain and add a dummy logStart event loop
        ROOT::RDataFrame df(*_tuple.GetTuple());

        ROOT::RDF::RNode latestDF( df);

        MessageSvc::Info("Create", (TString) "Recursive Define will be applied for all defined selections on this sample");
        //------- Recursive Define of selections for the "slices" to do on the input tuple : i.e. data splitted by trigger category,

        //Do the on the fly only for the necessary ones, including Backgrounds, have to add samples here if you want the wPID * wTRK * wL0 shapes of them inside TupleCreate files.
        if( _chTuple.IsSignalMC() || _chTuple.IsCrossFeedSample() || _chTuple.IsLeakageSample() ||_sample.first == "Bu2KEtaPrimeGEE" || _sample.first == "Bu2KPiPiEE"){
            if( SettingDef::Efficiency::option.Contains("OnTheFly") && ( _sample.first != "LPT" && _sample.first != "LPTSS") ){
                MessageSvc::Info("Create", (TString) "Attach Weights On The Fly");                
                //RNode df,  ConfigHolder & _configHolder, TString _weightOption
                latestDF = HelperProcessing::AttachWeights(latestDF, _tuple.GetConfigHolder(), SettingDef::Weight::option );
            }
        }
        
        bool _renamedRpK = false; 
        if( latestDF.HasColumn("wkin_RpK.wkin")){
            latestDF = latestDF.Define("wkin_RpK_wkin", "wkin_RpK.wkin");
            _renamedRpK=true; 
        }

        for (auto _aliasExpressionPair : _tuple.Aliases()) {
            TString _alias      = _aliasExpressionPair.first;
            TString _expression = ((TString) _aliasExpressionPair.second).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")");
            latestDF            = latestDF.Define(_aliasExpressionPair.first.Data(), _expression.Data());
        }

        vector< TString > _aliases;
        for (auto && _info : _sample.second.second) {
            auto    _selections     = GetSelections(_info, _tuple);
            TString _selectionLabel = get< 0 >(_selections);
            TString _expressionWC   = get< 1 >(_selections);
            latestDF                = latestDF.Define(_selectionLabel.Data(), _expressionWC.Data());
            if (CheckVectorContains(_aliases, _selectionLabel)) { MessageSvc::Error("Create", (TString) "Alias selection is not unique", "EXIT_FAILURE"); }
            _aliases.push_back(_selectionLabel);
        }
        //------- Actual Slicing and snapshot of cutted ntuple is done here (define the snapshotting as Lazy action, thus not triggering the event loop).
        //------- Each slice produce a single ROOT file with a single TTree in it
        ROOT::RDF::RSnapshotOptions _options;
        _options.fLazy = true;         // start the event loop when actually triggered
        _options.fMode = "RECREATE";   // recreate the tuple and the TFile
        vector< SnapRet_t > rets;      // the collector of snapshots
        for (auto && _info : _sample.second.second) {
            auto _selections     = GetSelections(_info, _tuple);
            auto _selectionLabel = get< 0 >(_selections);
            auto _expressionWC   = get< 1 >(_selections);
            auto _tupleName      = get< 2 >(_selections);
            auto _isMC           = get< 3 >(_selections);
            // reuse latestDF (which has all selections defined as new columns and filter it)
            auto    ss_filter = latestDF.Filter([](const double & wcut) { return wcut > 0.; }, {_selectionLabel.Data()});
            TString _fileName = _tupleName + ".root";
            TString _treeName = _tupleName;
            if( IOSvc::ExistFile(_fileName)){
                MessageSvc::Error( "ALREADY DONE, something is going wrong !", _fileName, "EXIT_FAILURE");
            }
            MessageSvc::Info("Create", (TString) "Write TTree", _treeName, "to File", _fileName);
            MessageSvc::Info(_selectionLabel, _expressionWC);
            // rets.emplace_back(ss_filter.Snapshot(_treeName.Data(), _fileName.Data(), {"Year"}, _options)); //Just for check, snapshot 1 variable only, debug things saving time with this.
            if( !_renamedRpK){
                rets.emplace_back(ss_filter.Snapshot(_treeName.Data(), _fileName.Data(), HelperProcessing::DropColumnsWildcard(ss_filter.GetColumnNames(), {"RndPoisson"}) , _options));            
            }else{
                //wkinRpK.bkin fix
                rets.emplace_back(ss_filter.Snapshot(_treeName.Data(), _fileName.Data(), HelperProcessing::DropColumnsWildcard(ss_filter.GetColumnNames(), {"wkin_RpK.wkin"} ), _options));
            }
            ofstream ofs(_tupleName + ".log", ofstream::out);
            ofs << "Label        : " << _selectionLabel << "\n";
            ofs << "Selection    : " << _expressionWC << "\n";
            ofs << "InputTuple(s): \n";
            for( auto & fileInput : _tuple.GetFileNames()){ 
                ofs << "\t" << fileInput << "\n";
            }
            ofs.close();
            _rootFiles.push_back(make_tuple(_treeName, _fileName, _isMC));
        }

        MessageSvc::Line();
        MessageSvc::Warning("MakeSnapshots", (TString) "Event loop - START");
        auto _size = *df.Count();
        MessageSvc::Warning("MakeSnapshots", (TString) "Event loop - STOP");
        MessageSvc::Warning("MakeSnapshots", (TString) "Entries =", to_string(_size));
        MessageSvc::Line();

        TString _graph = _sample.first + ".dot";
        ROOT::Internal::RDF::SaveGraph(df, _graph.Data());   // dump the computational graph of define/filter/snapshot
        // Close up everything for this processed tuple. Ready for next sample
        _tuple.Close();
    }

    //-------------------- At this stage on disk we have created for each slice we did a single FileName.root with inside TreeName (TTree*),
    // We go single threaded and we append the multiple Candidate flag to each of them
    // For each "produced" "tuple" we will attach multiple candidates
    //---- TO DO : attach branch for multiple candidate counting here on each "produced tuple" (we may need some extra info to know which "flag" to use from current nominal tuple)
    //---  Take the new tuples and do the MultipleCandidate job... sequentially for each ntuple, with TTreeReader...
    //---  (not very fast, but still..., much less entry to process than before, overhead is minimal)
    DisableMultiThreads();   // let's switch it off
    
    for (auto & _rootFile : _rootFiles) {
        TString _treeName = get< 0 >(_rootFile);
        TString _fileName = get< 1 >(_rootFile);
        bool    _isMC     = get< 2 >(_rootFile);
        MessageSvc::Line();
        MessageSvc::Info("Create", (TString) "Adding MultipleCandidate flags to", _fileName, _treeName, _isMC);
        TFile   _file(_fileName, "UPDATE");
        TTree * _tuple = (TTree *) _file.Get(_treeName);
        if (_tuple == nullptr || _tuple->GetEntries() == 0) {
            MessageSvc::Warning("Create", (TString) "Tuple does not exist / empty, SKIPPING");
            _file.Close();
            continue;
        }

        vector< FUNC_PTR > _isSingleFuncs = {&MultCandRandomKill};
        vector< TString >  _isSingleNames = {"RND"};
        if (_isMC) {
            _isSingleFuncs.push_back(&MultCandBestBkgCat);
            _isSingleNames.push_back("BKGCAT");
        }

        TupleReader _tupleReader = TupleReader((TChain *) _tuple);
        _file.cd();
        _tuple = (TTree *) _tupleReader.GetMultCandTuple(_isSingleFuncs, _isSingleNames);
        _tuple->CloneTree()->Write(_treeName, TObject::kOverwrite);
        _file.Close();
    }

    return;
}

#endif
