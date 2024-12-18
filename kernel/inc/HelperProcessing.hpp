#ifndef HELPERPROCESSING_HPP
#define HELPERPROCESSING_HPP
#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "HistoAdders.hpp"
#include "TLorentzVector.h"
#include "THn.h"
using ROOT::RDF::RNode;

namespace TruePort{ 
    //q2True from MCDT via RunNb,EvtNb attached on tuples to smear around 
    const TString JPs_TRUEM_PREFSR                   = "JPs_TRUEM_MCDT";         //given by ( B - Hadron system )
    const TString JPs_TRUEM_POSTFSR                  = "JPs_TRUEM_POSTFSR_MCDT"; //given by ( E1 + E2 ) system 
    // const TString JPs_TRUEM_PREFSR_OVERRIDE       = "JPs_TRUEM_MCDT_REPLACED";         //given by ( B - Hadron system )
    // const TString JPs_TRUEM_POSTFSR_OVERRIDE      = "JPs_TRUEM_POSTFSR_MCDT_REPLACED"; //given by ( E1 + E2 ) system 

    const TString B_TRUEM_PREFSR_PORT   = "B_TRUEM_PORT"; //Given by TRUEB( KINEMATIC)
    const TString B_TRUEM_POSTFSR_PORT  = "B_TRUEM_POSTFSR_PORT"; //Sum final states
    const TString B_TRUEM_PREFSR_PR_PORT= "B_TRUEM_PR_PORT"; //Given by TRUEB( KINEMATIC - MISSING PARTICLE KINEMATIC )
}

//Used to perform the porting of a single branch ( double via RunNb, EvtNb )
struct BranchPort{
    BranchPort() = default;
    BranchPort( UInt_t rNb, ULong64_t eNb,  double weight){
        runNumber = rNb;
        eventNumber =eNb;
        wVal = weight;
    };
    void Print()const{cout<<"(rNb,eNb,weight) : " << "( "<< runNumber <<" , " << eventNumber <<" , " << wVal << " ) "<< endl;return;}
    //Accessor ID unique
    pair<UInt_t, ULong64_t> KeyID() const{return make_pair(runNumber, eventNumber);}
    //Accessor weight value
    double BranchVal()const{return wVal;}
    UInt_t runNumber;
    ULong64_t eventNumber;
    double wVal = numeric_limits<double>::min();  
};

//used in 2012
struct TrackHLT1Info{
    double p;
    double pt;
    bool   hlt1_tos;
    TrackHLT1Info() = default;
    TrackHLT1Info( double _p, double _pt, bool _hlt1_tos){
        p  = _p;
        pt = _pt;
        hlt1_tos = _hlt1_tos;
    };
};
//used in 2016
struct TrackMVAHLT1Info{
    double pt; 
    double ipchi2;
    bool   hlt1tos;
    TrackMVAHLT1Info() = default;
    TrackMVAHLT1Info( double _pt, double _ipchi2, bool _isTOS){
        pt = _pt;
        ipchi2 = _ipchi2;
        hlt1tos = _isTOS;
    };
    const double PT() const{ return pt;}
    const double PTGEV() const{ return 0.001*pt;}
    const double IPCHI2() const{ return ipchi2;}
    const double LOGIPCHI2() const { return TMath::Log(ipchi2) ;}
    const bool   TOS() const { return hlt1tos;}
    const bool   PASS2DCUT(const double & b_parameter) const{ 
        bool _2dCut      =  ( LOGIPCHI2() > ( 1.0 / TMath::Sq(PTGEV() -1.0) +  (b_parameter/ 25000.0) * ( 25000.0 - PT() ) + TMath::Log(7.4)  ) );
        return _2dCut;
    };
};

class HelperProcessing{ 
	public : 
		HelperProcessing() = default;
		/*
    	Perform rootmv with macro ( rootmv is broken ) 
    	( TTree to Move from OLDFILENAME , to NEWFILENAME)
		*/
		static void moveTupleFromTo( TString _treeName, TString _oldFileName, TString _newFileName);
		/*
	    Perform HLT1TrackAllL0DecisionTOS alignment on 2012 MC sample rewriting the TOS decision boolean flag tightening up the final state particles P/PT cuts
	    TODO : generalize it for more Projects ( RPhi for example)	    
		*/
		static int AlignHLT1TrackAllL0DecisionTOS12MC( const Analysis & ana, const Prj & prj);
		/*
	    Perform HLT1TrackAllL0DecisionTOS alignment on 2012 MC sample rewriting the TOS decision boolean flag tightening up the final state particles P/PT cuts
	    TODO : generalize it for more Projects ( RPhi for example)
		*/		
		static int AlignHLT1TrackMVATOS16MC( const Analysis & ana, const Prj & prj );
		/*
		 * Return a vector of strings dropping black listed columns.
		*/
		static std::vector<string> DropColumns( ROOT::Detail::RDF::ColumnNames_t && current_columns, ROOT::Detail::RDF::ColumnNames_t && current_defined_columns, const vector< std::string> & blacklist);

		/*
		 * Return a vector of strings dropping columns that have <wildcard> in their name.
		*/
        static std::vector<string> DropColumnsWildcard( ROOT::Detail::RDF::ColumnNames_t && current_columns, const vector<std::string> & wildcard);


