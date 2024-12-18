#include "Functors.hpp"

 
double Functors::getmass(const double true_px, const double true_py,const  double true_pz ,const  double true_pe){
    TLorentzVector v;
    v.SetPxPyPzE( true_px , true_py, true_pz, true_pe);
    return v.Mag();
};
double Functors::getmass_1Body( const TLorentzVector & v1){
    return v1.Mag();
};       

//make the LV from x,y,z,e (P)
TLorentzVector Functors::make_lorentz( const double px, const double py, const double pz, const double pe){
    TLorentzVector v; 
    v.SetPxPyPzE( px,py,pz,pe);
    return v; 
};   
//make the LV from P1+P2
TLorentzVector Functors::combine_2Body( const TLorentzVector & v1, const TLorentzVector & v2){
    TLorentzVector v = v1+ v2; 
    return v; 
};    
double Functors::getmass_2Body( const TLorentzVector & v1, const TLorentzVector & v2){
    auto vv = v1+ v2; 
    return vv.Mag();
};       

//make the LV from P1+P2+P3
TLorentzVector Functors::combine_3Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3){
    TLorentzVector v = v1+ v2 +v3; 
    return v; 
};      
double Functors::getmass_3Body( const TLorentzVector & H, const TLorentzVector & L1, const TLorentzVector & L2){
    TLorentzVector Composite3Body = H + L1 + L2; 
    return Composite3Body.Mag();
};

//make the LV from P1+P2+P3+P4
TLorentzVector Functors::combine_4Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3 , const TLorentzVector & v4){
    TLorentzVector v = v1+ v2 +v3 + v4; 
    return v; 
};
double Functors::getmass_4Body( const TLorentzVector & H1, const TLorentzVector & H2, const TLorentzVector & L1, const TLorentzVector & L2){
    TLorentzVector Composite4Body = H1 + H2 + L1 + L2; 
    return Composite4Body.Mag();
};
//make the LV from P1+P2+P3+P4+P5
TLorentzVector Functors::combine_5Body( const TLorentzVector & v1, const TLorentzVector & v2, const TLorentzVector & v3 , const TLorentzVector & v4, TLorentzVector & v5){
    TLorentzVector v = v1+ v2 +v3 + v4 + v5; 
    return v; 
};
double Functors::getmass_5Body( const TLorentzVector & H1, const TLorentzVector & H2, const TLorentzVector & H3, const TLorentzVector & L1, const TLorentzVector & L2){
    TLorentzVector Composite4Body = H1 + H2 + H3 + L1 + L2; 
    return Composite4Body.Mag();
};
//make the LV from P1+P2+P3+P4+P5
BranchPort Functors::ConstructIt( const  UInt_t  & rNb , const ULong64_t & eNb, const double  & Branch){
    return BranchPort( rNb, eNb, Branch);    
};
// compute M( HEAD - HADRON - MISSING PARTRECO on MCDT) 
double Functors::getmass_HEAD_MINUS_H1_MISS( const TLorentzVector & HEAD, const TLorentzVector & HADRON1, const TLorentzVector & MISSING ){
    TLorentzVector LL_PREFSR = HEAD - HADRON1 - MISSING; 
    return LL_PREFSR.Mag();
};
// compute M( HEAD - HADRON - HADRON - MISSING PARTRECO on MCDT) [RKst, RPhi] 
double Functors::getmass_HEAD_MINUS_H1_H2_MISS( const TLorentzVector & HEAD, const TLorentzVector & HADRON1, const TLorentzVector & HADRON2, const TLorentzVector & MISSING ){
    TLorentzVector LL_PREFSR = HEAD - HADRON1 - MISSING; 
    return LL_PREFSR.Mag();
};
   


double Functors::getMassDelta( const TLorentzVector & HEAD, const TLorentzVector & SUBTRACT){
    TLorentzVector Composite = HEAD- SUBTRACT;
    return Composite.Mag();
}



