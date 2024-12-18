#ifndef XJPsRescaler_HPP
#define XJPsRescaler_HPP

#include "ConstDef.hpp"
#include "EnumeratorSvc.hpp"
class IdChains{
    public : 
    IdChains() = default;
    IdChains(  int finalStateParticle, int motherID, int gMotherID, int ggMotherID){
        m_TID          = abs(finalStateParticle); 
        m_MOTHER_TID   = abs(motherID);
        m_GMOTHER_TID  = abs(gMotherID); 
        m_GGMOTHER_TID = abs(ggMotherID);
    };
    IdChains( const IdChains & other){
        m_TID = other.ID(); 
        m_MOTHER_TID = other.MID();
        m_GMOTHER_TID = other.GMID();
        m_GGMOTHER_TID=other.GGMID();  
    };
    public : 
        int ID()const{ return m_TID;}
        int MID()const{ return m_MOTHER_TID;}
        int GMID()const{ return m_GMOTHER_TID;}
        int GGMID()const{ return m_GGMOTHER_TID;}
        bool MatchDecay( std::vector<int> && _myChain) const {
            if( _myChain.size() ==1){
                return m_TID == _myChain[0];
            }
            if( _myChain.size() ==2){
                return  m_TID == abs(_myChain[0]) && m_MOTHER_TID == abs(_myChain[1]);                
            }
            if( _myChain.size() ==3){
                return  m_TID == abs(_myChain[0]) && m_MOTHER_TID == abs(_myChain[1])  && m_GMOTHER_TID == abs(_myChain[2]);                 
            }
            if( _myChain.size() ==4){
                return  m_TID == abs(_myChain[0]) && m_MOTHER_TID == abs(_myChain[1])  && m_GMOTHER_TID == abs(_myChain[2]) && m_GGMOTHER_TID == abs(_myChain[3]);                 
            }
            return false; 
        }
        bool HasInChain( int ID) const{
            bool _return = false; 
            if(  m_MOTHER_TID == ID) _return = true;
            if(  m_GMOTHER_TID == ID) _return = true;
            if(  m_GGMOTHER_TID == ID) _return = true;
            return _return;
        }
        bool MatchUpstream( int IDFirstDau, int HeadPart) const {
            bool _return = false; 
            //B -> ID
            if(  m_MOTHER_TID == IDFirstDau && m_GMOTHER_TID == HeadPart) _return = true;
            if(  m_GMOTHER_TID == IDFirstDau && m_GGMOTHER_TID == HeadPart) _return = true;
            if(  m_GGMOTHER_TID == IDFirstDau) _return = true; //not clear if it would work; ( assumes B-> X ( -> Y -> K -> Particle)), only 3 nested 
            //example : 
            // B -> Psi -> Chi -> J/Psi -> e , can navigate up to GGMother for example. 
            return _return;
        }
        bool MatchID( int iD)const {
            return m_TID == abs(iD);
        }    
        bool MatchMother( int iD)const {
            return m_MOTHER_TID == abs(iD);
        }
        bool MatchGMother( int iD)const {
            return m_GMOTHER_TID == abs(iD);
        }
        bool MatchGGMother( int iD)const {
            return m_GGMOTHER_TID == abs(iD);
        }    
    private:
        int m_TID; 
        int m_MOTHER_TID;
        int m_GMOTHER_TID;
        int m_GGMOTHER_TID;        
};


class RescalerRXSamples{     
    public : 
        RescalerRXSamples() = default;
        RescalerRXSamples(const Prj & _prj){ m_project = _prj ;};
        double operator()(const IdChains & e1, const  IdChains & e2, const IdChains & K, const IdChains & pi);
    private :
        Prj m_project; 
};




#endif // !XJPsRescaler
