#ifndef FITTERSVC_CPP
#define FITTERSVC_CPP

#include "FitterSvc.hpp"

#include "ConstDef.hpp"
#include "CutDefRK.hpp"
#include "CutDefRKst.hpp"
#include "CutDefRPhi.hpp"

#include "MurmurHash3.h"

//m_shapeParameters = GetPars(m_pdf, m_var);

Str2VarMap GetPars(RooAbsPdf * pdf, RooArgSet obs, vector< string > pnames, TString _option) {
    Str2VarMap   out;
    RooArgSet *  params = pdf->getParameters(RooDataSet("v", "", obs));
    TIterator *  it     = params->createIterator();
    RooRealVar * arg;
    while ((arg = (RooRealVar *) it->Next())) {
        string                     complete_name = arg->GetName();
        int                        _pos          = complete_name.find("_");
        string                     varname       = complete_name.substr(0, _pos);
        vector< string >::iterator _it            = find(pnames.begin(), pnames.end(), varname);
        size_t                     posfor        = complete_name.find("__for");
        if (_option.Contains("orignames"))
            varname = (string) arg->GetName();
        else if (_option.Contains("nameandvar") && posfor != string::npos)
            varname += ("_" + complete_name.substr(posfor + 5, string::npos));
        if (_it != pnames.end() || pnames.empty()) out[varname] = arg;
    }
    return out;
}

Str2VarMap GetPars(RooAbsPdf * pdf, RooAbsReal * var, TString _option) { return GetPars(pdf, RooArgSet(*var), vector< string >(), _option); }

Str2VarMap FixPars(Str2VarMap * _map, vector< string > names, TString _option , double min, double max) {
    for (const auto _iter : *_map) {
        TString cname = _iter.first;
        bool    found = false;
        for (size_t i = 0; i < names.size(); i++) {            
            if ((names[i] == cname) || (cname.Contains(names[i]) && _option.Contains("contains"))) {
                found = true;
                break;
            }
        }
        if (_option.Contains("except")){
            if(_option.Contains("RangeUpdate") && found ){
                ((RooRealVar *)_iter.second)->setMin( min); 
                ((RooRealVar *)_iter.second)->setMax( max);
            }            
            found = !found;            
        }
        bool setconst = kTRUE;
        if (_option.Contains("free")) setconst = kFALSE;
        if (names.empty() || found){ 
            ((RooRealVar *)_iter.second)->setConstant(setconst);
        }
    }
    return *_map;
}

Str2VarMap FixPars(Str2VarMap * _map, string name, TString _option) { 
    return FixPars(_map, vector< string >(1, name), _option);
}

Str2VarMap ModifyPars(Str2VarMap * _map, vector< string > names, vector< RooRealVar * > c, vector< string > opt) {
    for (size_t i = 0; i < names.size(); i++) {
        string       parMapName = IsVarInMap(names[i], *_map, "");
        RooRealVar * par        = (RooRealVar *) ((*_map)[parMapName]);
        if (!par) {
            MessageSvc::Warning("ModifyPars", (TString) names[i], "not found");
            continue;
        }
        TString fname =  (TString) par->GetName();// + "__" + c[i]->GetName();
        if( c[i] != nullptr){
            fname = (TString) par->GetName() + "__" + c[i]->GetName();
        }else{
            fname = (TString) par->GetName() + "__NoPar";
        }
        string  fkey  = parMapName;
        if (opt[i].find("-n") != string::npos) {
            int posn    = opt[i].find("-n");
            int posdash = opt[i].find("-", posn + 2);
            fname += ("_" + opt[i].substr(posn + 2, posdash));
        }
        RooFormulaVar * fpar = nullptr;
        if (opt[i].find("shift") != string::npos) {
            if (opt[i].find("offset") != string::npos) {
                size_t par1 = opt[i].find("[");
                size_t par2 = opt[i].find("]");
                string val  = opt[i].substr(par1 + 1, par2 - par1 - 1);
                fpar        = new RooFormulaVar(fname + "_shifted_and_offsetted", ("@0+@1+" + val).c_str(), RooArgSet(*c[i], *par));
            } else
                fpar = new RooFormulaVar(fname + "_shifted", "@0+@1", RooArgSet(*c[i], *par));
        } else if (opt[i].find("offset") != string::npos) {
            size_t par1 = opt[i].find("[");
            size_t par2 = opt[i].find("]");
            string val  = opt[i].substr(par1 + 1, par2 - par1 - 1);
            fpar        = new RooFormulaVar(fname + "_offsettedOnly", ("@0+" + val).c_str(), RooArgSet(*par));
            MessageSvc::Info("offset applied alone");
            fpar->Print("v");
        } else if (opt[i].find("scale") != string::npos) {
            fpar = new RooFormulaVar(fname + "_scaled", "@0*@1", RooArgSet(*c[i], *par));
        } else if (opt[i].find("scalinv") != string::npos) {
            fpar = new RooFormulaVar(fname + "_scaledInv", "@1/@0", RooArgSet(*c[i], *par));
        }else if (opt[i].find("acon") != string::npos) {
            fpar = new RooFormulaVar(fname + "_acon", "sqrt(@0*@0 + @1*@1)", RooArgSet(*c[i], *par));
        } else {
            MessageSvc::Error("ModifyPars", (TString) "Invalid modify option (supported = -offset[number], -shift, -shift-offset[number], -scale, -acon)", "EXIT_FAILURE");
        }
        (*_map)[fkey] = fpar;
    }
    return *_map;
}

Str2VarMap ModifyPars(Str2VarMap * _map, string name, RooRealVar * c, string opt) {
    vector< string >       names(1, name);
    vector< RooRealVar * > vc(names.size(), c);
    vector< string >       vopt(names.size(), opt);
    return ModifyPars(_map, names, vc, vopt);
}

string GetOptionVal(string _option, string _string) {
    string _val = "";
    if (_option.find(_string) != string::npos) {
        size_t _par1 = _option.find(_string + "[");
        size_t _par2 = _option.find("]");
        _val         = _option.substr(_par1 + 1, _par2 - _par1 - 1);
    }
    return _val;
}

RooRealVar * AddPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap _map, string option) {
    RooRealVar * curpar = (RooRealVar *) stval_list[par];
    size_t       pos    = parstr.find(par + "[");

    string vname   = "";
    size_t posname = option.find("-n");
    if (posname != string::npos) vname = option.substr(posname + 2, option.find("-", posname + 2) - (posname + 2));

    string parMapName = "";
    if (_map.size() > 0) {
        parMapName = IsVarInMap(par, _map, vname);
        if (parMapName == "") MessageSvc::Warning("AddPar", (TString) par, (TString) parstr, (TString) option);
    }

    if (parMapName != ""){
        curpar = (RooRealVar *) _map[parMapName];
    }
    else if (pos != string::npos) {
        size_t endPar = parstr.find("]", pos);
        string s      = parstr.substr(pos + par.length() + 1, endPar - (pos + par.length() + 1));
        double par_v  = ((TString) s).Atof();
        size_t comma1 = s.find(',');
        if (comma1 != string::npos) {
            size_t comma2 = s.find(',', comma1 + 1);
            string smin   = s.substr(comma1 + 1, comma2 - comma1 - 1);
            double min    = ((TString) smin).Atof();
            string smax   = s.substr(comma2 + 1, endPar - comma2 - 1);
            double max    = ((TString) smax).Atof();
            curpar->setRange(min, max);
        }
	if( curpar->getMin() > par_v) curpar->setMin( par_v - 0.9 * std::abs(par_v));
	if( curpar->getMax() < par_v) curpar->setMax( par_v + 9 * std::abs(par_v));
        curpar->setVal(par_v);
        if (parstr.substr(pos - 1, 2).find("X") != string::npos) curpar->setConstant(kTRUE);
    }

    // if (!((TString) curpar->GetName()).Contains("shift") && !((TString) curpar->GetName()).Contains("scale") && !((TString) curpar->GetName()).Contains("offset")) curpar->setError(SettingDef::Fit::stepSizePar * TMath::Abs(curpar->getMax() - curpar->getMin()));
    if (TString(curpar->ClassName()) == "RooRealVar") curpar->setError(SettingDef::Fit::stepSizePar * TMath::Abs(curpar->getMax() - curpar->getMin()));

    return curpar;
}

TString GetPrintParName(TString namepdf_, string opt) {
    namepdf_        = namepdf_.ReplaceAll("__noprint__", "");
    namepdf_        = ((TString) namepdf_).ReplaceAll("bkg_", "");
    size_t  pos_    = ((string) namepdf_).find("_");
    TString namepdf = (TString)(((string) namepdf_).substr(0, pos_));
    TString nameana = (TString)((string) namepdf_).substr(pos_ + 1, string::npos);
    size_t  pos__   = ((string) nameana).find("__");
    nameana         = (TString)((string) nameana).substr(0, pos__);

    if (((string) namepdf_).find("sig") != string::npos) return "^{" + nameana + "}";

    TString pstrname = "_{" + namepdf + "}";
    if (opt.find("anaparlab") != string::npos) pstrname += "^{" + nameana + "}";
    return pstrname;
}

