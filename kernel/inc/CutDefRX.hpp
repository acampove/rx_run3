#ifndef CUTDEFRX_HPP
#define CUTDEFRX_HPP

#include "ConstDef.hpp"

#include "EnumeratorSvc.hpp"

#include "TCut.h"

#include <map>

using namespace std;

/**
 * \namespace CutDefRX
 * \brief RX cut definitions
 **/
namespace CutDefRX {

    namespace ID {

        const TString SPD  = "SPD";
        const TString TRG  = "TRG";
        const TString L0   = "L0";
        const TString HLT1 = "HLT1";
        const TString HLT2 = "HLT2";

        const TString PRE   = "PS";
        const TString BKG   = "BKG";
        const TString PID   = "PID";
        const TString MVA   = "MVA";
        const TString PRECO = "PR";
        const TString HOP   = "HOP";
        const TString BREM  = "BREM";

        const TString Q2    = "Q2";
        const TString BMASS = "BMASS";
        const TString SB    = "SB";

        const TString MCT    = "MCT";
        const TString SINGLE = "SINGLE";

        const TString EXTRA = "EXTRA";

        const TString FULL = "FULL";

    }   // namespace ID

    /**
     * \namespace CutDefRX::Stripping
     * \brief RX Stripping cut definitions, applicable to MCDecayTuples.
     **/    
    namespace Stripping {         
        const TCut EE = "StrippingBu2LLK_NoPID_eeLine2==1"; 
        const TCut MM = "StrippingBu2LLK_NoPID_mmLine==1";
    }

    /**
     * \namespace CutDefRX::Trigger
     * \brief RX Trigger cut definitions
     **/
    namespace Trigger {

        const TCut L0M1 = "M1_L0MuonDecision_TOS";
        const TCut L0M2 = "M2_L0MuonDecision_TOS";
        const TCut L0M  = L0M1 || L0M2;
        const TCut L0E1 = "E1_L0ElectronDecision_TOS";
        const TCut L0E2 = "E2_L0ElectronDecision_TOS";
        const TCut L0E  = L0E1 || L0E2;
        const TCut L0I  = "{HEAD}_L0MuonDecision_TIS || {HEAD}_L0HadronDecision_TIS || {HEAD}_L0ElectronDecision_TIS";

        const TCut L0MI  = "{HEAD}_L0MuonDecision_TIS";
        const TCut L0DMI = "{HEAD}_L0DiMuonDecision_TIS";
        const TCut L0HI  = "{HEAD}_L0HadronDecision_TIS";
        const TCut L0DHI = "{HEAD}_L0DiHadronDecision_TIS";
        const TCut L0EI  = "{HEAD}_L0ElectronDecision_TIS";
        const TCut L0O   = "{HEAD}_L0Global_TOS";   

        const TCut Hlt1TIS = "{HEAD}_Hlt1Phys_TIS";
        const TCut Hlt2TIS = "{HEAD}_Hlt2Phys_TIS";
        const TCut HltTIS  = Hlt1TIS && Hlt2TIS;
        
        const TCut SPlotnTracks = "nTracks < 330";

        namespace Run1 {

            const TCut nSPD = "nSPDHits < 600";

            const TCut Hlt1TrkA = "{HEAD}_Hlt1TrackAllL0Decision_TOS";
            const TCut Hlt1TrkM = "{HEAD}_Hlt1TrackMuonDecision_TOS";

            const TCut Hlt2Topo  = "{HEAD}_Hlt2Topo2BodyBBDTDecision_TOS || {HEAD}_Hlt2Topo3BodyBBDTDecision_TOS";
            const TCut Hlt2TopoM = "{HEAD}_Hlt2TopoMu2BodyBBDTDecision_TOS || {HEAD}_Hlt2TopoMu3BodyBBDTDecision_TOS";
            const TCut Hlt2TopoE = "{HEAD}_Hlt2TopoE2BodyBBDTDecision_TOS || {HEAD}_Hlt2TopoE3BodyBBDTDecision_TOS";

            const TCut Hlt2Topo4B  = "{HEAD}_Hlt2Topo4BodyBBDTDecision_TOS";
            const TCut Hlt2Topo4BM = "{HEAD}_Hlt2TopoMu4BodyBBDTDecision_TOS";
            const TCut Hlt2Topo4BE = "{HEAD}_Hlt2TopoE4BodyBBDTDecision_TOS";

