#ifndef EFFICIENCYSVC_HPP
#define EFFICIENCYSVC_HPP

#include "EnumeratorSvc.hpp"

#include "SettingDef.hpp"
#include "IOSvc.hpp"
#include "RooRealVar.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2Poly.h"
#include "VariableBinning.hpp"
#include <ROOT/RDF/HistoModels.hxx>
class ConfigHolder;
class RooRealVar;
class TEfficiency;

namespace EfficiencyFileContent {
    const vector< TString > Types = {"flt", "gen",   // Filtering and Generator Efficiency
                                     "sel", "sel_rnd", "tot", "tot_rnd", "lumi"};
}

inline bool EffDebug() noexcept { return SettingDef::debug.Contains("EFF"); }

RooRealVar * LoadEfficiencyForFit(ConfigHolder & _configHolder, TString _mode = "BKGOVERSIGNAL") noexcept;

bool IsEfficienciesCorrelated(const ConfigHolder & _configA, const ConfigHolder & _configB) noexcept;

double GetVariance(const ConfigHolder & _config) noexcept;

double GetCovariance(const ConfigHolder & _configA, const ConfigHolder & _configB) noexcept;

/**
 * \brief
 * @param[in]  _project, _year, _polarity
 * @return pair < value > of the number of passed events from filtering file
 * If the year is Run1 it does the filling
 */
map< pair< Year, Polarity >, int > GetNPasFiltering(const Prj & _prj, const Year & _year, const Polarity & _polarities, const TString & _sample, bool _debug = false) noexcept;

/**
 * @brief      Gets the n total filtering.
 *
 * @param[in]  _prj         The project
 * @param[in]  _year        The year
 * @param[in]  _polarities  The polarities
 * @param[in]  _sample      The sample
 * @param[in]  _debug       The debug
 *
 * @return     The n total filtering, reading files.
 */
map< pair< Year, Polarity >, int > GetNTotFiltering(const Prj & _prj, const Year & _year, const Polarity & _polarities, const TString & _sample, bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _project, _year, _polarity
 * @return pair < value, error> of the integrated luminosity value associated to that (grabbed from LPT sample!)
 * If the year is Run1 it does the average automatically of the Generartor Level efficiencies found in the yaml files
 */
pair< double, double > LoadLuminosity(const Prj & _project, const Year & _year, const Polarity & _polarity, bool _debug = false, bool _silent = false) noexcept;

pair< double, double > LoadLuminosity(const TString & _project, const TString & _year, const TString & _polarity, bool _debug = false, bool _silent = false) noexcept;

/**
 * @brief      Gets the mct entries reading file
 *
 * @param[in]  _prj       The project
 * @param[in]  _year      The year
 * @param[in]  _polarity  The polarity
 * @param[in]  _sample    The sample
 * @param[in]  _debug     The debug
 *
 * @return     The mct entries.
 */
RooRealVar * LoadLuminosityVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, bool _debug = false) noexcept;

