#ifndef TRUTHMATCHSVC_CPP
#define TRUTHMATCHSVC_CPP

#include "TruthMatchingSvc.hpp"

#include "ConstDef.hpp"
#include "CutDefRX.hpp"

#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "CutHolder.hpp"
#include "EventType.hpp"

#include "TCut.h"

#include "vec_extends.h"
#include <fmt_ostream.h>

//DIRECT MATCH
TCut MatchID(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), TRUEID, ID));
    return TCut(_matching);
}

TCut MatchMotherID(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), MOTHERID, ID));
    return TCut(_matching);
}
TCut MatchGrandMotherID(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), GMOTHERID, ID));
    return TCut(_matching);
}
TCut MatchGrandGrandMotherID(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), GGMOTHERID, ID));
    return TCut(_matching);
}

//DIRECT MATCH + allow Fake
TCut MatchIDWithGhost(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), TRUEID, ID));
    TString _isFake   = TString(fmt::format("TMath::Abs({0}_{1}) == 0", _particleLabel.Data(), TRUEID));
    return TCut(_matching) || TCut(_isFake);
}

TCut MatchMotherIDWithGhost(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), MOTHERID, ID));
    TString _isFake   = TString(fmt::format("TMath::Abs({0}_{1}) == 0", _particleLabel.Data(), TRUEID));
    return TCut(_matching) || TCut(_isFake);
}


TCut MatchGrandMotherIDWithGhost(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), MOTHERID, ID));
    TString _isFake   = TString(fmt::format("TMath::Abs({0}_{1}) == 0", _particleLabel.Data(), TRUEID));
    return TCut(_matching) || TCut(_isFake);
}



