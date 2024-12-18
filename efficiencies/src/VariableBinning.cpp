#ifndef VARIABLEBINNING_CPP
#define VARIABLEBINNING_CPP
#include "VariableBinning.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "TFile.h"
#include "TTree.h"
#include <fmt_ostream.h>

VariableBinning::VariableBinning(TString _csv , TString _varX, TString _varY) {
    m_CsvFile  = _csv;
    m_RootFile = m_CsvFile;
    m_RootFile.ReplaceAll("IsoBinCuts.csv", "IsoBinResults.root");
    m_varID = BuildVarID();
    if (m_varID == "ERROR") { MessageSvc::Error("CANNOT LOAD VAR FROM CSV FILE", "", "EXIT_FAILURE"); }
    m_loaded = false;
    Init();
    m_varX = _varX;
    m_varY = _varY;
    if (!m_is1D) { m_varID.ReplaceAll("-", "_"); }
    // MessageSvc::Debug("VariableBinning (arg)", m_varID +" "+ m_CsvFile,  m_RootFile);
    return;
}

VariableBinning::VariableBinning(ROOT::RDF::TH1DModel & model1D  , TString varX){
    m_varID = varX; 
    m_varX = varX; 
    m_loaded = true; 
    m_is1D = true; 
    auto BinsEdges = model1D.fBinXEdges;

    for( int i =0; i < BinsEdges.size() -1; ++i){
        double lowEdge = BinsEdges[i];
        double upperEdge = BinsEdges[i+1];
        m_binCuts.push_back( TString::Format( "%s>=%.8f && %s<%.8f", varX.Data(), lowEdge, varX.Data(), upperEdge ));
    }
    m_histo1D = static_cast< TH1D * >(model1D.GetHistogram()->Clone(m_varID.Data()));
    m_histo1D->SetDirectory(0);
    m_histo1D->Reset("ICES");    
    m_histo1DModel = model1D;
    m_isFromModel=true;
    return;
}

VariableBinning::VariableBinning(const VariableBinning & other) {
    m_CsvFile  = other.m_CsvFile;
    m_RootFile = other.m_RootFile;
    m_isFromModel = other.m_isFromModel;
    if(!other.m_isFromModel){
        m_varID    = BuildVarID();
    }else{
        m_varID = other.m_varID;
    }
    m_varX     = other.m_varX;
    m_varY     = other.m_varY;

    if (m_varID == "ERROR") { MessageSvc::Error("CANNOT LOAD VAR FROM CSV FILE", "", "EXIT_FAILURE"); }
    m_loaded = false;
    if(!other.m_isFromModel){
        Init();
        if (!m_is1D) { m_varID.ReplaceAll("-", "_"); }
    }else{
        m_is1D         = other.m_is1D;
        m_binCuts      = other.m_binCuts;
        m_histo1DModel = other.m_histo1DModel;
        m_histo1D      =  (TH1D*)other.m_histo1D->Clone();
        m_histo1D->SetDirectory(0);
        m_histo1D->Reset("ICES");        
        m_loaded = true;
    }
    // MessageSvc::Debug("VariableBinning (copy)", m_varID +" "+ m_CsvFile,  m_RootFile);
    return;
}

// VariableBinning::VariableBinning(const VariableBinning && other) {
//     m_CsvFile             = other.csvFile();
//     m_RootFile            = other.rootFile();
//     m_varID               = BuildVarID();
//     if (m_varID == "ERROR") { MessageSvc::Error("CANNOT LOAD VAR FROM CSV FILE", "", "EXIT_FAILURE"); }
//     m_loaded = false;
//     Init();
//     if (!m_is1D) { m_varID.ReplaceAll("-", "_"); }
//     MessageSvc::Debug("VariableBinning (copy)", m_varID +" "+ m_CsvFile,  m_RootFile);
//     return;
// }