double GetMCTEntries(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _force = false, TString _type = "ngng_evt", bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _prj, _year, _polarity, name of the sample
 * @return pair < value, error>, first is value, second is error
 * If the year is Run1/2 it does the average automatically of the Generartor Level efficiencies found in the yaml files, same story for Polarities when combined
 */
pair< double, double > GetGeneratorEfficiency(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, const Q2Bin & _q2bin ,  bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _year, _polarity, name of the sample
 * @param[in] _varName; name of the RooRealVar assigned
 * @return RooRealVar named with _varName
 * If the year is Run1 it does the average automatically of the Generartor Level efficiencies found in the yaml files
 */
RooRealVar * GetGeneratorEfficiencyVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, const Q2Bin & _q2bin , bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _prj, _year, _polarity, name of the sample
 * @return pair < value, error>, first is value, second is error
 * If the year is Run1/2 it does the average automatically of the Filtering efficiencies found in the yaml files, same story for Polarities when combined
 * TODO : if renormalized nPas MCDecayTuple , enable the nPas_Mod ....
 */
pair< double, double > GetFilteringEfficiency(const Prj & _prj, const Year & _year, const Polarity & _polarities, const TString & _sample /*,const int nPas_Mod = -9999*/, bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _varName, _prj, _year, _polarity, name of the sample : _varName used to assign the variable name
 * @return RooRealVar, first is value, second is error
 * If the year is Run1/2 it does the average automatically of the Filtering efficiencies found in the yaml files, same story for Polarities when combined
 * Internally calls GetFilteringEfficiency
 * TODO : if renormalized nPas MCDecayTuple , enable the nPas_Mod ....
 */
RooRealVar * GetFilteringEfficiencyVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample /*, const int nPasMod = -9999,*/, bool _debug = false) noexcept;

/**
 * \brief
 * @param[in]  _nPas, _nTot : passed and total (2 numbers)
 * @return RooRealVar having lower/upper errors from TEfficiency AsymError build using histograms ratio with 1 bin filled with nPas/nTot.
 */
RooRealVar * GetEfficiencyVar(double _nPas, double _nTot, bool _debug = false) noexcept;

/**
 * \brief
 * @param  _hPas   [description]
 * @param  _hTot   [description]
 * @return tuple< RooRealVar(integrated eff), TEfficiency from ratio, TH1D ratio first is value >
 * If the year is Run1/2 it does the average automatically of the Filtering efficiencies found in the yaml files, same story for Polarities when combined
 * Internally calls GetFilteringEfficiency
 * TODO : extend this to TH1, adapt code to support 2D plots as well!
 * @param  _hPasIn   [Histo1D IN Num, not modificable ]
 * @param  _hTotIn   [Hist1D IN Den, not modificable ]
 * @param  _tEff   [TEfficiency from Ratio]
 * @param  _hEff   [TH1D or efficiency]
 */
tuple< RooRealVar *, TEfficiency *, TH1D * > GetEfficiencyResults(const TH1 & _hPasIn, const TH1 & _hTotIn);

/**
 * @brief      Makes a ratio plot.
 *
 * @param      h_num  The h number
 * @param      h_den  The h den
 * @param[in]  padID  The pad id
 * @param[in]  logy   The logy
 */
void MakeRatioPlot(pair< TH1D *, TH1D * > & h_num_pair, pair< TH1D *, TH1D * > & h_den_pair, int padID = 1, bool logy = false);

/**
 * @brief      Gets all bin splits.
 *
 * @param[in]  _prj      The project
 * @param[in]  _year     The year
 * @param[in]  _trg      The trg
 * @param[in]  _trgConf  The trg conf
 *
 * @return     A vector for <Name, EXpression> of ALL BINS DEFINED AND FOUND from IsoBinCuts.csv files
 */

extern vector< VariableBinning > GetAllBinLabels(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf);


/**
 * @brief      Get The Histograms housing the "bins" for the flatness plot
 *
 * @param[in]  _prj      The project
 * @param[in]  _year     The year
 * @param[in]  _trg      The trg
 * @param[in]  _trgConf  The trg conf
 *
 * @return     All variable template histo.
 */
extern map< TString, TH1 * > GetAllVarTemplateHisto(const Prj & _prj, const Year & _year, const Trigger & _trg, const TriggerConf & _trgConf);

/**
 * @brief      EfficiencyContent object , It house a RooRealVar and stores info on the Numerator/Denominator to use, it is at all effect a RooRealVar masked ( or creator )
 */
struct EfficiencyContent {
    TString      Name;
    double       Numerator;
    double       Denominator;
    RooRealVar * EfficiencyVar = nullptr;
    EfficiencyContent()        = default;
    EfficiencyContent(TString _Name) { Name = _Name; }
    EfficiencyContent(TString _Name, double _Num, double _Den) {
        Name = _Name;
        MakeRooRealVar(_Num, _Den);
    }
    EfficiencyContent(TString _Name, pair< double, double > _number_error) {
        Name          = _Name;
        EfficiencyVar = new RooRealVar(_Name, _Name, _number_error.first);
        EfficiencyVar->setError(_number_error.second);
        UpdateName();
    }
    /**
     * @brief      MakeRooReaklVar by taking benefit of the "GetEfficiencyVar"
     */
    void MakeRooRealVar(double _Num, double _Den) {
        Numerator     = _Num;
        Denominator   = _Den;
        EfficiencyVar = GetEfficiencyVar(Numerator, Denominator);
        EfficiencyVar->SetName(Name);
        EfficiencyVar->SetTitle(Name);
        return;
    }

    /**
     * @brief      Update naming scheme of the RooRealVar
     */
    void UpdateName() {
        EfficiencyVar->SetName(Name);
        EfficiencyVar->SetTitle(Name);
        EfficiencyVar->setConstant();
        return;
    }
    /**
     * @brief      Retrieve the Value of the underlying Efficiency RooRealVar
     *
     * @return     The value.
     */
    double getVal() { return EfficiencyVar->getVal(); }
    /**
     * @brief      Retrieve the Error of the underlying Efficiency RooRealVar
     *
     * @return     The error.
     */
    double getError() { return EfficiencyVar->getError(); }
    /**
     * @brief      Retrieve the relative Error of the underlying Efficiency RooRealVar
     *
     * @return     The relative error.
     */
    double getRelError() { return EfficiencyVar->getError() / EfficiencyVar->getVal(); }
};

/**
 * @brief      Add to the _EfficiencyMap which does contain the "sel" = selection only label efficiencies the corresponsing Efficiencies multiplying by Generator and Filtering
 *
 * @param      _Efficiencies  The efficiencies map which gets updated in the method appending the "tot_rnd", "tot_xxx" nicknames....methods used in efficiencyCreate.cpp multiplying already existing pieces
 */
extern void AddTOTEfficienciesToMap(map< TString, EfficiencyContent > & _Efficiencies);

/**
 * @brief      Add the generator level efficiency to _Efficiencies maps given a ConfigHolder.
 * @param      _Efficiencies  The efficiency map to be updated (passed by reference !!! DO NOT MODIFY THIS!)
 * @param[in]  _ConH          The configHolder giving directives to where Grab generator level efficiencies
 */
extern void AddGeneratorEfficiencyToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH);

/**
 * @brief      Add thes filtering efficiency to _Efficiencies map given the ConfigHolder input infos
 *
 * @param      _Efficiencies  The efficiency Map to update (passed by reference !!! DO NOT MODIFY THIS!)
 * @param[in]  _ConH          The ConfigHolder used to instruct the Filterign efficiency grabbing
 */
extern void AddFilteringEfficiencyToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH);