            const TCut Hlt1MM = Hlt1TrkA;   // || Hlt1TrkM;
            const TCut Hlt2MM = Hlt2Topo || Hlt2TopoM;
            const TCut Hlt2MM_noTopoMu =  Hlt2Topo ;

            const TCut Hlt1EE = Hlt1TrkA;
            const TCut Hlt2EE = Hlt2Topo || Hlt2TopoE;

            const TCut Hlt2Topo4BMM = Hlt2Topo4B || Hlt2Topo4BM;
            const TCut Hlt2Topo4BEE = Hlt2Topo4B || Hlt2Topo4BE;

        }   // namespace Run1

        namespace Run2p1 {

            const TCut nSPD = "nSPDHits < 450";

            const TCut Hlt1TrkMVA = "{HEAD}_Hlt1TrackMVADecision_TOS";
            const TCut Hlt1TrkM   = "{HEAD}_Hlt1TrackMuonDecision_TOS";

            const TCut Hlt2Topo = "{HEAD}_Hlt2Topo2BodyDecision_TOS || {HEAD}_Hlt2Topo3BodyDecision_TOS";

            const TCut Hlt2Topo4B = "{HEAD}_Hlt2Topo4BodyDecision_TOS";

            const TCut Hlt1MM = Hlt1TrkMVA;   // || Hlt1TrkM;

            const TCut Hlt1EE = Hlt1TrkMVA;

            namespace Y2015 {

                const TCut Hlt2TopoM = "{HEAD}_Hlt2TopoMu2BodyDecision_TOS || {HEAD}_Hlt2TopoMu3BodyDecision_TOS";

                const TCut Hlt2Topo4BM = "{HEAD}_Hlt2TopoMu4BodyDecision_TOS";

                const TCut Hlt2MM = Hlt2Topo || Hlt2TopoM;
                const TCut Hlt2MM_noTopoMu =  Hlt2Topo ;

                const TCut Hlt2EE = Hlt2Topo;

                const TCut Hlt2Topo4BMM = Hlt2Topo4B || Hlt2Topo4BM;
                const TCut Hlt2Topo4BEE = Hlt2Topo4B;

            }   // namespace Y2015

            namespace Y2016 {

                const TCut Hlt2TopoM = "{HEAD}_Hlt2TopoMu2BodyDecision_TOS || {HEAD}_Hlt2TopoMu3BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu2BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu3BodyDecision_TOS";
                const TCut Hlt2TopoE = "{HEAD}_Hlt2TopoE2BodyDecision_TOS || {HEAD}_Hlt2TopoE3BodyDecision_TOS || {HEAD}_Hlt2TopoEE2BodyDecision_TOS || {HEAD}_Hlt2TopoEE3BodyDecision_TOS";

                const TCut Hlt2Topo4BM = "{HEAD}_Hlt2TopoMu4BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu4BodyDecision_TOS";
                const TCut Hlt2Topo4BE = "{HEAD}_Hlt2TopoE4BodyDecision_TOS || {HEAD}_Hlt2TopoEE4BodyDecision_TOS";

                const TCut Hlt2MM = Hlt2Topo || Hlt2TopoM;
                const TCut Hlt2MM_noTopoMu =  Hlt2Topo ;

                const TCut Hlt2EE = Hlt2Topo || Hlt2TopoE;

                const TCut Hlt2Topo4BMM = Hlt2Topo4B || Hlt2Topo4BM;
                const TCut Hlt2Topo4BEE = Hlt2Topo4B || Hlt2Topo4BE;

                //////////////////////////////////////////////
                // 2016
                //
                // TCKCat 0, b = 1.1 (default on MC)
                // TCKCat 1, b = 1.6
                // TCKCat 2, b = 2.3
                //
                // MD : 89.8% (1.1) :  0.0% (1.6) : 10.2% (2.3)
                // MU : 30.0% (1.1) : 15.5% (1.6) : 54.5% (2.3)
                //////////////////////////////////////////////
                const double Hlt1TrkMVA_fracMD_TCKcat1 = 0.000;
                const double Hlt1TrkMVA_fracMD_TCKcat2 = 0.102;
                const double Hlt1TrkMVA_fracMU_TCKcat1 = 0.155;
                const double Hlt1TrkMVA_fracMU_TCKcat2 = 0.545;

