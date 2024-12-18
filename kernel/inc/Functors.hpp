#ifndef FUNCTORS_HPP
#define FUNCTORS_HPP
#include "TLorentzVector.h"
#include "HelperProcessing.hpp"

struct AngularInfos{
    AngularInfos() = default;
    AngularInfos( bool bzero, const TLorentzVector& mu1, const TLorentzVector& mu2, const TLorentzVector& kaon, const TLorentzVector& pion){
      //Set up boost vectors 
      //THis works given the RX tuple definition for L1, L2 setup
      TLorentzVector muplus =  bzero?mu1:mu2; 
      TLorentzVector muminus = bzero?mu2:mu1;
      TLorentzVector b = muplus + muminus + kaon + pion;
      TLorentzVector mumu = muplus + muminus;
      TLorentzVector kpi = kaon + pion;
      TVector3 mumuboost(-mumu.BoostVector());
      TVector3 kpiboost(-kpi.BoostVector());
      TVector3 bboost(-b.BoostVector());
      //determine costhetal
      TLorentzVector muminusd(muminus);
      muminusd.Boost(mumuboost);
      TLorentzVector muplusd(muplus);
      muplusd.Boost(mumuboost);
      TLorentzVector bd(b);
      bd.Boost(mumuboost);
      if (bzero) m_ctl = cos(muplusd.Vect().Angle(-bd.Vect()));
      else m_ctl = cos(muminusd.Vect().Angle(-bd.Vect()));     
      //determine costhetak
      TLorentzVector kaondd(kaon);
      kaondd.Boost(kpiboost);
      TLorentzVector bdd(b);
      bdd.Boost(kpiboost);
      m_ctk = cos(kaondd.Vect().Angle(-bdd.Vect()));     
      //determine phi
      TLorentzVector kaonddd(kaon);
      kaonddd.Boost(bboost);
      TLorentzVector pionddd(pion);
      pionddd.Boost(bboost);
      TLorentzVector muminusddd(muminus);
      muminusddd.Boost(bboost);
      TLorentzVector muplusddd(muplus);
      muplusddd.Boost(bboost);
      TVector3 normalkpi = kaonddd.Vect().Cross(pionddd.Vect());
      TVector3 normalmumu = muplusddd.Vect().Cross(muminusddd.Vect());  
      TLorentzVector kpiddd(kpi);
      kpiddd.Boost(bboost);  
      //costheta = norm.norm, all in b frame        
      if (bzero){
        m_phi = normalkpi.Angle(normalmumu);  
        //sintheta = [norm cross norm ]dot kst norm all in b frame, if sin theta less than one, in last two quadrants - flip        
        if ((normalmumu.Cross(normalkpi)).Dot(kpiddd.Vect()) < 0.0) m_phi = -m_phi;
      }else{
        m_phi = normalkpi.Angle(-normalmumu);
        if ((normalmumu.Cross(normalkpi)).Dot(kpiddd.Vect()) < 0.0) m_phi = -m_phi;        
      }
      return;
    }
    double CosThetaL() const{ return m_ctl;}
    double CosThetaK() const{ return m_ctk;}
    double Phi() const{ return m_phi;}
    //Private members
    double m_ctl;
    double m_ctk;
    double m_phi;     
};

// A collection of functions useful for useful RDataFrame operation(s)
class Functors{
    Functors() = default;
    //mass of a particle passing px,py,pz,pe
    public : 
    static double getmass(const double true_px, const double true_py,const  double true_pz ,const  double true_pe);
    //make the LV from x,y,z,e (P)
    static double getmass_1Body( const TLorentzVector & v1);


    static TLorentzVector make_lorentz( const double px, const double py, const double pz, const double pe);
    //make the LV from P1+P2
    static TLorentzVector combine_2Body( const TLorentzVector & v1, const TLorentzVector & v2);
    static double getmass_2Body( const TLorentzVector & v1, const TLorentzVector & v2);
    //make the LV from P1+P2+P3
    static TLorentzVector combine_3Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3);
    static double getmass_3Body( const TLorentzVector & H, const TLorentzVector & L1, const TLorentzVector & L2);
    //make the LV from P1+P2+P3+P4
    static TLorentzVector combine_4Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3 , const TLorentzVector & v4);
    static double getmass_4Body( const TLorentzVector & H1, const TLorentzVector & H2, const TLorentzVector & L1, const TLorentzVector & L2);
    //make the LV from P1+P2+P3+P4+P5
    static TLorentzVector combine_5Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3 , const TLorentzVector & v4, TLorentzVector & v5);
    static double getmass_5Body( const TLorentzVector & H1, const TLorentzVector & H2, const TLorentzVector & H3, const TLorentzVector & L1, const TLorentzVector & L2);

    static BranchPort ConstructIt( const  UInt_t  & rNb , const ULong64_t & eNb, const double  & Branch);
    // compute M( HEAD - HADRON - MISSING PARTRECO on MCDT) 
    static double getmass_HEAD_MINUS_H1_MISS( const TLorentzVector & HEAD, const TLorentzVector & HADRON1, const TLorentzVector & MISSING );
    // compute M( HEAD - HADRON - HADRON - MISSING PARTRECO on MCDT) [RKst, RPhi] 
    static double getmass_HEAD_MINUS_H1_H2_MISS( const TLorentzVector & HEAD, const TLorentzVector & HADRON1, const TLorentzVector & HADRON2, const TLorentzVector & MISSING );
    
    static double getMassDelta( const TLorentzVector & HEAD, const TLorentzVector & SUBTRACT);

};

#endif
