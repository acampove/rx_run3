#ifndef XJPSRESCALER_CPP
#define XJPSRESCALER_CPP

#include "XJPsRescaler.hpp"

double RescalerRXSamples::operator()(const IdChains & e1, const  IdChains & e2, const IdChains & K, const IdChains & pi){
    //This is for RKst! 

    // bool ee_jpsi_B =     e1.MatchDecay( { PDG::ID::E , PDG::ID::JPs , PDG::ID::Bu}  ) && e2.MatchDecay( { PDG::ID::E , PDG::ID::JPs , PDG::ID::Bu}  );
    // bool ee_psi_B =      e1.MatchDecay( { PDG::ID::E , PDG::ID::Psi , PDG::ID::Bu}  ) && e2.MatchDecay( { PDG::ID::E , PDG::ID::Psi , PDG::ID::Bu}  );
    // bool ee_jpsi_psi_B = e1.MatchDecay( { PDG::ID::E , PDG::ID::JPs , PDG::ID::Psi, PDG::ID::Bu}  ) && e2.MatchDecay( { PDG::ID::E , PDG::ID::JPs , PDG::ID::Psi, PDG::ID::Bu }  );
    double myWeight = 1.0;
    if( m_project == Prj::RKst){
        double kshort_weights = 1.;
        
        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::KShort } ) ){
            kshort_weights = 0.5;
        }
        if( K.MatchDecay(  {PDG::ID::Pi , PDG::ID::KShort } ) ){
            kshort_weights = 0.5;
        }
        //std::cout<< "*=(Ks0)" << kshort_weights<<std::endl;
        myWeight*=kshort_weights;
        // Decay Myphi
        //  0.9974  K+         K-                     VSS ;
        //  0.0026  pi0        gamma                  VSP_PWAVE ;
        // Enddecay
        // Decay phi
        // 0.489000000 K+      K-                                      VSS; #[Reconstructed PDG2011]
        // 0.342000000 K_L0    K_S0                                    VSS; #[Reconstructed PDG2011]
        // 0.0425   rho+ pi-                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0425   rho0 pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0425   rho- pi+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0250   pi+  pi- pi0                    PHSP;
        // 0.013090000 eta     gamma                                   VSP_PWAVE; #[Reconstructed PDG2011]
        // 0.000687600 pi0     gamma                                   VSP_PWAVE; #[Reconstructed PDG2011]
        // 0.000295400 e+      e-                                      PHOTOS      VLL; #[Reconstructed PDG2011]
        // 0.000287000 mu+     mu-                                     PHOTOS      VLL; #[Reconstructed PDG2011]
        // 0.000113000 pi0     pi0     gamma                           PHSP; #[Reconstructed PDG2011]
        // 0.000115000 eta     e+      e-                              PHSP; #[Reconstructed PDG2011]
        // 0.000322000 f_0     gamma                                   PHSP; #[Reconstructed PDG2011]
        // 0.000074000 pi+     pi-                                     PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000047000 omega   pi0                                     PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000041000 pi+     pi-     gamma                           PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000004000 pi+     pi-     pi+     pi-                     PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000011200 pi0     e+      e-                              PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000072700 pi0     eta     gamma                           PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000062500 eta'    gamma                                   PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // 0.000014000 mu+     mu-     gamma                           PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // Enddecay

        /* 
        Bu2XJPsEE : 
        Decay Myphi
        0.7597  K+         K-                     VSS ;
        0.0665  rho+       pi-                    VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        0.0665  rho0       pi0                    VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        0.0665  rho-       pi+                    VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        0.0387  pi+        pi-        pi0         PHSP ;
        0.0020  pi0        gamma                  VSP_PWAVE ;
        Enddecay
        Bd2XJPsEE : 
        Decay Myphi
        0.9974  K+         K-                     VSS ;
        0.0026  pi0        gamma                  VSP_PWAVE ;
        */        
        double phi_weight =1.;
        if(K.MatchDecay(  {PDG::ID::K, PDG::ID::Phi , PDG::ID::Bd })) phi_weight    = 0.5/0.9974;
        if(K.MatchDecay(  {PDG::ID::K, PDG::ID::Phi , PDG::ID::Bu })) phi_weight    = 0.5/0.7597;
        if(pi.MatchUpstream(PDG::ID::Rho0,  PDG::ID::Phi) && pi.MatchID(PDG::ID::Pi) ) phi_weight    = 0.0425/0.0665;
        if(pi.MatchUpstream(PDG::ID::Rho_c, PDG::ID::Phi) && pi.MatchID(PDG::ID::Pi) ) phi_weight    = 0.0425/0.0665;
        if(pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Phi})) phi_weight = 0.0425/0.0665;        
        //std::cout<< "*=(phi)" << phi_weight<<std::endl;
        myWeight *= phi_weight;

        // Decay Myeta
        // 0.5913  gamma      gamma                  PHSP ;
        // 0.3386  pi-        pi+        pi0         ETA_DALITZ ;
        // 0.0701  gamma      pi-        pi+         PHSP ;
        // Enddecay
        // Decay eta
        // 0.393100000 gamma   gamma                                   PHSP; #[Reconstructed PDG2011]
        // 0.325700000 pi0     pi0     pi0                             PHSP; #[Reconstructed PDG2011]
        // 0.227400000 pi-     pi+     pi0                             ETA_DALITZ; #[Reconstructed PDG2011]
        // 0.046000000 gamma   pi-     pi+                             PHSP; #[Reconstructed PDG2011]
        // 0.007000000 gamma   e+      e-                              PHSP; #[Reconstructed PDG2011]
        // 0.000310000 gamma   mu+     mu-                             PHSP; #[Reconstructed PDG2011]
        // 0.000270000 gamma   gamma   pi0                             PHSP; #[Reconstructed PDG2011]
        // 0.000214200 pi+     pi-     e+      e-                      PHSP; #[Reconstructed PDG2011]
        // 0.000005800 mu+     mu-                                     PHSP;  #[New mode added] #[Reconstructed PDG2011]
        // Enddecay        
        double eta_weight = ( K.MatchDecay( {PDG::ID::Pi, PDG::ID::Eta} ) || pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Eta}) ) ? 0.28/0.4 : 1.;
        //std::cout<< "*=(eta)" << eta_weight<<std::endl;
        myWeight *= eta_weight;

        double kst_weights = 1.;
        //================== Deal with the K*0 decay fractions 
        //Correct for K*0 decays fractions! 
        // Decay MyK*0
        //  0.7993  K+         pi-                    VSS ;  ---> 0.66
        //  0.1995  MyK0       pi0                    VSS ;  ---> 0.33 Ks + Kl but forced Ks in dec file  so we will rescale it to  (0.33 / 2.) directly
        //  0.0012  MyK0       gamma                  VSP_PWAVE ;            
        if( K.MatchDecay(  {PDG::ID::K ,  PDG::ID::Kst})) kst_weights =  0.66 / 0.7993;    
        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::Kst})) kst_weights =  0.66 / 0.7993;    

        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::KShort, PDG::ID::Kst})) kst_weights =  0.33 / 0.2;
        //================== Deal with the K*+ decay fractions 
        // Decay MyK*+
        // 0.4993  MyK0       pi+                    VSS ; --->  0.6657 
        // 0.4993  K+         pi0                    VSS ; --->  0.3323
        // 0.0015  K+         gamma                  VSP_PWAVE ;
        // Enddecay    
        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::Kst_c})){
            kst_weights =  0.66  / 0.4993; 
        }
        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::KShort , PDG::ID::Kst_c})){
            kst_weights =  0.66  / 0.4993; 
        }        
        if( K.MatchDecay( {PDG::ID::K , PDG::ID::Kst_c})){
            kst_weights =  0.33 / 0.4993;
        }
        myWeight*= kst_weights;
        //std::cout<< "*=(K*0,+-)" << kst_weights<<std::endl;


        double k10_weights = 1.;
        // Decay K_10 (DECAY.DEC)
        // 0.2800   rho-  K+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1400   rho0  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1067   K*+   pi-                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0533   K*0   pi0                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // #To large masses can cause infinit loops
        // 0.1100   omega  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1444   K0    pi+ pi-                   PHSP;
        // 0.1244   K+    pi- pi0                   PHSP;
        // 0.0412   K0    pi0 pi0                   PHSP;
        // Enddecay        
        // Decay MyK_10 (OUR FILES )
        // 0.3796  rho-       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0949  rho0       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0965  MyK*+        pi-                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0602  MyK*0        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0744  Myomega    MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0979  MyK0       pi+        pi-         PHSP ;
        // 0.1686  K+         pi-        pi0         PHSP ;
        // 0.0279  MyK0       pi0        pi0         PHSP ;
        // Enddecay     
        //========== K_10 -> rho- ( pi- pi0) K+ ===============//
        // 0.2800   rho-  K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.3796   rho-  K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        if(   K.MatchDecay( {PDG::ID::K , PDG::ID::K_1_1270_z}) && 
                pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho_c, PDG::ID::K_1_1270_z}) ){                                                                                           
            k10_weights = 0.2800/0.3796;
        }
        // 0.1400   rho0  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0949   rho0  MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        if(  pi.MatchDecay( {PDG::ID::Pi , PDG::ID::Rho0, PDG::ID::K_1_1270_z})) {
            k10_weights = 0.1400 /0.0949;
        }
        // 0.1067   K*+         pi-                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0965  MyK*+        pi-                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_1_1270_z} ) && K.MatchDecay( { PDG::ID::K, PDG::ID::Kst_c, PDG::ID::K_1_1270_z}) ){
            k10_weights = 0.1067/ 0.0965;
        }
        //========== K_10 -> K+         pi-        pi0 =============//
        //YES 0.1686  K+         pi-        pi0         PHSP ;
        // 0.1244   K+        pi- pi0                   PHSP;
        // 0.1686  K+         pi-        pi0         PHSP ;
        if(  K.MatchDecay( {PDG::ID::K , PDG::ID::K_1_1270_z}) &&  
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_1_1270_z}) ){
            k10_weights=  0.1244/0.1686;
        }
        //========== K_10 -> K*0 pi0 =============            
        // 0.0533   K*0   pi0                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0602  MyK*0        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        if( K.MatchDecay( {PDG::ID::K , PDG::ID::Kst, PDG::ID::K_1_1270_z} ) && pi.MatchDecay( {PDG::ID::Pi , PDG::ID::Kst, PDG::ID::K_1_1270_z} ) ){
            k10_weights = 0.0533 / 0.0602;
        }
        //========== K_10 -> K0 omega =============                 
        // 0.1100   omega  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0744  Myomega    MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Omega, PDG::ID::K_1_1270_z})){
            k10_weights = 0.1100/0.0744;
        }
        myWeight*= k10_weights;
        //std::cout<< "*=(k10weight)" << k10_weights<<std::endl;

        // In DecFile 
        // Decay MyK_1+
        // 0.1838  rho+       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.1838  rho0       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.1166  MyK*0        pi+                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0467  MyK*+        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.1440  Myomega    K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.1895  K+         pi+        pi-         PHSP ;
        // 0.0816  MyK0       pi+        pi0         PHSP ;
        // 0.0541  K+         pi0        pi0         PHSP ;
        // Enddecay            

        // Decay K_1+
        // 0.2800   rho+  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1400   rho0  K+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1067   K*0   pi+                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0533   K*+   pi0                       VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // #To large masses can cause infinit loops
        // 0.1100   omega  K+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.1444   K+    pi+ pi-                   PHSP;
        // 0.1244   K0    pi+ pi0                   PHSP;
        // 0.0412   K+    pi0 pi0                   PHSP;
        // Enddecay
        double k1c_weight = 1.;
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho_c, PDG::ID::K_1_1270_c } )){
            // 0.1838  rho+       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            // 0.2800   rho+  K0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            k1c_weight = 0.2800/0.1838;
        }
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho0, PDG::ID::K_1_1270_c } )){
            // 0.1838  rho0       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            // 0.1400   rho0  K+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            k1c_weight = 0.1400/0.1838;
        }
        if( ( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Kst, PDG::ID::K_1_1270_c}) &&  K.MatchDecay( {PDG::ID::Pi, PDG::ID::Kst, PDG::ID::K_1_1270_c} ) ) || 
            ( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_1_1270_c})             &&  K.MatchDecay( {PDG::ID::K,  PDG::ID::Kst, PDG::ID::K_1_1270_c} ) ) ) {
            // 0.1166  MyK*0        pi+                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            // 0.1067    K*0        pi+                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            k1c_weight = 0.1067/0.1166 ;                
        }        
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Omega, PDG::ID::K_1_1270_c}) && K.MatchDecay( {PDG::ID::K, PDG::ID::K_1_1270_c}) ){
            // 0.1440  Myomega    K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            // 0.1100   omega  K+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            k1c_weight = 0.1100/ 0.1440;
        }
        if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_1_1270_c}) && K.MatchDecay( {PDG::ID::K, PDG::ID::K_1_1270_c}) ){
            // 0.1895  K+         pi+        pi-         PHSP ;
            // 0.1444   K+    pi+ pi-                   PHSP;
            k1c_weight = 0.1444/0.1895;            
        }
        myWeight*= k1c_weight;
        //std::cout<< "*=(k1c_weight)" << k1c_weight<<std::endl;

        // MyK_2*-      
        // Decay MyK_2*+
        //  0.2478  MyK0       pi+                    TSS ;
        //  0.2485  K+         pi0                    TSS ;
        //  0.2095  MyK*0        pi+                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0673  MyK*0        pi+        pi0       PHSP ;
        //  0.0839  MyK*+        pi0                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0539  MyK*+        pi+        pi-       PHSP ;
        //  0.0449  rho0       K+                     TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0442  rho+       MyK0                   TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        // Enddecay      
        //------------------------------------>
        // Decay K_2*+
        // 0.3340   K0  pi+                         TSS;
        // 0.1670   K+  pi0                         TSS;
        // 0.1645   K*0 pi+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0450   K*0 pi+ pi0                     PHSP;
        // 0.0835   K*+ pi0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0450   K*+ pi+ pi-                     PHSP;
        // 0.0450   K*+ pi0 pi0                     PHSP;
        // 0.0580   rho+ K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   rho0 K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   omega K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // Enddecay
        double k_2st_c_weight = 1.;

        /*
            0.2478  MyK0       pi+                    TSS ;
            0.3340    K0       pi+                    TSS;
        */
        if( pi.MatchDecay( {PDG::ID::Pi , PDG::ID::KShort ,PDG::ID::K_2_1430_c } )){
            k_2st_c_weight = 0.3340/0.2485;
        }
        if( pi.MatchDecay( {PDG::ID::Pi ,PDG::ID::K_2_1430_c } ) && K.MatchDecay( {PDG::ID::Pi, PDG::ID::KShort, PDG::ID::K_2_1430_c}) ){
            k_2st_c_weight = 0.3340/0.2485;
        }
        /*
            0.2485   K+  pi0    TSS;->
            0.1670   K+  pi0    TSS;
        */
        if( K.MatchDecay( {PDG::ID::K , PDG::ID::K_2_1430_c})){
            k_2st_c_weight = 0.1670/0.2485;
        }

        /*
            0.2095  MyK*0        pi+                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
            0.1645  K*0          pi+                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;->
            ->
            0.0673   MyK*0       pi+        pi0     PHSP ;
            0.0450   K*0 pi+ pi0                     PHSP;

        */
        if( K.MatchDecay( {PDG::ID::K, PDG::ID::Kst, PDG::ID::K_2_1430_c} ) && 
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Kst, PDG::ID::K_2_1430_c}) ) {
            k_2st_c_weight = 0.1645/0.2095; 
        }
        if( K.MatchDecay( {PDG::ID::K, PDG::ID::Kst, PDG::ID::K_2_1430_c} ) && 
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_2_1430_c} ) ) {
            k_2st_c_weight = 0.1645/0.2095; 
        }
        /*
            0.0839  MyK*+        pi0                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
            0.0539  MyK*+        pi+        pi-       PHSP ;
            //------------------
            0.0835   K*+ pi0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
            0.0450   K*+ pi+ pi-                     PHSP;
            0.0450   K*+ pi0 pi0                     PHSP;
        */
        if( K.MatchDecay(  {PDG::ID::K,   PDG::ID::Kst_c, PDG::ID::K_2_1430_c} ) && 
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Kst_c, PDG::ID::K_2_1430_c} ) ) {
                k_2st_c_weight = ( 0.0835 + 0.0450*2) /( 0.0839 +0.0539 ); 
        }  
        if( K.MatchDecay( {PDG::ID::K,   PDG::ID::Kst_c, PDG::ID::K_2_1430_c} ) && 
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_2_1430_c} ) ) {
                k_2st_c_weight = 0.0450/0.0539; 
        }
        // 0.0442   rho+ MyK0                   TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        // 0.0580   rho+ K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        if(  pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho_c, PDG::ID::K_2_1430_c} ) ){
                k_2st_c_weight = 0.0580/0.0442; 
        }
        // 0.0449   rho0 K+                     TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //=> 
        // 0.0290   rho0 K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   omega K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        //accomodate omega K+ from rho0 here! (total fraction is 0.029*2), the Dec File has no omegaK+ decay unfortunately.
        if( K.MatchDecay( {PDG::ID::K, PDG::ID::K_2_1430_c} ) && 
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho0, PDG::ID::K_2_1430_c} ) ) {
                k_2st_c_weight = (0.0290 * 2 )/0.0449 ; 
        }
        myWeight*= k_2st_c_weight;
        //std::cout<< "*=(k_2st_c_weight)" << k_2st_c_weight<<std::endl;


        // Decay Myanti-K_2*0
        //  0.4407  K-         pi+                    TSS ;
        //  0.1492  MyK*-        pi+                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.1105  Myanti-K0  pi0                    TSS ;
        //  0.0932  Myanti-K*0   pi0                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0786  rho+       K-                     TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0599  Myanti-K*0   pi0        pi0       PHSP ;
        //  0.0480  MyK*-        pi+        pi0       PHSP ;
        //  0.0200  rho0       Myanti-K0              TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        // Enddecay
        // Decay K_2*0
        // 0.3340   K+  pi-                         TSS;
        // 0.1670   K0  pi0                         TSS;
        // 0.1645   K*+ pi-                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0835   K*0 pi0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0450   K*0 pi+ pi-                     PHSP;
        // 0.0450   K*0 pi0 pi0                     PHSP;
        // 0.0450   K*+ pi- pi0                     PHSP;
        // 0.0580   rho- K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   rho0 K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   omega K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // Enddecay
        double k_2st_z_weight =1.0; 
        //  0.4407  K-         pi+                    TSS ;
        //----------------------------------------
        // 0.3340   K+  pi-                         TSS;
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = 0.3340/0.4407 ;
        }        

        //  0.1492  MyK*-        pi+                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0480  MyK*-        pi+        pi0       PHSP ;
        //----------------------------------------
        // 0.1645   K*+ pi-                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0450   K*+ pi- pi0                     PHSP;
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::Kst_c, PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Kst_c , PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = (0.1645+0.0450)/( 0.1492+0.0480) ;
        }
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::Kst_c, PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight =(0.1645+0.0450)/( 0.1492+0.0480);
        }

        //  0.0932  Myanti-K*0   pi0                  TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //  0.0599  Myanti-K*0   pi0        pi0       PHSP ;
        //--------------------------------------------------------
        // 0.0835   K*0 pi0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0450   K*0 pi+ pi-                     PHSP;
        // 0.0450   K*0 pi0 pi0                     PHSP;
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::Kst, PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Kst, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = ( 0.0835+0.0450*2) /( 0.0932 +0.0599  ) ;
        }        
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::Kst, PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = 1. ;//not covered! 
        }

        // 0.0786  rho+       K-                     TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //--------------------------------------------
        // 0.0580   rho- K+                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        if( K.MatchDecay( { PDG::ID::K,  PDG::ID::K_2_1430_z} ) && pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Rho_c, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = 0.0580/0.0786 ;//not covered! 
        }    

        // 0.0200  rho0       Myanti-K0              TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
        //---------------------------------------------------------------
        // 0.0290   rho0 K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        // 0.0290   omega K0                         TVS_PWAVE 0.0 0.0 1.0 0.0 0.0 0.0;
        if(  pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Rho0, PDG::ID::K_2_1430_z } ) ) {
            k_2st_z_weight = ( 0.0290*2) /0.0200;//no omega in dec file.
        }
        myWeight*=k_2st_z_weight;
        //std::cout<< "*=(k_2st_z_weight)" << k_2st_z_weight<<std::endl;

        //---------------------------------------------------------------
        // Decay MyK'_10
        // 0.5849  MyK*+        pi-                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.3594  MyK*0        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0278  rho-       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0070  rho0       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0093  MyK0       pi+        pi-         PHSP ;
        // 0.0069  Myomega    MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0047  MyK0       pi0        pi0         PHSP ;
        //--------------------------------------------------------------- DECAY.DEC
        // Decay K'_10
        // 0.3100   K*0  pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.6300   K*+  pi-                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0200   rho- K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0100   rho0 K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0100   omega K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0133   K0  pi+  pi-                    PHSP;
        // 0.0067   K0  pi0  pi0                    PHSP;
        // Enddecay        
        //---------------------------------------------------------------
        double weight_K_1_1400_z =1.;
        // 0.5849  MyK*+        pi-                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //---------------------------------------------------------------        
        // 0.6300   K*+        pi-                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( {PDG::ID::K , PDG::ID::Kst_c, PDG::ID::K_1_1400_z}) && 
            ( pi.MatchDecay( { PDG::ID::Pi, PDG::ID::K_1_1400_z}) || pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Kst_c, PDG::ID::K_1_1400_z}) )
        ){
            weight_K_1_1400_z = 0.6300/0.5849;
        }


        // 0.0278   rho- K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //---------------------------------------------------------------        
        // 0.0200   rho- K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( {PDG::ID::K ,PDG::ID::K_1_1400_z}) && 
            pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Rho_c, PDG::ID::K_1_1400_z})) {
            weight_K_1_1400_z =  0.0200/0.0278;
        }

        // 0.3594  MyK*0        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //---------------------------------------------------------------        
        // 0.3100   K*0  pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( {PDG::ID::K , PDG::ID::Kst, PDG::ID::K_1_1400_z}) &&  
            pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Kst, PDG::ID::K_1_1400_z}) ) {
            weight_K_1_1400_z =  0.3100/0.3594;
        }


        if( ! ( K.MatchMother( PDG::ID::K_1_1400_z) || K.MatchGMother( PDG::ID::K_1_1400_z) || K.MatchGGMother( PDG::ID::K_1_1400_z) ) ){
            // 0.0070  rho0       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            //---------------------------------------------------------------        
            // 0.0100  rho0       K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            if( pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Rho0, PDG::ID::K_1_1400_z}) || 
                pi.MatchDecay( { PDG::ID::Pi, PDG::ID::KShort, PDG::ID::K_1_1400_z})
            ){
                weight_K_1_1400_z =  0.0100/0.0070;
            }
            // 0.0069  Myomega    MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
            //---------------------------------------------------------------        
            // 0.0100  omega K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
            if( pi.MatchDecay( { PDG::ID::Pi, PDG::ID::Omega, PDG::ID::K_1_1400_z}) || 
                pi.MatchDecay( { PDG::ID::Pi, PDG::ID::KShort, PDG::ID::K_1_1400_z})
            ){
                weight_K_1_1400_z =  0.0100/0.0069;
            }
            // 0.0093  MyK0       pi+        pi-         PHSP ;
            //---------------------------------------------------------------        
            // 0.0133   K0  pi+  pi-                    PHSP;
            if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::K_1_1400_z})){
                weight_K_1_1400_z =  0.0133/0.0093;
            }
        }
        myWeight*= weight_K_1_1400_z;
        //std::cout<< "*=(weight_K_1_1400_z)" << weight_K_1_1400_z<<std::endl;



        // Enddecay        
        // Decay MyK'_1+
        // 0.6714  MyK*0        pi+                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.2646  MyK*+        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0170  K+         pi+        pi-         PHSP ;
        // 0.0128  rho+       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0128  rho0       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0128  Myomega    K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0086  K+         pi0        pi0         PHSP ;
        // Enddecay
        //-------------------------------------------------
        // Decay K'_1+
        // 0.6300   K*0  pi+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.3100   K*+  pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0133   K+  pi+  pi-                    PHSP;
        // 0.0200   rho+ K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0100   rho0 K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0100   omega K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        // 0.0067   K+  pi0  pi0                    PHSP;
        // Enddecay        

        double weight_K_1_1400_c =1.;
        // 0.6714  MyK*0        pi+                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //-------------------------------------------------
        // 0.6300   K*0  pi+                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( { PDG::ID::K , PDG::ID::Kst, PDG::ID::K_1_1400_c}) && 
            ( pi.MatchDecay( { PDG::ID::Pi , PDG::ID::Kst, PDG::ID::K_1_1400_c} ) || pi.MatchDecay( {PDG::ID::Kst}) ) )  {
                weight_K_1_1400_c = 0.6300/ 0.6714;
        }

        // 0.2646  MyK*+        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //-------------------------------------------------
        // 0.3100   K*+  pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( { PDG::ID::K , PDG::ID::Kst_c, PDG::ID::K_1_1400_c}) && 
            pi.MatchDecay( { PDG::ID::Pi , PDG::ID::Kst_c, PDG::ID::K_1_1400_c} )) {
                weight_K_1_1400_c =0.3100/0.2646;
        }

        // 0.2646  MyK*+        pi0                  VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //-------------------------------------------------
        // 0.3100   K*+  pi0                        VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( { PDG::ID::K , PDG::ID::Kst_c, PDG::ID::K_1_1400_c}) && 
            pi.MatchDecay( { PDG::ID::Pi , PDG::ID::Kst_c, PDG::ID::K_1_1400_c}) ) {
                weight_K_1_1400_c =0.3100/0.2646;
        }        

        // 0.0128  rho+       MyK0                   VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //-------------------------------------------------
        // 0.0200   rho+ K0                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( !( K.MatchMother(PDG::ID::K_1_1400_c) || K.MatchGMother(PDG::ID::K_1_1400_c) || K.MatchGGMother( PDG::ID::K_1_1400_c)) ){
            if( pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho_c , PDG::ID::K_1_1400_c})){
                weight_K_1_1400_c =0.0200 /0.0128;
            }            
        }
        // 0.0128  rho0       K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        //-------------------------------------------------
        // 0.0100   rho0 K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( { PDG::ID::K, PDG::ID::K_1_1400_c }) &&  
            pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Rho0, PDG::ID::K_1_1400_c}) ){
            weight_K_1_1400_c = 0.0100  /0.0128;
        }

        // 0.0170  K+         pi+        pi-         PHSP ;
        //-------------------------------------------------
        // 0.0133   K+  pi+  pi-                    PHSP;
        if( K.MatchDecay( { PDG::ID::K , PDG::ID::K_1_1400_c}) && 
            pi.MatchDecay( { PDG::ID::Pi , PDG::ID::K_1_1400_c} ) ) {
                weight_K_1_1400_c =0.0133/0.0170;
        }

        // 0.0128  Myomega    K+                     VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0 ;
        // 0.0100   omega K+                         VVS_PWAVE 1.0 0.0 0.0 0.0 0.0 0.0;
        if( K.MatchDecay( {PDG::ID::K, PDG::ID::K_1_1400_c}) && pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Omega, PDG::ID::K_1_1400_c} )) {
            weight_K_1_1400_c =0.0100/0.0128;
        }
        // 0.0086  K+         pi0        pi0         PHSP ;
        //-------------------------------------------------
        // 0.0067   K+  pi0  pi0                    PHSP;
        if( !( pi.MatchMother( PDG::ID::K_1_1400_c) || pi.MatchGMother( PDG::ID::K_1_1400_c) || pi.MatchGGMother( PDG::ID::K_1_1400_c)  )){
            if( K.MatchDecay( { PDG::ID::K, PDG::ID::K_1_1400_c} )){
                weight_K_1_1400_c = 0.0067/0.0086;
            }
        }
        //std::cout<< "*=(weight_K_1_1400_c)" << weight_K_1_1400_c<<std::endl;

        myWeight*= weight_K_1_1400_c;


        //std::cout<< "------------ Total = " << myWeight<<std::endl;





        /// The Bs -> Ks Kst0 J/Psi(Psi) is simulated with a PHSP model, however the Kst0 peaking structure is fully in the B.R. 
        //  Moreover the B.R> is off by a factor 2 since it uses only KS ( the dec fraction ). We must scale the B.R. by a factor 2x . 
        double myBsKsKstJPsScale = 1.;
        if( K.MatchDecay( {PDG::ID::K, PDG::ID::Bs} ) && pi.MatchDecay( {PDG::ID::Pi, PDG::ID::Bs}) ){
            if( e1.MatchUpstream( PDG::ID::JPs, PDG::ID::Bs) ||  e2.MatchUpstream( PDG::ID::JPs, PDG::ID::Bs) ){
                // 0.1077  MyJ/psi    Myphi      PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
                // 0.0427  MyJ/psi    MyK0       K-         pi+         PHSP ;# * 2 to match actual B.R. on part reco * 3                      
                //https://arxiv.org/pdf/1405.3219.pdf         
                //9.2E-4 Bs -> J/Psi K0 K+ K-
                //1.08 E-3
                myBsKsKstJPsScale  = (9.2E-4/1.08E-3 )/( 0.0427/0.1077) *3;//FACTOR 3 to accomodate the fact efficiency is off by a factor 3! 
            }
            if(e1.MatchUpstream( PDG::ID::Psi, PDG::ID::Bs) || e2.MatchUpstream( PDG::ID::Psi, PDG::ID::Bs) ){
                //  0.0748  Mypsi(2S)  Myphi      PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm; 5.4E-4 Psi/JPsi is off...
                //  0.0143  Mypsi(2S)  MyK0       K-         pi+         PHSP ;         ??? should scale up i think 
                myBsKsKstJPsScale  = 2.0  * 3.;            //DON"T KNOW WHAT TO DO HERE. Maybe a simple factor 2 is enough to compensate for KL+KS and make a link to the same thing the Kst does 
                //FACTOR 3 to accomodate the fact efficiency is off by a factor 3! 
            }    
        }
        myWeight*=myBsKsKstJPsScale;
    }


    // Decay Mypsi(3770)
    // 1.0000  MyJ/psi    pi+        pi-         PHSP ;
    // Enddecay
    // Decay Myh_c
    // 1.0000  MyJ/psi    pi0                    PHSP ;
    // Enddecay
    // Decay Mychi_c0
    // 1.0000  gamma      MyJ/psi                PHSP ;
    // Enddecay
    // Decay Mychi_c1
    // 1.0000  MyJ/psi    gamma                 VVP 1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 ;
    // Enddecay
    // Decay Mychi_c2
    // 1.0000  gamma      MyJ/psi                PHSP ;
    // Enddecay
    // Decay MyJ/psi
    // 1.0000  e+        e-                    PHOTOS VLL ;
    // Enddecay
    // Decay Mypsi(2S)
    // 0.4762  MyJ/psi    pi+        pi-         VVPIPI ;
    // 0.2346  MyJ/psi    pi0        pi0         VVPIPI ;
    // 0.1741  e+        e-                    PHOTOS VLL; Psi -> ee = 7.93 E-3 , JPsi->ee : 5.97E-3
    // 0.0517  gamma      Mychi_c1               PHSP ;
    // 0.0306  MyJ/psi    Myeta                  PARTWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
    // 0.0281  gamma      Mychi_c2               PHSP ;
    // 0.0028  gamma      Mychi_c0               PHSP ;
    // 0.0018  MyJ/psi    pi0                    PARTWAVE 0.0 0.0 1.0 0.0 0.0 0.0 ;
    // 0.0000  Myh_c      pi0                    PHSP ;
    // Enddecay

    // Bd->JPsiX
    // 0.1920  MyJ/psi    MyK*0                  SVV_HELAMP PKHplus PKphHplus PKHzero PKphHzero PKHminus PKphHminus ;
    // 0.0753  Mypsi(2S)  MyK*0                  SVV_HELAMP PKHplus PKphHplus PKHzero PKphHzero PKHminus PKphHminus ;
    // 0.0010  Mychi_c0   MyK*0                     SVS ;
    // 0.0287  Mychi_c1   K+         pi-         PHSP ;
    // 0.0076  Mychi_c2   K+         pi-         PHSP ;
    // Bu->JPsiX
    // 0.1596  MyJ/psi    K+                            SVS ;    1.0006E-3
    // 0.0729  Mypsi(2S)  K+                            SVS ;      6.19E-4
    // 0.0010  Mychi_c0   K+                            PHSP ;      1.5E-4
    // 0.0346  Mychi_c1   K+                            SVS ;      4.85E-4
    // 0.0064  Mychi_c2   K+         pi-        pi+     PHSP ; 1.34E-4
    // 0.0003  Mypsi(3770) K+                           SVS ;   4.9E-4
    //Only those 2 are really allowed in DecFile! 
    double jpsiDecWeight = 1.;
    for( auto & headType : { PDG::ID::Bu, PDG::ID::Bd, PDG::ID::Bs}){
        if(  ( e1.MatchUpstream( PDG::ID::Psi,headType) || e2.MatchUpstream( PDG::ID::Psi, headType  ) )){        
            //Psi -> J/PsiX = 61%  (real)
            //Psi -> ee is 0.1741 , to be subtracted! ( 1-0.1741)
            //Psi -> ee B.R. realONE is 1.32 times J/Psi -> ee , but inside this dec file J/Psi ->ee is 1.0 while Psi->ee is 0.17 
            if( e1.MatchMother( PDG::ID::JPs) || e2.MatchMother( PDG::ID::JPs)){
                //this is a cascade decay, therefore the "actual real-expectations" when multiplying by top level B.R. has to be 
                //The frac of Psi -> J/Psi anything is set to (1-0.1741) 
                //the weight has to be "real-fraction/( original-fraction)" --> 0.6254/ (original)
                jpsiDecWeight = 0.6254 / ( 1-0.1741); //75% 
            }
            if( e1.MatchMother( PDG::ID::Psi) || e2.MatchMother( PDG::ID::Psi)){
                //decays where Psi -> ee / JPsi ->ee are going to be scaleFront * 0.1741 / rest... this is not correct! 
                //We need to correct the Psi ->ee to be in ratio to J/Psi -> ee scale value to get relative corrected sizes.
                jpsiDecWeight =  1.32 / 0.1741; //we need to pump up the psi->ee decay since Psi->ee / J/Psi->ee is computed to be 0.177 inside the dec file, but it actually need to be 1.32 
            }
        }
    }
    myWeight*= jpsiDecWeight;   

    double branchingRatioFractioNCorrectionPsiJPs = 1.0;
    if(  ( e1.MatchUpstream( PDG::ID::Psi,PDG::ID::Bd) || e2.MatchUpstream( PDG::ID::Psi, PDG::ID::Bd  ) )){        
        //in dec file  0.0753/0.1920 = 0.39 
        //1.27e-3 , 5.9 E-4 =0.46
        //B.R. real ratio is *1.17 
        myWeight*= 1.17;
    }
    if(  ( e1.MatchUpstream( PDG::ID::Psi,PDG::ID::Bu) || e2.MatchUpstream( PDG::ID::Psi, PDG::ID::Bu  ) )){        
        // 0.1596  MyJ/psi    K+                            SVS ;    1.0006E-3
        // 0.0729  Mypsi(2S)  K+                            SVS ;      6.19E-4
        //0.0729 / 
        myWeight*= 1.35;
    }    

    // Psi/JPs decay reweighting with scales B.R. 
    double myPsiOverJPsScale = 1.;
    if( K.HasInChain( PDG::ID::Bs) || pi.HasInChain( PDG::ID::Bs) || e1.HasInChain( PDG::ID::Bs) || e1.HasInChain( PDG::ID::Bs) ){
        //Bs -> Psi  Phi 5.4E-4
        //Bs -> JPsi Phi 1.08E-3
        //  0.1077  MyJ/psi    Myphi      PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
        //  0.0748  Mypsi(2S)  Myphi      PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
        if( e1.HasInChain(PDG::ID::Psi) || e2.HasInChain( PDG::ID::Psi) || K.HasInChain( PDG::ID::Psi) || pi.HasInChain( PDG::ID::Psi) ){        
            myPsiOverJPsScale = ( 5.4E-4/1.08E-3) / ( 0.0748/0.1077  );
        }
    }
    if( K.HasInChain( PDG::ID::Bu) || pi.HasInChain( PDG::ID::Bu) || e1.HasInChain( PDG::ID::Bu) || e1.HasInChain( PDG::ID::Bu) ){
        //B+ -> Psi  K+
        //B+ -> JPsi K+
        /*
            0.1595  MyJ/psi    K-                     SVS ; 1.0006E-3
            0.0729  Mypsi(2S)  K-                     SVS ; 6.19E-4
        */
        if( e1.HasInChain(PDG::ID::Psi) || e2.HasInChain( PDG::ID::Psi) || K.HasInChain( PDG::ID::Psi) || pi.HasInChain( PDG::ID::Psi) ){
            myPsiOverJPsScale =  ( 6.19E-4/ 1.0006E-3 ) / ( 0.0729/ 0.1595 );
        }
    }
    if( K.HasInChain( PDG::ID::Bd) || pi.HasInChain( PDG::ID::Bd) || e1.HasInChain( PDG::ID::Bd) || e1.HasInChain( PDG::ID::Bd) ){
        //B0 -> Psi  Kst0
        //B0 -> JPsi Kst0     
        //  0.1850  MyJ/psi    Myanti-K*0             SVV_HELAMP PKHminus PKphHminus PKHzero PKphHzero PKHplus PKphHplus ; 1.27E-3
        //  0.0761  Mypsi(2S)  Myanti-K*0             SVV_HELAMP PKHminus PKphHminus PKHzero PKphHzero PKHplus PKphHplus ;  5.9E-4
        //PDG values :         
        if( e1.HasInChain(PDG::ID::Psi) || e2.HasInChain( PDG::ID::Psi) || K.HasInChain( PDG::ID::Psi) || pi.HasInChain( PDG::ID::Psi) ){
            myPsiOverJPsScale =  (5.9E-4 /1.27E-3 ) / (0.0761 / 0.1850);
        }
    }
    myWeight*=myPsiOverJPsScale;

    return myWeight;        
    };


#endif // !XJPSRESCALER_CPP