/**
 * @brief      Add the Luminosity of the ConfigHolder to the map
 * @param      _Efficiencies  The efficiency map to update (passed by reference !!! DO NOT MODIFY THIS!)
 * @param[in]  _ConH          The con h
 */
extern void AddLumiToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH);

/**
 * @brief      Extract All Efficiency Results from TFile for a given basename = ConfigHolder associated to the File GetKey()
 *
 * @param[in]  _base_name  The base name of eff vars eff_ConH.Gety() +"_"+VARIANTS
 * @param      _file       The TFile to read
 * @param[in]  _option     The option, if "ISO", load the "BINS as well but, base name has to contains "/" to inspect a given directory for a given bin!
 *
 * @return     a map housing the Effficiency
 *  map, example =   { "sel" : , "flt" :  , "tot" , "sel_rnd"} ......
 */
extern map< TString, EfficiencyContent > RetrieveAllResults(TString _base_name, TFile & _file, TString _option = "");

/**
 * @brief      Gets the map histo template.
 *
 * @param[in]  _project  The project
 * @param[in]  _year     The year
 * @param[in]  _trigger  The trigger
 * @param[in]  _trgConf  The trg conf
 * @param[in]  _type     The type ["YIELDS", "EFFS"]
 * @return     The map histo template.
 */
map< TString, ROOT::RDF::TH1DModel > GetMapHistoTemplate(const Prj & _project, const Year & _year, const Trigger & _trigger, const TriggerConf & _trgConf, TString _type = "YIELDS");
/**
 * @brief      Gets the map histo template,  internally calls the  hashed types.
 *
 * @param[in]  _project  The project
 * @param[in]  _year     The year
 * @param[in]  _trigger  The trigger
 * @param[in]  _trgConf  The trg conf
 * @param[in]  _type     The type
 *
 * @return     The map histo template.
 */