Str2VarMap GetStr2VarMap(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap _map, string opt, TString title) {
    Str2VarMap parout;
    if (typepdf_.find("Poly") != string::npos || typepdf_.find("Cheb") != string::npos) return parout;

    Str2VarMap stval_list;
    namepdf_ = namepdf_.ReplaceAll("__noprint__", "");
    if (title == "") title = namepdf_;
    TString pstrname = GetPrintParName(title, opt);

    //Mass parameters
    stval_list["m"]   = new RooRealVar("m_" + namepdf_, "m" + pstrname, val->getVal(), val->getMin(), val->getMax());
    stval_list["m2"]  = new RooRealVar("m2_" + namepdf_, "m_{2}" + pstrname, val->getVal(), val->getMin(), val->getMax());
    stval_list["m3"]  = new RooRealVar("m3_" + namepdf_, "m_{3}" + pstrname, val->getVal(), val->getMin(), val->getMax());

    stval_list["mg"]  = new RooRealVar("mg_" + namepdf_, "m_{gauss}" + pstrname, val->getVal(), val->getMin(), val->getMax());
    stval_list["mcb"] = new RooRealVar("mcb_" + namepdf_, "m_{cb}" + pstrname, val->getVal(), val->getMin(), val->getMax());
    stval_list["m0"]  = new RooRealVar("m0_" + namepdf_, "m_{0}" + pstrname, val->getVal(), val->getMin(), val->getMax());
    stval_list["mO"]   = new RooRealVar("mO_" + namepdf_, "mO" + pstrname, val->getVal(), 3500, 6500);
    //Sigma parameters
    stval_list["s"]   = new RooRealVar("s_" + namepdf_, "\\sigma" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["s1"]  = new RooRealVar("s1_" + namepdf_, "\\sigma_{1}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["s2"]  = new RooRealVar("s2_" + namepdf_, "\\sigma_{2}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["s3"]  = new RooRealVar("s3_" + namepdf_, "\\sigma_{3}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["sg"]  = new RooRealVar("sg_" + namepdf_, "\\sigma_{gauss}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["scb"] = new RooRealVar("scb_" + namepdf_, "\\sigma_{cb}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["sE"]  = new RooRealVar("sE_" + namepdf_, "\\sigma_{eff}" + pstrname, 2e-3, 1e-5, 5e-1);
    
    stval_list["sR"]  = new RooRealVar("sR_" + namepdf_, "\\sigma_{gauss}(R)" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["sL"]  = new RooRealVar("sL_" + namepdf_, "\\sigma_{gauss}(L)" + pstrname, 1e2, 1e-2, 1e3);

    //Tail parameters alpha
    stval_list["a"]   = new RooRealVar("a_" + namepdf_, "\\alpha" + pstrname, 1, 1e-3, 1e2);
    stval_list["a2"]  = new RooRealVar("a2_" + namepdf_, "\\alpha_{2}" + pstrname, 1, 1e-3, 1e2);
    stval_list["a3"]  = new RooRealVar("a3_" + namepdf_, "\\alpha_{3}" + pstrname, -1, -1e2, -1e-3);
    stval_list["acb"] = new RooRealVar("acb_" + namepdf_, "\\alpha_{cb}" + pstrname, 1, 1e-3, 1e2);

    stval_list["aL"]   = new RooRealVar("aL_" + namepdf_, "\\alpha(L)" + pstrname, 1, 1e-3, 1e2);
    stval_list["aR"]   = new RooRealVar("aR_" + namepdf_, "\\alpha(R)" + pstrname, 1, 1e-3, 1e2);

    //Tail parameters
    stval_list["n"]   = new RooRealVar("n_" + namepdf_, "n" + pstrname, 1, 0.01, 1e3);
    stval_list["n2"]  = new RooRealVar("n2_" + namepdf_, "n_{2}" + pstrname, 1, 0.01, 1e3);
    stval_list["n3"]  = new RooRealVar("n3_" + namepdf_, "n_{3}" + pstrname, 1, 0.01, 1e3);
    stval_list["ncb"] = new RooRealVar("ncb_" + namepdf_, "n_{cb}" + pstrname, 1, 0.01, 1e3);
    
    stval_list["nL"]   = new RooRealVar("nL_" + namepdf_, "n(L)" + pstrname, 1, 0.01, 1e3);
    stval_list["nR"]   = new RooRealVar("nR_" + namepdf_, "n(R)" + pstrname, 1, 0.01, 1e3);

    //Combinatorial parameters
    stval_list["b"] = new RooRealVar("b_" + namepdf_, "b" + pstrname, -1e-3, -1., 0.);
    stval_list["c"] = new RooRealVar("c_" + namepdf_, "c" + pstrname, -1, -1e2, 0.); //
    stval_list["g"] = new RooRealVar("g_" + namepdf_, "\\gamma" + pstrname, 30, 5., 1e2); //For B-Wigner
    stval_list["p"] = new RooRealVar("p_" + namepdf_, "p" + pstrname, 1, 0., 1e2);
    
    //Generic parameters
    stval_list["c1"] = new RooRealVar("c1"+ namepdf_, "c1" + pstrname, 0, -1e9, 1e9);
    stval_list["c2"] = new RooRealVar("c2"+ namepdf_, "c2" + pstrname, 0, -1e9, 1e9);

    //Fraction parameters
    stval_list["f"]   = new RooRealVar("f_"  + namepdf_, "f"     + pstrname, 0.5, 0., 1.);
    stval_list["f2"]  = new RooRealVar("f2_" + namepdf_, "f_{2}" + pstrname, 0.5, 0., 1.);
    stval_list["f3"]  = new RooRealVar("f3_" + namepdf_, "f_{3}" + pstrname, 0.5, 0., 1.);

    stval_list["fg"]  = new RooRealVar("fg_" + namepdf_, "f_{gauss}" + pstrname, 0.5, 0., 1.);
    stval_list["fcb"] = new RooRealVar("fcb_" + namepdf_, "f_{cb}" + pstrname, 0.5, 0., 1.);

    //Ipatia tail parameters
    stval_list["l"] = new RooRealVar("l_" + namepdf_, "l" + pstrname, -5, -10., -1.);
    stval_list["z"] = new RooRealVar("z_" + namepdf_, "z" + pstrname, 0.005, 0., 0.01);
    
    //JohnsonPar only
    stval_list["nu"]  = new RooRealVar("nu_" + namepdf_, "\\nu" + pstrname, 0., -100., 100.);
    stval_list["tau"] = new RooRealVar("tau_" + namepdf_, "\\tau" + pstrname, 1., 0., 1000.);

    //Landau only 
    stval_list["sl"]  = new RooRealVar("sl_" + namepdf_, "sl" + pstrname, 1, 0.1, 1e3);

    map< string, vector< string > > par_list;

    vector< string > ArgusPar{"m0", "p", "c"};
    vector< string > BGausPar{"m", "s1", "s2"};
    vector< string > BreitWignerPar{"m", "s", "g"};
    vector< string > CBGaussPar{"m", "s", "a", "n", "sg", "fg"};
    vector< string > CBPar{"m", "s", "a", "n"};
    vector< string > DCBGaussPar_Sn{"m", "s", "s2", "f", "a", "a2", "n", "sg", "fg"};
    vector< string > DCBGaussPar_Ss{"m", "s", "f", "a", "a2", "n", "n2", "sg", "fg"};
    vector< string > DCBGaussPar{"m", "s", "s2", "f", "a", "a2", "n", "n2", "sg", "fg"};
    vector< string > DCBPar_Sn{"m", "s", "s2", "f", "a", "a2", "n"};
    vector< string > DCBPar_Ss{"m", "s", "f", "a", "a2", "n", "n2"};
    vector< string > DCBPar_Stail_mean{"m", "s", "f", "a", "n", "s2"};

    vector< string > DCBPar{"m", "s", "s2", "f", "a", "a2", "n", "n2"};
    vector< string > DGausPar{"m", "s", "s2", "f"};
    vector< string > DSCBPar{"m", "s", "a", "n", "a2", "n2"};

    vector< string > BifurDSCBPar{"m", "sL", "sR", "aL", "nL", "aR", "nR"};

    vector< string > DSCBGaussPar{"m", "s", "a", "n", "a2", "n2", "sg", "fg"};

    vector< string > ExpAGaussPar{"m", "s", "b"};
    vector< string > ExpCGaussPar{"s", "b"};
    vector< string > ExpPar{"b"};
    vector< string > ExpTailDGPar{"m", "s", "s2", "a", "a2", "f"};
    vector< string > ExpTailTGPar{"m", "s", "s2", "a", "a2", "mg", "sg", "f", "fg"};
    vector< string > ExpTurnOnPar{"mO", "sE", "b"};
    vector< string > GammaPar{    "g", "b", "m"};
    vector< string > GausPar{     "m", "s"};
    vector< string > Ipatia2Par{  "m", "s", "b", "l", "z", "a", "n", "a2", "n2"};
    vector< string > IpatiaPar{   "m", "s", "b", "l", "z", "a", "n"};
    vector< string > JohnsonPar{  "m", "s", "nu", "tau"};
    vector< string > TCBPar_Sn{   "m", "s", "s2", "s3", "f", "f2", "a", "a2", "a3", "n"};
    vector< string > TCBPar_Ss{   "m", "s", "f", "f2", "a", "a2", "a3", "n", "n2", "n3"};
    vector< string > TCBPar{      "m", "s", "s2", "s3", "f", "f2", "a", "a2", "a3", "n", "n2", "n3"};
    vector< string > TGausPar{    "m", "s", "s2", "s3", "f", "f2"};

    vector< string > DGAndExpPar{ "m", "s", "m2", "s2", "b", "f", "fg"};
    vector< string > DSCBDGaussPar{ "m", "s", "a", "n", "a2", "n2" , "mg", "sg", "fg", "m2", "s2","f2"};
    vector< string > CBDGaussPar{   "m", "s", "a", "n", "mg", "sg", "fg", "m2", "s2","f2"};
    vector< string > LandauPar{"m0", "sl"};
    vector< string > LinearPar{"c1", "c2"};
    vector< string > LandauAndExpPar{"m0", "sl", "b", "f"};

    vector< string > TGaussAndExpPar{ "m", "m2","m3","s", "s2", "s3", "b", "f", "f2", "f3"};
    vector< string > DGaussAndExpPar{ "m", "m2","s", "s2", "b", "f", "f2"};

    vector< string> ExpTurnOnGauss{  "mO", "sE", "b", "m", "s", "f"};
    vector< string> ExpTurnOnDGauss{ "mO", "sE", "b", "m", "s", "m2","s2", "f", "f2"};
    par_list["Argus"]       = ArgusPar;
    par_list["BGauss"]      = BGausPar;
    par_list["BreitWigner"] = BreitWignerPar;
    par_list["CB"]          = CBPar;
    par_list["CBGauss"]     = CBGaussPar;
    par_list["DCB"]         = DCBPar;
    par_list["DCBGauss"]    = DCBGaussPar;
    par_list["DCBGauss_Sn"] = DCBGaussPar_Sn;
    par_list["DCBGauss_Ss"] = DCBGaussPar_Sn;
    par_list["DCB_Sn"]      = DCBPar_Sn;
    par_list["DCB_Ss"]      = DCBPar_Ss;
    par_list["DCB_Ss"]      = DCBPar_Ss;
    par_list["DCB_Stail_mean"]= DCBPar_Stail_mean;
    par_list["DGauss"]      = DGausPar;
    par_list["DSCB"]        = DSCBPar;
    par_list["BifurDSCB"]   = BifurDSCBPar;

    par_list["DSCBGauss"]   = DSCBGaussPar;
    par_list["CBDGauss"]    = CBDGaussPar; //CB   + Gauss1( free mean, sigma) + Gauss2 (free mean, sigma)
    par_list["DSCBDGauss"]    = DSCBDGaussPar; //DSCB + Gauss1( free mean, sigma) + Gauss2 (free mean, sigma)
    par_list["Exp"]         = ExpPar;
    par_list["ExpAGauss"]   = ExpAGaussPar;
    par_list["ExpCGauss"]   = ExpCGaussPar;
    par_list["ExpTailDG"]   = ExpTailDGPar;
    par_list["ExpTailTG"]   = ExpTailTGPar;
    par_list["ExpTurnOn"]   = ExpTurnOnPar;
    par_list["Gamma"]       = GammaPar;
    par_list["Gauss"]       = GausPar;
    par_list["InvArgus"]    = ArgusPar;
    par_list["Ipatia"]      = IpatiaPar;
    par_list["Ipatia2"]     = Ipatia2Par;
    par_list["Johnson"]     = JohnsonPar;
    par_list["TCB"]         = TCBPar;
    par_list["TCB_Sn"]      = TCBPar_Sn;
    par_list["TCB_Ss"]      = TCBPar_Ss;
    par_list["TGauss"]      = TGausPar;
    par_list["TGaussAndExp"] = TGaussAndExpPar;
    par_list["DGaussAndExp"] = DGaussAndExpPar;

    par_list["DGAndExp"]    = DGAndExpPar;
    par_list["Landau"]      = LandauPar;
    par_list["Linear"]      = LinearPar;
    par_list["LandauAndExp"]= LandauAndExpPar;
    par_list["ExpTurnOnGauss"] = ExpTurnOnGauss;
    par_list["ExpTurnOnDGauss"] = ExpTurnOnDGauss;

    auto GetTypePDF = [&]( std::string inputFullString){
        /*
            Simple Function that given the full option it identify the "MAIN" PDF to use
        */
        TString _myPDFString(inputFullString);
        _myPDFString = _myPDFString.Tokenize("-")->At(0)->GetName();        

        /* 
            ORDER MATTER, that's quite crappy, that's life :( 
        */
        //DSCBAndDGAuss_Sm
        _myPDFString = _myPDFString.ReplaceAll("AndCB", "")
                                   .ReplaceAll("AndGauss", "")                                   
                                   .ReplaceAll("AndDGauss","")
                                   .ReplaceAll("AndBiFurGauss","");
        //those are pdfs with already fully configured parameter sharing setup 
        //the rest _Sx configurations are for the AndXXX_S... setups
        std::vector< TString > _pdfs_with_configDirect = { 
            "DCBGauss_Sn",
            "DCBGauss_Ss",
            "DCB_Sn",
            "DCB_Ss",
            "DCB_Stail_mean",
            "TCB_Sn",
            "TCB_Ss"
        };
        bool _is_config_in_AND = false ; 
        for( auto & el : _pdfs_with_configDirect){
            if(_myPDFString.Contains(el)){ _is_config_in_AND = true; break;}
        }
        if( !_is_config_in_AND){
            //possible setups in AndXXX_Scc , to strip out  
            _myPDFString = _myPDFString.ReplaceAll("_Sms", "")
                                       .ReplaceAll("_Sm2", "")
                                       .ReplaceAll("_Sm", "")
                                       .ReplaceAll("_Ss" , "");
        }
        std::string spdf = _myPDFString.Data();
        return spdf;
    };

    TString _FULLPDFString(typepdf_);
    bool plusgaus       = _FULLPDFString.Contains("AndGauss");
    bool plusdgaus       = _FULLPDFString.Contains("AndDGauss");
    bool plusBiFurgaus  = _FULLPDFString.Contains("AndBiFurGauss");
    bool pluscb         = _FULLPDFString.Contains("AndCB");  
    typepdf_ = GetTypePDF( typepdf_);
    std::string parstr    =  ""; 
    if( _FULLPDFString.Contains("-")){
        TString _toLook(_FULLPDFString);
        TString _toToken(_FULLPDFString);
        //Replace everything before the first "-" symbol. Should be left with everything for parameters initial settings!
        parstr = _toLook.ReplaceAll( _toToken.Tokenize("-")->At(0)->GetName(), "").Data(); 
    }
    MessageSvc::Debug("Identified TypePDF    to grab", TString(typepdf_));
    MessageSvc::Debug("Identified ParKeyList to Configure", TString(parstr));

    vector< string > pars = par_list[typepdf_];
    if (pars.size() == 0) MessageSvc::Error("GetStr2VarMap", (TString) "Empty Str2VarMap", "EXIT_FAILURE");

    for (const auto & par : pars) { 
        MessageSvc::Debug("Add Par", (TString)par);
        parout[par] = AddPar(par, parstr, stval_list, _map, opt); 
    }

    if (plusgaus ) {
        MessageSvc::Debug("AddPars for AndGauss case");
        parout["mg"] = AddPar("mg", parstr, stval_list, _map, opt);
        parout["sg"] = AddPar("sg", parstr, stval_list, _map, opt);
        parout["fg"] = AddPar("fg", parstr, stval_list, _map, opt);
    }
    if( plusdgaus){
        MessageSvc::Debug("AddPars for AndDGauss case");
        parout["mg"] = AddPar("mg", parstr, stval_list, _map, opt);
        parout["sg"] = AddPar("sg", parstr, stval_list, _map, opt);
        parout["fg"] = AddPar("fg", parstr, stval_list, _map, opt);         
        parout["m2"] = AddPar("m2", parstr, stval_list, _map, opt);
        parout["s2"] = AddPar("s2", parstr, stval_list, _map, opt);
        parout["f2"] = AddPar("f2", parstr, stval_list, _map, opt);   
        //keep working on "f2" only ! 
        if( SettingDef::Fit::useRecursiveFractions){
            //If the PDF has been modified by replacing "fg" with a scale parameter * original fg , fg is now a formula. 
            //Being a formula, we need to modify the rest (f2) accordingly.
            //f3 added for the MAIN CORE PDF! 
            MessageSvc::Warning("AdnDGauss case, useRecursive Function is True, special treatment");            
            bool FirstTime = dynamic_cast<RooFormulaVar*>(_map["fg"]) == nullptr ? true : false;             
            if( FirstTime){         
                MessageSvc::Warning("AdnDGauss case, useRecursive Function is True (round before ModifyPDF, fg parameter is NOT yet 'scaled' ");            
                TString _name_formulaFraction = TString::Format("First_Recursive_%s",parout["f2"]->GetName() );
                TString _name_formulaFraction2= _name_formulaFraction;
                _name_formulaFraction2.ReplaceAll("f2", "f3");
                parout["f2Recursive"] = new RooFormulaVar( _name_formulaFraction, "(1.-@0)*@1", RooArgList( *parout["fg"] ,  *parout["f2"]) )  ;
                parout["f3Recursive"] = new RooFormulaVar( _name_formulaFraction2, "(1.-@0-@1)", RooArgList( *parout["fg"] ,  *parout["f2Recursive"]));   
            }else{
                MessageSvc::Warning("AdnDGauss case, useRecursive Function is True (round before ModifyPDF, fg parameter has been 'scaled'");            
                MessageSvc::Warning("Deleting f2Recursive and f3Recursive from maps, re-adding them as formulas");
                delete parout["f2Recursive"];
                delete parout["f3Recursive"];
                TString _name_formulaFraction = TString::Format("Modified_Recursive_%s",parout["f2"]->GetName() );
                TString _name_formulaFraction2= _name_formulaFraction;
                _name_formulaFraction2.ReplaceAll("f2", "f3");                
                parout["f2Recursive"] = new RooFormulaVar( _name_formulaFraction,  "(1.-@0)*@1", RooArgList( *parout["fg"] ,  *parout["f2"] ) ) ;
                parout["f3Recursive"] = new RooFormulaVar( _name_formulaFraction2, "1.-@0-@1",   RooArgList( *parout["fg"] ,  *parout["f2Recursive"]));   
                //this is already a formula , we must udate it to absorbe the "Fg" scaling formulation
            }
        }
    }
    if( plusBiFurgaus){
        MessageSvc::Debug("AddPars for AndBifurGauss case");
        parout["mg"] = AddPar("mg", parstr, stval_list, _map, opt);
        parout["s1"] = AddPar("s1", parstr, stval_list, _map, opt);
        parout["s2"] = AddPar("s2", parstr, stval_list, _map, opt);
        parout["fg"] = AddPar("fg", parstr, stval_list, _map, opt);    
    }
    if (pluscb ) {
        MessageSvc::Debug("AddPars for AndCB case");
        parout["mcb"] = AddPar("mcb", parstr, stval_list, _map, opt);
        parout["scb"] = AddPar("scb", parstr, stval_list, _map, opt);
        parout["acb"] = AddPar("acb", parstr, stval_list, _map, opt);
        parout["ncb"] = AddPar("ncb", parstr, stval_list, _map, opt);
        parout["fcb"] = AddPar("fcb", parstr, stval_list, _map, opt);
    }

    for( auto & par : parout){
        TString _key(par.first);
        MessageSvc::Debug(TString::Format("Key par %s",_key.Data()), par.second);
    }
    MessageSvc::Debug(TString::Format("Return parout"));
    return parout;
}

RooAbsPdf * StringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap _map, string opt, TString title) {
    string typepdf_ = (string) typepdf;
    if (title == "") {
        title = typepdf_;
        title.Remove(title.First('-'), title.Length());
    }
    TString namepdf_ = ((TString) namepdf).ReplaceAll("bkg_", "");
    namepdf_         = namepdf_.ReplaceAll("_print", "").ReplaceAll("__noprint__", "");

    // opt += "-n" + (string)var->GetName();

    MessageSvc::Line();
    MessageSvc::Info("StringToPdf (name,title)", (TString) namepdf, title != namepdf ? (TString) title : "");
    MessageSvc::Info("StringToPdf (typepdf)", (TString) typepdf);
    if (opt != "") MessageSvc::Info("StringToPdf (option)", (TString) opt);
    // if (_map.size() != 0) PrintPars(_map);

    Str2VarMap p = GetStr2VarMap(typepdf_, namepdf_, var, _map, opt, title);
    auto _PDF_SELECT_ = []( std::string _IN_typepdf_ , TString _TypePDF ){
        TString _ss(_IN_typepdf_);
        return _ss.BeginsWith( _TypePDF);
    };
    auto _HAS_TAG  = []( std::string _TYPEPDF_, TString _TagBit){
        TString _ss(_TYPEPDF_);
        return _ss.Contains(_TagBit);
    };

    RooAbsPdf * pdf = nullptr;    
    MessageSvc::Debug("Searching with typepdf_ = ", TString("(")+TString(typepdf_)+TString(")"));
    if (_PDF_SELECT_(typepdf_, "Gauss")) {
        //Gaussian build [Gauss-m-s] Need
        MessageSvc::Debug("RooGaussian constructor");
        pdf = new RooGaussian(namepdf_, title, *var, *p["m"], *p["s"]);
    } else if (_PDF_SELECT_(typepdf_,"BGauss")) {
        //BifurGaussian build [Gauss-m-s1-s2] Need
        MessageSvc::Debug("RooBifurGauss constructor");
        pdf = new RooBifurGauss(namepdf_, title, *var, *p["m"], *p["s1"], *p["s2"]);
    } else if( _PDF_SELECT_(typepdf_, "DGaussAndExp")){
        MessageSvc::Debug("DGaussAndExp constructor (g1+g2+expo)");
        RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["m"],  *p["s"]);
        RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m2"], *p["s2"]);        
        RooExponential * expo= new RooExponential(namepdf_ + "_expo", "Expo",  *var, *p["b"]);
        pdf = new RooAddPdf( namepdf_, title, RooArgList(*gauss1, *gauss2, *expo), RooArgList(*p["f"], *p["f2"]));            
    } else if(_PDF_SELECT_(typepdf_, "DGauss")  ){ 
        //DoubleGaussian build [Gauss-m-s-s2-f] , shared mass default
        MessageSvc::Debug("DGauss constructor");
        //DoubleGauss with same mean! 
        RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["m"], *p["s"]);
        RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m"], *p["s2"]);
        pdf = new RooAddPdf(namepdf_, title, RooArgList(*gauss1, *gauss2), RooArgList(*p["f"]));
    }else if(_PDF_SELECT_(typepdf_, "TGaussAndExp")){
        MessageSvc::Debug("TripleGaussian + Expo constructor, diff mean, diff sigma!");
        RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["m"],  *p["s"]);
        RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m2"], *p["s2"]);
        RooGaussian * gauss3 = new RooGaussian(namepdf_ + "_gauss3", "Gauss3", *var, *p["m3"], *p["s3"]);
        RooExponential * expo= new RooExponential(namepdf_ + "_expo", "Expo",  *var, *p["b"]);
        pdf = new RooAddPdf( namepdf_, title, RooArgList(*gauss1, *gauss2, *gauss3, *expo), RooArgList(*p["f"], *p["f2"], *p["f3"]));
    } else if(_PDF_SELECT_(typepdf_,"TGauss")){
        //TripleGaussian build [TGauss-m-s2-s3-f-f2] , shared mass default
        MessageSvc::Debug("TripleGaussian constructor, share mean!");
        RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["m"], *p["s"]);
        RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m"], *p["s2"]);
        RooGaussian * gauss3 = new RooGaussian(namepdf_ + "_gauss3", "Gauss3", *var, *p["m"], *p["s3"]);
        RooArgList _fracList(*p["f"], *p["f2"] );
        pdf = new RooAddPdf(namepdf_, title, RooArgList(*gauss1, *gauss2, *gauss3), _fracList );
    }else if(_PDF_SELECT_(typepdf_,"CB")) {
        //TripleGaussian build [CB-m-s-a-n] 
        MessageSvc::Debug("RooCBShape constructor");
        RooAbsPdf * cb = new RooCBShape(namepdf_, "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        if(_PDF_SELECT_(typepdf_,"CBDGauss") ){
            //CBDGauss build [CBDGauss [CB-m-s-a-n-mg-sg-m2-s2] (all indepenent)
            MessageSvc::Debug("RooCBShape + Gaussian + Gaussian constructor");
            RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["mg"], *p["sg"]);
            RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m2"], *p["s2"]);
            RooArgList _fracList( *p["fg"], *p["f2"] );
            pdf = new RooAddPdf(namepdf_, title, RooArgList(*gauss1, *gauss2, *cb),_fracList );
        }else if( _PDF_SELECT_(typepdf_,"CBGauss")) {
            MessageSvc::Debug("RooCBShape + Gaussian constructor");
            RooGaussian * gauss = new RooGaussian(namepdf_ + "_gauss", "Gauss", *var, *p["m"], *p["sg"]);
            cb->SetName((TString) cb->GetName() + "_cb");
            pdf = new RooAddPdf(namepdf_, title, RooArgSet(*gauss, *cb), RooArgList(*p["fg"]));
        } else{
            pdf = cb;
        }
    } else if(_PDF_SELECT_(typepdf_,"DCB")) {
        MessageSvc::Debug("RooCBShape + RooCBShape constructor (same mass by default!)");
        RooCBShape * cb1 = new RooCBShape(namepdf_ + "_cb1", "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        RooCBShape * cb2 = nullptr;
        if (_HAS_TAG(typepdf, "_Sn")) {
            MessageSvc::Debug("RooCBShape + RooCBShape (same n-Par)");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n"]);
        }
        else if (_HAS_TAG(typepdf, "_Ss")){
            MessageSvc::Debug("RooCBShape + RooCBShape (same sigma-par)");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s"], *p["a2"], *p["n2"]);
        }else if( _HAS_TAG(typepdf, "_Stail_mean")){
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s2"], *p["a"], *p["n"]);
        }
        else{
            MessageSvc::Debug("RooCBShape + RooCBShape (same mass)");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n2"]);
        }
        if (_PDF_SELECT_(typepdf_,"DCBGauss")){
            MessageSvc::Debug("RooCBShape + RooCBShape constructor (add the Gauss , same mass, different sigma)");
            RooGaussian * gauss = new RooGaussian(namepdf_ + "_gauss", "Gauss", *var, *p["m"], *p["sg"]);
            RooArgList _fracList( *p["fg"], *p["f"] );
            //TODO: formla var it ! RecursiveFraction( _fracList);              
            pdf                 = new RooAddPdf(  namepdf_, title, RooArgSet(*gauss, *cb1, *cb2), _fracList);
        } else{
            pdf = new RooAddPdf(namepdf_, title, RooArgList(*cb1, *cb2), RooArgList(*p["f"]));
        }
    } else if (_PDF_SELECT_(typepdf_,"TCB")) {
        MessageSvc::Debug("Triple Crystal Ball, share mass by construction");
        RooCBShape * cb1 = new RooCBShape(namepdf_ + "_cb1", "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        RooCBShape * cb2 = nullptr;
        RooCBShape * cb3 = nullptr;
        if (_HAS_TAG(typepdf, "_Sn")){
            MessageSvc::Debug("Triple Crystal Ball, share (mass+n-order) parameter");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n"]);
            cb3 = new RooCBShape(namepdf_ + "_cb3", "CB", *var, *p["m"], *p["s3"], *p["a3"], *p["n"]);
        } else if (_HAS_TAG(typepdf, "_Ss")) {
            MessageSvc::Debug("Triple Crystal Ball, share (mass+sigma) parameter");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s"], *p["a2"], *p["n2"]);
            cb3 = new RooCBShape(namepdf_ + "_cb3", "CB", *var, *p["m"], *p["s"], *p["a3"], *p["n3"]);
        } else {
            MessageSvc::Debug("Triple Crystal Ball, share (mass) parameter");
            cb2 = new RooCBShape(namepdf_ + "_cb2", "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n2"]);
            cb3 = new RooCBShape(namepdf_ + "_cb3", "CB", *var, *p["m"], *p["s3"], *p["a3"], *p["n3"]);
        }
        RooArgList _fracList( *p["f"], *p["f2"] );
        pdf = new RooAddPdf(namepdf_, title, RooArgList(*cb1, *cb2, *cb3),_fracList);    
    }else if(_PDF_SELECT_(typepdf_,"DSCB")) {
        MessageSvc::Debug("Double Sided Crystal Ball constructor");
        RooDoubleSidedCBShape * dscb = new RooDoubleSidedCBShape(namepdf_ + "_dscb", "DSCB", *var, *p["m"], *p["s"], *p["a"], *p["n"], *p["a2"], *p["n2"]);
        if (_PDF_SELECT_(typepdf_,"DSCBDGauss")){
            MessageSvc::Debug("Double Sided Crystal Ball + 2 Gaussian (free masses and sigmas)");
            //Add 2 Gaussians with free mean and sigmas (for 1 Brem case!)            
            RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *p["mg"], *p["sg"]);        
            if( _HAS_TAG(typepdf_, "_Sm2")){
                //The second gaussian has shared sigma to double sided-crystal ball 
                MessageSvc::Debug("Double Sided Crystal Ball 2nd gaussian share mass");
                RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m"], *p["s2"]);
                RooArgList _fracList( *p["fg"], *p["f2"] );
                pdf = new RooAddPdf( namepdf_, title, RooArgSet( *gauss1, *gauss2 , *dscb), _fracList);
            }else{
                RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *p["m2"], *p["s2"]);
                RooArgList _fracList( *p["fg"], *p["f2"] );
                pdf = new RooAddPdf( namepdf_, title, RooArgSet( *gauss1, *gauss2 , *dscb), _fracList );
            }
        }else if (_PDF_SELECT_(typepdf_,"DSCBGauss")) {
            MessageSvc::Debug("Double Sided Crystal Ball + 1 Gaussian (same masse and free sigmas)");
            RooGaussian * gauss = new RooGaussian(namepdf_ + "_gauss", "Gauss", *var, *p["m"], *p["sg"]);
            pdf                 = new RooAddPdf(namepdf_, title, RooArgSet(*gauss, *dscb), RooArgList(*p["fg"]));
        } else{
            pdf = dscb;
        }
    }else if( _PDF_SELECT_( typepdf_, "BifurDSCB")){
        MessageSvc::Debug("BiSmear Double Sided Crystal Ball constructor");
        RooBifurDSCBShape * bifurDSCB = new RooBifurDSCBShape( 
            namepdf_ +"_bifurdscb", 
            "BifurDSCB", 
            *var, 
            *p["m"], //mass
            *p["sL"], //sigmaLeft
            *p["sR"], //sigmaRight
            *p["aL"], //alphaLeft
            *p["nL"], //nLeft
            *p["aR"], //alphaRight
            *p["nR"]  //nRight
        );
        pdf = bifurDSCB;
    }else if (_PDF_SELECT_(typepdf_,"Ipatia")) {
        MessageSvc::Debug("Ipatia constructor (choosing Ipatia,Ipatia2)");    
        if (_PDF_SELECT_(typepdf_,"Ipatia2")){
            MessageSvc::Debug("RooIpatia2 used");    
            pdf = new RooIpatia2(namepdf_, title, *var, *p["l"], *p["z"], *p["b"], *p["s"], *p["m"], *p["a"], *p["n"], *p["a2"], *p["n2"]);
        }
        else{
            MessageSvc::Debug("RooIpatia used");    
            pdf = new RooIpatia(namepdf_, title, *var, *p["l"], *p["z"], *p["b"], *p["s"], *p["m"], *p["a"], *p["n"]);
        }
    }else if (_PDF_SELECT_(typepdf_, "ExpTailDG") ) {
        MessageSvc::Debug("RooExpGaussExp + RooGaussian constructor (same mass)");    
        auto gaussExpTail = new RooExpGaussExp(namepdf_ + "_gaussExpTail", "GaussExpTail", *var, *p["m"], *p["s"], *p["a"], *p["a2"]);
        auto centreGauss  = new RooGaussian(namepdf_ + "_centreGauss", "CentreGauss", *var, *p["m"], *p["s2"]);
        pdf               = new RooAddPdf(namepdf_, title, RooArgList(*gaussExpTail, *centreGauss), RooArgList(*p["f"]));
    }else if (_PDF_SELECT_(typepdf_,"ExpTailTG") ) {
        MessageSvc::Debug("RooExpGaussExp + RooGaussian(1)[(same mass,sigma)] + RooGaussian(2)[(new mass,new sigma)] constructor ");    
        auto gaussExpTail = new RooExpGaussExp(namepdf_ + "_gaussExpTail", "GaussExpTail", *var, *p["m"], *p["s"], *p["a"], *p["a2"]);
        auto centreGauss  = new RooGaussian(namepdf_ + "_centreGauss", "CentreGauss", *var, *p["m"], *p["s"]);
        auto bremGauss    = new RooGaussian(namepdf_ + "_bremGauss", "BremGauss", *var, *p["mg"], *p["sg"]);
        RooArgList _fracList( *p["f"], *p["fg"] );
        pdf               = new RooAddPdf(namepdf_, title, RooArgList(*gaussExpTail, *centreGauss, *bremGauss), _fracList );
    } else if (_PDF_SELECT_(typepdf_,"ExpGaussExp")){
        MessageSvc::Debug("RooExpGaussExp alone");    
        pdf = new RooExpGaussExp(namepdf_, title, *var, *p["m"], *p["s"], *p["a"], *p["a2"]);
    } else if (_PDF_SELECT_( typepdf_,"ExpAGauss") ) {
        MessageSvc::Debug("RooExpAndGauss alone");    
        pdf = new RooExpAndGauss(namepdf_, title, *var, *p["m"], *p["s"], *p["b"]);
    } else if (_PDF_SELECT_( typepdf_,"ExpCGauss") ) {
        MessageSvc::Debug("RooGExpModel alone");    
        pdf = new RooGExpModel(namepdf_, title, *var, *p["s"], *p["b"]);
    }else if( _PDF_SELECT_(typepdf_, "ExpTurnOnGauss")){
        MessageSvc::Debug("Exponential With Turn On + Gaussian");
        auto expoTurn = new RooExpTurnOn(namepdf_+"_expturn", namepdf_+"_expturn", *var, *p["mO"], *p["sE"], *p["b"]);
        auto gauss    = new RooGaussian(namepdf_+"_gauss", namepdf_+"_gauss", *var, *p["m"], *p["s"]);
        pdf           = new RooAddPdf(namepdf_, title, RooArgList(*expoTurn, *gauss), RooArgList(*p["f"]) );
        // vector< string> ExpTurnOnGauss{  "mO", "sE", "b", "m", "s", "f"};
        vector< string> ExpTurnOnDGauss{ "mO", "sE", "b", "m", "s", "m2","s2", "f", "f2"};
    }else if( _PDF_SELECT_(typepdf_ , "ExpTurnOnDGauss")){
        MessageSvc::Debug("Exponential With Turn On + 2 Gaussian");
        auto expoTurn = new RooExpTurnOn(namepdf_+"_expturn", namepdf_+"_expturn", *var, *p["mO"], *p["sE"], *p["b"]);
        auto gauss1   = new RooGaussian(namepdf_+"_gauss1", namepdf_+"_gauss1", *var, *p["m"], *p["s"]);
        auto gauss2   = new RooGaussian(namepdf_+"_gauss2", namepdf_+"_gauss2", *var, *p["m2"], *p["s2"]);
        pdf           = new RooAddPdf(namepdf_, title, RooArgList(*expoTurn, *gauss1, *gauss2), RooArgList(*p["f"], *p["f2"]));
    }else if (_PDF_SELECT_( typepdf_,"ExpTurnOn") ) {
        MessageSvc::Debug("Exponential with turn-on-curve constructor");
        /*
            Turn On Function is ( 1./ ( 1+ e^{ sE(x_{turnOn} -x))  * exp( b*x)})) 
        */
        // pdf = new RooGenericPdf(namepdf, namepdf, "(1./(1.+TMath::Exp(@0*(@1-@2))))*TMath::Exp(@3*@2)", RooArgList(*p["sE"], *p["mO"], *var, *p["b"]));
        if( SettingDef::Fit::useNumericalExpTurnOn){
            MessageSvc::Warning("Using RooExpTurnOn with numerical integral");
            pdf = new RooExpTurnOnNumerical(namepdf_, namepdf_, *var, *p["mO"], *p["sE"], *p["b"]);
        }else{
            MessageSvc::Warning("Using RooExpTurnOn with analytical integral");
            pdf = new RooExpTurnOn(namepdf_, namepdf_, *var, *p["mO"], *p["sE"], *p["b"]);
        }
    } else if (_PDF_SELECT_( typepdf,"Exp") ) {
        MessageSvc::Debug("RooExponential alone");    
        pdf = new RooExponential(namepdf_, title, *var, *p["b"]);
    } else if (_PDF_SELECT_( typepdf_,"Poly")  || _PDF_SELECT_(typepdf_, "Cheb")) {
        TString str_npar = (TString)(typepdf_.substr(4, string::npos));
        int     npar     = str_npar.Atof();

        vector< double > pvals;
        vector< double > mins;
        vector< double > maxs;

        for (int vv = 0; vv <= npar; vv++) {
            double pval = 0, min = -1, max = 1;
            size_t posval = typepdf_.find(Form("-c%i[", vv));
            if (posval != string::npos) {
                size_t endPar = typepdf_.find("]", posval);
                string s      = typepdf_.substr(posval + 3, string::npos);
                size_t comma1 = s.find(',');
                size_t epar   = s.find(']');
                if (comma1 < epar) {
                    string sval   = s.substr(1, comma1);
                    pval          = ((TString) sval).Atof();
                    size_t comma2 = s.find(',', comma1 + 1);
                    string smin   = s.substr(comma1 + 1, comma2 - comma1 - 1);
                    min           = ((TString) smin).Atof();
                    string smax   = s.substr(comma2 + 1, endPar - comma2 - 1);
                    max           = ((TString) smax).Atof();
                } else {
                    string sval = s.substr(1, endPar - 1);
                    pval        = ((TString) sval).Atof();
                }
            }

            pvals.push_back(pval);
            mins.push_back(min);
            maxs.push_back(max);
        }
        RooArgList * parList  = new RooArgList("parList");
        TString      pstrname = GetPrintParName(title, opt);
        for (int i = 0; i <= npar; i++) {
            RooRealVar * v = new RooRealVar(namepdf_ + Form("_c%i", i), title + Form(" c%i" + pstrname, i), pvals[i], mins[i], maxs[i]);
            parList->add(*v);
        }

        if (typepdf_.find("Poly") != string::npos){
            MessageSvc::Debug("RooPolynomial alone");    
            pdf = new RooPolynomial(namepdf_, title, *var, *parList);
        }
        else{
            MessageSvc::Debug("RooChebychev alone");    
            pdf = new RooChebychev(namepdf_, title, *var, *parList);
        }
    } else if (typepdf_.find("Argus") != string::npos) {
        MessageSvc::Debug("Argus constructor");    
        if (typepdf_.find("InvArgus") != string::npos){
            MessageSvc::Debug("RooInverseArgus constructor");    
            pdf = new RooInverseArgus(namepdf_, title, *var, *p["m0"], *p["c"], *p["p"]);
        }
        else{
            MessageSvc::Debug("RooArgusBG constructor");    
            pdf = new RooArgusBG(namepdf_, title, *var, *p["m0"], *p["c"], *p["p"]);
        }
    } else if (typepdf_.find("BreitWigner") != string::npos) {
        MessageSvc::Debug("RooBreitWigner constructor");    
        pdf = new RooBreitWigner(namepdf_, title, *var, *p["m"], *p["g"]);
    } else if (typepdf_.find("Gamma") != string::npos) {
        MessageSvc::Debug("RooGamma constructor");    
        pdf = new RooGamma(namepdf_, title, *var, *p["g"], *p["b"], *p["m"]);
    } else if (typepdf_.find("Johnson") != string::npos) {
        MessageSvc::Debug("RooJohnson constructor");    
        pdf = new RooJohnson(namepdf_, title, *var, *p["m"], *p["s"], *p["nu"], *p["tau"]);
    } else if (typepdf_.find("DGAndExp") != string::npos) {
        MessageSvc::Debug("Expo + Gaussian1 + Gaussian2 (all indep) constructor");    
        RooGaussian *    gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss", *var, *p["m"], *p["s"]);
        RooGaussian *    gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss", *var, *p["m2"], *p["s2"]);
        RooExponential * exp    = new RooExponential(namepdf_ + "_exp", "Exp", *var,   *p["b"]);
        RooArgList _fracList( *p["f"], *p["fg"] );
        pdf                     = new RooAddPdf(namepdf_, title, RooArgList(*exp, *gauss1, *gauss2), _fracList );
    }else  if( typepdf_.find("LandauAndExp") != string::npos){
        MessageSvc::Debug("Landau + Exponential constructor");    
        RooExponential * exp         = new RooExponential(namepdf_ + "_exp", "Exp", *var, *p["b"]);
        RooLandauAnalytical * landau = new RooLandauAnalytical(namepdf_+"_landau", "Landau", *var, *p["m0"], *p["sl"]);    
        pdf = new RooAddPdf(namepdf_, title, RooArgList( *landau, *exp), RooArgList( *p["f"]));      
    }else if (typepdf_.find("Landau") != string::npos) {
        MessageSvc::Debug("Landau constructor");    
        pdf = new RooLandauAnalytical(namepdf_, title, *var, *p["m0"], *p["sl"]);        
    }else if (typepdf_.find("Linear") != string::npos) {
        MessageSvc::Debug("Linear constructor");
        TString genericTitle = fmt::format("{0}+{1}*({2})", p["c1"]->GetName(), p["c2"]->GetName(), var->GetName());
        pdf = new RooGenericPdf(namepdf_, genericTitle, RooArgList(*p["c1"], *p["c2"], *var));      	
    }else{ 
        MessageSvc::Warning("NOTHING FOUND in the nasty if-else if cascade!");
    }
    
    if (typepdf_.find("AndGauss") != string::npos) {
        MessageSvc::Debug("AndGauss routine");    
        TString extra = ((TString) title).ReplaceAll("AndGauss_Sms", "").ReplaceAll("AndGauss_Sm", "").ReplaceAll("AndGauss", "");
        extra.ToLower();
        pdf->SetName((TString) pdf->GetName() + "_" + extra);
        RooGaussian * gauss = new RooGaussian(namepdf_ + "_gauss", "Gauss", *var, *p["mg"], *p["sg"]);
        if (typepdf_.find("AndGauss_Sms") != string::npos){
            MessageSvc::Debug("Add Gauss Same sigma, same mass ");    
            gauss = new RooGaussian(namepdf_ + "_Andgauss_Sms", "Gauss", *var, *p["m"], *p["s"]);
        }else if (typepdf_.find("AndGauss_Sm") != string::npos){
            MessageSvc::Debug("Add Gauss same mass ");    
            gauss = new RooGaussian(namepdf_ + "_Andgauss_Sm", "Gauss", *var, *p["m"], *p["sg"]);
        }else{
            MessageSvc::Debug("Add Gauss free mass , free sigma  ");    
        }
        pdf = new RooAddPdf(namepdf_ + "_agauss", title, RooArgList(*gauss, *pdf), RooArgList(*p["fg"]));
    }
    if (typepdf_.find("AndDGauss") != string::npos) {
        MessageSvc::Debug("AndDGauss routine");    
        TString extra = ((TString) title).ReplaceAll("AndDGauss_Sms", "").ReplaceAll("AndDGauss_Sm", "").ReplaceAll("AndDGauss_Sm2", "").ReplaceAll("AndDGauss", "");
        extra.ToLower();
        pdf->SetName((TString) pdf->GetName() + "_" + extra);
        auto * m1 = _HAS_TAG(typepdf_,  "_Sm")  ? p["m"] : p["mg"];
        auto * m2 = _HAS_TAG(typepdf_,  "_Sm2") ? p["m"] : p["m2"];
        //!!!!!!!!!!!! EXTREMELY IMPORTANT FOR ANDDGAUSS TO HAVE sg IN GAUSS1 FUNCTION !!!!!!!!!!!!!!!!!!!!!!!!!!//
        RooGaussian * gauss1 = new RooGaussian(namepdf_ + "_gauss1", "Gauss1", *var, *m1, *p["sg"]);
        RooGaussian * gauss2 = new RooGaussian(namepdf_ + "_gauss2", "Gauss2", *var, *m2, *p["s2"]);
        MessageSvc::Debug("Add DGauss free mass(1+2) , free sigma (1+2) ");        
        if(_HAS_TAG(typepdf_,"_Sm") ){
            MessageSvc::Debug("mass(1) == mass(main)");        
        }
        if(_HAS_TAG(typepdf_,"_Sm2")){
            MessageSvc::Debug("mass(2) == mass(main)");        
        }
        if( SettingDef::Fit::useRecursiveFractions){
            MessageSvc::Warning("StringToPdf Using f2Recursive and f3Recursive");
            RooArgList _fracList(*p["fg"], *p["f2Recursive"],*p["f3Recursive"] );
            // if( SettingDef::Fit::useRooRealSumPDF){
            //     MessageSvc::Warning("StringToPdf RooRealSumPDF used ! ");
            //     pdf = new RooRealSumPdf(namepdf_ + "_adgauss_Recursive", title, RooArgList(*gauss1, *gauss2,  *pdf), _fracList , true); //MAYBE THIS IS MORE CORRECT AND ALLOWS THE FBREM TO GO NEGATIVE! 
            // }else{
            MessageSvc::Warning("StringToPdf RooAddPdf used ! ");
            pdf = new RooAddPdf(namepdf_ + "_adgauss_Recursive", title, RooArgList(*gauss1, *gauss2,  *pdf), _fracList);
            // }
        }else{
            RooArgList _fracList(*p["fg"], *p["f2"]);
            pdf = new RooAddPdf(namepdf_ + "_adgauss", title, RooArgList(*gauss1, *gauss2,  *pdf), _fracList);
        }
        //!!!! WE HAVE 3 PDFS , they must normalize to 1. 
        //!!!! WE ALWAYS make the pdfs so that there are n, n-1 coefficients [0,1] bound. 
        //!!!! HOWEVER IF WE END UP SCALING ONE COMPONENT FRACTION, the sum of coeffs will not be 1 , breaking the normalization , see for example 
        //!!!! 0G * frac0 + 1G * frac1 + ( 1- frac0 + frac1) * 2G 
        //!!!! IF inside 1G we scale one component we need to redo-the pdf so that 1G expand to 
        //!!!! f0*pdf0 + ( 1-f0) * f1 * pdf1 and re-store things as they should! 
        //!!!! VERY IMPORTANT ( i think) is that the first fraction is the one to scale, and not the other ones else the "scaling happens only to the last component"        
    }
    if (typepdf_.find("AndBiFurGauss") != string::npos) {
        MessageSvc::Debug("AndBiFurGauss found"); 
        TString extra = ((TString) title).ReplaceAll("AndBiFurGauss_Sms", "").ReplaceAll("AndBiFurGauss_Sm", "").ReplaceAll("AndBiFurGauss", "");
        extra.ToLower();
        pdf->SetName((TString) pdf->GetName() + "_" + extra);        
        RooBifurGauss *bGauss = new RooBifurGauss( namepdf_ + "_bifurGauss", "AndBifurGauss", *var, *p["mg"], *p["s1"], *p["s2"]);        
        pdf = new RooAddPdf(namepdf_ + "_abifurGaus", title, RooArgList(*bGauss, *pdf), RooArgList(*p["fg"]));
    }
    if (typepdf_.find("AndCB") != string::npos) {
        TString extra = ((TString) title).ReplaceAll("AndCB_Sms", "").ReplaceAll("AndCB_Sm", "").ReplaceAll("AndCB", "");
        extra.ToLower();
        pdf->SetName((TString) pdf->GetName() + "_" + extra);
        RooCBShape * cb = new RooCBShape(namepdf_ + "_cb", "AndCB", *var, *p["mcb"], *p["scb"], *p["acb"], *p["ncb"]);
        if (typepdf_.find("AndCB_Sms") != string::npos){
            cb = new RooCBShape(namepdf_ + "_ANDcb_Sms", "AndCB_Sms", *var, *p["m"], *p["s"], *p["acb"], *p["ncb"]);
        }else if (typepdf_.find("AndCB_Sm") != string::npos){
            cb = new RooCBShape(namepdf_ + "_ANDcb_Sm", "AndCB_Sm", *var, *p["m"], *p["scb"], *p["acb"], *p["ncb"]);
        }
        pdf = new RooAddPdf(namepdf_ + "_acb", title, RooArgList(*cb, *pdf), RooArgList(*p["fcb"]));
    }
    Str2VarMap _usedVariables;
    for (auto _strParPair : p) {
        if (pdf->dependsOn(*_strParPair.second)) {
            _usedVariables[_strParPair.first] = _strParPair.second;
        }
    }
    PrintPars(_usedVariables);

    MessageSvc::Info("StringToPdf", pdf);
    MessageSvc::Line();
    return pdf;
}