                //The hlt line content is : [in_range(1000,25000,PT) & 2DCut ] || [PT > 25000 && IPCHI2_OWNPV > 7.4]
                const TCut Hlt1TrkMVA_TRACK_range2D = "TMath::Sqrt( {PARTICLE}_PX * {PARTICLE}_PX + {PARTICLE}_PY * {PARTICLE}_PY) > 1000 && TMath::Sqrt( {PARTICLE}_PX * {PARTICLE}_PX + {PARTICLE}_PY * {PARTICLE}_PY) < 25000";
                const TCut Hlt1TrkMVA_TRACK_ORCond  = "TMath::Sqrt( {PARTICLE}_PX * {PARTICLE}_PX + {PARTICLE}_PY * {PARTICLE}_PY) > 25000 && {PARTICLE}_IPCHI2_OWNPV > 7.4";
                const TCut Hlt1TrkMVA_TRACK_2DCut   = "TMath::Log({PARTICLE}_IPCHI2_OWNPV) > 1.0 / TMath::Sq((TMath::Sqrt(TMath::Sq({PARTICLE}_PX)+TMath::Sq({PARTICLE}_PY))/1000) - 1.0) + {b} / 25 * (25 - TMath::Sqrt(TMath::Sq({PARTICLE}_PX)+TMath::Sq({PARTICLE}_PY))/1000) + TMath::Log(7.4)";
                const TCut HLT1TrkMVA_TRACK_ISTOS   = "{PARTICLE}_Hlt1TrackMVADecision_TOS";
                const TCut Hlt1TrkMVA_Track = HLT1TrkMVA_TRACK_ISTOS && ( Hlt1TrkMVA_TRACK_ORCond || (Hlt1TrkMVA_TRACK_range2D && Hlt1TrkMVA_TRACK_2DCut ));

                // const TCut Hlt1TrkMVA_Track = "({PARTICLE}_Hlt1TrackMVADecision_TOS && (TMath::Log({PARTICLE}_IPCHI2_OWNPV) > 1.0 / TMath::Sq((TMath::Sqrt(TMath::Sq({PARTICLE}_PX)+TMath::Sq({PARTICLE}_PY))/1000) - 1.0) + {b} / 25 * (25 - TMath::Sqrt(TMath::Sq({PARTICLE}_PX)+TMath::Sq({PARTICLE}_PY))/1000) + TMath::Log(7.4)))";

            }   // namespace Y2016

            const TCut Hlt2EE = Hlt2Topo;
            const TCut Hlt2MM = Hlt2Topo || Y2015::Hlt2TopoM;

        }   // namespace Run2p1

        namespace Run2p2 {

            const TCut nSPD = "nSPDHits < 450";

            const TCut Hlt1TrkMVA = "{HEAD}_Hlt1TrackMVADecision_TOS";
            const TCut Hlt1TrkM   = "{HEAD}_Hlt1TrackMuonDecision_TOS";

            const TCut Hlt2Topo  = "{HEAD}_Hlt2Topo2BodyDecision_TOS || {HEAD}_Hlt2Topo3BodyDecision_TOS";
            const TCut Hlt2TopoM = "{HEAD}_Hlt2TopoMu2BodyDecision_TOS || {HEAD}_Hlt2TopoMu3BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu2BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu3BodyDecision_TOS";
            const TCut Hlt2TopoE = "{HEAD}_Hlt2TopoE2BodyDecision_TOS || {HEAD}_Hlt2TopoE3BodyDecision_TOS || {HEAD}_Hlt2TopoEE2BodyDecision_TOS || {HEAD}_Hlt2TopoEE3BodyDecision_TOS";

            const TCut Hlt2Topo4B  = "{HEAD}_Hlt2Topo4BodyDecision_TOS";
            const TCut Hlt2Topo4BM = "{HEAD}_Hlt2TopoMu4BodyDecision_TOS || {HEAD}_Hlt2TopoMuMu4BodyDecision_TOS";
            const TCut Hlt2Topo4BE = "{HEAD}_Hlt2TopoE4BodyDecision_TOS || {HEAD}_Hlt2TopoEE4BodyDecision_TOS";

            const TCut Hlt1MM = Hlt1TrkMVA;   // || Hlt1TrkM;
            const TCut Hlt2MM = Hlt2Topo || Hlt2TopoM;
            const TCut Hlt2MM_noTopoMu =  Hlt2Topo ;

            const TCut Hlt1EE = Hlt1TrkMVA;
            const TCut Hlt2EE = Hlt2Topo || Hlt2TopoE;

            const TCut Hlt2Topo4BMM = Hlt2Topo4B || Hlt2Topo4BM;
            const TCut Hlt2Topo4BEE = Hlt2Topo4B || Hlt2Topo4BE;

        }   // namespace Run2p2

    }   // namespace Trigger