TCut MatchGrandGrandMotherIDWithGhost(TString _particleLabel, int ID) {
    TString _matching = TString(fmt::format("TMath::Abs({0}_{1}) == {2}", _particleLabel.Data(), GGMOTHERID, ID));
    TString _isFake   = TString(fmt::format("TMath::Abs({0}_{1}) == 0", _particleLabel.Data(), TRUEID));
    return TCut(_matching) || TCut(_isFake);
}
TCut TruthMatching(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2bin, const TString & _sample, TString _option, bool _debug, bool _swap) {
    if (!CheckVectorContains(SettingDef::AllowedConf::Samples.at(_prj), _sample)) {
        MessageSvc::Error("TruthMatching", to_string(_prj), to_string(_ana), _sample, "not in allowed config", "EXIT_FAILURE");
        return TCut(NOCUT);
    }

    if (_debug) MessageSvc::Debug("TruthMatching", to_string(_prj), to_string(_ana), _sample, to_string(_swap));

    TCut _cut(NOCUT);

    // We label the di-lepton intermediate state as JPs, regardless of q2 and charmonium state
    TString _head, _hh, _h1, _h2, _ll;
    switch (_prj) {
        case Prj::RKst:
            _head = "B0";
            _hh   = "Kst";
            _h1   = "K";
            _h2   = "Pi";
            _ll = "JPs";
            break;
        case Prj::RK:
            _head = "Bp";
            _hh   = "";
            _h1   = "K";
            _h2   = "";
            _ll = "JPs";
            break;
        case Prj::RPhi:
            _head = "Bs";
            _hh   = "Phi";
            _h1   = "K1";
            _h2   = "K2";
            _ll = "JPs";
            break;
        default: MessageSvc::Error("TruthMatching", (TString) "Invalid project", to_string(_prj), "EXIT_FAILURE"); break;
    }

    TString _l1, _l2;
    int     _l1ID = 0, _l2ID = 0;
    switch (_ana) {
        case Analysis::MM:
            _l1   = "M1";
            _l2   = "M2";
            _l1ID = PDG::ID::M;
            _l2ID = PDG::ID::M;
            break;
        case Analysis::EE:
            _l1   = "E1";
            _l2   = "E2";
            _l1ID = PDG::ID::E;
            _l2ID = PDG::ID::E;
            break;
        case Analysis::ME:
            _l1   = "M1";
            _l2   = "E2";
            _l1ID = PDG::ID::M;
            _l2ID = PDG::ID::E;
            break;
        default: MessageSvc::Error("TruthMatching", (TString) "Invalid analysis", to_string(_ana), "EXIT_FAILURE"); break;
    }

    int _headID = 0;
    if (_sample.BeginsWith("Bd")) _headID = PDG::ID::Bd;
    if (_sample.BeginsWith("Bu")) _headID = PDG::ID::Bu;
    if (_sample.BeginsWith("Bs")) _headID = PDG::ID::Bs;
    if (_sample.BeginsWith("Lb")) _headID = PDG::ID::Lb;
    if (_headID == 0) MessageSvc::Error("TruthMatching", (TString) "Invalid HEAD", _sample, to_string(_headID), "EXIT_FAILURE");

    TCut _headMatched = MatchID(_head, _headID);

    TCut _finalStatesMatchedHH      = TCut(NOCUT);
    TCut _finalStatesMatchedHH_SWAP = TCut(NOCUT);
    if (_sample.Contains("2Pi")) { _finalStatesMatchedHH = MatchID(_h1, PDG::ID::Pi); }
    if (_sample.Contains("2K")) {  _finalStatesMatchedHH = MatchID(_h1, PDG::ID::K); }
    if (_sample.Contains("2Kst") || _sample.Contains("2KPi")) {
        _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::Pi);
        _finalStatesMatchedHH_SWAP = MatchID(_h1, PDG::ID::Pi) && MatchID(_h2, PDG::ID::K);
        if (_prj == Prj::RK) {
            _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K);
            _finalStatesMatchedHH_SWAP = MatchID(_h1, PDG::ID::Pi);
        }
    }
    if (_sample.Contains("2Phi")) {
        _finalStatesMatchedHH = MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::K);
        if (_prj == Prj::RK) { _finalStatesMatchedHH = MatchID(_h1, PDG::ID::K); }
    }
    if (_sample.Contains("2pK")) {
        _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::P);
        _finalStatesMatchedHH_SWAP = MatchID(_h1, PDG::ID::P) && MatchID(_h2, PDG::ID::K);        
        if (_prj == Prj::RK) {
            _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K);
            _finalStatesMatchedHH_SWAP = MatchID(_h1, PDG::ID::P);
        }
        if( _option.Contains("tmCustomWithSwap")){
            auto _combo = _finalStatesMatchedHH || _finalStatesMatchedHH_SWAP;
            _finalStatesMatchedHH = _combo;
            _finalStatesMatchedHH_SWAP = _combo;
            MessageSvc::Warning("tmCustomWithSwap", (TString)"Enabling Swaps in HH misID and also correct misID assignment");
        }
    }
    if (_sample.Contains("2X")) {
        if (_prj == Prj::RKst) {
            _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::Pi);
            _finalStatesMatchedHH_SWAP = (MatchID(_h1, PDG::ID::Pi) && MatchID(_h2, PDG::ID::K)) || (MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::K)) || (MatchID(_h1, PDG::ID::Pi) && MatchID(_h2, PDG::ID::Pi)) || (MatchID(_h1, PDG::ID::K) && MatchID(_h2, PDG::ID::P)) || (MatchID(_h1, PDG::ID::P) && MatchID(_h2, PDG::ID::K));
        }
        if (_prj == Prj::RK) {
            _finalStatesMatchedHH      = MatchID(_h1, PDG::ID::K);
            _finalStatesMatchedHH_SWAP = MatchID(_h1, PDG::ID::Pi) || MatchID(_h1, PDG::ID::P);
        }
        if( _option.Contains("tmCustomWithSwap")){
            auto _combo = _finalStatesMatchedHH || _finalStatesMatchedHH_SWAP;
            _finalStatesMatchedHH = _combo;
            _finalStatesMatchedHH_SWAP = _combo;
        }
    }
    if (_swap && (_finalStatesMatchedHH_SWAP != TCut(NOCUT))) _finalStatesMatchedHH = _finalStatesMatchedHH || _finalStatesMatchedHH_SWAP;

    TCut _finalStatesMatchedLL = MatchID(_l1, _l1ID) && MatchID(_l2, _l2ID);

    TCut _finalStatesMatched2X = TCut(NOCUT);
    if (_prj == Prj::RK) {
        _finalStatesMatched2X = !MatchID(_h1, _headID) && !MatchID(_ll, _headID) && !MatchID(_l1, _headID) && !MatchID(_l2, _headID); // && _finalStatesMatchedLL;// && _finalStatesMatchedHH ;
        if( _option.Contains("tmCustomVeto2XSwap")){
            _finalStatesMatched2X = _finalStatesMatched2X && ( MatchID(_h1, PDG::ID::K) && MatchID(_l1, _l1ID) && MatchID(_l2, _l2ID) && !MatchMotherID(_l1, PDG::ID::Photon) && !MatchMotherID(_l2, PDG::ID::Photon) );
        }
        _finalStatesMatched2X = _finalStatesMatched2X && !( MatchMotherID(_l1, PDG::ID::Photon) || MatchMotherID(_l2, PDG::ID::Photon) );
    } else {
        _finalStatesMatched2X = !MatchID(_h2, _headID) && !MatchID(_h1, _headID) && !MatchID(_ll, _headID) && !MatchID(_l1, _headID) && !MatchID(_l2, _headID); //&& _finalStatesMatchedLL;// && _finalStatesMatchedHH;
        if( _prj == Prj::RKst && _option.Contains("tmCustomVeto2XSwap")){
            if( _sample.BeginsWith("Bs2")){
                _finalStatesMatched2X = _finalStatesMatched2X && ( MatchID(_h1, PDG::ID::K) && 
                                                                   MatchID(_l1, _l1ID) && MatchID(_l2, _l2ID) && 
                                                                   !MatchMotherID(_l1, PDG::ID::Photon) && 
                                                                   !MatchMotherID(_l2, PDG::ID::Photon) );
            }else{
                _finalStatesMatched2X = _finalStatesMatched2X && ( MatchID(_h1, PDG::ID::K) && 
                                                                   MatchID(_h2, PDG::ID::Pi) && 
                                                                   MatchID(_l1, _l1ID) && 
                                                                   MatchID(_l2, _l2ID) && 
                                                                   !MatchMotherID(_l1, PDG::ID::Photon) && 
                                                                   !MatchMotherID(_l2, PDG::ID::Photon) );
            }
        }
        //NOTE : veto out the cases where E < gamma < E in the tuples from the 2XJPs samples
        _finalStatesMatched2X = _finalStatesMatched2X && !( MatchMotherID(_l1, PDG::ID::Photon) || MatchMotherID(_l2, PDG::ID::Photon) );
    }

    if (_prj == Prj::RKst) {
        if (_sample == "Bd2KstMM") {        _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("M1", PDG::ID::Bd) && MatchMotherID("M2", PDG::ID::Bd) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd); }
        if (_sample == "Bd2KstEE") {        _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("E1", PDG::ID::Bd) && MatchMotherID("E2", PDG::ID::Bd) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd); }
        if (_sample == "Bd2KstJPsMM") {     _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KstSwapJPsMM") { _cut = _headMatched && _finalStatesMatchedHH_SWAP   && _finalStatesMatchedLL && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KstJPsEE") {     _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstSwapJPsEE") { _cut = _headMatched && _finalStatesMatchedHH_SWAP   && _finalStatesMatchedLL && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstPsiMM") {     _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KstSwapPsiMM") { _cut = _headMatched && _finalStatesMatchedHH_SWAP   && _finalStatesMatchedLL && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KstPsiEE") {     _cut = _headMatched && _finalStatesMatchedHH        && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchGrandMotherID("Pi", PDG::ID::Bd) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstSwapPsiEE") { _cut = _headMatched && _finalStatesMatchedHH_SWAP   && _finalStatesMatchedLL && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        
        if (_sample == "Bd2KPiJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bd) && MatchMotherID("Pi", PDG::ID::Bd) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KPiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bd) && MatchMotherID("Pi", PDG::ID::Bd) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }

        if (_sample == "Bd2KstPsiPiPiJPsEE") { 
            _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && 
            MatchMotherID("K", PDG::ID::Kst) && ( MatchMotherID("Pi", PDG::ID::Kst) || MatchMotherID("Pi", PDG::ID::Psi) ) && 
            MatchGrandMotherID("K", PDG::ID::Bd) && _finalStatesMatchedHH &&  MatchGrandMotherID("Pi", PDG::ID::Bd) && 
            MatchMotherID(     "E1", PDG::ID::JPs) &&      MatchMotherID("E2", PDG::ID::JPs) && 
            MatchGrandMotherID("E1", PDG::ID::Psi) && MatchGrandMotherID("E2", PDG::ID::Psi); 
        }
        
        if (_sample == "Bu2KMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bd2KPiPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bd) && MatchMotherID("Pi", PDG::ID::Bd) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }
        if (_sample == "Bd2KPiPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bd) && MatchMotherID("Pi", PDG::ID::Bd) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bs2KstJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2KstJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bs2KstPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2KstPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bu2KPsiPiPiJPsEE"){ 
            _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && 
                    MatchMotherID("K",PDG::ID::Bu) && MatchMotherID("Pi", PDG::ID::Psi) && MatchGrandMotherID("Pi",PDG::ID::Bu) && 
                    MatchID("E1",PDG::ID::E) && MatchMotherID("E1", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Psi) && MatchGrandGrandMotherID("E1",PDG::ID::Bu) &&
                    MatchID("E2",PDG::ID::E) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E2", PDG::ID::Psi) && MatchGrandGrandMotherID("E2",PDG::ID::Bu);
        }
        if (_sample == "Bs2KsKstJPsEE"){            

            TCut _EE_JPs_Bs   = MatchMotherID(_l1, PDG::ID::JPs) &&  MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandMotherID(_l1,PDG::ID::Bs) && MatchGrandMotherID(_l2,PDG::ID::Bs);
            TCut _EE_JPs_X_Bs = MatchMotherID(_l1, PDG::ID::JPs) &&  MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandGrandMotherID(_l1,PDG::ID::Bs) && MatchGrandGrandMotherID(_l2,PDG::ID::Bs);
            TCut _EE_Psi_Bs   = MatchMotherID(_l1, PDG::ID::Psi) &&  MatchMotherID(_l2, PDG::ID::Psi) && MatchGrandGrandMotherID(_l1,PDG::ID::Bs) && MatchGrandGrandMotherID(_l2,PDG::ID::Bs);

            _cut = _headMatched && MatchID(_h1,PDG::ID::K) &&  MatchID(_h2,PDG::ID::Pi)  && 
            MatchMotherID(_h1,PDG::ID::Bs) && MatchMotherID(_h2, PDG::ID::Bs) && 
            _finalStatesMatchedLL && ( 
                _EE_JPs_Bs ||
                _EE_JPs_X_Bs ||
                _EE_Psi_Bs
            );
            TCut _lep =  _EE_JPs_Bs;
            if (_option.Contains("PRH")) _cut = _cut &&  _lep;                
            if (_option.Contains("PRL")) _cut = _cut && !_lep;                
        }
        if (_sample == "Bs2KPiJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bs) && MatchMotherID("Pi", PDG::ID::Bs) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2KPiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bs) && MatchMotherID("Pi", PDG::ID::Bs) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("E1", PDG::ID::Bs) && MatchMotherID("E2", PDG::ID::Bs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs); }
        if (_sample == "Bs2PhiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("M1", PDG::ID::Bs) && MatchMotherID("M2", PDG::ID::Bs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs); }
        if (_sample == "Bs2PhiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Phi) && MatchMotherID("Pi", PDG::ID::Phi) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("Pi", PDG::ID::Bs) && MatchGrandMotherID("K", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Lb2pKMM") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("M1", PDG::ID::Lb) && MatchMotherID("M2", PDG::ID::Lb); }
        if (_sample == "Lb2pKEE") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("E1", PDG::ID::Lb) && MatchMotherID("E2", PDG::ID::Lb); }
        if (_sample == "Lb2pKJPsMM") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Lb) && MatchGrandMotherID("M2", PDG::ID::Lb); }
        if (_sample == "Lb2pKJPsEE") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Lb) && MatchGrandMotherID("E2", PDG::ID::Lb); }
        if (_sample == "Lb2pKPsiMM") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Lb) && MatchGrandMotherID("M2", PDG::ID::Lb); }
        if (_sample == "Lb2pKPsiEE") { _cut = _headMatched && ( _finalStatesMatchedHH) && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb) && MatchMotherID("Pi", PDG::ID::Lb) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Lb) && MatchGrandMotherID("E2", PDG::ID::Lb); }
        if (_sample == "Bu2K1EE") {
            // see here : https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles/blob/master/dkfiles/Bu_K1ee=DecProdCut.dec
            // B+ -> ... K1(1270)+ -> (K*0 -> K+pi-)pi+
            TCut _kstPi_match = MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) &&   // want to truth match also Mis-ID case? (not sure)
                                MatchGrandMotherID("K", PDG::ID::K_1_1270_c) && MatchGrandMotherID("Pi", PDG::ID::K_1_1270_c) && MatchGrandGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);
            // B+ -> ... K(1270)+ -> (rho -> pi+pi-)K+
            TCut _rhoK_match = MatchMotherID("Pi", PDG::ID::Rho0) && MatchMotherID("K", PDG::ID::K_1_1270_c) && MatchGrandMotherID("Pi", PDG::ID::K_1_1270_c) && MatchGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);

            TCut _omega_match = MatchMotherID("Pi", PDG::ID::Omega) && MatchMotherID("K", PDG::ID::K_1_1270_c) && MatchGrandMotherID("Pi", PDG::ID::K_1_1270_c) && MatchGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);
            TCut _finalCut    = (_omega_match || _rhoK_match || _kstPi_match) && _finalStatesMatchedLL;
            _cut              = _finalCut;
        }
        if (_sample == "Bu2K2EE") {
            TCut _kstPi_match = MatchMotherID("K", PDG::ID::Kst) && MatchMotherID("Pi", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::K_2_1430_c) && MatchGrandMotherID("Pi", PDG::ID::K_2_1430_c) && MatchGrandGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);

            TCut _rhoK_match = MatchMotherID("Pi", PDG::ID::Rho0) && MatchMotherID("K", PDG::ID::K_2_1430_c) && MatchGrandMotherID("Pi", PDG::ID::K_2_1430_c) && MatchGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);

            TCut _omega_match = MatchMotherID("Pi", PDG::ID::Omega) && MatchMotherID("K", PDG::ID::K_2_1430_c) && MatchGrandMotherID("Pi", PDG::ID::K_2_1430_c) && MatchGrandMotherID("K", PDG::ID::Bu) && MatchGrandGrandMotherID("Pi", PDG::ID::Bu);
            TCut _finalCut    = (_omega_match || _rhoK_match || _kstPi_match) && _finalStatesMatchedLL;
            _cut              = _finalCut;
        }
        if (_sample == "Bu2KPiPiEE") {    _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("Pi", PDG::ID::Bu) && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Bu)  && MatchMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KPiPiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("Pi", PDG::ID::Bu) && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }    
        if (_sample == "Bu2KPiPiPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("Pi", PDG::ID::Bu) && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }    
        if (_sample.Contains("2XJPs")) {
            _cut = _headMatched && _finalStatesMatched2X && !TRUTHMATCHING::bkgCatSignal && TRUTHMATCHING::bkgCatPhysics;        
            if (_q2bin == Q2Bin::JPsi){
                // Veto Bd2KstJPsLL and Bd2KPiLL
                // if (_sample.BeginsWith("Bd")) _cut = _cut && !TruthMatching(_prj, _ana, _q2bin, "Bd2KstJPs" + to_string(_ana), _option, _debug, true) && !TruthMatching(_prj, _ana, _q2bin, "Bd2KPiJPs" + to_string(_ana), _option, _debug, true);
                if (_sample.BeginsWith("Bd")){
                    TCut _catSelection = "B0_BKGCAT < 70 && B0_BKGCAT >=10";
                    //L1/2 chain match : L from J/Psi and J/Psi from B        
                    TCut _L1ChainMatch = MatchMotherID(_l1, PDG::ID::JPs ) && MatchGrandMotherID( _l1, PDG::ID::Bd) && MatchID(_l1, _l1ID); 
                    TCut _L2ChainMatch = MatchMotherID(_l2, PDG::ID::JPs ) && MatchGrandMotherID( _l2, PDG::ID::Bd) && MatchID(_l2, _l2ID);
                    //H1/2 chain match : H from Kst and Kst from B   
                    TCut _H1ChainMatch  = MatchMotherID(_h1, PDG::ID::Kst ) &&  MatchID( _h1, PDG::ID::K)  && MatchGrandMotherID( _h1, PDG::ID::Bd);
                    TCut _H2ChainMatch  = MatchMotherID(_h2, PDG::ID::Kst ) &&  MatchID( _h2, PDG::ID::Pi) && MatchGrandMotherID( _h2, PDG::ID::Bd);
                    TCut _HHChainMatch  = _H1ChainMatch  && _H2ChainMatch;
                    //H1/2 chain match : H from Kst(1430) tails  and Kst from B  
                    TCut _H1ChainMatchK2  = MatchMotherID(_h1, PDG::ID::K_2_1430_z ) &&  MatchID( _h1, PDG::ID::K)  && MatchGrandMotherID( _h1, PDG::ID::Bd);
                    TCut _H2ChainMatchK2  = MatchMotherID(_h2, PDG::ID::K_2_1430_z ) &&  MatchID( _h2, PDG::ID::Pi) && MatchGrandMotherID( _h2, PDG::ID::Bd);
                    TCut _HHChainMatchK2  = _H1ChainMatchK2  && _H2ChainMatchK2;
                    //Double swap via Kst0 
                    TCut _H1ChainMatchSwap  = MatchMotherID(_h1, PDG::ID::Kst ) &&  MatchID( _h1, PDG::ID::Pi)  && MatchGrandMotherID( _h1, PDG::ID::Bd);
                    TCut _H2ChainMatchSwap  = MatchMotherID(_h2, PDG::ID::Kst ) &&  MatchID( _h2, PDG::ID::K)   && MatchGrandMotherID( _h2, PDG::ID::Bd);
                    TCut _HHChainMatchSwap  =_H1ChainMatchSwap  && _H2ChainMatchSwap;
                    //Double swap via K2
                    TCut _H1ChainMatchK2Swap  = MatchMotherID(_h1, PDG::ID::K_2_1430_z ) &&  MatchID( _h1, PDG::ID::Pi)  && MatchGrandMotherID( _h1, PDG::ID::Bd);
                    TCut _H2ChainMatchK2Swap  = MatchMotherID(_h2, PDG::ID::K_2_1430_z ) &&  MatchID( _h2, PDG::ID::K)   && MatchGrandMotherID( _h2, PDG::ID::Bd);
                    TCut _HHChainMatchK2Swap  = _H1ChainMatchK2Swap  && _H2ChainMatchK2Swap;
                    //Via B0 -> Kpi J/Psi [ no intermediate resonances ]
                    TCut _H1ChainMatch_SWave      = MatchMotherID(_h1, PDG::ID::Bd ) &&  MatchID( _h1, PDG::ID::K); 
                    TCut _H2ChainMatch_SWave      = MatchMotherID(_h2, PDG::ID::Bd ) &&  MatchID( _h2, PDG::ID::Pi);
                    TCut _HHChainMatch_SWave      =_H1ChainMatch_SWave  && _H2ChainMatch_SWave;
                    //Double swap KPi  
                    TCut _H1ChainMatchSwap_SWave      = MatchMotherID(_h1, PDG::ID::Bd ) &&  MatchID( _h1, PDG::ID::Pi); 
                    TCut _H2ChainMatchSwap_SWave      = MatchMotherID(_h2, PDG::ID::Bd ) &&  MatchID( _h2, PDG::ID::K);
                    TCut _HHChainMatchSwap_SWave      =_H1ChainMatchSwap_SWave  && _H2ChainMatchSwap_SWave;
                    TCut _HHSignalMatch               = (_HHChainMatch     || _HHChainMatch_SWave);
                    TCut _HHSignalMatchK2             = (_HHChainMatch     || _HHChainMatch_SWave || _HHChainMatchK2);
                    TCut _HHSignalSwapMatch   = (_HHChainMatchSwap || _HHChainMatchSwap_SWave);
                    TCut _HHSignalSwapMatchK2 = (_HHChainMatchSwap || _HHChainMatchSwap_SWave || _HHChainMatchK2Swap);
                    TCut _H1SignalMatch     = (_H1ChainMatch     || _H1ChainMatch_SWave);
                    TCut _H1SignalSwapMatch = (_H1ChainMatchSwap || _H1ChainMatchSwap_SWave);
                    TCut _H2SignalMatch     = (_H2ChainMatch     || _H2ChainMatch_SWave );
                    TCut _H2SignalSwapMatch = (_H2ChainMatchSwap || _H2ChainMatchSwap_SWave);
                    //Keep all 
                    TCut _cat40        = TCut("B0_BKGCAT==40");
                    //Keep all 
                    TCut _cat20        = TCut("B0_BKGCAT==20");
                    //In Cat 10 you want to Keep everything fully matchin decay with final state chains decay matching
                    TCut _cat10        = TCut("B0_BKGCAT==10") && !(  _L1ChainMatch && _L2ChainMatch && _HHSignalMatchK2 );
                    //CAT30 : If cat is 30 , we want to keep everything except the K to Pi swap which is modelled with the Bu2PiJPs decay directly
                    TCut _cat30        = TCut("B0_BKGCAT==30") && !(  _L1ChainMatch && _L2ChainMatch && _HHSignalSwapMatchK2 );//keep all misID cases exept the signal-like, modelled independently , absorbed in signal yields. 
                    //CAT50 : If cat is 50 , we want to remove as in cat10.
                    TCut _cat50        = TCut("B0_BKGCAT==50")  && !( _L1ChainMatch && _L2ChainMatch && _HHSignalMatchK2 );
                    //CAT50 : If cat is 50 , we want to remove as in cat10 but allowing 1 ghost in the chain 
                    TCut _cat60        = TCut("B0_BKGCAT==60") && !(                         
                        (_L1ChainMatch && ( _HHSignalMatch || _HHSignalSwapMatch)  && MatchID(_l2,0)) ||  //l2 is the ghost
                        (_L2ChainMatch && ( _HHSignalMatch || _HHSignalSwapMatch)  && MatchID(_l1,0)) ||  //l1 is the ghost
                        (_L1ChainMatch && _L2ChainMatch &&  (_H1SignalMatch || _H1SignalSwapMatch)  && MatchID(_h2,0) )       ||   //h2 is the ghost 
                        (_L1ChainMatch && _L2ChainMatch &&  (_H2SignalMatch || _H2SignalSwapMatch)  && MatchID(_h1,0) )
                    );

                    _cut = _finalStatesMatched2X && TRUTHMATCHING::bkgCatPhysics && _catSelection && (
                        _cat10 || _cat30 || _cat40 || _cat20 || _cat50 || _cat60
                    );
                    if( _option.Contains("tmCustomVetoPsi2JPsX")){
                        TCut _l1ChainVeto = MatchID(_l1, _l1ID) && MatchMotherID(_l1, PDG::ID::JPs) && ( 
                            (MatchGrandMotherID(_l1, PDG::ID::Psi) && MatchGrandGrandMotherID( _l1, PDG::ID::Bd) )
                            || 
                            (MatchGrandGrandMotherID(_l1, PDG::ID::Psi) ) //Psi -> X( ->JPs X) Y
                        );
                        TCut _l2ChainVeto = MatchID(_l2, _l2ID) && MatchMotherID(_l2, PDG::ID::JPs) && (
                            (MatchGrandMotherID(_l2, PDG::ID::Psi) && MatchGrandGrandMotherID( _l2, PDG::ID::Bd) )
                            ||
                            (MatchGrandGrandMotherID(_l2, PDG::ID::Psi) )
                        ); //Psi -> X( ->JPs X) Y
                        TCut _h1ChainVeto = MatchID(_h1, PDG::ID::K)   && MatchMotherID(_h1, PDG::ID::Kst) &&  MatchGrandMotherID(_h1, PDG::ID::Bd);
                        TCut _h2ChainVeto = MatchID(_h2, PDG::ID::Pi)  && MatchMotherID(_h2, PDG::ID::Kst) &&  MatchGrandMotherID(_h2, PDG::ID::Bd);
                        _cut = _cut && !( _l1ChainVeto && _l2ChainVeto && _h1ChainVeto && _h2ChainVeto );
                    }
                    /*
                    no Psi2XJps , modelled and constrained 
                    TCut _filter = "B0_BKGCAT==40"
                    TCut _l1Chain = MatchID(_l1, _l1T)
                    */
                }
                // Veto Bs2KstJPs and Bs2PhiJPs (already included as dedicated component!)
                if (_sample.BeginsWith("Bs")){ 
                    TCut _catConsider  = "B0_BKGCAT==20 || B0_BKGCAT==30 || B0_BKGCAT==40 || B0_BKGCAT==50 || B0_BKGCAT==60";                    
                    TCut _chainE1Match = MatchID(_l1, _l1ID)       && MatchMotherID(_l1, PDG::ID::JPs) && MatchGrandMotherID(_l1, PDG::ID::Bs);
                    TCut _chainE2Match = MatchID(_l2, _l2ID)       && MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandMotherID(_l2, PDG::ID::Bs);
                    TCut _chainK_Kst   = MatchID(_h1, PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Kst) && MatchGrandMotherID(_h1, PDG::ID::Bs);
                    TCut _chainPi_Kst  = MatchID(_h2, PDG::ID::Pi) && MatchMotherID(_h2, PDG::ID::Kst) && MatchGrandMotherID(_h2, PDG::ID::Bs);
                    //Used for Veto 
                    TCut _ISBs2KstJPs = _chainE1Match && _chainE2Match && _chainK_Kst && _chainPi_Kst;
                    TCut _chainK_Phi   = MatchID(_h1, PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Phi) && MatchGrandMotherID(_h1, PDG::ID::Bs);
                    TCut _chainPi_Phi  = MatchID(_h2, PDG::ID::K)  && MatchMotherID(_h2, PDG::ID::Phi) && MatchGrandMotherID(_h2, PDG::ID::Bs);
                    TCut _ISBs2PhiJPs = _chainE1Match && _chainE2Match && _chainK_Phi && _chainPi_Phi;
                    TCut _chainKK1   = MatchID(_h1, PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Bs);
                    TCut _chainPiK2  = MatchID(_h2, PDG::ID::K)  && MatchMotherID(_h2, PDG::ID::Bs);
                    TCut _ISBs2KKJPs  = _chainE1Match && _chainE2Match && ( _chainKK1 && _chainPiK2);
                    //Veto in CAT30 ( all reconstructed ) the :
                    // +Bs => KKJ/Psi ( covered by Phi J/Psi constraint) , 
                    // +Bs => Kst0( -> K Pi) J/Psi( => ee)                                     
                    // +Bs => Phi( -> KK ) J/Psi 
                    TCut _cat30Cut    =  TCut("B0_BKGCAT==30") &&  (_ISBs2PhiJPs || _ISBs2KKJPs); //Veto CAT30 events from Bs leaking under our signal , it's in the Bs2Phi accounted already (roughly with g-const)
                    TCut _otherCat    =  TCut("(B0_BKGCAT==50 || B0_BKGCAT==20)") && _ISBs2KstJPs ;
                    TCut _leptonMatch =  MatchID(_l1, _l1ID) &&  MatchID(_l2, _l2ID);
                    _cut = _cut && _leptonMatch  &&  _catConsider && !_cat30Cut && !_otherCat;
                }
                if( _sample.BeginsWith("Bu")){
                    TCut _catConsider  = "(B0_BKGCAT==20 || B0_BKGCAT==30 || B0_BKGCAT==40 || B0_BKGCAT==50 || B0_BKGCAT==60)";
                    TCut _leptonMatch =  MatchID(_l1, _l1ID) &&  MatchID(_l2, _l2ID);
                    _cut = _cut && _leptonMatch && _catConsider; 
                    if( _option.Contains("tmCustomVetoBu2Psi2JPsX")){
                        TCut _kchain = MatchID( "K", PDG::ID::K)   && MatchMotherID("K",PDG::ID::Bu); 
                        TCut _pichain= MatchID( "Pi", PDG::ID::Pi) && MatchMotherID("Pi", PDG::ID::Psi) && MatchGrandMotherID("Pi",PDG::ID::Bu);
                        TCut _l1chain= MatchID("E1",PDG::ID::E)    && MatchMotherID("E1", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Psi) && MatchGrandGrandMotherID("E1",PDG::ID::Bu);
                        TCut _l2chain= MatchID("E2",PDG::ID::E)    && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E2", PDG::ID::Psi) && MatchGrandGrandMotherID("E2",PDG::ID::Bu);
                        _cut = _cut && !( _kchain && _pichain && _l1chain && _l2chain);
                    }
                }
                //From B+ decay , keep everything...! 
                TCut _lep = MatchGrandMotherID(_l1, _headID) 
                         && MatchGrandMotherID(_l2, _headID) 
                         && MatchMotherID(_l1, PDG::ID::JPs) 
                         && MatchMotherID(_l2, PDG::ID::JPs);
                if (_option.Contains("PRH")) _cut = _cut &&  _lep;                
                if (_option.Contains("PRL")) _cut = _cut && !_lep;

                TCut _addCuts = TCut(NOCUT);
                TCut _rhoC = MatchMotherID("Pi", PDG::ID::Rho_c);
                TCut _rhoZ = MatchMotherID("Pi", PDG::ID::Rho0);
                TCut _omega= MatchMotherID("Pi", PDG::ID::Omega);
                if( _option.Contains("rho")){
                    if( _option.Contains("norho")){                        
                        if(      _option.Contains("norhoC")) _addCuts = !_rhoC; 
                        else if(      _option.Contains("norhoZ")) _addCuts = !_rhoZ; 
                        else _addCuts = !(_rhoC || _rhoZ);
                    }else{
                        if(      _option.Contains("rhoC")) _addCuts = _rhoC; 
                        else if(      _option.Contains("rhoZ")) _addCuts = _rhoZ; 
                        else _addCuts = (_rhoC || _rhoZ);
                    }
                }
                if( _option.Contains("omega")){
                    if(_option.Contains("noomega")) _addCuts = !_omega;
                    else _addCuts = _omega; 
                }
                if( IsCut(_addCuts)) _cut = _cut && _addCuts;
            }
            else if (_q2bin == Q2Bin::Psi){
                //TODO : review it! 
                if (_sample.BeginsWith("Bd")){
            		//Veto Bd2KstPsiLL and Bd2KPiPsi
                    TCut _notPsiMode  = !TruthMatching(_prj, _ana, _q2bin, "Bd2KstPsi" + to_string(_ana), _option, _debug, true) && !TruthMatching(_prj, _ana, _q2bin, "Bd2KPiPsi" + to_string(_ana), _option, _debug, true);
        		    //Veto leakage
                    TCut _notJPsiMode = !TruthMatching(_prj, _ana, _q2bin, "Bd2KstJPs" + to_string(_ana), _option, _debug, true) && !TruthMatching(_prj, _ana, _q2bin, "Bd2KPiJPs" + to_string(_ana), _option, _debug, true);
                    _cut = _cut && _notPsiMode && _notJPsiMode;
                }
                //Veto Bs2KstPsi and Bs2PhiPsi (already included as dedicated component!)
                if (_sample.BeginsWith("Bs")) _cut = _cut && !TruthMatching(_prj, _ana, _q2bin, "Bs2KstPsi" + to_string(_ana), _option, _debug, true) && !TruthMatching(_prj, _ana, _q2bin, "Bs2PhiPsi" + to_string(_ana), _option, _debug, true);;
                // TODO ( not ready to veto this until we have new MC for Psi-> PiPIJPs Kst)
                TCut _psiToJPsi = MatchGrandMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID(_l2, PDG::ID::Psi) && MatchMotherID(_l1, PDG::ID::JPs) && MatchMotherID(_l2, PDG::ID::JPs);
                TCut _psiToX_XToJPsi = MatchGrandGrandMotherID(_l1, PDG::ID::Psi) && MatchGrandGrandMotherID(_l2, PDG::ID::Psi) && MatchMotherID(_l1, PDG::ID::JPs) && MatchMotherID(_l2, PDG::ID::JPs);

                if (_ana == Analysis::EE){
                    _cut = _cut && !_psiToJPsi && !_psiToX_XToJPsi;		
                }
                /*
                        if (_option.Contains("PRH")) _cut = _cut && !_psiToJPsi;
                        if (_option.Contains("PRL")) _cut = _cut && _psiToJPsi;
                */
            }
        }
    }

    if (_prj == Prj::RK) {
        if (_sample == "Bu2KMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::Bu) && MatchMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Bu) && MatchMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2KPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }
        if (_sample == "Bu2KPsiPiPiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Psi) && MatchGrandMotherID("E2", PDG::ID::Psi); }	

        if (_sample == "Bu2PiJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bu) && MatchGrandMotherID("M2", PDG::ID::Bu); }
        if (_sample == "Bu2PiJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu); }

        if (_sample == "Bu2KstEE")  { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst_c) && MatchGrandMotherID("K", PDG::ID::Bu) && MatchMotherID("E1", PDG::ID::Bu) && MatchMotherID("E2", PDG::ID::Bu); }
        
        if (_sample == "Lb2pKJPsEE"){ _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Lb)  && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Lb) && MatchGrandMotherID("E2", PDG::ID::Lb); }
        if (_sample == "Bu2KEtaPrimeGEE"){
            _cut = _headMatched  && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Bu) &&  MatchMotherID("E1", PDG::ID::Eta_prime) && MatchMotherID("E2", PDG::ID::Eta_prime) && MatchGrandMotherID("E1", PDG::ID::Bu) && MatchGrandMotherID("E2", PDG::ID::Bu);
        }
        if (_sample == "Bd2KstEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchMotherID("E1", PDG::ID::Bd) && MatchMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstJPsEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstJPsMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }        
        if (_sample == "Bd2KstPsiEE") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchGrandMotherID("E1", PDG::ID::Bd) && MatchGrandMotherID("E2", PDG::ID::Bd); }
        if (_sample == "Bd2KstPsiMM") { _cut = _headMatched && _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchGrandMotherID("M1", PDG::ID::Bd) && MatchGrandMotherID("M2", PDG::ID::Bd); }	
        if (_sample.Contains("2XJPs")){
            _cut = _headMatched && _finalStatesMatched2X && !TRUTHMATCHING::bkgCatSignal && TRUTHMATCHING::bkgCatPhysics;
            if (_q2bin == Q2Bin::JPsi){
                if (_sample.BeginsWith("Bu")){
                    TCut _catSelection = "Bp_BKGCAT < 70 && Bp_BKGCAT >=10";
                    //CAT10 : If cat is 10 , we want to keep the K+ ( ccbar ==> ee ) with ccbar != J/Psi                     
                    TCut _L1ChainMatch = MatchMotherID(_l1, PDG::ID::JPs ) && MatchGrandMotherID( _l1, PDG::ID::Bu) && MatchID(_l1, _l1ID); 
                    TCut _L2ChainMatch = MatchMotherID(_l2, PDG::ID::JPs ) && MatchGrandMotherID( _l2, PDG::ID::Bu) && MatchID(_l2, _l2ID);
                    TCut _HChainMatch  = MatchMotherID(_h1, PDG::ID::Bu ) &&  MatchID( _h1, PDG::ID::K);
                    TCut _HPiChainMatch= MatchMotherID(_h1, PDG::ID::Bu ) &&  MatchID( _h1, PDG::ID::Pi);
                    //Keep all 
                    TCut _cat40        = TCut("Bp_BKGCAT==40");
                    //Keep all 
                    TCut _cat20        = TCut("Bp_BKGCAT==20");
                    //In Cat 10 you want to Keep everything fully matchin decay with final state chains decay matching
                    TCut _cat10        = TCut("Bp_BKGCAT==10") && !(  _L1ChainMatch && _L2ChainMatch && _HChainMatch );
                    //CAT30 : If cat is 30 , we want to keep everything except the K to Pi swap which is modelled with the Bu2PiJPs decay directly
                    TCut _cat30        = TCut("Bp_BKGCAT==30") && !(  _L1ChainMatch && _L2ChainMatch && _HPiChainMatch );
                    //CAT50 : If cat is 50 , we want to remove as in cat10.
                    TCut _cat50        = TCut("Bp_BKGCAT==50")  && !(  _L1ChainMatch && _L2ChainMatch && _HChainMatch );
                    //CAT50 : If cat is 50 , we want to remove as in cat10 but allowing 1 ghost in the chain 
                    TCut _cat60        = TCut("Bp_BKGCAT==60") && !( 
                        (_L1ChainMatch && ( _HPiChainMatch || _HChainMatch)  && MatchID(_l2,0)) ||  //l2 is the ghost
                        (_L2ChainMatch && ( _HPiChainMatch || _HChainMatch)  && MatchID(_l1,0)) ||  //l1 is the ghost
                        (_L1ChainMatch && _L2ChainMatch && MatchID(_h1,0)) //kaon is the ghost                        
                    );                
                    _cut = _finalStatesMatched2X && TRUTHMATCHING::bkgCatPhysics && _catSelection && (
                        _cat10 || _cat30 || _cat40 || _cat20 || _cat50 || _cat60
                    );          

                    if( _option.Contains("tmCustomVetoPsi2JPsX")){
                        //Remove the B+ => Psi( -> J/Psi X ) K+ decays and all partners ...                    
                        TCut _l1ChainVeto = MatchID(_l1, _l1ID) && MatchMotherID(_l1, PDG::ID::JPs) && ( 
                                ( MatchGrandMotherID(_l1, PDG::ID::Psi) && MatchGrandGrandMotherID( _l1, PDG::ID::Bu) )
                                ||
                                ( MatchGrandGrandMotherID( _l1, PDG::ID::Psi) )
                        ); 

                        TCut _l2ChainVeto = MatchID(_l2, _l2ID) && MatchMotherID(_l2, PDG::ID::JPs) && ( 
                            (MatchGrandMotherID(_l2, PDG::ID::Psi) && MatchGrandGrandMotherID( _l2, PDG::ID::Bu) ) 
                            ||
                            (MatchGrandGrandMotherID(_l2, PDG::ID::Psi) ) 
                        );
                        TCut _h1ChainVeto = MatchID(_h1, PDG::ID::K)   && MatchMotherID(_h1,PDG::ID::Bu);
                        _cut = _cut && !( _l1ChainVeto && _l2ChainVeto && _h1ChainVeto );
                    }
                }
                // RENATO HACK when fitting with the Psi2XJPs decay in the sample list( in case you fit with the Psi2XJPs modelled and constrained...)
                // THIS MUST BE IN IN THAT SCENARIO, not the baseline so far...
                // if (_sample.BeginsWith("Bu")){
                //     if( _ana == Analysis::EE){
                //         auto _vetoPsi2JPsX = ! TruthMatching(_prj, _ana, _q2bin, "Bu2KPsiPiPiJPsEE", _option, _debug, false );
                //         MessageSvc::Warning("ADDING VETO FOR B+ -> K Psi(->J/Psi X) , EE final states in Part-reco sample ");
                //         _cut = _cut && _vetoPsi2JPsX;
                //     }
                // }
                TCut _lep = MatchGrandMotherID(_l1, _headID) && MatchGrandMotherID(_l2, _headID) && MatchMotherID(_l1, PDG::ID::JPs) && MatchMotherID(_l2, PDG::ID::JPs);
        		//----- IMPORTANT HERE, we may have to VETO the B+ => (Psi->JPs Pi Pi) K+
        		//----- AND BKGCAT10 has to be included to include B-> (Psi->ee) K+  (will be very very small...)                
                if (_option.Contains("PRH")) _cut = _cut &&  _lep;
                if (_option.Contains("PRL")) _cut = _cut && !_lep;
            }
            else if (_q2bin == Q2Bin::Psi){
                auto matchLMothers= [&]( const int & pdgID,   bool posLogic){ 
                    TCut _llmatch = MatchMotherID(_l1, pdgID) && MatchMotherID(_l2, pdgID);
                    return posLogic? _llmatch : !_llmatch; 
                };
                auto matchLGMothers= [&]( const int & pdgID,  bool posLogic){ 
                    TCut _llmatch = MatchGrandMotherID(_l1, pdgID) && MatchGrandMotherID(_l2, pdgID);
                    return posLogic? _llmatch : !_llmatch; 
                };
                auto matchLGGMothers= [&]( const int & pdgID, bool posLogic){ 
                    TCut _llmatch = MatchGrandGrandMotherID(_l1, pdgID) && MatchGrandGrandMotherID(_l2, pdgID);
                    return posLogic? _llmatch : !_llmatch; 
                };
                //JPsi -> LL 
                TCut _LL_from_JPs           = matchLMothers(PDG::ID::JPs, true);
                //Psi2S -> LL
                TCut _LL_from_Psi           = matchLMothers(PDG::ID::Psi, true);
                //B-> (Psi2S -> LL ) X 
                TCut _LL_from_Xc            = matchLMothers(PDG::ID::JPs, false) && matchLMothers( PDG::ID::Psi, false);
                //Psi->LL and Psi from B directly
                TCut _LL_from_Psi_PsifromB  = _LL_from_Psi && matchLGMothers(_headID,true);
                //JPs->LL and JPs from resonance which is not a Psi
                TCut _JPs_from_Psi          = _LL_from_JPs && matchLGMothers(PDG::ID::Psi, true);
                //JPsi->LL and JPsi from a Psi in cascade
                TCut _JPs_from_PsiCascade   = _LL_from_JPs && matchLGGMothers(PDG::ID::Psi, true);
                //JPsi->LL and JPsi from not a Psi or a Psi in cascade
                TCut _JPs_from_Other        = _LL_from_JPs && matchLGMothers(PDG::ID::Psi, false) && matchLGGMothers(PDG::ID::Psi, false);
                //Psi->LL and Psi from other
                TCut _Psi_from_Other        = _LL_from_Psi && ( matchLGMothers(_headID, false) || matchLGGMothers(_headID, false));
                //B-> H X RES->( Y LL)
                TCut _H_fromB               =  MatchMotherID(_h1, _headID);
                //B-> STR(->H X) RES->( Y LL)
                TCut _H_fromResonance       = !_H_fromB && ( MatchGrandMotherID(_h1,_headID ) || MatchGrandGrandMotherID(_h1,_headID));
                if (_sample.BeginsWith("Bu")){
                    //B+ -> K+(Y) (Z-> Psi/JPs->EE)
                    //TODO : full review with the new knowledge we have on vetoes with DTF_Psi/JPs cuts and the necessity of including K from B with an extra pi0 . 
                    TCut _notSignalMode =  !TruthMatching(_prj, _ana, _q2bin, "Bu2KPsi" + to_string(_ana), _option, _debug, true); //remove         Bu2KPsi on RK
                    TCut _notJPsiMode   =  !TruthMatching(_prj, _ana, _q2bin, "Bu2KJPs" + to_string(_ana), _option, _debug, true); //remove leakage Bu2KJPs on RK
                    _cut = _cut && _notSignalMode; //also for Muon mode
                    TCut _isPartRecoH   = _LL_from_Psi_PsifromB && !_H_fromB;            
                    if (_ana == Analysis::EE){                        
                        _cut = _cut && _notSignalMode && _notJPsiMode; 
                        //We model the Psi-> XJPs with intermediate resonances with PiPi JPs
                        TCut _backgroundConsidered =  _JPs_from_Psi || _JPs_from_PsiCascade || _JPs_from_Other || _Psi_from_Other || _LL_from_Xc;
                        TCut _backgroundVetoed     = _H_fromB  && ( _JPs_from_Psi || _JPs_from_PsiCascade);
                        _cut = _cut && _backgroundConsidered && !_backgroundVetoed;
                    }
                }
                if( _sample.BeginsWith("Bd" && _ana == Analysis::EE )){
                    //B0 -> K+(Y-) (Z->Psi/Jps->EE)  , must veto PartReco only here i think 
                    TCut _kPartReco     = MatchMotherID(_h1, PDG::ID::Kst); //we wil keep everything which is has not a Psi->LL direct decay. Part-Reco from 1 missing particle will not show up i think .      
                    TCut _backgroundConsidered =  (_JPs_from_Psi || _JPs_from_PsiCascade || _JPs_from_Other || _Psi_from_Other || _LL_from_Xc );
                    _cut = _cut && !_kPartReco && (_backgroundConsidered);
                }
                //remove Bu2Psi(->pipiJ/Psi)K since it's modelled!
                // TCut _psiToJPsi = MatchGrandMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID(_l2, PDG::ID::Psi) && MatchMotherID(_l1, PDG::ID::JPs) && MatchMotherID(_l2, PDG::ID::JPs); 
                /*
                if (_option.Contains("PRH")) _cut = _cut && !_psiToJPsi;
                if (_option.Contains("PRL")) _cut = _cut && _psiToJPsi;
                */
            }
        }
    }

    if (_prj == Prj::RPhi) {
        if (_sample == "Bs2PhiMM") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("M1", PDG::ID::Bs) && MatchMotherID("M2", PDG::ID::Bs) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiEE") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("E1", PDG::ID::Bs) && MatchMotherID("E2", PDG::ID::Bs) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiJPsMM") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiJPsEE") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiPsiMM") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bs2PhiPsiEE") { _cut = _finalStatesMatchedHH && _finalStatesMatchedLL && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchMotherID("K1", PDG::ID::Phi) && MatchMotherID("K2", PDG::ID::Phi) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }

        if (_sample == "Bd2KstMM") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::Pi)) || (MatchID("K1", PDG::ID::Pi) && MatchID("K2", PDG::ID::K))) && MatchMotherID("M1", PDG::ID::Bs) && MatchMotherID("M2", PDG::ID::Bs) && MatchMotherID("K1", PDG::ID::Kst) && MatchMotherID("K2", PDG::ID::Kst) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs); }
        if (_sample == "Bd2KstEE") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::Pi)) || (MatchID("K1", PDG::ID::Pi) && MatchID("K2", PDG::ID::K))) && MatchMotherID("E1", PDG::ID::Bs) && MatchMotherID("E2", PDG::ID::Bs) && MatchMotherID("K1", PDG::ID::Kst) && MatchMotherID("K2", PDG::ID::Kst) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs); }
        if (_sample == "Bd2KstJPsMM") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::Pi)) || (MatchID("K1", PDG::ID::Pi) && MatchID("K2", PDG::ID::K))) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Kst) && MatchMotherID("K2", PDG::ID::Kst) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("M1", PDG::ID::Bs) && MatchGrandMotherID("M2", PDG::ID::Bs); }
        if (_sample == "Bd2KstJPsEE") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::Pi)) || (MatchID("K1", PDG::ID::Pi) && MatchID("K2", PDG::ID::K))) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Kst) && MatchMotherID("K2", PDG::ID::Kst) && MatchGrandMotherID("K1", PDG::ID::Bs) && MatchGrandMotherID("K2", PDG::ID::Bs) && MatchGrandMotherID("E1", PDG::ID::Bs) && MatchGrandMotherID("E2", PDG::ID::Bs); }

        if (_sample == "Lb2pKMM") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("M1", PDG::ID::Lb) && MatchMotherID("M2", PDG::ID::Lb) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb); }
        if (_sample == "Lb2pKEE") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("E1", PDG::ID::Lb) && MatchMotherID("E2", PDG::ID::Lb) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb); }
        if (_sample == "Lb2pKJPsMM") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("M1", PDG::ID::JPs) && MatchMotherID("M2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb) && MatchGrandMotherID("M1", PDG::ID::Lb) && MatchGrandMotherID("M2", PDG::ID::Lb); }
        if (_sample == "Lb2pKJPsEE") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("E1", PDG::ID::JPs) && MatchMotherID("E2", PDG::ID::JPs) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb) && MatchGrandMotherID("E1", PDG::ID::Lb) && MatchGrandMotherID("E2", PDG::ID::Lb); }
        if (_sample == "Lb2pKPsiMM") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("M1", PDG::ID::Psi) && MatchMotherID("M2", PDG::ID::Psi) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb) && MatchGrandMotherID("M1", PDG::ID::Lb) && MatchGrandMotherID("M2", PDG::ID::Lb); }
        if (_sample == "Lb2pKPsiEE") { _cut = _finalStatesMatchedLL && ((MatchID("K1", PDG::ID::K) && MatchID("K2", PDG::ID::P)) || (MatchID("K1", PDG::ID::P) && MatchID("K2", PDG::ID::K))) && MatchMotherID("E1", PDG::ID::Psi) && MatchMotherID("E2", PDG::ID::Psi) && MatchMotherID("K1", PDG::ID::Lb) && MatchMotherID("K2", PDG::ID::Lb) && MatchGrandMotherID("E1", PDG::ID::Lb) && MatchGrandMotherID("E2", PDG::ID::Lb); }

        if (_sample.Contains("2XJPs")) {
    	    _cut = _headMatched && _finalStatesMatched2X && !TRUTHMATCHING::bkgCatSignal;
            TCut _lep = MatchGrandMotherID(_l1, _headID) && MatchGrandMotherID(_l2, _headID) && MatchMotherID(_l1, PDG::ID::JPs) && MatchMotherID(_l2, PDG::ID::JPs);
            if (_option.Contains("PRH")) _cut = _cut && _lep;
            if (_option.Contains("PRL")) _cut = _cut && !_lep;
            if (_sample.BeginsWith("Bs")) _cut = _cut && !TruthMatching(_prj, _ana, _q2bin, "Bs2PhiJPs" + to_string(_ana), _option, _debug, true) && !TruthMatching(_prj, _ana, _q2bin, "Bs2KKJPs" + to_string(_ana), _option, _debug, true);
        }
    }

    if (_debug) MessageSvc::Debug("TruthMatching", &_cut);

    if (_cut == TCut(NOCUT)) MessageSvc::Error("TruthMatching", to_string(_prj), to_string(_ana), _sample, "not supported", "EXIT_FAILURE");

    return _cut;
}