RooAbsPdf * TF1ToPdf(TString _name, TString _formula, const RooArgList & _pars, RooRealVar * _var) {
    MessageSvc::Line();
    MessageSvc::Info("TF1ToPdf", _name, _formula);
    MessageSvc::Info("TF1ToPdf", &_pars);

    TF1 * _tf1 = new TF1(_name + "_TF1", _formula);
    MessageSvc::Info("TF1ToPdf", _tf1);

    RooAbsPdf * _pdf = nullptr;
    if (_var != nullptr) {
        RooAbsReal * _func = RooFit::bindFunction(_tf1, *_var, _pars);
        MessageSvc::Info("TF1ToPdf", _func);
        RooRealVar * _frac = new RooRealVar(_name + "_frac", _name + "_frac", 1);
        _frac->setConstant(1);
        _pdf = new RooRealSumPdf(_name, _name, RooArgList(*_func), RooArgList(*_frac));
    } else {
        //_pdf = RooFit::bindPdf(_tf1, *_var, _pars);
        _pdf = new RooTFnPdfBinding(_name, _name, _tf1, _pars);
    }

    MessageSvc::Info("TF1ToPdf", _pdf);
    MessageSvc::Line();
    return _pdf;
}

RooAbsReal * BlindParameter(RooRealVar * _par, BlindMode _mode, double _scaleLimit) {
    /*
        ScaleLimit is 10. by default, but if passed to be -1, the blinding will be on the same scale value.
    */
    UnblindParameter * _unblindedParameter = nullptr;    
    if (_par != nullptr) {
        TString _name  = TString(_par->GetName());
        TString _title = TString(_par->GetTitle()) + " (unblind)";
        MessageSvc::Info("BlindParameter", to_string(_mode), _name);
        switch (_mode) {
            case BlindMode::OffsetScale: _unblindedParameter = new UnblindOffsetScale(_name, _title, _scaleLimit, *_par); break;
            default:
                MessageSvc::Error("BlindMode", (TString) "Invalid BlindMode", to_string(_mode), "EXIT_FAILURE");
                return _unblindedParameter;
                break;
        }        
        _par->SetName((TString) _par->GetName() + "_blinded");
        _par->SetTitle((TString) _par->GetTitle() + " (blind)");
        
    } else
        MessageSvc::Info("BlindParameter", (TString) "Nothing to blind");
    return (RooAbsReal *) _unblindedParameter;
}

