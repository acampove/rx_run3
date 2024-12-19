#ifndef VARIABLEBINNING_HPP
#define VARIABLEBINNING_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "TH1D.h"
#include "TH2Poly.h"
#include "TObjString.h"
#include "TString.h"
#include <ROOT/RDF/HistoModels.hxx>
#include <algorithm>
#include <vector>
class VariableBinning {
  private:
    TString m_CsvFile  = "ERROR";
    TString m_RootFile = "ERROR";
    TString m_varID    = "ERROR";
    TString m_varX     = "ERROR";
    TString m_varY     = "ERROR";
    TString m_labelY   = "ERROR";
    TString m_labelX   = "ERROR";
    bool              m_isFromModel=false;
    vector< TString > m_binCuts = {};
    bool              m_is1D    = false;
    TH1D *            m_histo1D = nullptr;

    ROOT::RDF::TH1DModel m_histo1DModel;

    TH2Poly * m_histo2D = nullptr;

    bool m_loaded = false;
    /**
     * @brief      Internal method to intialize the VariableBinning content [ load histo, and load set of cuts ]
     */
    void Init();
    /**
     * @brief      Extract content from the CSV file
     */
    void LoadCutsFromCSV();
    /**
     * @brief      Load the Histogram Template content
     */
    void LoadHistoTemplate();

  public:
    /**
     * @brief      Constructs the object as a COPY, re-do all job
     *
     * @param[in]  other  The other VariableBinning defs
     */
    VariableBinning(const VariableBinning & other);


    VariableBinning(ROOT::RDF::TH1DModel & model1D, TString _varX);

    /**
     * @brief      Constructs the object from a csv file to read
     *
     * @param[in]  _csv  The csv
     */
    // VariableBinning(const VariableBinning && other);
    VariableBinning(TString _csv, TString _varX, TString _varY);

    VariableBinning() = default;
    /**
     * @brief      The unique ID of the variable, used for bookkeping
     *
     * @return
     */
    TString BuildVarID() const {
        auto   _varID         = m_CsvFile;
        auto * _strCollection = ((TString) m_CsvFile).Tokenize("/");
        // _strCollection->Print();
        _varID = TString(((TObjString *) (*_strCollection).Last())->String());
        // cout<<RED<<"curr "    << m_varID<< endl;
        _varID.ReplaceAll("_IsoBinCuts.csv", "");
        return _varID;
        delete _strCollection;
    }
    TString varID() const { return m_varID; };

    bool                    is1D() const { return m_is1D; }
    int                     nBins() const { return m_binCuts.size(); }
    int                     nCuts() const { return nBins(); }
    const vector< TString > Cuts() const { return m_binCuts; }
    const TString           csvFile() const { return m_CsvFile; }
    const TString           rootFile() const { return m_RootFile; }
    const TString           varX() const { return m_varX; };
    const TString           varY() const { return m_varY; };
    TString                 cut(int i) const {
        if (i > m_binCuts.size()) { MessageSvc::Error("Out of range ,asking for ", to_string(i) + " but nBins = " + to_string(nBins()), "EXIT_FAILURE"); }
        return m_binCuts.at(i);
    }
    pair< TString, TString > Label_X() const { return make_pair(m_varID + "_x", m_varX); }
    pair< TString, TString > Label_Y() const {
        if (m_is1D) { MessageSvc::Error("VariableBinning is not 1D"); }
        return make_pair(m_varID + "_y", m_varY);
    }

    // void FillHisto( const vector<double> x, const vector<double> weight )
    void FillHisto(float x, float Weight = 1.);
    void FillHisto(float x, float y, float Weight = 1.);
    // TH1* GetHisto() ;
    TH1 * GetHistoClone(TString _newName, TString _newTitle) const;

    ROOT::RDF::TH1DModel GetHisto1DModel(TString _newName, TString _newTitle) const;
    ROOT::RDF::TH1DModel GetRawHisto1DModel(TString _newName, TString _newTitle, int nBinsX) const;

    TH1D * GetRawHisto1D(TString _newName, TString _newTitle, int nBinsX) const;
    TH2D * GetRawHisto2D(TString _newName, TString _newTitle, int nBinsX, int nBinsY) const;
    void   DrawOnHist(TTree * _tree, TH1 * _histo, TString _weight = "(1.)", TCut _cut = "(1)");
    /**
     * Print it
     */
    void Print() const{
        MessageSvc::Line();
        MessageSvc::Info("csvFile", m_CsvFile);
        MessageSvc::Info("rootFile", m_RootFile);
        MessageSvc::Info("varID", m_varID);
        MessageSvc::Info("varX", m_varX);
        MessageSvc::Info("varY", m_varX);
        MessageSvc::Info("is1D", to_string(m_is1D));
        MessageSvc::Info("isLoaded", to_string(m_loaded));
        MessageSvc::Info("hasHisto1D", to_string(m_histo1D != nullptr));
        MessageSvc::Info("hasHisto2D", to_string(m_histo2D != nullptr));
        return;
    }
};

//============= Bookkeping  VariableBinning for Flatness ===================//

/**
 * @brief      Unpack  the BinID , nBins
 *
 * vector< tuple< TString, vector< tuple< TString, int , double, double, TString > > > > SettingDef::Tuple::isoBins = {};
 *
 * @return     unpacked vector<tuple>  parsed in yaml files.. ( ID, nBins, varXNameTuple, varYNameTuple)
 */
vector< tuple< TString, int ,TString, TString > > UnpackIDs_nBins(const vector< tuple< TString, vector< tuple< TString, int , double, double, TString , TString > > > > & _isos );
/**
 * @brief      Load the vector of VariableBinning objects following bookkeping schemes
 *
 * @param[in]  _prj          The project
 * @param[in]  _year         The year
 * @param[in]  _trigger      The trigger
 * @param[in]  _triggerConf  The trigger conf
 *
 * @return     The variable binning.
 */
vector< VariableBinning > GetVariableBinning(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf);

#endif