void VariableBinning::Init() {
    m_varID = BuildVarID();
    LoadCutsFromCSV();
    LoadHistoTemplate();
    m_loaded = true;
    return;
}
void VariableBinning::LoadCutsFromCSV() {
    vector< vector< TString > > _readLines = IOSvc::ParseFile(m_CsvFile, ":");
    for (const auto & el : _readLines) {
        if (el.size() != 2) { MessageSvc::Error("ERROR PARSING format CSV BIN ID : CUT not satisfied on ", m_CsvFile, "EXIT_FAILURE"); }
        m_binCuts.push_back(el[1]);
    }
    return;
}
void VariableBinning::LoadHistoTemplate() {
    // MessageSvc::Debug("LoadHistoTemplate");
    TFile * f = IOSvc::OpenFile(m_RootFile, OpenMode::READ);
    if (f == nullptr) { MessageSvc::Error("File doesn't exists", m_RootFile, "EXIT_FAILURE"); }
    if (f->Get("template") == nullptr) { MessageSvc::Error("template histo doesn't exist in ", m_RootFile, "EXIT_FAILURE"); }
    TString _className = f->Get("template")->ClassName();
    if (_className == "TH1D") {
        // Shrink axis ranges? check last bin width in x, y separately and "chunk it down..." TODO
        auto * h = (TH1D *) f->Get("template");
        if (h == nullptr) { MessageSvc::Error("LoadHistoTemplate nullptr", "", "EXIT_FAILURE"); }
        m_histo1D = static_cast< TH1D * >(h->Clone(m_varID.Data()));
        m_histo1D->SetDirectory(0);
        m_histo1D->Reset("ICES");
        // m_varX         = m_histo1D->GetXaxis()->GetTitle();   // X-NAME OF VARIABLE TO US EIS THE NAME OF THE HISTO x-AXIS!!!!
        m_is1D         = true;
        TString _name  = m_histo1D->GetName();
        TString _title = m_histo1D->GetTitle();
        m_histo1DModel = ROOT::RDF::TH1DModel(*m_histo1D);
    } else if (_className == "TH2Poly") {
        auto * h  = (TH2Poly *) f->Get("template");
        m_histo2D = static_cast< TH2Poly * >(h->Clone(m_varID.Data()));
        m_histo2D->SetDirectory(0);
        m_histo2D->Reset("ICES");
        // m_varX = m_histo2D->GetXaxis()->GetTitle();   // X-NAME OF VARIABLE TO US EIS THE NAME OF THE HISTO x-AXIS!!!!
        // m_varY = m_histo2D->GetYaxis()->GetTitle();   // X-NAME OF VARIABLE TO US EIS THE NAME OF THE HISTO x-AXIS!!!!
        m_is1D = false;
    } else {
        MessageSvc::Error("Histo TYPE not supporteed", "", "EXIT_FAILURE");
    }
    /*
     * Let's shrink the last bin range to be "visible OK"
     */
    // if( m_is1D){
    //     if( AdaptingBounds( m_histo1D)){
    //         m_histo1D =  SquezeBoundaries(m_histo1D);
    //     }
    // }
    f->Close();
    // MessageSvc::Debug("LoadHistoTemplate DONE");
    return;
}

void VariableBinning::FillHisto(float x, float Weight) { m_histo1D->Fill(x, Weight); }
void VariableBinning::FillHisto(float x, float y, float Weight) { m_histo2D->Fill(x, y, Weight); }