vector< double > GetFractions(EventType & _eventType, const RooRealVar & _var, vector< TCut > _cuts) {
    MessageSvc::Line();
    MessageSvc::Info("GetFractions", _eventType.GetSample(), _var.GetName(), to_string(_cuts.size()));

    vector< double > _entries;
    double           _total = 0;
    for (size_t i = 0; i < _cuts.size(); i++) {
        _entries.push_back(_eventType.GetHisto(_var, _cuts[i])->Integral());
        _total += _entries[i];
    }

    vector< double > _fractions;
    for (size_t i = 0; i < _cuts.size(); i++) { _fractions.push_back(_entries[i] / _total); }

    MessageSvc::Info("GetFractions", _eventType.GetSample());
    MessageSvc::Info("Cuts", _cuts);
    MessageSvc::Info("Entries", _entries);
    MessageSvc::Info("Total", to_string(_total));
    MessageSvc::Info("Fractions", _fractions);
    MessageSvc::Line();
    return _fractions;
}

RooAddPdf * SumPDFs(TString _name, RooArgList _pdfList, vector< double > _coefs, RooRealVar * _var, TString _option) {
    MessageSvc::Line();
    MessageSvc::Info("SumPDFs", _name, to_string(_pdfList.getSize()), to_string(_coefs.size()));

    if (_pdfList.getSize() != _coefs.size() + 1) MessageSvc::Error("SumPDFs", "_pdfList.size != _coefList.size + 1", "EXIT_FAILURE");

    RooArgList _coefList;
    for (size_t i = 0; i < _coefs.size(); i++) {
        RooRealVar * _frac = new RooRealVar(Form("f%d_" + _name, i), Form("f%d_" + _name, i), _coefs[i], 0, 1);
        if (_option.Contains("const"))
            _frac->setConstant(1);
        else
            _frac->setConstant(0);
        _coefList.add(*_frac);
    }

    MessageSvc::Info("PDFs", &_pdfList);
    MessageSvc::Info("Coefs", &_coefList, "v");
    // RecursiveFraction(_coefList);
    RooAddPdf * _pdf = new RooAddPdf(_name, _name, _pdfList, _coefList);

    TCanvas   _canvas("canvas");
    RooPlot * _frame = _var->frame(Title(_name), Bins(_var->getBins()));
    TLegend   _legend(0.7, 0.7, 1.0, 1.0);
    _legend.SetFillColor(kWhite);
    _legend.SetLineColor(kWhite);
    _pdf->plotOn(_frame, LineColor(GetColor(0)), LineWidth(5));
    _legend.AddEntry(_pdf, _pdf->GetName(), "L");
    _pdfList = _pdf->pdfList();
    for (int i = 0; i < _pdfList.getSize(); ++i) {
        _pdf->plotOn(_frame, Components(*_pdfList.at(i)), LineColor(GetColor(i + 1)), LineStyle(kDashed + i), Name(_pdfList.at(i)->GetName()), MoveToBack());
        _legend.AddEntry(_frame->findObject(_pdfList.at(i)->GetName()), _pdfList.at(i)->GetName(), "L");
    }
    _frame->Draw();
    _legend.Draw("SAME");
    _canvas.Print("SumPDFs_" + _name + ".pdf");

    MessageSvc::Info("SumPDFs", _pdf);
    MessageSvc::Line();
    return _pdf;
}

