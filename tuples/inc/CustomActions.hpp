#ifndef CUSTOMACTIONS_HPP
#define CUSTOMACTIONS_HPP

#include <ROOT/RDataFrame.hxx>
#include <iostream>
#include <array>
#include <string>
#include <algorithm>
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
using namespace ROOT::VecOps;


/**
 * Simple helper class to call df.Define( "column", VecDoubleAdder( MYVECTOR), {"_rdfentry"}) 
 * I.e flatten the vector column into the TTree, used for BS results dumping.
 * Sperseeded by using the suggestes solution in https://root-forum.cern.ch/t/sum-for-each-rvec-column-returning-an-rvec/42259/9 --> use Reduce operation with VecSum calls.
 * df.Reduce(VecSum, {"vec_column"}, RVec<double>(100, 0)); //initialization of it?
*/
class VecDoubleAdder {
public:
    VecDoubleAdder( RVec<double> & h , 
                    TString _finalBranchName
                     ) : m_column(h) , m_finalBranchName(_finalBranchName){}
    VecDoubleAdder( const VecDoubleAdder&& other) : m_column(other.m_column), m_finalBranchName(other.m_finalBranchName){}
    VecDoubleAdder( const VecDoubleAdder& other)  : m_column(other.m_column), m_finalBranchName(other.m_finalBranchName){}
    double operator()( ULong64_t entry) { 
        return m_column.at(entry);
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
private:
    RVec<double>   m_column;
    TString m_finalBranchName;
};




/**
 * Allow easy sum of vector columsn with  
 * df.Reduce(VecSum, {"vec-column"}, RVec<double>(100, 0));
*/
RVec<double> VecSum(const RVec<double> &v1, const RVec<double> &v2) {
  return v1 + v2;
}


/**
 * T is the type of the Vector Column , N is the expected length of each vector column : in RX case 100 Bootstrapped values 
 * 
 * Example of usage 
 * auto sumWCols = SumVecCol< RVec<double>, 100>();            
 * auto sumWCol =  node.Book<RVec<double>>(std::move(sumWCols), {"val1"} ); //< this returns an RVec<double> object!
*/
template<typename T , int N  >
class SumVecCol : public ROOT::Detail::RDF::RActionImpl<SumVecCol<T,N>> {
    public  : 
    const int NBSLOTS = N;
    using Result_t = T;   
    bool m_debug  = true;
    TString m_name; 
    std::shared_ptr<Result_t> fResultSum;
    std::vector< Result_t > fSumsPerSlot;

    SumVecCol(TString _name){
        m_name = _name;
        // if(m_debug){ 
        std::cout<< "DEPRECATED, use Reduce with VecSum call!!!, Create with name "<< _name<<std::endl;
        // }
        const auto nSlots = ROOT::IsImplicitMTEnabled() ?  ROOT::GetThreadPoolSize() : 1;
        // std::cout<<"# Create Custom Action SumVecCol(Init) : use nSLOTS "<< nSlots << " , " << m_name<< std::endl;
        for (auto slot : ROOT::TSeqU(nSlots)){
            T slotSum; 
            slotSum.resize(N);
            for( int j = 0; j < N; ++j){
                slotSum[j] = 0; 
            }
            fSumsPerSlot.push_back( slotSum);
            (void)slot;
        }
        // std::cout<<"- SumVecSlots size() = "<< fSumsPerSlot.size() << "   with name " << m_name<< std::endl;
        // for( int i = 0; i < nSlots ; ++i){
        //     std::cout<<" - SumVecSlots["<<i<<"] size = "<< fSumsPerSlot[i].size() << std::endl;
        // }
        //Initialize for safety the final results to start from 0. the weight counting 
        T myVec(N); 
        for( int i = 0; i < N; ++i){
            myVec[i] = 0.;
        }
        fResultSum = std::make_shared<T>( myVec ); //should be enough to avoid going out of scope?
        // std::cout<<"- SumResults size() = "<< (*fResultSum).size()<< std::endl;    
    }
    SumVecCol( SumVecCol &&)= default;  
    SumVecCol( const SumVecCol &) = delete;  
    void Exec(unsigned int slot, const T &vs){     
        // if(m_debug){ 
        //     std::cout<< "Exec slot "<< slot<<std::endl;
        // }         
        // for( int i = 0 ; i < vs.size(); ++i){
        fSumsPerSlot[slot] += vs; //should be using RVec<double> sum operation?
        // }            
    }
    void Initialize() {
        //No init! Maybe share_ptr making has to go in constructor?
    }
    void Finalize(){
        // std::cout<<"# Booked Custom Action SumVecCol(Finalize)" << std::endl;
        for( auto & m : fSumsPerSlot){
            // std::cout<<"Result+=SlotResult"<<std::endl;
            *fResultSum += m;
        }      
    }
    std::shared_ptr<T> GetResultPtr() const { 
        //  std::cout<<"ResultPtr"<<std::endl;
        return  fResultSum;
    }
    void InitTask(TTreeReader *, unsigned int) {}