map< TString, ROOT::RDF::TH1DModel > GetMapHistoTemplateS(const TString & _project, const TString & _year, const TString & _trigger, const TString & _trgConf, TString _type = "YIELDS");

/**
 * @brief      Retrieves a root file containing eefficiency results
 *
 * @param[in]  _prj           The project
 * @param[in]  _ana           The ana
 * @param[in]  _q2Bin         The quarter 2 bin
 * @param[in]  _yy            { parameter_description }
 * @param[in]  _pol           The pol
 * @param[in]  _trg           The trg
 * @param[in]  triggerConf    The trigger conf
 * @param[in]  _Sample        The sample
 * @param[in]  _weightConfig  The weight configuration
 * @param[in]  _weightOption  The weight option
 *
 * @return     The root file efficiency.
 */
TString RetrieveRootFileEfficiency(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _yy, const Polarity & _pol, const Trigger & _trg, const TriggerConf & triggerConf, const TString & _Sample, const TString & _weightOption, const TString & _weightConfig);
/**
 * @brief      As  beefore but string version
 *
 * @param[in]  _prj           The project
 * @param[in]  _ana           The ana
 * @param[in]  _q2Bin         The quarter 2 bin
 * @param[in]  _yy            { parameter_description }
 * @param[in]  _pol           The pol
 * @param[in]  _trg           The trg
 * @param[in]  triggerConf    The trigger conf
 * @param[in]  _Sample        The sample
 * @param[in]  _weightOption  The weight option
 * @param[in]  _weightConfig  The weight configuration
 *
 * @return     The root file efficiency s.
 */
TString RetrieveRootFileEfficiencyS(const TString & _prj, const TString & _ana, const TString & _q2Bin, const TString & _yy, const TString & _pol, const TString & _trg, const TString & triggerConf, const TString & _Sample, const TString & _weightOption, const TString & _weightConfig);
/**
 * @brief      As before but  via ConfigHolder + wOpt, cOpt
 *
 * @param[in]  co             { parameter_description }
 * @param[in]  _weightOption  The weight option
 * @param[in]  _weightConfig  The weight configuration
 *
 * @return     The root file efficiency co.
 */
TString RetrieveRootFileEfficiencyCO(const ConfigHolder & co, const TString & _weightOption, const TString & _weightConfig);

/**
 * @brief Namespace handling  binned histos efficiencies
 *
 * Helper Function(s) building the 3 sets  of  histograms needed
 * SumW  is basically  Draw("Var", "weight * (  cut &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
 * NormN is basically  Draw("Var", "weightNormN * (  cutNorm &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
 * NormD is basically  Draw("Var", "weightNormN * (  cutNorm &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
 * Efficiency histo  is  extracted with SumW->Scale( normN(integral)/normD(integral) /  nMCDecayTuple(sumW) )
 * Integrals are done in the full range including overflow/underflows. SumW after scaling gets under/overflows also filled.
 * Details...if you want to "empty the efficiency in  first/last bin do it by hand moving BinContent/Error(0, nBins+1) " to (  1, nBins)
 * We do like thiis because fits are  "in ranges",  in  case we will fit bin0 and last bin up  to -inf,+inf we have to change this code...
 *
 */
class BinnedEffs {
  public:
    /**
     * \brief Constructor
     */
    BinnedEffs() = default;

    //--------------------------------------------------------------------------------------------------------
    //
    //-------- What can be  loadede in a root file containing directories for each VarID histogram
    //
    //--------------------------------------------------------------------------------------------------------
    static const vector< TString > m_modes;   //= { "normD", "normN","sumW" };
    static const vector< TString > m_types;   //= { ""  ,    "rnd",  "bkgcat"};