void SaveToDOT(RooAbsArg * _rooAbsArg, TString _file, TString _option) {
    _file.ReplaceAll("_Results.log", ".dot");
    MessageSvc::Line();
    MessageSvc::Info("SaveToDOT", (TString) _rooAbsArg->GetName(), _file, _option);
    _rooAbsArg->graphVizTree(_file);
    //_rooAbsArg->graphVizTree(std::ostream &os);
    MessageSvc::Line();
    return;
}

void SaveToTEX(RooFitResult * _fitResults, TString _file, bool _minos) {
    MessageSvc::Line();
    MessageSvc::Info("SaveToTEX", (TString) _fitResults->GetName(), _file);

    TString _constParsFileName      = _file;
    TString _initFloatParsFileName  = _file;
    TString _finalFloatParsFileName = _file;
    _constParsFileName.ReplaceAll("_Results.log", "_constPars.tex");
    _initFloatParsFileName.ReplaceAll("_Results.log", "_floatParsInit.tex");
    _finalFloatParsFileName.ReplaceAll("_Results.log", "_floatParsFinal.tex");
    ofstream _constParsOutFile(_constParsFileName);
    ofstream _initFloatParsOutFile(_initFloatParsFileName);
    ofstream _finalFloatParsOutFile(_finalFloatParsFileName);

    auto _constParArg = Format("NU", AutoPrecision(1), VerbatimName(), LatexTableStyle());
    auto _initParArg  = Format("NU", AutoPrecision(1), VerbatimName(), LatexTableStyle());
    if (_fitResults->constPars().getSize() != 0) _fitResults->constPars().printLatex(_constParsOutFile, 1, 0, 0, RooLinkedList(), &_constParArg);
    if (_fitResults->floatParsInit().getSize() != 0) _fitResults->floatParsInit().printLatex(_initFloatParsOutFile, 1, 0, 0, RooLinkedList(), &_initParArg);
    if (_fitResults->floatParsFinal().getSize() != 0) {
        if (_minos) {
            auto _finalParArg = Format("NAU", AutoPrecision(1), VerbatimName(), LatexTableStyle());
            _fitResults->floatParsFinal().printLatex(_finalFloatParsOutFile, 1, 0, 0, RooLinkedList(), &_finalParArg);
        } else {
            auto _finalParArg = Format("NEU", AutoPrecision(1), VerbatimName(), LatexTableStyle());
            _fitResults->floatParsFinal().printLatex(_finalFloatParsOutFile, 1, 0, 0, RooLinkedList(), &_finalParArg);
        }
    }

    _constParsOutFile.close();
    _initFloatParsOutFile.close();
    _finalFloatParsOutFile.close();

    MessageSvc::Line();
    return;
}