    std::string GetActionName(){
        return "SumVecCol";
    }   
};


/**
 * T is the type of the Scalar Column used
 * Example of usage with RDataFrame
 * auto covi = Covariance<double>();            
 * auto covXY = df.Book<double,double>(std::move(covi), {"xColumn", "yColumn"} ) );
*/
template< typename T> 
class Covariance : public ROOT::Detail::RDF::RActionImpl<Covariance<T>>{ 
    public : 
        using Covariance_t = T;    
        using Result_t = Covariance_t;
    private : 
        std::vector<Covariance_t>  _xyproductSUM;  //one per data processing slot
        std::vector<Covariance_t>  _xStatsSUM;     //one per data processing slot
        std::vector<Covariance_t>  _yStatsSUM;     //one per data processing slot
        std::vector<int> _nEntries;                //one per data processing slot
        std::shared_ptr<Covariance_t> _covariance;        
    public : 
        Covariance( ){
            const auto nSlots = ROOT::IsImplicitMTEnabled() ?  ROOT::GetThreadPoolSize() : 1;
            for (auto i : ROOT::TSeqU(nSlots)){
                 _xyproductSUM.emplace_back(0.);
                 _xStatsSUM.emplace_back(0.);
                 _yStatsSUM.emplace_back(0.);   
                 _nEntries.emplace_back(0);
                 (void)i;
            }
            _covariance =  std::make_shared<double>(0.);
        }
        Covariance( Covariance &&)= default;
        Covariance( const Covariance &) = delete;
        std::shared_ptr<Covariance_t> GetResultPtr() const { 
            return  _covariance;
        }
        void Initialize() {}
        void InitTask(TTreeReader *, unsigned int) {}
        template <typename... ColumnTypes>
        void Exec(unsigned int slot, ColumnTypes... values){
            std::array<double, sizeof...(ColumnTypes)> valuesArr{static_cast<double>(values)...};     
            _nEntries[slot] ++;
            _xyproductSUM[slot] += valuesArr[0]*valuesArr[1];
            _xStatsSUM[slot] += valuesArr[0];
            _yStatsSUM[slot] += valuesArr[1];
        }
        void Finalize(){
            for( auto  slot : ROOT::TSeqU(1, _xyproductSUM.size())){
                _xyproductSUM[0] += _xyproductSUM[slot];
                _xStatsSUM[0]    += _xStatsSUM[slot];
                _yStatsSUM[0]    += _yStatsSUM[slot];
                _nEntries[0]     += _nEntries[slot];
            }
            /*

            */
            *_covariance  =  (1./( _nEntries[0]-1.)) * (  ( _xyproductSUM[0] ) -  1./(_nEntries[0]) * ( (_xStatsSUM[0])) * ( (_yStatsSUM[0])) )  ;
        }
        std::string GetActionName(){
            return "Covariance";
        }
};

/*
 A smart Helper for RDataFrame to fill with a weight-vector like column 100 histograms  for a given observable into a TH2D ( avoid to create 100 histograms on the fly )
 Usage example : 
    ROOT::RDataFrame dd("DecayTuple","/eos/lhcb/wg/RD/RKstar/tuples/v10/RKst/TupleProcess_MM_TRK_BDT-DTF_HLT_L0_PIDCalib_BS/Bd2KstMM/MC12MD/0/TupleProcess.root");
    BS1DHistoFiller helper{"myThN",                          // Name
                                    "A TH1D in 2D for BS",        // Title
                                    50,                     // NBins
                                    0,                  // Axes min values
                                    50000};               // Axes max values
    Support constructor with 
    - {"name", "title", nBinsX, minX,maxX}
    - {TH1DModel} [to use for irregular x-bin width]

    // We book the action: it will be treated during the event loop.
    auto node = dd.Define("weight", "RndPoisson * wfBDT_BS_BKIN_MULT_Bp_MM_L0L * wiPIDCalib"); //RVec<double> column!
    auto bsH = node.Book<double, ROOT::VecOps::RVec<double> >(std::move(helper), {"B0_PT", "weight"});
*/
template< unsigned int N>
class BS1DHistoFiller : public ROOT::Detail::RDF::RActionImpl<BS1DHistoFiller<N>> {
public:
   /// This type is a requirement for every helper.
   using Result_t = TH2D;
private:
   std::vector<std::shared_ptr<TH2D>> fHistos; // one per data processing slot
public:
   /// This constructor takes all the parameters necessary to build the TH1D to TH2D(for BS indexes)
   BS1DHistoFiller(std::string_view name, std::string_view title, int nbinsX, double xmin, double xmax ){
      const auto nSlots = ROOT::IsImplicitMTEnabled() ? ROOT::GetThreadPoolSize() : 1;
      //std::cout<<"nSlots = "<< nSlots<<std::endl;
      for (auto i : ROOT::TSeqU(nSlots)) {
        auto myname = (i==0)? TString::Format("%s",std::string(name).c_str()) : TString::Format("%s[%i]",std::string(name).c_str(),i);  
        fHistos.emplace_back(std::make_shared<TH2D>(myname, 
                                                    std::string(title).c_str(), 
                                                    nbinsX, 
                                                    xmin, 
                                                    xmax, 
                                                    N , -0.5, N-0.5));         
        (void)i;
      }
   }
   /// This constructor takes all the parameters necessary to build the TH1D to TH2D(for BS indexes)
   BS1DHistoFiller(ROOT::RDF::TH1DModel & model){
      const auto nSlots = ROOT::IsImplicitMTEnabled() ? ROOT::GetThreadPoolSize() : 1;
      //std::cout<<"nSlots = "<< nSlots<<std::endl; TO PRINT for MT checking 
      for (auto i : ROOT::TSeqU(nSlots)) {
        auto name = (i==0)? TString::Format("%s",std::string(model.fName).c_str()) : TString::Format("%s[%i]",std::string(model.fName).c_str(),i);  
        if( model.fBinXEdges.size() == 0){
            /* case in which bin bounds are NOT defined in the model, use xLow,xUp,nBins */
            fHistos.emplace_back(std::make_shared<TH2D>(name, 
                                                        std::string(model.fTitle).c_str(), 
                                                        model.fNbinsX, 
                                                        model.fXLow, model.fXUp,
                                                        N , -0.5, N-0.5));
        }else{
            /* case in which bin bounds are defined in the model */
            fHistos.emplace_back(std::make_shared<TH2D>(name, 
                                                        std::string(model.fTitle).c_str(), 
                                                        model.fNbinsX, 
                                                        model.fBinXEdges.data(),
                                                        N , -0.5, N-0.5));            
        }
        (void)i;
      }
   }         
   BS1DHistoFiller(BS1DHistoFiller &&) = default;
   BS1DHistoFiller(const BS1DHistoFiller &) = delete;
   std::shared_ptr<TH2D> GetResultPtr() const { return fHistos[0]; }
   void Initialize() {}
   void InitTask(TTreeReader *, unsigned int) {}
   /// This is a method executed at every entry
   void Exec(unsigned int slot, double xValue, ROOT::VecOps::RVec<double> & myVecColumn )
   {
      // Since THnT<T>::Fill expects a double*, we build it passing through a std::array.
      //   std::array<double, sizeof...(ColumnTypes)> valuesArr{static_cast<double>(values)...};
      for( int i = 0 ; i < N; ++i){
          fHistos[slot]->Fill(xValue , i, myVecColumn.at(i) );
      }
   }
   /// This method is called at the end of the event loop. It is used to merge all the internal THnTs which
   /// were used in each of the data processing slots.
   void Finalize()
   {
      auto &res = fHistos[0];
      for (auto slot : ROOT::TSeqU(1, fHistos.size())) {
         res->Add(fHistos[slot].get());
      }
   }
    
   std::string GetActionName(){
      return "BS1DHistoFiller";
   }
};


/*
TODO  : the TH2D version via TH3D
*/

#endif // !CUSTOMACTIONS_HPP