    /**
     * @brief ID to store and saved to file
     *
     * @param type
     * @param mode
     * @return TString
     */
    static TString ID(const TString & type, const TString & mode , bool raw = false);
    /**
     * @brief Get the Efficiencies  in  bins From File object
     *
     * @param f
     * @param variableBinning
     * @return map< TString,  TH1D *>
     */
    // workaround in pyROOT
    static map< TString, TH1D > GetEfficienciesFromFile(const TString & fileName, const TString & csvFile, bool normBin = false);

    // workaround in pyROOT
    static map< TString, vector<TH1D> > GetEfficienciesFromFileBS(const TString & fileName, const TString & csvFile, bool normBin = false);



    // If you know the list of varIDs ,  this method is way more efficient! 
    // Loads by default both "raw" and non "raw - with isobins"
    static map< TString, TH1D > GetEfficienciesFromFile(TFile & f, const TString & varID, bool normBin =false);

    static map< TString, vector<TH1D> > GetEfficienciesFromFileBS(TFile & f, const TString & varID, bool normBin =false);

    /**
     * @brief Core function to evaluate  histogram with BinContent == eff in bin
     *
     * @param sumW
     * @param normN
     * @param normD
     * @param nMCDecay
     * @return TH1D* for eficiencies
     * NB: naming scheme handled here.s
     */
    static TH1D EvalEfficiencyHisto(TH1D & sumW, TH1D & normN, TH1D & normD, const pair< double, double > & nMCDecay, bool normBin=false);
};


 /**
     * @brief Simple wrapper reading an efficiency.root file and generating the Bootstrapped Weight expressions to use
     *
     * @param TFile with efficiency information and how they have been obtained
*/
struct EfficiencyInfos{
    EfficiencyInfos() = default;
    EfficiencyInfos( TString _fileInput, TString _cHolderKeySource = ""){
        auto *_f = IOSvc::OpenFile(_fileInput, OpenMode::READ);
        _inputFile = _fileInput;
        _fullSelection           = TString( _f->Get("fullSelection")->GetTitle() );
        _weight_full             = TString( _f->Get("weight_full")->GetTitle() );
        _weight_normNum          = TString( _f->Get("weight_normNum")->GetTitle() );
        _weight_normDen          = TString( _f->Get("weight_normDen")->GetTitle() );
        _weight_MCDECAY          = TString( _f->Get("weight_MCDECAY")->GetTitle() );
        _normSelection           = TString( _f->Get("normSelection")->GetTitle() );
        _MCDecaySelection        = TString( _f->Get("MCDecaySelection")->GetTitle() );
        _KEY = _cHolderKeySource; 
        auto *n = (RooRealVar*) _f->Get("nMCDecay");
        _nMCDecay = n->getVal();
        IOSvc::CloseFile( _f);
        delete n;
    };