void SaveToYAML(RooFitResult * _fitResults, TString _file, TString _option) {
    std::this_thread::sleep_for (std::chrono::seconds(1));
    _file.ReplaceAll("_Results.log", "_floatParsFinal.yaml");
    MessageSvc::Line();
    MessageSvc::Info("SaveToYAML", (TString) _fitResults->GetName(), _file, _option);
    RooArgList _pars = _fitResults->floatParsFinal();
    if (_pars.getSize() != 0) {
        YAML::Emitter _emitter;
        _emitter << YAML::BeginMap;
        RooArgList _floatPars = _pars;
        // for (int i = 0; i < _floatPars.getSize(); ++i) { _emitter << YAML::Key << ((RooRealVar *) _floatPars.at(i))->GetName() << *((RooRealVar *) _floatPars.at(i)); }
        for (int i = 0; i < _floatPars.getSize(); ++i) {
            RooRealVar * _var = (RooRealVar *) _floatPars.at(i);
            TString _string = "[" + to_string(_var->getVal());
            _string += ", " + to_string(_var->getError());
            if( _var->hasAsymError()){
                _string += " ( "+ to_string( _var->getErrorLo()) + " ; " + to_string( _var->getErrorHi()) +" )";
            }
            if (_var->hasMin() && _var->hasMax()) {
                _string += ", " + to_string(_var->getMin());
                _string += ", " + to_string(_var->getMax());
            } else {
                _string += ", 0, 0";
            }
            _string += "]";
            _emitter << YAML::Key << _var->GetName() << _string;
        }
        _emitter << YAML::EndMap;
        ofstream _oFile(_file, ios::app);
        if (!_oFile.is_open()) MessageSvc::Error("Unable to open file", _file, "EXIT_FAILURE");
        _oFile << _emitter.c_str() << endl;
        _oFile.close();
    }

    MessageSvc::Line();
    std::this_thread::sleep_for (std::chrono::seconds(1));
    return;
}