TCut TruthMatching(const ConfigHolder & _configHolder, TString _option, bool _debug) {
    MessageSvc::Info("TruthMatching sample ", _configHolder.GetSample());
    MessageSvc::Info("TruthMatching option ", _option);

    TCut _cut(NOCUT);
    if (_option.Contains("tm") && !_option.Contains("tmSig") && !_option.Contains("tmSwap") && !_option.Contains("tmBkg") && !_option.Contains("tmCustom")) {
        //general truth matching ("tm" as global)
        if (_configHolder.IsSignalMC()) {
            MessageSvc::Info("TruthMatching IsSignalMC TRUE ");
            _cut = TRUTHMATCHING::bkgCatSig2;
            if (_option.Contains("tmTight")) _cut = TRUTHMATCHING::bkgCatSignal;
            if (_option.Contains("tmNoGhost")) _cut = TRUTHMATCHING::bkgCatSig1;
        } else {            
            MessageSvc::Info("TruthMatching IsSignalMC FALSE ");
            _cut = TRUTHMATCHING::bkgCatBkg;
        }
        if (_option.Contains("tmIncludeSwap")) _cut = _cut || TRUTHMATCHING::bkgCatSwap;
    } else {
        MessageSvc::Info("CutOption doesnt contain tmStuff ");
        if (_option.Contains("tmSig")) _cut = TRUTHMATCHING::bkgCatSig2;
        if (_option.Contains("tmSigTight")) _cut = TRUTHMATCHING::bkgCatSignal;
        if (_option.Contains("tmSigNoGhost")) _cut = TRUTHMATCHING::bkgCatSig1;
        if (_option.Contains("tmSwap")) _cut = TRUTHMATCHING::bkgCatSwap;
        if (_option.Contains("tmBkg")) _cut = TRUTHMATCHING::bkgCatBkg;
        if (_option.Contains("tmIncludeSwap")) _cut = _cut || TRUTHMATCHING::bkgCatSwap;
        if (_option.Contains("tmIncludeGhost")) _cut = _cut || TRUTHMATCHING::bkgCatGhost;
        if (_option.Contains("tmCustom")){
    	    //TODO : swap and non swap included?
            if (_option.Contains("tmCustomSwap"))
                _cut = TruthMatching(_configHolder.GetProject(), _configHolder.GetAna(), _configHolder.GetQ2bin(), _configHolder.GetSample(), _option, _debug, true);
            else{
                //tmCustom-tmCustomWithSwap ( tmCustomWithSwap will enable swaps in OR )
                _cut = TruthMatching(_configHolder.GetProject(), _configHolder.GetAna(), _configHolder.GetQ2bin(), _configHolder.GetSample(), _option, _debug, false);	    
            }
        }
    }
    if( _option.Contains("VetoGhost")){
        _cut = _cut && !TRUTHMATCHING::bkgCatGhost;
    }
    if( _option.Contains("OneGhost")){
        map< pair<Prj, Analysis>, TCut > _extraTMCut = { { {Prj::RK, Analysis::EE},   TCut("( ( (E1_TRUEID==0) + (E2_TRUEID==0) + (K_TRUEID==0) <=1 ) )")},
                                                         { {Prj::RK, Analysis::MM},   TCut("( ( (M1_TRUEID==0) + (M2_TRUEID==0) + (K_TRUEID==0) <=1 ) )")},
                                                         { {Prj::RKst, Analysis::EE}, TCut("( ( (E1_TRUEID==0) + (E2_TRUEID==0) + (K_TRUEID==0) + (Pi_TRUEID==0) <=1 ) )")},
                                                         { {Prj::RKst, Analysis::MM}, TCut("( ( (M1_TRUEID==0) + (M2_TRUEID==0) + (K_TRUEID==0) + (Pi_TRUEID==0) <=1 ) )")},
                                                         { {Prj::RPhi, Analysis::EE}, TCut("( ( (E1_TRUEID==0) + (E2_TRUEID==0) + (K1_TRUEID==0) + (K2_TRUEID==0) <=1 ) )")},
                                                         { {Prj::RPhi, Analysis::MM}, TCut("( ( (M1_TRUEID==0) + (M2_TRUEID==0) + (K1_TRUEID==0) + (K2_TRUEID==0) <=1 ) )")}
        };
        _extraTMCut.at( make_pair( _configHolder.GetProject(), _configHolder.GetAna()));
        _cut = _cut &&  _extraTMCut.at( make_pair( _configHolder.GetProject(), _configHolder.GetAna()));
    }
    if( _option.Contains("OneGhostNMatches")){ // && _option.Contains("tmSig")){    
        TCut _signalLikeness = TCut(Form("SignalLikeness>=%i", _configHolder.GetNBodies()-1));   //not entirely correct here to be clear, since you don't include the swaps. 
    	if(_option.Contains("CustomSignalLikeness")){
            map<  TString, TString > idMatch{   { "L1" , Form("TMath::Abs({L1}_TRUEID) == %i",  _configHolder.GetAna() == Analysis::EE ?  PDG::ID::E : PDG::ID::M) },
                                                { "L2" , Form("TMath::Abs({L2}_TRUEID) == %i",  _configHolder.GetAna() == Analysis::EE ?  PDG::ID::E : PDG::ID::M) }, 
                                                { "H1",  Form("TMath::Abs({H1}_TRUEID) == %i",  PDG::ID::K) } , 
                                                { "H2",  Form("TMath::Abs({H2}_TRUEID) == %i", _configHolder.GetProject() == Prj::RPhi ? PDG::ID::K : PDG::ID::Pi) }
            };
            if( _configHolder.GetAna() == Analysis::EE){
                idMatch["L1"] = idMatch["L1"].ReplaceAll("{L1}","E1");
                idMatch["L2"] = idMatch["L2"].ReplaceAll("{L2}","E2");
            }else if ( _configHolder.GetAna() == Analysis::MM){
                idMatch["L1"] = idMatch["L1"].ReplaceAll("{L1}","M1");
                idMatch["L2"] = idMatch["L2"].ReplaceAll("{L2}","M2");
            }
            if( _configHolder.GetAna() == Analysis::EE) idMatch["L1"] = idMatch["L1"].ReplaceAll("{L1}","E1");
            if( _configHolder.GetProject() == Prj::RK)        _signalLikeness = TCut( Form( " ((%s) + (%s) + (%s)       )  >= 2", idMatch.at("L1").Data(), idMatch.at("L2").Data(), idMatch.at("H1").Data()));	    
            else if( _configHolder.GetProject() == Prj::RKst) _signalLikeness = TCut( Form( " ((%s) + (%s) + (%s) + (%s))  >= 3", idMatch.at("L1").Data(), idMatch.at("L2").Data(), idMatch.at("H1").Data(), idMatch.at("H2").Data()));	    
            else if( _configHolder.GetProject() == Prj::RPhi) _signalLikeness = TCut( Form( " ((%s) + (%s) + (%s) + (%s))  >= 3", idMatch.at("L1").Data(), idMatch.at("L2").Data(), idMatch.at("H1").Data(), idMatch.at("H2").Data()));	  
            else MessageSvc::Warning("Custom Signal Likeness Not Implemented for Project asked");
        }
        auto _add_cut = _signalLikeness;
        _cut = _cut && _add_cut;
    }
    if( _option.Contains("MatchCat60Mothers") && _option.Contains("tmSig") && _configHolder.IsSignalMC() ){
        MessageSvc::Warning("Adding A filter on top for CAT60");
        TCut _l1Chain =  "1>0";
        TCut _l2Chain =  "1>0";
        switch (_configHolder.GetAna()){
            case Analysis::EE : {
                _l1Chain = MatchID("E1", PDG::ID::E);
                _l2Chain = MatchID("E2", PDG::ID::E);
                break;
            }
            case Analysis::MM : {
                _l1Chain = MatchID("M1", PDG::ID::M);
                _l2Chain = MatchID("M2", PDG::ID::M);
                break;
            }
            default :  MessageSvc::Error("Analysis switch not implemented (RK)"); break;
        }
        if( _configHolder.GetProject() == Prj::RK){ 
            TCut _hChainMatch = MatchID("K", PDG::ID::K) && MatchMotherID( "K", PDG::ID::Bu);                                           
            int _motherLeptons = PDG::ID::Bu;
            if( _configHolder.GetSample() == "Bu2KJPsEE" || _configHolder.GetSample() == "Bu2KJPsMM" ){
                _motherLeptons = PDG::ID::JPs; 
            }else if( _configHolder.GetSample() == "Bu2KPsiEE" || _configHolder.GetSample() == "Bu2KPsiMM" ){
                _motherLeptons = PDG::ID::Psi; 
            }else if( _configHolder.GetSample() == "Bu2KEE" || _configHolder.GetSample() == "Bu2KMM"){
                _motherLeptons = PDG::ID::Bu; 
            }else{
                MessageSvc::Error("TruthMatchingSvc, invalid switch on IsSignalMC() for RK", _configHolder.GetSample(), "EXIT_FAILURE");                        
            }
            _l1Chain = _l1Chain && MatchMotherID( _configHolder.GetAna()==Analysis::EE ? "E1" : "M1", _motherLeptons);
            _l2Chain = _l2Chain && MatchMotherID( _configHolder.GetAna()==Analysis::EE ? "E2" : "M2", _motherLeptons);
            TString _myMatching = TString::Format( "( (%s) + (%s) + (%s) ) >=2",  _l1Chain.GetTitle(), _l2Chain.GetTitle() , _hChainMatch.GetTitle() ); 
            _cut = _cut && _myMatching;                        
        }
        if( _configHolder.GetProject() == Prj::RKst){ 
            TCut _h1ChainMatch = MatchID("K", PDG::ID::K) && MatchMotherID( "K", PDG::ID::Kst) && MatchGrandMotherID("K", PDG::ID::Bd);
            TCut _h2ChainMatch = MatchID("Pi", PDG::ID::Pi) && MatchMotherID( "Pi", PDG::ID::Kst) && MatchGrandMotherID("Pi", PDG::ID::Bd);
            int _motherLeptons = PDG::ID::Bd;
            if( _configHolder.GetSample() == "Bd2KstJPsEE" || _configHolder.GetSample() == "Bd2KstJPsMM" ){
                _motherLeptons = PDG::ID::JPs; 
            }else if( _configHolder.GetSample() == "Bd2KstPsiEE" || _configHolder.GetSample() == "Bd2KstPsiMM" ){
                _motherLeptons = PDG::ID::Psi; 
            }else if( _configHolder.GetSample() == "Bd2KstEE" || _configHolder.GetSample() == "Bd2KstMM"){
                _motherLeptons = PDG::ID::Bd; 
            }else{
                MessageSvc::Error("TruthMatchingSvc, invalid switch on IsSignalMC() for RKst", _configHolder.GetSample());                        
            }
            _l1Chain = _l1Chain && MatchMotherID( _configHolder.GetAna()==Analysis::EE ? "E1" : "M1", _motherLeptons);
            _l2Chain = _l2Chain && MatchMotherID( _configHolder.GetAna()==Analysis::EE ? "E2" : "M2", _motherLeptons);
            TString _myMatching = TString::Format( "( (%s) + (%s) + (%s) + (%s) ) >=3",  _l1Chain.GetTitle(), _l2Chain.GetTitle() , _h1ChainMatch.GetTitle(), _h2ChainMatch.GetTitle() ); 
            _cut = _cut && _myMatching;                        
        }
    }
    /*
     TODO: finish prorotype 
    if( _option.Contains("GhostDecayMatch") && _option.Contains("tmSig")){
        // We label the di-lepton intermediate state as JPs, regardless of q2 and charmonium state
        TString _head, _hh, _h1, _h2, _ll;
        Prj      _prj    = _configHolder.GetProject();
        Analysis _ana    = _configHolder.GetAna();
        TString  _sample = _configHolder.GetSample();
        switch (_prj) {
            case Prj::RKst:
                _head = "B0";
                _hh   = "Kst";
                _h1   = "K";
                _h2   = "Pi";
                _ll = "JPs";
                break;
            case Prj::RK:
                _head = "Bp";
                _hh   = "";
                _h1   = "K";
                _h2   = "";
                _ll = "JPs";
                break;
            case Prj::RPhi:
                _head = "Bs";
                _hh   = "Phi";
                _h1   = "K1";
                _h2   = "K2";
                _ll = "JPs";
                break;
            default: MessageSvc::Error("TruthMatching (GhostDecayMatch)", (TString) "Invalid project", to_string(_prj), "EXIT_FAILURE"); break;
        }
        TString _l1, _l2;
        int     _l1ID = 0, _l2ID = 0;
        switch (_ana) {
            case Analysis::MM:
                _l1   = "M1";
                _l2   = "M2";
                _l1ID = PDG::ID::M;
                _l2ID = PDG::ID::M;
                break;
            case Analysis::EE:
                _l1   = "E1";
                _l2   = "E2";
                _l1ID = PDG::ID::E;
                _l2ID = PDG::ID::E;
                break;
            case Analysis::ME:
                _l1   = "M1";
                _l2   = "E2";
                _l1ID = PDG::ID::M;
                _l2ID = PDG::ID::E;
                break;
            default: MessageSvc::Error("TruthMatching (GhostDecayMatch)", (TString) "Invalid analysis", to_string(_ana), "EXIT_FAILURE"); break;
        }
        int _headID = 0;
        if (_sample.BeginsWith("Bd")) _headID = PDG::ID::Bd;
        if (_sample.BeginsWith("Bu")) _headID = PDG::ID::Bu;
        if (_sample.BeginsWith("Bs")) _headID = PDG::ID::Bs;
        if (_sample.BeginsWith("Lb")) _headID = PDG::ID::Lb; 
        TCut _cat60Cut   = TCut("Bp_BKGCAT==60");
        TCut _notCat60   = TCut("Bp_BKGCAT!=60");
        if( _prj == Prj::RK){            
            if( _sample.Contains("Bu2KJPs"){
                //J/Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)      && MatchMotherID(_l1, PDG::ID::JPs) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)      && MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K) && MatchMotherID(_h1, _headID); 
                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && MatchID( _h1, 0)) || 
                                      (_l1DecMatch && _h1DecMatch && MatchID( _l2, 0)) || 
                                      (_l2DecMatch && _h1DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);
            }
            else if( _sample.Contains("Bu2KPsi")){
                //Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)      && MatchMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)      && MatchMotherID(_l2, PDG::ID::Psi) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K) && MatchMotherID(_h1, _headID); 
                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && MatchID( _h1, 0)) || 
                                      (_l1DecMatch && _h1DecMatch && MatchID( _l2, 0)) || 
                                      (_l2DecMatch && _h1DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);                
            }
            else if( _sample.Contains("Bu2KEE") || _sample.Contains("Bu2KMM") ){
                //Rare mode signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)      && MatchMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)      && MatchMotherID(_l2, PDG::ID::Psi) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K) && MatchMotherID(_h1, _headID); 
                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && MatchID( _h1, 0)) || 
                                      (_l1DecMatch && _h1DecMatch && MatchID( _l2, 0)) || 
                                      (_l2DecMatch && _h1DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);       
            }
        }
        else if( _prj == Prj::RKst){
            if( _sample.Contains("Bd2KstJPs"){
                //J/Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, PDG::ID::JPs) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Kst) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::Pi) && MatchMotherID(_h2, PDG::ID::Kst) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);
            }
            else if( _sample.Contains("Bd2KstPsi")){
                //Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, PDG::ID::Psi) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Kst) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::Pi) && MatchMotherID(_h2, PDG::ID::Kst) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);         
            }
            else if( _sample.Contains("Bd2KstEE") || _sample.Contains("Bd2KstMM") ){
                //Rare mode signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Kst) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::Pi) && MatchMotherID(_h2, PDG::ID::Kst) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);          
            }            
        }
        else if( _prj == Prj::RPhi){
            if( _sample.Contains("Bs2PhiJPs"){
                //J/Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, PDG::ID::JPs) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, PDG::ID::JPs) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Kst) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::K) && MatchMotherID(_h2, PDG::ID::Kst) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);
            }
            else if( _sample.Contains("Bs2PhiPsi")){
                //Psi signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, PDG::ID::Psi) && MatchGrandMotherID( _l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, PDG::ID::Psi) && MatchGrandMotherID( _l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Phi) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::K)  && MatchMotherID(_h2, PDG::ID::Phi) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);         
            }
            else if( _sample.Contains("Bs2PhiEE") || _sample.Contains("Bs2PhiMM") ){
                //Rare mode signal sample
                TCut _l1DecMatch = MatchID( _l1 , _l1ID)       && MatchMotherID(_l1, _headID);
                TCut _l2DecMatch = MatchID( _l2 , _l1ID)       && MatchMotherID(_l2, _headID);
                TCut _h1DecMatch = MatchID( _h1 , PDG::ID::K)  && MatchMotherID(_h1, PDG::ID::Phi) && MatchGrandMotherID( _h1, _headID);
                TCut _h2DecMatch = MatchID( _h2 , PDG::ID::K)  && MatchMotherID(_h2, PDG::ID::Phi) && MatchGrandMotherID( _h2, _headID); 

                TCut _cat60Clean =  ( (_l1DecMatch && _l2DecMatch && _h2DecMatch && MatchID( _h1, 0)) ||
                                      (_l1DecMatch && _l2DecMatch && _h1DecMatch && MatchID( _h2, 0)) ||
                                      (_l1DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l2, 0)) ||
                                      (_l2DecMatch && _h1DecMatch && _h2DecMatch && MatchID( _l1, 0)) );
                _cut = _cut && (  (_cat60Clean && _cat60Cut ) || _notCat60);          
            }            
        }        
    }
    */
    return _cut;
}

#endif