        static RNode AttachWeights( RNode df, ConfigHolder & theConfigHolder, TString _weightOption );
        static RNode AttachWeights( RNode df, const ConfigHolder & theConfigHolder, TString _weightOption );        

        /*
            Add HLT weights 
        */
        static void AddHLTWeights( EventType & et, bool _useET = false);
        static RNode AppendHLTColumns( RNode df, ConfigHolder & theConfigHolder, TString _weightOption ); 

        /*
            Add Poisson weights for bootstrapping
        */
        static RNode AppendBSColumns( RNode df ) ;      

        /*
            Add L0 weights  ( L0I, L0L-Comb, L0L ) for EE/MM . Bootstrapped maps added if SettingDef::Weight::useBS = true;
        */
        static void AddL0Weights( EventType & et  , bool _useET = false);
        static RNode AppendL0Columns( RNode df, ConfigHolder & theConfigHolder , TString _weightOption );        

        /*
            Add PID weights for EE/MM
        */
        static void AddPIDWeights( EventType & et  , bool _useET = false);
        /*
            Add PID weights on the fly passing the ConfigHolder only , must pass a wildcards tag to drop columns in snapshotting
        */
        static RNode AppendPIDColumns( RNode df, ConfigHolder & theConfigHolder , vector<string> & _wildcards );        
        /*
            Add TRK weights (for tuple Process and snapshots)
        */
        static void AddTRKWeights( EventType & et , bool _useET = false );
       /*
            Add TRK weights on the fly passing the ConfigHolder only
            It is needed to identify the "year" of the smearing parameters to use. 
        */
        static RNode AppendTRKColumns( RNode df, ConfigHolder & _configHolder );     
       /*
            Add RW1D weights on the fly passing the ConfigHolder only
        */
        static RNode AppendRW1DColumns( RNode df, ConfigHolder & theConfigHolder, TString _weightOption ); 

        /*
            Add TrueQ2 from MCDT values and dump ntuple from EventType
        */
        static void PortTrueQ2( EventType & et , bool _useET = false );
        /*
        Add Q2SMearing values on the fly and return a RDataFrame node with the branch added as columns, pass in the input node, the year [ must be 11,12,15,16,17 or 18,
        not supporing q2 smearing merged computation yet] and the Project used for the NODE used [ decide whether Bp,B0,Bs are the label of branches for BKGCAT]
        */
        static RNode AppendQ2SmearColumns( RNode df, const Prj & _prj , const Year & _year );
       /*
        Add BSmearing values on the fly and return a RDataFrame node with the branch added as columns, pass in the input node, the year [ must be 11,12,15,16,17 or 18,
        not supporing q2 smearing merged computation yet] and the Project used for the NODE used [ decide whether Bp,B0,Bs are the label of branches for BKGCAT]
        */        
        static RNode AppendBSmearColumns(  RNode df, const Prj & _prj, const Year & _year);

        /*
            Take the Kst_M_TRUE values from MCDecayTuple and bring them in to DecayTuple ( -1 as non matching Rn,Enb matching)
            Allow to cut on Kst_Narrow window for RK processed samples, Requires MCDecayTuple to have been produced!            
        */
        static void PortTrueKstMass( EventType & et);

        /*
            Take the Kst_M_TRUE values from MCDecayTuple and bring them in to DecayTuple ( -1 as non matching Rn,Enb matching)
            Allow to cut on Kst_Narrow window for RK processed samples, Requires MCDecayTuple to have been produced!            
        */
        static void PortTrueBMass( EventType & et);

        /*
	    Merge the SS Data tuples from Leptons and Hadrons
        */
        static void MergeSSData();

        /*
             Use the TH1DHistoAdder functor to attach branches to ntuples ( input var = inVar1D )
        */
        static RNode ApplyWeights1D(RNode df, const vector<TH1DHistoAdder> &hisotgrams , TString inVar1D ,unsigned int i = 0);

        /*
             Use the BSTH1DHistoAdder functor to attach branches vector to ntuples ( input var = inVar1D )
        */
        static RNode ApplyBSWeights1D(RNode df, const vector<BSTH1DHistoAdder> &hisotgrams , TString inVar1D , unsigned int i=0);

        /*
             Use the TH2DHistAdder functors list to attach branches to ntuples ( input varX = inVar_X, input varY = inVar_Y )
        */      
        static RNode ApplyWeights2D(RNode df, const vector<TH2DHistAdder> &hisotgrams , TString inVar_X , TString inVar_Y, unsigned int i=0);

        /*
             Use the TH2DHistAdder functors list to attach branches to ntuples ( input varX = inVar_X, input varY = inVar_Y )
        */      
        static RNode ApplyWeights2D_3Args(RNode df, const vector<TH2DHistAdderL0EBremSplit> &hisotgrams , TString inVar_X , TString inVar_Y, TString catVar, unsigned int i=0);

        /*
             Use the BSTH2DHistAdder functors list to attach branches to ntuples ( input varX = inVar_X, input varY = inVar_Y ), made out of vector<TH2D> for bootstrapping
        */      
        static RNode ApplyBSWeights2D(RNode df, const vector<BSTH2DHistAdder> &hisotgrams , TString inVar_X , TString inVar_Y, unsigned int i=0);

        /*
            Append to signal MC sample decy samples the Dec model weights! 
        */
        static RNode AppendDecModelWeights( RNode df);
};


#endif