vector< RooRealVar * > LoadFromYAML(TString _file) {
    MessageSvc::Line();
    MessageSvc::Info("LoadFromYAML", _file);

    vector< RooRealVar * > _pars;
    if (IOSvc::ExistFile(_file)) {
        YAML::Node _yaml = YAML::LoadFile(_file.Data());
        for (YAML::iterator _it = _yaml.begin(); _it != _yaml.end(); ++_it) {
            TString _name   = _it->first.as< TString >();
            TString _string = _it->second.as< TString >();
            _string.ReplaceAll("[", "").ReplaceAll("]", "").ReplaceAll(" ", "");
            auto * _strCollection = ((TString) _string).Tokenize(",");
            if (_strCollection->GetEntries() != 4) { MessageSvc::Error("LoadFromYAML", (TString) "Wrong format, use [vale, error, min, max]", "EXIT_FAILURE"); }
            double       _val   = TString(((TObjString *) (*_strCollection).At(0))->String()).Atof();
            double       _error = TString(((TObjString *) (*_strCollection).At(1))->String()).Atof();
            double       _min   = TString(((TObjString *) (*_strCollection).At(2))->String()).Atof();
            double       _max   = TString(((TObjString *) (*_strCollection).At(3))->String()).Atof();
            RooRealVar * _var   = new RooRealVar(_name, _name, _val);
            _var->setError(_error);
            if ((_min != 0) && (_max != 0)) _var->setRange(_min, _max);
            MessageSvc::Info("LoadFromYAML", _var);
            _pars.push_back(_var);
        }
    }

    MessageSvc::Info("LoadFromYAML", (TString) "Parameters", to_string(_pars.size()));
    MessageSvc::Line();
    return _pars;
}