    /**
     * \namespace CutDefRX::Brem
     * \brief RX Brem cut definitions
     **/
    namespace Brem {

        const TCut G0 = "(E1_BremMultiplicity + E2_BremMultiplicity) == 0";
        const TCut G1 = "(E1_BremMultiplicity + E2_BremMultiplicity) == 1";
        const TCut G2 = "(E1_BremMultiplicity + E2_BremMultiplicity) >= 2";

    }   // namespace Brem

    /**
     * \namespace CutDefRX::Mass
     * \brief RX Mass cut definitions
     **/
    namespace Mass {

        const TCut JPsMM = (TCut)((TString) Form("TMath::Abs(JPs_M - %f) < 100", PDG::Mass::JPs));
        const TCut JPsEE = "TMath::Sq(JPs_M/1000) > 6 && TMath::Sq(JPs_M/1000) < 11";

        const TCut PsiMM = (TCut)((TString) Form("TMath::Abs(JPs_M - %f) < 100", PDG::Mass::Psi));
        const TCut PsiEE = "TMath::Sq(JPs_M/1000) > 11 && TMath::Sq(JPs_M/1000) < 15";
        const TCut PsiWideEE = "TMath::Sq(JPs_M/1000) > 9.92 && TMath::Sq(JPs_M/1000) < 16.4";

        const double fDeltaB = 60;

        const TCut BT = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_JPs_M - {HEADMASS}) < %f", fDeltaB));

        const TCut BL = "{HEAD}_DTF_M > 4500";

        const double MinBMM = 5100;
        const double MaxBMM = 6100;

        const double MinBEE = 4600;
        const double MaxBEE = 6200;

        const TCut BMM = (TCut)((TString) Form("{HEAD}_DTF_M > %f && {HEAD}_DTF_M < %f", MinBMM, MaxBMM));
        const TCut BEE = (TCut)((TString) Form("{HEAD}_DTF_M > %f && {HEAD}_DTF_M < %f", MinBEE, MaxBEE));

        const double MinBJPsMM = 5100;
        const double MaxBJPsMM = 6100;

        const double MinBJPsEE = 4600;
        const double MaxBJPsEE = 6200;

        const TCut BJPsMM = (TCut)((TString) Form("{HEAD}_DTF_M > %f && {HEAD}_DTF_M < %f", MinBJPsMM, MaxBJPsMM));
        const TCut BJPsEE = (TCut)((TString) Form("{HEAD}_DTF_M > %f && {HEAD}_DTF_M < %f", MinBJPsEE, MaxBJPsEE));

        const double MinBDTFJPsMM = 5100;
        const double MaxBDTFJPsMM = 6100;

        const double MinBDTFJPsEE = 5150;
        const double MaxBDTFJPsEE = 6100;

        const TCut BDTFJPsMM = (TCut)((TString) Form("{HEAD}_DTF_JPs_M > %f && {HEAD}_DTF_JPs_M < %f", MinBDTFJPsMM, MaxBDTFJPsMM));
        const TCut BDTFJPsEE = (TCut)((TString) Form("{HEAD}_DTF_JPs_M > %f && {HEAD}_DTF_JPs_M < %f", MinBDTFJPsEE, MaxBDTFJPsEE));

        const double MinBDTFPsiMM = 5150;
        const double MaxBDTFPsiMM = 5600;

        const double MinBDTFPsiEE = 5150;
        const double MaxBDTFPsiEE = 5600;

        const TCut BDTFPsiMM = (TCut)((TString) Form("{HEAD}_DTF_Psi_M > %f && {HEAD}_DTF_Psi_M < %f", MinBDTFPsiMM, MaxBDTFPsiMM));
        const TCut BDTFPsiEE = (TCut)((TString) Form("{HEAD}_DTF_Psi_M > %f && {HEAD}_DTF_Psi_M < %f", MinBDTFPsiEE, MaxBDTFPsiEE));