void VariableBinning::DrawOnHist(TTree * _tree, TH1 * _histo, TString _weight, TCut _cut) {
    if (_histo == nullptr) { MessageSvc::Error("VariableBinning::DrawOnHist", "Hist is nullptr", "EXIT_FAILURE"); }
    if (_tree == nullptr) { MessageSvc::Error("VariableBinning::DrawOnHist", "TTree is nullptr", "EXIT_FAILURE"); }
    _histo->SetName(TString(_histo->GetName()).ReplaceAll(" ", "_").ReplaceAll("(", "_").ReplaceAll(")", "_"));
    if (m_is1D) {
        TString _drawingString = TString(fmt::format("{0}>>{1}", m_varX, _histo->GetName()));
        TString _exprCut       = TString(fmt::format("{0}*{1}", _weight, TString(_cut)));
        MessageSvc::Info("DrawOnHist", _drawingString + "," + _exprCut);
        _tree->Draw(_drawingString, _exprCut);
        MessageSvc::Info("Histo filled with sumW = ", to_string(_histo->GetSumOfWeights()));
        return;
    } else {
        TString _drawingString = TString(fmt::format("{0}:{1}>>{2}", m_varY, m_varX, _histo->GetName()));
        TString _exprCut       = TString(fmt::format("{0}*{1}", _weight, TString(_cut)));
        MessageSvc::Info("DrawOnHist", _drawingString + "," + _exprCut);
        _tree->Draw(_drawingString, _exprCut);
        MessageSvc::Info("Histo filled with sumW = ", to_string(_histo->GetSumOfWeights()));
        return;
    }
    return;
}

TH1 * VariableBinning::GetHistoClone(TString _newName, TString _title) const {
    MessageSvc::Info("GetHistoClone ", _newName, _title);
    if (!m_loaded) MessageSvc::Error("Cannot get histo if Not Loaded", "", "EXIT_FAILURE");
    if (m_is1D) {
        auto * h1 = (TH1 *) m_histo1D->Clone(_newName);
        h1->SetTitle(_title);
        h1->SetDirectory(0);
        return h1;
    } else {
        auto * h1 = (TH1 *) m_histo2D->Clone(_newName);
        h1->SetTitle(_title);
        h1->SetDirectory(0);
        return h1;
    }
    MessageSvc::Error("Cannot Retur Histo", "", "EXIT_FAILURE");
}

ROOT::RDF::TH1DModel VariableBinning::GetHisto1DModel(TString _newName, TString _title) const {
    if (!m_loaded) MessageSvc::Error("Cannot get histo if Not LOaded", "", "EXIT_FAILURE");
    if (!m_is1D) MessageSvc::Error("GetHisto1DModel cannot be loaded for nD > 1", "", "EXIT_FAILURE");
    auto x_bounds = make_pair(m_histo1D->GetXaxis()->GetXmin(), m_histo1D->GetXaxis()->GetXmax());

    auto titleX = m_histo1D->GetXaxis()->GetTitle();
    auto titleY = m_histo1D->GetYaxis()->GetTitle();

    auto histModel(m_histo1DModel);
    histModel.fName  = _newName;
    histModel.fTitle = _title + ";" + titleX + ";" + titleY;
    return histModel;
}