pair< double, double > GetSPlotYield(TString _project, TString _ana, TString _year, TString _trigger) {
    MessageSvc::Line();
    MessageSvc::Info("GetSPlotYield", _project, _ana, _year, _trigger);

    vector< TString > _years    = GetYears(_year);
    vector< TString > _triggers = GetTriggers(_trigger, true);

    double _yield = 0;
    double _error = 0;
    for (const auto _YEAR_ : _years) {
        for (const auto _TRIGGER_ : _triggers) {
            TString _file = IOSvc::GetTupleDir("spl", _project, _ana, "", _YEAR_, _TRIGGER_);
            //_file         = fmt::format("{0}/{1}/SPlotSim{2}JPsDTF_RooMinimizer_Results.log", _file, _year, ((TString) _project).ReplaceAll("R", ""));
            _file = fmt::format("{0}/{1}/FitHolder_{2}-{3}-jps-{4}-{5}_SPlot_RooMinimizer_Results.log", _file, _YEAR_, _project, _ana, _TRIGGER_, _YEAR_);
            MessageSvc::Info("GetSPlotYield", _file);

            double                      _val, _err;
            vector< vector< TString > > _data = IOSvc::ParseFile(_file, "");
            for (auto _line : _data) {
                TString _el = _line.at(0);
                if (_el.Contains("nsig_") && _el.Contains(_project) && _el.Contains(_ana) && _el.Contains(_YEAR_) && _el.Contains(_TRIGGER_)) {
                    // MessageSvc::Print(_el);
                    vector< TString > _elt = TokenizeString(_el, " ");
                    // for (auto _val : _elt) { MessageSvc::Print(_val); }
                    _val = _elt.at(3).Atof();
                    _err = _elt.at(5).Atof();
                    _yield += _val;
                    _error += TMath::Sq(_err);
                    break;
                }
            }
            MessageSvc::Info("GetSPlotYield", to_string(_val), "+/-", to_string(_err));
        }
    }
    _error = TMath::Sqrt(_error);

    if (_triggers.size() > 1) {
        MessageSvc::Line();
        MessageSvc::Info("GetSPlotYield", to_string(_yield), "+/-", to_string(_error));
    }
    if ((_yield == 0) && (_error == 0)) MessageSvc::Warning("GetSPlotYield", (TString) "Yield not found");
    MessageSvc::Line();
    return make_pair(_yield, _error);
}

TMatrixDSym MatrixToROOTMatrix(const vector< vector< double > > & _CMatrix) {
    // Check if size if correct
    int _nRows = _CMatrix.size();
    for (const auto & _column : _CMatrix) {
        if (_column.size() != _nRows) { MessageSvc::Error("MatrixToROOTMatrix", (TString) "Number of Rows and Columns do not match", "EXIT_FAILURE"); }
    }
    TMatrixDSym _ROOTMatrix(_nRows);
    for (int i = 0; i < _nRows; i++) {
        for (int j = 0; j < _nRows; j++) { _ROOTMatrix(i, j) = _CMatrix[i][j]; }
    }
    return _ROOTMatrix;
}

TString HashString(const TString& _string) {
    int length = _string.Length();
    uint64_t out[2];
    MurmurHash3_x64_128(_string.Data(), length, 0x6384BA69, out);
    TString _hashString = TString::ULLtoa(out[0], 16) + TString::ULLtoa(out[1], 16);
    return _hashString;
}

#endif