        const TCut HOP_low       = "{HEAD}_HOP_M > 4800";        
        const TCut HOP_central   = "{HEAD}_HOP_M > 4700";
        const TCut HOP2D_low     = "({HEAD}_HOP_M > 5072 + TMath::Log({HEAD}_FDCHI2_OWNPV))";
        const TCut HOP2D_central = "({HEAD}_HOP_M > 4926 + 10 * TMath::Log({HEAD}_FDCHI2_OWNPV))";

    }   // namespace Mass

    /**
     * \namespace CutDefRX::Quality
     * \brief RX Quality cut definitions
     **/
    namespace Quality {
        /*
        2011
        ('MC          ',  '0x0037')
        ('Muon1(Pt)   ', {'0x0037': 37  , '0x0038': 37  , '0x0035': 37  , '0x0032': 37  })
        ('Electron(Et)', {'0x0037': 125 , '0x0038': 125 , '0x0035': 125 , '0x0032': 125 })
        ('Electron ET ', {'0x0037': 2500, '0x0038': 2500, '0x0035': 2500, '0x0032': 2500})
        ('Hadron(Et)  ', {'0x0037': 175 , '0x0038': 175 , '0x0035': 175 , '0x0032': 175 })

        2012
        ('MC          ',                                                                  '0x0045')
        ('Muon1(Pt)   ', {'0x0042': 44  , '0x003D': 44  , '0x0044': 44  , '0x0046': 44  , '0x0045': 44  })
        ('Electron(Et)', {'0x0042': 136 , '0x003D': 136 , '0x0044': 148 , '0x0046': 143 , '0x0045': 148 })
        ('Electron ET ', {'0x0042': 2720, '0x003D': 2720, '0x0044': 2960, '0x0046': 2860, '0x0045': 2960})
        ('Hadron(Et)  ', {'0x0042': 181 , '0x003D': 181 , '0x0044': 184 , '0x0046': 187 , '0x0045': 187 })

        2015
        ('MC          ',  '0x00A2' )
        ('Muon1(Pt)   ', {'0x00A2': 56  , '0x00A8': 56  , '0x00A3': 48  , '0x00A1': 38  })
        ('Electron(Et)', {'0x00A2': 112 , '0x00A8': 112 , '0x00A3': 95  , '0x00A1': 75  })
        ('Electron ET ', {'0x00A2': 2688, '0x00A8': 2688, '0x00A3': 2280, '0x00A1': 1800})
        ('Hadron(Et)  ', {'0x00A2': 150 , '0x00A8': 167 , '0x00A3': 129 , '0x00A1': 105 })

        2016
        ('MC          ',                  '0x160F')
        ('Muon1(Pt)   ', {'0x1609': 36  , '0x160F': 36  , '0x1612': 32  , '0x160E': 30  , '0x1611': 30  })
        ('Electron(Et)', {'0x1609': 110 , '0x160F': 100 , '0x1612': 109 , '0x160E': 108 , '0x1611': 109 })
        ('Electron ET ', {'0x1609': 2640, '0x160F': 2400, '0x1612': 2616, '0x160E': 2592, '0x1611': 2616})
        ('Hadron(Et)  ', {'0x1609': 164 , '0x160F': 156 , '0x1612': 162 , '0x160E': 154 , '0x1611': 162 })

        2017
        ('MC          ',                  '0x1709')
        ('Muon1(Pt)   ', {'0x1707': 34  , '0x1709': 28  , '0x1708': 22  , '0x1705': 38  , '0x1704': 26  , '0x1706': 38  })
        ('Electron(Et)', {'0x1707': 96  , '0x1709': 88  , '0x1708': 88  , '0x1705': 112 , '0x1704': 94  , '0x1706': 112 })
        ('Electron ET ', {'0x1707': 2304, '0x1709': 2112, '0x1708': 2112, '0x1705': 2688, '0x1704': 2256, '0x1706': 2688})
        ('Hadron(Et)  ', {'0x1707': 155 , '0x1709': 144 , '0x1708': 134 , '0x1705': 162 , '0x1704': 148 , '0x1706': 162 })

        2018
        ('MC          ',                                  '0x18A4')
        ('Muon1(Pt)   ', {'0x1801': 35  , '0x18A2': 35  , '0x18A4': 35  })
        ('Electron(Et)', {'0x1801': 99  , '0x18A2': 99  , '0x18A4': 99  })
        ('Electron ET ', {'0x1801': 2376, '0x18A2': 2376, '0x18A4': 2376})
        ('Hadron(Et)  ', {'0x1801': 158 , '0x18A2': 158 , '0x18A4': 158 })
        */
        const map< Year, float> L0Calo_ECAL_realET_Thresholds{ 
           { Year::Y2011 , 2500.},
           { Year::Y2012 , 3000.},
           { Year::Y2015 , 3000.},
           { Year::Y2016 , 2700.},
           { Year::Y2017 , 2700.},
           { Year::Y2018 , 2400.}
        };
        // L0Data_Muon1_Pt
        const  map< Year, int > L0Muon_ADCThreshold{ { Year::Y2011 , 37 },
                                              { Year::Y2012 , 44 },
                                              { Year::Y2015 , 56 },
                                              { Year::Y2016 , 36 },
                                              { Year::Y2017 , 34 },
                                              { Year::Y2018 , 35 }}; // From mix of Data and MC thresholds ensuring alignment.
        const map< Year, int > L0Electron_ADCThreshold{ { Year::Y2011 , 125 },
                                                  { Year::Y2012 , 148 },
                                                  { Year::Y2015 , 112 },
                                                  { Year::Y2016 , 100 },
                                                  { Year::Y2017 , 88  },
                                                  { Year::Y2018 , 99  }}; // MC thresholds

        // HLT1TCK pair: bool = true: add these lines; bool = flase: exclude these lines
        const  map< Year , pair< bool, vector<int> > > HLT1TCK{ { Year::Y2011 , make_pair(false, vector<int>{7602230, 6094899, 4849715}) },
                                                                { Year::Y2012 , make_pair(false, vector<int>{9175104, 8781888}) },
                                                                { Year::Run1 , make_pair(false, vector<int>{7602230, 6094899, 4849715, 9175104, 8781888}) },
                                                                { Year::Y2015 , make_pair(true, vector<int>{18088104, 17301666, 17170594}) },
                                                                { Year::Y2016 , make_pair(true, vector<int>{288888335}) },
                                                                { Year::Run2p1 , make_pair(true, vector<int>{18088104, 17301666, 17170594, 288888335}) },
                                                                { Year::Y2017 , make_pair(true, vector<int>{291575561}) },
                                                                { Year::Y2018 , make_pair(true, vector<int>{}) },
                                                                { Year::Run2p2 , make_pair(true, vector<int>{}) }};
        namespace Run1 {
            namespace Y2011 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 37";
            }
            namespace Y2012 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 44";
            }
            const TCut trgAccL0M_ADCPT = ("Year == 11" && Y2011::trgAccL0M_ADCPT) || ("Year == 12" && Y2012::trgAccL0M_ADCPT);
        }   // namespace Run1
        namespace Run2p1 {
            namespace Y2015 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 56";
            }
            namespace Y2016 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 36";
            }
            const TCut trgAccL0M_ADCPT = ("Year == 15" && Y2015::trgAccL0M_ADCPT) || ("Year == 16" && Y2016::trgAccL0M_ADCPT);
        }   // namespace Run2p1
        namespace Run2p2 {
            namespace Y2017 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 28";
            }
            namespace Y2018 {
                const TCut trgAccL0M_ADCPT = "L0Data_Muon1_Pt > 35";
            }
            const TCut trgAccL0M_ADCPT = ("Year == 17" && Y2017::trgAccL0M_ADCPT) || ("Year == 18" && Y2018::trgAccL0M_ADCPT);
        }   // namespace Run2p2

        // L0Data_Electron_Et
        namespace Run1 {
            namespace Y2011 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 125";
            }
            namespace Y2012 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 148";
            }
            const TCut trgAccL0E_ADCET = ("Year == 11" && Y2011::trgAccL0E_ADCET) || ("Year == 12" && Y2012::trgAccL0E_ADCET);
        }   // namespace Run1
        namespace Run2p1 {
            namespace Y2015 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 112";
            }
            namespace Y2016 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 110";
            }
            const TCut trgAccL0E_ADCET = ("Year == 15" && Y2015::trgAccL0E_ADCET) || ("Year == 16" && Y2016::trgAccL0E_ADCET);
        }   // namespace Run2p1
        namespace Run2p2 {
            namespace Y2017 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 112";
            }
            namespace Y2018 {
                const TCut trgAccL0E_ADCET = "L0Data_Electron_Et > 99";
            }
            const TCut trgAccL0E_ADCET = ("Year == 17" && Y2017::trgAccL0E_ADCET) || ("Year == 18" && Y2018::trgAccL0E_ADCET);
        }   // namespace Run2p2

        // L0Data_Hadron_Et
        namespace Run1 {
            namespace Y2011 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 175";
            }
            namespace Y2012 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 187";
            }
            const TCut trgAccL0H_ADCET = ("Year == 11" && Y2011::trgAccL0H_ADCET) || ("Year == 12" && Y2012::trgAccL0H_ADCET);
        }   // namespace Run1
        namespace Run2p1 {
            namespace Y2015 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 167";
            }
            namespace Y2016 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 164";
            }
            const TCut trgAccL0H_ADCET = ("Year == 15" && Y2015::trgAccL0H_ADCET) || ("Year == 16" && Y2016::trgAccL0H_ADCET);
        }   // namespace Run2p1
        namespace Run2p2 {
            namespace Y2017 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 162";
            }
            namespace Y2018 {
                const TCut trgAccL0H_ADCET = "L0Data_Hadron_Et > 158";
            }
            const TCut trgAccL0H_ADCET = ("Year == 17" && Y2017::trgAccL0H_ADCET) || ("Year == 18" && Y2018::trgAccL0H_ADCET);
        }   // namespace Run2p2

        const TCut trgAccL0E_ECALregion    = "TMath::Min(E1_L0Calo_ECAL_region,E2_L0Calo_ECAL_region) >= 0";
        const TCut trgAccL0E_ECALregion_E2 = "E2_L0Calo_ECAL_region >= 0";

        // E_L0Calo_ECAL_realET
        namespace Run1 {
            namespace Y2011 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2500 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2500 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2500 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2500)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2500) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2500)";
            }   // namespace Y2011
            namespace Y2012 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000)";

            }   // namespace Y2012
            const TCut trgAccL0E_ET_v1 = ("Year == 11" && Y2011::trgAccL0E_ET_v1) || ("Year == 12" && Y2012::trgAccL0E_ET_v1);
            const TCut trgAccL0E_ET = ("Year == 11" && Y2011::trgAccL0E_ET) || ("Year == 12" && Y2012::trgAccL0E_ET);
        }   // namespace Run1
        namespace Run2p1 {
            namespace Y2015 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 3000) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 3000)";
            }   // namespace Y2015
            namespace Y2016 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700)";
            }   // namespace Y2016
            const TCut trgAccL0E_ET_v1 = ("Year == 15" && Y2015::trgAccL0E_ET_v1) || ("Year == 16" && Y2016::trgAccL0E_ET_v1);
            const TCut trgAccL0E_ET = ("Year == 15" && Y2015::trgAccL0E_ET) || ("Year == 16" && Y2016::trgAccL0E_ET);
        }   // namespace Run2p1
        namespace Run2p2 {
            namespace Y2017 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2700) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2700)";
            }   // namespace Y2017
            namespace Y2018 {
                const TCut trgAccL0E_ET_v1 = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2400 && !E2_L0ElectronDecision_TOS) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2400 && !E1_L0ElectronDecision_TOS) || (E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2400 && E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2400)";
                const TCut trgAccL0E_ET = "(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > 2400) || (E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > 2400)";
            }   // namespace Y2018
            const TCut trgAccL0E_ET_v1 = ("Year == 17" && Y2017::trgAccL0E_ET_v1) || ("Year == 18" && Y2018::trgAccL0E_ET_v1);
            const TCut trgAccL0E_ET = ("Year == 17" && Y2017::trgAccL0E_ET) || ("Year == 18" && Y2018::trgAccL0E_ET);
        }   // namespace Run2p2

        // const TCut etaAccM = "TMath::Min(M1_ETA,M2_ETA) > 2.0 && TMath::Max(M1_ETA,M2_ETA) < 4.0";
        // const TCut etaAccE = "TMath::Min(E1_ETA,E2_ETA) > 2.0 && TMath::Max(E1_ETA,E2_ETA) < 4.0";

        const TCut inAccM  = "M1_InAccMuon==1 && M2_InAccMuon==1";
        const TCut inAccE  = "E1_InAccEcal==1 && E2_InAccEcal==1";
        const TCut inAccME = "M1_InAccMuon==1 && E2_InAccEcal==1";

        const TCut inCaloHoleE1 = "TMath::Abs(E1_L0Calo_ECAL_xProjection) < 363.6 && TMath::Abs(E1_L0Calo_ECAL_yProjection) < 282.6";
        const TCut inCaloHoleE2 = "TMath::Abs(E2_L0Calo_ECAL_xProjection) < 363.6 && TMath::Abs(E2_L0Calo_ECAL_yProjection) < 282.6";
        const TCut inCaloHoleE  = !(inCaloHoleE1 || inCaloHoleE2);

        const TCut pidAccM  = "M1_hasRich==1 && M2_hasRich==1 && M1_isMuon==1 && M2_isMuon==1";   // && M1_hasMuon && M2_hasMuon"; ISMUON is already hasMuon ( double counting with wPID * (noPID cut )
        const TCut pidAccM_noIsMuon  = "M1_hasRich==1 && M2_hasRich==1";   // && M1_hasMuon && M2_hasMuon"; ISMUON is already hasMuon ( double counting with wPID * (noPID cut )

        const TCut pidAccE  = "E1_hasRich==1 && E2_hasRich==1 && E1_hasCalo==1 && E2_hasCalo==1";
        const TCut pidAccME = "M1_hasRich==1 && E2_hasRich==1 && E2_hasCalo==1";   // && M1_hasMuon

        const TCut pidCalibAccM  = "TMath::Min(M1_PT,M2_PT) > 800 && TMath::Min(M1_P,M2_P) > 3000";
        const TCut pidCalibAccE  = "TMath::Min(E1_PT,E2_PT) > 500 && TMath::Min(E1_P,E2_P) > 3000";
        const TCut pidCalibAccME = "TMath::Min(M1_PT,E2_PT) > 500 && TMath::Min(M1_P,E2_P) > 3000";

        const TCut stripAccM  = "TMath::Min(M1_PT,M2_PT) > 300 && TMath::Min(M1_IPCHI2_OWNPV,M2_IPCHI2_OWNPV) > 9";
        const TCut stripAccE  = "TMath::Min(E1_PT,E2_PT) > 300 && TMath::Min(E1_IPCHI2_OWNPV,E2_IPCHI2_OWNPV) > 9 && TMath::Min(TMath::Sqrt(TMath::Sq(E1_TRACK_PX)+TMath::Sq(E1_TRACK_PY)),TMath::Sqrt(TMath::Sq(E2_TRACK_PX)+TMath::Sq(E2_TRACK_PY))) > 200 && JPs_PT > 500";
        const TCut stripAccME = "TMath::Min(M1_PT,E2_PT) > 300 && TMath::Min(M1_IPCHI2_OWNPV,E2_IPCHI2_OWNPV) > 9 && TMath::Min(TMath::Sqrt(TMath::Sq(M1_PX)+TMath::Sq(M1_PY)),TMath::Sqrt(TMath::Sq(E2_TRACK_PX)+TMath::Sq(E2_TRACK_PY))) > 200 && JPs_PT > 500";

        const TCut trackAccM  = "TMath::Max(M1_TRACK_CHI2NDOF,M2_TRACK_CHI2NDOF) < 3 && TMath::Max(M1_TRACK_GhostProb,M2_TRACK_GhostProb) < 0.4";
        const TCut trackAccE  = "TMath::Max(E1_TRACK_CHI2NDOF,E2_TRACK_CHI2NDOF) < 3 && TMath::Max(E1_TRACK_GhostProb,E2_TRACK_GhostProb) < 0.4";
        const TCut trackAccME = "TMath::Max(M1_TRACK_CHI2NDOF,E2_TRACK_CHI2NDOF) < 3 && TMath::Max(M1_TRACK_GhostProb,E2_TRACK_GhostProb) < 0.4";

        const TCut DTFLL  = "{HEAD}_DTF_status == 0";
        const TCut DTFJPs = "{HEAD}_DTF_JPs_status == 0";
        const TCut DTFPsi = "{HEAD}_DTF_Psi_status == 0";


        const TCut ECALDistance = "TMath::Sqrt(TMath::Sq(E1_L0Calo_ECAL_xProjection - E2_L0Calo_ECAL_xProjection) + TMath::Sq(E1_L0Calo_ECAL_yProjection - E2_L0Calo_ECAL_yProjection)) > 100";
    }   // namespace Quality

    /**
     * \namespace CutDefRX::Background
     * \brief RX Background cut definitions
     **/
    namespace Background {

        // Partially Reconstructed
        const TCut partRecoJPsMM = "{HEAD}_DTF_JPs_M > 5150";
        const TCut partRecoJPsEE = "{HEAD}_DTF_JPs_M > 5200";

        // Sideband
        const TCut upperSBMM = "{HEAD}_DTF_M > 5400";
        const TCut upperSBEE = "{HEAD}_DTF_M > 5600";
        const TCut lowerSB   = "{HEAD}_DTF_JPs_M < 5150";

    }   // namespace Background

    /**
     * \namespace CutDefRX::PID
     * \brief RX PID cut definitions
     **/
    namespace PID {

        const TCut pidStripM  = "M1_isMuon==1 && M2_isMuon==1";
        const TCut pidStripE  = "TMath::Min(E1_PIDe,E2_PIDe) > 0";
        const TCut pidStripME = "M1_isMuon==1 && E2_PIDe > 0";

    }   // namespace PID

}   // namespace CutDefRX

#endif