    TString _inputFile;
    TString _fullSelection;
    TString _normSelection;
    TString _weight_full;
    TString _weight_normNum;
    TString _weight_normDen;
    TString _MCDecaySelection;
    TString _weight_MCDECAY;
    TString _KEY; 
    double _nMCDecay;
    TString AddBSLabels( TString _weightExpressionBase ){
        TString _oldExpression = _weightExpressionBase;
        _oldExpression = _oldExpression.ReplaceAll(" ","");
        if( _oldExpression == "") return "RndPoisson2";
        TString _newWeight = Form( "RndPoisson2 * %s", _oldExpression.Data());
        // if( _oldExpression == "") return "RndPoisson_Custom";
        // TString _newWeight = Form( "RndPoisson_Custom * %s", _oldExpression.Data());
        //--- L0/HLT BS string
        _newWeight = _newWeight.ReplaceAll("effCL","effCL_BS").ReplaceAll("effMC","effMC_BS").ReplaceAll(" ","");//.ReplaceAll("(1.)", "");

        if(  GetBaseVer(SettingDef::Tuple::gngVer).Atoi() < 10){
            MessageSvc::Warning("GNG VER < 10 , no BS of PID,TRK,BDT done!");
            return _newWeight;
        }
        //--- PID BS string
        if( (!_newWeight.Contains("PIDCalibKDE")) && (!_newWeight.Contains("PIDCalib_Weight")) && _newWeight.Contains("PIDCalib") ) {
            _newWeight = _newWeight.ReplaceAll("wPIDCalib","wPIDCalib_BS").ReplaceAll("wiPIDCalib","wiPIDCalib_BS");
        }
        else if(_newWeight.Contains("PIDCalib_Weight")) {
            _newWeight = _newWeight.ReplaceAll("wPIDCalib_Weight","wPIDCalib_Weight_BS").ReplaceAll("wiPIDCalib_Weight","wiPIDCalib_Weight_BS");
        }

        //--- TRK BS string
        if( (!_newWeight.Contains("TRKCalib_BS")) && _newWeight.Contains("TRKCalib") ) {
            _newWeight = _newWeight.ReplaceAll("TRKCalib","TRKCalib_BS");
        }
	//--- BDT-BKIN BS string alone 
	//if(  (!_newWeight.Contains("BDT_BS_BKIN")) && _newWeight.Contains("BDT_BKIN") ) {
	//  _newWeight = _newWeight.ReplaceAll("BDT_BKIN","BDT_BS_BKIN");
	//}
	
        //--- BDT-BKIN-MULT BS string
        if( (!_newWeight.Contains("BDT_BS_BKIN_MULT")) && _newWeight.Contains("BDT_BKIN_MULT") ) {
            _newWeight = _newWeight.ReplaceAll("BDT_BKIN_MULT","BDT_BS_BKIN_MULT");
        }

        //--- BDT2-BKIN-MULT BS string
        if( (!_newWeight.Contains("BDT2_BS_BKIN_MULT")) && _newWeight.Contains("BDT2_BKIN_MULT") ) {
            _newWeight = _newWeight.ReplaceAll("BDT2_BKIN_MULT","BDT2_BS_BKIN_MULT");
        }

        //--- BDT3-BKIN-MULT BS string (for EE) from v10 onwards...
        if( (!_newWeight.Contains("BDT3_BS_BKIN_MULT")) && _newWeight.Contains("BDT3_BKIN_MULT") ) {
            _newWeight = _newWeight.ReplaceAll("BDT3_BKIN_MULT","BDT3_BS_BKIN_MULT");
        }  

        return _newWeight;
    }
    TString fullWeightBS(){
        return AddBSLabels( _weight_full);
    }
    TString weightMCDecayBS(){
        return AddBSLabels( _weight_MCDECAY);
    }
    TString normNumWeightBS(){
        return AddBSLabels( _weight_normNum);
    }    
    TString normDenWeightBS(){
        return AddBSLabels( _weight_normDen);
    }      
    TString fullWeight(){
        return _weight_full;
    }
    TString fullSelection(){
        return _fullSelection;
    }
    TString normSelection(){
        return _normSelection;
    }    
    TString MCDecaySelection(){
        return _MCDecaySelection;
    }

    TString GetKey(){
        return _KEY; 
    }
    double nMCDecay(){
        return _nMCDecay;
    }
    TString SourceFile(){
        return _inputFile;
    }

    void Print(){
        MessageSvc::Line(); 
        MessageSvc::Info("EfficiencyInfo Print for BS calculation", _KEY );
        MessageSvc::Line();
        std::cout<< GREEN  <<"# Source File    : "<< SourceFile()      << std::endl;
        std::cout<< GREEN  <<"# KEY            : "<< GetKey()          << std::endl;
        std::cout<< YELLOW <<"# selection(f)   : "<< fullSelection()     << std::endl;
        std::cout<< YELLOW <<"# selection(n)   : "<< normSelection()     << std::endl;
        std::cout<< YELLOW <<"# selection(mcdt): "<< MCDecaySelection()  << std::endl;
        std::cout<< GREEN  <<"# weightBS(f)    : "<< fullWeightBS()      << std::endl;
        std::cout<< GREEN  <<"# weightBS(n_num): "<< normNumWeightBS()   << std::endl;
        std::cout<< GREEN  <<"# weightBS(n_den): "<< normDenWeightBS()   << std::endl;
        std::cout<< GREEN  <<"# weightBS(mcdt) : "<< weightMCDecayBS()   << RESET<<std::endl;
        MessageSvc::Line();


    }    
};

#endif