TH1D * VariableBinning::GetRawHisto1D(TString _newName, TString _title, int nBinsX) const {
    auto x_bounds = make_pair(m_histo1D->GetXaxis()->GetXmin(), m_histo1D->GetXaxis()->GetXmax());
    if (m_is1D) {
        TH1D * h1 = new TH1D(_newName, _title, nBinsX, x_bounds.first, x_bounds.second);
        return h1;
    } else {
        MessageSvc::Error("Invalid GetRawHisto1D", "", "EXIT_FAILURE");
    }
    return nullptr;
}
TH2D * VariableBinning::GetRawHisto2D(TString _newName, TString _title, int nBinsX, int nBinsY) const {
    auto x_bounds = make_pair(m_histo2D->GetXaxis()->GetXmin(), m_histo2D->GetXaxis()->GetXmax());
    auto y_bounds = make_pair(m_histo2D->GetYaxis()->GetXmin(), m_histo2D->GetYaxis()->GetXmax());
    if (!m_is1D) {
        TH2D * h2 = new TH2D(_newName, _title, nBinsX, x_bounds.first, x_bounds.second, nBinsY, y_bounds.first, y_bounds.second);
        return h2;
    } else {
        MessageSvc::Error("Invalid GetRawHisto1D", "", "EXIT_FAILURE");
    }
    return nullptr;
}
ROOT::RDF::TH1DModel VariableBinning::GetRawHisto1DModel(TString _newName, TString _title, int nBinsX) const {
    if (m_is1D) {
        auto x_bounds = make_pair(m_histo1D->GetXaxis()->GetXmin(), m_histo1D->GetXaxis()->GetXmax());
        auto titleX   = m_histo1D->GetXaxis()->GetTitle();
        auto titleY   = m_histo1D->GetYaxis()->GetTitle();
        _title        = _title + ";" + titleX + ";" + titleY;
        ROOT::RDF::TH1DModel model(_newName, _title, nBinsX, x_bounds.first, x_bounds.second);
        return model;
    } else {
        MessageSvc::Error("Invalid GetRawHisto1DModel", "", "EXIT_FAILURE");
    }
    return ROOT::RDF::TH1DModel();
}
/// Bookkeping
//vector< tuple< TString, int > > UnpackIDs_nBins(const vector< tuple< TString, vector< tuple< TString, int , double, double, TString , TString > > > > & _isos);

vector< tuple< TString, int, TString, TString > > UnpackIDs_nBins(const vector< tuple< TString, vector< tuple< TString, int , double, double, TString, TString > > > > & _isos ) {
    vector< tuple< TString, int ,TString, TString > > _IDs_nBins = {};
    for (auto const & el : _isos) {
        int  nBins      = 1;
        auto _vars_info = std::get< 1 >(el);
        auto _varX = std::get<0>( _vars_info.at(0) );        
        auto _varY = _varX ; 
        int i  = 0; 
        for (auto & vv : _vars_info) { 
            nBins = nBins * std::get<1>(vv);
            if( i != 0){
                _varY = std::get<0>( vv);
            }
            i++;
        }
        _IDs_nBins.push_back(make_tuple(std::get< 0 >(el), nBins, _varX, _varY));
    }
    return _IDs_nBins;
}

vector< VariableBinning > GetVariableBinning(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf) {
    const Year                     _toUseYear = hash_year(GetRunFromYear(to_string(_year)));
    vector< tuple< TString, int  , TString, TString > > _isoVars   = UnpackIDs_nBins(SettingDef::Tuple::isoBins );
    vector< VariableBinning >      _vars;
    TString                        _dir = IOSvc::GetFlatnessDir(_prj, _toUseYear, _trigger, _triggerConf);
    // MessageSvc::Info("GetVariableBinning( ", TString(fmt::format("{0} {1} {2} {3}", to_string(_prj), to_string(_toUseYear), to_string(_trigger), to_string(_triggerConf))), "");
    // MessageSvc::Info("GetVariableBinning direct from DIRECTORY: ", _dir, "");
    int i = 0;
    for (auto & var : _isoVars) {
        auto _ID   = std::get< 0>(var);
        TString _varDIr = IOSvc::GetFlatnessDir(_prj, _toUseYear, _trigger, _triggerConf,_ID);
        auto _varX = std::get< 2>(var);
        auto _varY = std::get< 3>(var);
        if( _prj == Prj::RK){    _varX.ReplaceAll("{HEAD}", "Bp"); _varY.ReplaceAll("{HEAD}", "Bp");}
        if( _prj == Prj::RKst){  _varX.ReplaceAll("{HEAD}", "B0"); _varY.ReplaceAll("{HEAD}", "B0");}
        if( _prj == Prj::RPhi){  _varX.ReplaceAll("{HEAD}", "Bs"); _varY.ReplaceAll("{HEAD}", "Bs");}

        _vars.emplace_back( _dir + "/" + _ID + "_IsoBinCuts.csv", _varX, _varY );
        // cout<< "filled \n --------"<<endl;
    }
    return _vars;
}
#endif
