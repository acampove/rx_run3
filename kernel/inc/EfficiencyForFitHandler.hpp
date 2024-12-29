#pragma once

#include "EnumeratorSvc.hpp"

namespace YAML{
    class Node;
    class Emitter;
}

class InfoEff{
    private : 
    TString m_wOption     = "NULL"; 
    TString m_wConfig     = "NULL"; 
    TString m_effVersion  = "NULL";
    TString m_effVariable = "NULL";
    public : 
    InfoEff()= default;
    /**
     * @brief Construct a new Info Eff object
     * 
     * @param YAML::Node  (node to parse)
     */
    InfoEff( const YAML::Node & inNode );
    /**
     * @brief Construct a new Info Eff object
     * 
     * @param _wOpt  (wOption)
     * @param _wConf (wConfig)
     * @param _eVer  (efficiencyVersion)
     * @param _eVar  (efficiencyVariable)
     */
    InfoEff( const TString & _wOpt, const TString & _wConf, const TString & _eVer, const TString & _eVar);
    /**
     * @brief Construct a new Info Eff object (Copy constructur)
     * 
     * @param other 
     */
    InfoEff( const InfoEff & other);

    /**
     * @brief Emitter on a yaml emitter, assumes Map and end Map being created already
     * 
     * @param _emitter 
     */

    /**
     * @brief Accessors to Private members
     * 
     * @return TString 
     */
    TString wOption() const;
    TString effVersion() const;
    TString wConfig() const;
    TString effVariable() const;
    TString covTuple() const;
    TString TreeName() const;
    void EmitToYaml(  YAML::Emitter  & _emitter) const;
    void Print() const;

    void UpdateWeightOption( TString _weightOption);
    void UpdateWeightConfig( TString _weightConfig);


    // ClassDef(InfoEff, 1);
};

class EfficiencyForFitHandler{   
    public :  
    /**
     * @brief Internal Struct handling switches of each Efficiency Slot configurable
     * 
     */
 
    public : 
        EfficiencyForFitHandler()= default;
        /**
         * @brief Construct a new Efficiency For Fit Handler object from a Yaml node
         * 
         * @param _EffForFitConfiguration 
         */
        EfficiencyForFitHandler(  const YAML::Node & _EffForFitConfiguration);// , const YAML::Node & FitYamNode);
        /**
         * @brief PrintOut content
         * 
         */
        void Print() const;
        /**
         * @brief Slot identifier 
         * 
         * @param _prj 
         * @param _q2bin 
         * @return TString 
         */
        TString slot( const Prj & _prj, const Q2Bin & _q2bin);
        /**
         * @brief Get the Efficiency Info object to use to extract efficiencies from disk
         * 
         * @param _prj   (the project)
         * @param _q2bin (the q2bin)
         * @param type   (the type = BKGOVERSIGNAL, RRATIO)
        */        
        InfoEff GetEfficiencyInfo( const Prj & _prj, const Q2Bin & _q2bin, const TString & type = "BKGOVERSIGNAL");
        /**
         * @brief Emit to a yaml file 
         * 
         * @param _emitter 
         */
        void EmitToYaml(  YAML::Emitter  & _emitter) const ;

        TString CovTuple() const;//we will return the info about from where to load the TTree in the ntuple which contains the raw info to fit the R-Ratios
        

        void UpdateSigEfficiencyWeightConf( TString _weightConfig);
        void UpdateSigEfficiencyWeightOption( TString _weightOption);

        bool HasEpsSignalSlot( Prj _prj , Q2Bin _q2Bin);
        
        private : 
        map< pair< Prj, Q2Bin> , InfoEff > m_bkg_constraints; //for bkg/signal ratio 
        map< pair< Prj, Q2Bin> , InfoEff > m_sig_efficiencies;//for rRatio final results
        TString m_covTuple;

        
        //TODO : add covariance matrices loader
        // ClassDef(EfficiencyForFitHandler, 1);

};


EfficiencyForFitHandler GetEffContainer( const YAML::Node & _node);

EfficiencyForFitHandler GetEffContainer( const TString & yaml_ConfigFile);
