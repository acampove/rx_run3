#include "TupleHolder.hpp"

#include "SettingDef.hpp"

#include "core.h"
#include "TKey.h"
#include "fmt_ostream.h"
#include "vec_extends.h"

ClassImp(TupleHolder)

TupleHolder::TupleHolder() : m_configHolder() 
{
    MessageSvc::Debug("TupleHolder", "Using default constructor");

    m_tupleOption = SettingDef::Tuple::option;
    m_tupleName   = SettingDef::Tuple::tupleName;
    m_fileName    = SettingDef::Tuple::fileName;

    _Check();
}

TupleHolder::TupleHolder(
        const ConfigHolder & _configHolder, 
        const TString      & _tupleOption) : m_configHolder(_configHolder) 
{
    MessageSvc::Debug("TupleHolder", "Using constructor with ConfigHolder and string option:\"", _tupleOption, "\"");

    m_tupleOption = _tupleOption; 
    m_tupleName   = _configHolder.GetConfig("tree_name");
    m_fileName    = SettingDef::Tuple::fileName;
    _Check();
}

TupleHolder::TupleHolder(
        const ConfigHolder & _configHolder, 
        const TString      & _fileName, 
        const TString      & _tupleName, 
        const TString      & _tupleOption) : m_configHolder(_configHolder) 
{
    MessageSvc::Debug("TupleHolder", "Using constructor with filename, tuplename and string option");
    MessageSvc::Debug("TupleHolder", "FileName:  ",  _fileName);
    MessageSvc::Debug("TupleHolder", "TupleName: ", _tupleName);

    m_tupleOption = _tupleOption;

    _Check();    
    Init(true, _fileName, _tupleName);
}

TupleHolder::TupleHolder(const TupleHolder & _tupleHolder) : m_configHolder(_tupleHolder.GetConfigHolder()) 
{
    MessageSvc::Debug("TupleHolder", "TupleHolder");

    m_tupleOption   = _tupleHolder.Option();
    m_tupleDir      = _tupleHolder.TupleDir();
    m_tupleName     = _tupleHolder.TupleName();
    m_fileName      = SettingDef::Tuple::fileName;
    m_tupleReader   = _tupleHolder.GetTupleReader();
    m_branches      = _tupleHolder.Branches();
    m_aliases       = _tupleHolder.Aliases();
    m_isInitialized = _tupleHolder.IsInitialized();

    _Check();
}

void TupleHolder::Init(bool _force, TString _fileName, TString _tupleName) 
{
    if (! _force && IsInitialized())
        return;

    if ( SettingDef::Tuple::noInit )
    {
        MessageSvc::Warning( "Init", "Initialize ", "Tuple::noInit==True, skipping Tuple loading");
        m_isInitialized = true;
        return; //Lazy initialization
    }

    MessageSvc::Info("Init ", "Initializing tuple holder");

    if ( SettingDef::Tuple::useURLS )
    {
        MessageSvc::Warning("Change eos.list to urls.list, prototyping of updates to ganga module");
        m_fileList = "urls.list";
    }

    PrintInline();

    if (SettingDef::Tuple::branches) 
        m_branches = GetBranches("INC");
    else
        MessageSvc::Debug("Init ", "Not turning off branches");

    if (SettingDef::Tuple::aliases) 
        m_aliases = GetAliases("INC");
    else
        MessageSvc::Debug("Init", "Not getting aliases");

    _SetTupleDir();
    _SetFileTupleNames(_fileName, _tupleName);
    _CreateTupleReader();
    _SetAllBranchesFromTree();
}

ostream & operator<<(ostream & os, const TupleHolder & _tupleHolder) 
{
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "TupleHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "tupleOption", _tupleHolder.Option());
    MessageSvc::Print((ostream &) os, "tupleDir", _tupleHolder.TupleDir());
    MessageSvc::Print((ostream &) os, "tupleName", _tupleHolder.TupleName());
    MessageSvc::Print((ostream &) os, "tupleBranches", to_string(_tupleHolder.Branches().size()));
    MessageSvc::Print((ostream &) os, "tupleAliases", to_string(_tupleHolder.Aliases().size()));
    os << _tupleHolder.GetTupleReader();
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

TupleHolder TupleHolder::GetMCDecayTupleHolder() 
{
    MessageSvc::Info("TupleHolder", (TString) "GetMCDecayTupleHolder");

    TString _tupleName           = SettingDef::Tuple::tupleName;
    SettingDef::Tuple::tupleName = "MCDT";
    TupleHolder _tupleHolder     = TupleHolder(this->GetConfigHolder(), this->Option());
    SettingDef::Tuple::tupleName = _tupleName;

    return _tupleHolder;
}

void TupleHolder::_Check() 
{
    if ( m_tupleOption.Contains("pro["))
        MessageSvc::Warning("TupleHolder check, skipped, pro[XXX] specificed");

    if ( m_tupleOption.Contains("cre["))
        MessageSvc::Warning("TupleHolder check, skipped, cre[XXX] specificed");

    MessageSvc::Debug("Check", "----------------------------");
    MessageSvc::Debug("Check", "Checking options ");
    for ( auto _opt : TokenizeString(m_tupleOption, SettingDef::separator)) 
    {        
        MessageSvc::Debug("Check", "    ", _opt);

        if (!CheckVectorContains(SettingDef::AllowedConf::TupleOptions, _opt)) 
        {
            cout << RED << *this << RESET << endl;
            MessageSvc::Fatal("Check", "Option \"" + _opt + "\" not in SettingDef::AllowedConf::TupleOptions");
        }
    }
    MessageSvc::Debug("Check", "----------------------------");
}

void TupleHolder::_SetAllBranchesFromTree()
{
    if (m_branches.size() != 0) 
        return;

    if (GetTuple()->GetListOfBranches() == nullptr) 
        return;

    for (auto * _branch : *GetTuple()->GetListOfBranches()) 
        m_branches.push_back(_branch->GetName());
}

void TupleHolder::Close() 
{
    MessageSvc::Info(Color::Cyan, "TupleHolder", (TString) "Close ...");
    m_tupleReader.Close();
    m_isInitialized = false;
}

void TupleHolder::Reset() 
{
    MessageSvc::Info(Color::Cyan, "TupleHolder", (TString) "Reset ...");
    m_tupleDir = "";
    m_fileName = "";
    Close();
}

void TupleHolder::_CreateTupleProcessName()
{
    auto sample         = m_configHolder.GetSample();
    auto is_lambda_reso = sample.Contains("Lb2pKJPs") || sample.Contains("Lb2pKPsi");

    if (m_tupleDir.Contains("_PQ") && is_lambda_reso) 
    {	
        if (m_tupleName == "DT" ) m_tupleName = "DecayTreeTuple/DecayTree";
        if (m_tupleName == "MCT") m_tupleName = "MCDecayTreeTuple/MCDecayTree";
        if (m_tupleName == "ET" ) m_tupleName = "EventTuple/EventTuple";

        m_tupleName.ReplaceAll("/", to_string(m_configHolder.GetAna()) + "/");
    } 
    else 
    {
        if (m_tupleName == "DT" ) m_tupleName = SettingDef::Tuple::DT;
        if (m_tupleName == "MCT") m_tupleName = SettingDef::Tuple::MCT;
        if (m_tupleName == "ET" ) m_tupleName = SettingDef::Tuple::ET;
        if (m_tupleName == "LT" ) m_tupleName = SettingDef::Tuple::LT;
    }      

    if (m_configHolder.GetProject() == Prj::RL) 
    {
        switch (m_configHolder.GetAna()) 
        {
            case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple_CutTree"; break;
            case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple_CutTree"; break;
            case Analysis::ME:
                m_tupleName = "Lb2LemuTuple_CutTree";
                if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) 
                    m_tupleName = "Lb2JpsiL_mmTuple_CutTree";
                break;
            default: 
                MessageSvc::Fatal("Wrong analysis", to_string(m_configHolder.GetAna())); 
                break;
        }
    }
}

void TupleHolder::_CreateTupleGangaName()
{
    if (m_tupleName == "DT")  m_tupleName = "DecayTreeTuple/DecayTree";
    if (m_tupleName == "MCT") m_tupleName = "MCDecayTreeTuple/MCDecayTree";
    if (m_tupleName == "ET")  m_tupleName = "EventTuple/EventTuple";

    m_tupleName.ReplaceAll("/", to_string(m_configHolder.GetAna()) + "/");

    if (m_configHolder.GetSample().Contains("SSHH")) m_tupleName.ReplaceAll("/", "_SSHH/"); // SS Hadrons location
    if (m_tupleName == "LT") m_tupleName  = "GetIntegratedLuminosity/LumiTuple";
}

//TODO: Clean this up
void TupleHolder::_CreateTupleRLRKSName()
{
    if (m_tupleOption.Contains("gng")) 
    {
        if (m_configHolder.GetProject() == Prj::RL) {
            switch (m_configHolder.GetAna()) {
                case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple/DecayTree"; break;
                case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple/DecayTree"; break;
                case Analysis::ME:
                                   m_tupleName = "Lb2LemuTuple/DecayTree";
                                   if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) m_tupleName = "Lb2JpsiL_mmTuple/DecayTree";
                                   break;
                default: MessageSvc::Fatal("Wrong analysis", to_string(m_configHolder.GetAna())); break;
            }
        }
        if (m_configHolder.GetProject() == Prj::RKS) 
        {
            switch (m_configHolder.GetAna()) 
            {
                case Analysis::MM: 
                    m_tupleName = "Bd2JpsiKs_mmTuple/DecayTree"; break;
                default: 
                    MessageSvc::Fatal("Wrong analysis", to_string(m_configHolder.GetAna())); break;
            }
        }
    }

    if (m_tupleOption.Contains("pro")) 
    {            
        if (m_configHolder.GetProject() == Prj::RL) 
        {
            switch (m_configHolder.GetAna()) 
            {
                case Analysis::MM: m_tupleName = "Lb2JpsiL_mmTuple_CutTree"; break;
                case Analysis::EE: m_tupleName = "Lb2JpsiL_eeTuple_CutTree"; break;
                case Analysis::ME:
                                   m_tupleName = "Lb2LemuTuple_CutTree";
                                   if (m_configHolder.GetQ2bin() == Q2Bin::JPsi) m_tupleName = "Lb2JpsiL_mmTuple_CutTree";
                                   break;
                default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
            }
        }

        if (m_configHolder.GetProject() == Prj::RKS) 
        {
            switch (m_configHolder.GetAna()) {
                case Analysis::MM: m_tupleName = "Bd2JpsiKs_mmTuple_CutTree"; break;
                default: MessageSvc::Error("Wrong analysis", to_string(m_configHolder.GetAna()), "EXIT_FAILURE"); break;
            }
        }
    }
}

TString TupleHolder::_GetTuplePAPName()
{
    auto is_run3 = m_configHolder.IsRun3();

    if (is_run3) 
    {
        auto tupleName = m_configHolder.GetTreeName();
        MessageSvc::Debug("_GetTuplePAPName", "Picking tuple name from config holder: ", tupleName);

        return tupleName;
    }

    if (m_tupleName != "")
    {
        MessageSvc::Debug("_GetTuplePAPName", "Picking default tuple name: ", m_tupleName);

        return m_tupleName;
    }

    MessageSvc::Fatal(TString("_GetTuplePAPName"), "Tuple name was not found for Run2 sample:", m_tupleName);

    return ""; //Line above will throw exception, this line will silence compiler warning
}

void TupleHolder::_SetFileTupleNames(const TString &_fileName, const TString &_tupleName)
{
    m_fileName  = _fileName  != "" ? _fileName  : SettingDef::Tuple::fileName;
    m_tupleName = _tupleName != "" ? _tupleName : SettingDef::Tuple::tupleName;

    MessageSvc::Debug("_SetFileTupleNames", "FileName: " , m_fileName );
    MessageSvc::Debug("_SetFileTupleNames", "TupleName: ", m_tupleName);

    if (m_tupleOption.Contains("pap")) 
    {
        m_tupleName = _GetTuplePAPName();

        MessageSvc::Debug("CreateTupleName", "Tuple name: ", m_tupleName);
        return;
    }

    if ( m_tupleOption.Contains("pro")) 
        _CreateTupleProcessName();

    if (m_tupleOption.Contains("gng")) 
        _CreateTupleGangaName();

    if (m_tupleOption.Contains("cre")) 
        m_tupleName = m_configHolder.GetTupleName(m_tupleOption); 

    if (m_tupleOption.Contains("spl")) 
    {
        if (m_tupleOption.Contains("splp")) 
            // SPLOT FROM TUPLEPROCESS
            m_tupleName = SettingDef::Tuple::DT;
        else 
            // SPLOT FROM TUPLECREATE
            m_tupleName = m_configHolder.GetTupleName(m_tupleOption);
    }

    // TupleRapidSim
    if (m_tupleOption.Contains("rap")) 
        m_tupleName = SettingDef::Tuple::RST; 

    if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) 
        _CreateTupleRLRKSName();

    MessageSvc::Debug("CreateTupleName", "Tuple name: ", m_tupleName);
}

void TupleHolder::_SetTupleDir() 
{
    MessageSvc::Debug("CreateTupleDir", "File name: "   , m_fileName   );
    MessageSvc::Debug("CreateTupleDir", "Tuple option: ", m_tupleOption);

    if (m_fileName == "") 
    {
        m_tupleDir = IOSvc::GetTupleDir(m_tupleOption, m_configHolder);
        MessageSvc::Debug("_SetTupleDir", "Using default tuple directory: ", m_tupleDir);

        return;
    }

    m_tupleDir = m_fileName;
    //Removes everythng from last '/' till the end.
    //Modifies the string in place
    m_tupleDir.Remove(m_fileName.Last('/'), m_fileName.Length());

    MessageSvc::Debug("CreateTupleDir", "Tuple directory: ", m_tupleDir);
}

TString TupleHolder::_TupleNameForCRE()
{
    MessageSvc::Warning("TupleHolder, chain TTree in TupleCreate, force disabling implicit MT. Root is bugged for MT with TChains having different TTreeNames , see https://root-forum.cern.ch/t/really-solved-rdataframe-for-tchain-loaded-with-ttrees-with-different-names/41756");
    MessageSvc::Warning("TupleHolder, Disabling ImplicitMT now, please check if L0I, L0L separately and L0I+L0L together gives you same nentries counting!");
    ROOT::DisableImplicitMT();

    TString  _sample  = m_configHolder.GetSample();
    Trigger  trigger  = m_configHolder.GetTrigger();
    Year     year     = m_configHolder.GetYear();
    Polarity polarity = m_configHolder.GetPolarity();

    TString  _tupleName;

    if (m_configHolder.GetTrigger() == Trigger::All) 
    {
        // TupleCreate with TriggerConf::Exclusive
        MessageSvc::Info("CreateTupleReader", "Chaining TriggerConf::Exclusive tuples for Trigger::All");
        m_tupleName.ReplaceAll(_sample, _sample + "_" + to_string(Trigger::L0I));
        _tupleName = m_tupleName;
        _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");

        return _tupleName;
    } 

    // TupleCreate with TriggerConf::Exclusive
    MessageSvc::Info("CreateTupleReader", "Chaining TriggerConf::", SettingDef::Config::triggerConf, " tuples for Trigger::", to_string(trigger));
    _tupleName = m_tupleName;
    if (hash_triggerconf(SettingDef::Config::triggerConf) == TriggerConf::Exclusive) 
    {
        if (trigger == Trigger::L0I) 
            _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");

        if (trigger == Trigger::L0L) 
            _tupleName.ReplaceAll(to_string(Trigger::L0L) + "exclusive", to_string(Trigger::L0I));
    } 
    else if (hash_triggerconf(SettingDef::Config::triggerConf) == TriggerConf::Exclusive2) 
    {
        if (trigger == Trigger::L0I) 
            m_tupleName.ReplaceAll(to_string(Trigger::L0I) + "exclusive", to_string(Trigger::L0I));

        if (trigger == Trigger::L0L) 
            m_tupleName.ReplaceAll(to_string(Trigger::L0L), to_string(Trigger::L0L) + "exclusive");

        _tupleName = m_tupleName;

        if (trigger == Trigger::L0I) 
            _tupleName.ReplaceAll(to_string(Trigger::L0I), to_string(Trigger::L0L) + "exclusive");

        if (trigger == Trigger::L0L) 
            _tupleName.ReplaceAll(to_string(Trigger::L0L) + "exclusive", to_string(Trigger::L0I));
    } 
    else
        MessageSvc::Fatal("CreateTupleReader", "Incompatible TriggerConf to merge: ", SettingDef::Config::triggerConf);

    MessageSvc::Info("CreateTupleReader", m_configHolder.GetSample(), to_string(year), to_string(polarity), m_tupleName, m_tupleDir);
    if (_tupleName != m_tupleName) 
        MessageSvc::Info("_TupleNameForCRE", m_configHolder.GetSample(), to_string(year), to_string(polarity), _tupleName, m_tupleDir);

    return _tupleName;
}

TString TupleHolder::_GetSampleName()
{
    TString  _sample  = m_configHolder.GetSample();
    if( _sample == "Bs2KsKstJPsEE") 
        _sample = "Bs2XJPsEE";// HACK for this , use the inclusive and tmCustom to filter.

    // TupleGanga  
    if (!m_tupleOption.Contains("cre"))
        _sample.ReplaceAll("KstSwap", "Kst");

    return _sample;
}

void TupleHolder::_RenameGangaSampleName(TString &_sample)
{
    if (!m_configHolder.IsMC()) 
    {
        _sample.ReplaceAll("LPTSS", "LPT_SS");
        _sample.ReplaceAll("LPT_SSHH", "LPT_SS"); // SS Hadrons saved in same DVNtuple
                                                  //
        return;
    }


    if( !_sample.Contains("EtaPrime") && !_sample.Contains("EtaG") ) 
        _sample.ReplaceAll("Eta", "Eta_"); //hack to do due to naming EtaPrimeGEE samples, no splitting ! 

    _sample.ReplaceAll("JPs", "JPs_");
    _sample.ReplaceAll("Psi", "Psi_");
    _sample.ReplaceAll("Pi0", "Pi0_");
    _sample.ReplaceAll("KstG", "KstG_");
    _sample.ReplaceAll("EESS", "EE_SS");
    _sample.ReplaceAll("MMSS", "MM_SS");
}

bool TupleHolder::_AddPAPSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities)
{
    if (m_fileName != "")
    {
        MessageSvc::Debug("_AddPAPSamplesToReader", " Adding file ", m_fileName, ":", m_tupleName);
        auto status = m_tupleReader.AddFile(m_fileName, m_tupleName);
        return status;
    }

    auto data_dir = m_configHolder.GetDataDir();
    auto sample   = m_configHolder.GetSample();
    auto hlt2     = m_configHolder.GetHlt2();

    TString sample_wildcard = data_dir + "/" + sample + "/" + hlt2 + "/*.root";

    MessageSvc::Debug("_AddPAPSamplesToReader", " Adding files with wildcard: ", sample_wildcard);
    m_tupleReader.AddFiles(sample_wildcard);

    return true;
}

bool TupleHolder::_AddGangaSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities)
{
    bool _addList = true ;

    _RenameGangaSampleName(_sample);
    for (const auto & _year : _years) 
    {
        for (const auto & _polarity : _polarities) 
        {
            if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) 
                _addList = m_tupleReader.AddList(m_tupleDir + "/" + _type + _year + "_" + _sample + _polarity + ".list");
            else 
                _addList = m_tupleReader.AddList(m_tupleDir + "/" + _type + _year + "_" + _sample + _polarity + "/" + m_fileList);
        }
    }

    return _addList;
}

bool TupleHolder::_AddProSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities)
{
    if ( m_tupleOption.Contains("pro["))
        MessageSvc::Warning("_AddProSamplesToReader", "proVer not global via Option ", m_tupleOption);                

    bool _addList  = false;
    int _mustEXIST = _years.size() * _polarities.size();          
    for (const auto & _year : _years) 
    {
        for (const auto & _polarity : _polarities) 
        {
            if ( 
                    SettingDef::Tuple::addTuple == false && 
                    SettingDef::Weight::useBS            && 
                    SettingDef::Weight::iBS     >=0 )
            { 
                MessageSvc::Warning("_AddProSamplesToReader", "Break AddList TupleProcess pro, no add List (for BS and iBS >0 flag, if you see this in non-BS fits mode, this is a bug)"); 
                break; /*TO FAST BS FITS from TupleProcess...*/
            }

            TString _list = m_tupleDir + "/" + _sample + "/" + _type + _year + _polarity + "/" + m_fileList;
            _list = IOSvc::XRootDFileName(_list);
            if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) 
            { 
                _list = m_tupleDir + "/" + _sample + "/" + _type + _year + ".list"; 
            }   // DIFFERENT NAME

            TString _listBkp = _list;
            if (IOSvc::ExistFile(_list)) 
            {
                TString _listTmp = "";
                if (_list.Contains("/eos/")) 
                {
                    _listTmp = SettingDef::IO::outDir +"/" + m_fileList;
                    _listTmp.ReplaceAll(".//", "./");
                    MessageSvc::Debug("_AddProSamplesToReader", "Copying", _list, _listTmp);
                    IOSvc::CopyFile(_list, _listTmp);
                    _list = _listTmp;
                }

                MessageSvc::Info("_AddProSamplesToReader", "Adding", _listBkp);
                _addList = m_tupleReader.AddList(_list);
                if (_listTmp != "") 
                    IOSvc::RemoveFile(_listTmp);
            } 
            else 
            {
                MessageSvc::Warning("_AddProSamplesToReader", _listBkp, "does not exist, trying glob");
                if (m_tupleDir.Contains("/eos/") && !IsBATCH("CERN")) 
                    MessageSvc::Warning("_AddProSamplesToReader", "Cannot glob eos outside CERN");

                if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) 
                {   // DIFFERENT NAME
                    m_tupleReader.AddFiles(m_tupleDir + "/" + _sample + "/" + _type + _year + "*.root");
                } 
                else 
                {
                    m_tupleReader.AddFiles(m_tupleDir + "/" + _sample + "/" + _type + _year + _polarity + "/*/*.root");
                }
            }
            if ((m_configHolder.GetProject() == Prj::RL) || (m_configHolder.GetProject() == Prj::RKS)) break;   // NO SPLIT BY POLARITY
        }    
    }
    /*
       Protection for eos. If you deal with TupleCreate / LPT and SignalMC, the whole set of data must be loaded ( 11MD + 11MU + 12MU + 12MD ). 
       I.e avoid this to silently load only a part of what is needed. 
       */
    if ( (m_configHolder.GetProject()==Prj::RK || m_configHolder.GetProject()==Prj::RKst) && SettingDef::Tuple::gngVer >=10 && SettingDef::Tuple::proVer != "SKIM")
    {        
        if( m_configHolder.IsSignalMC() && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0)
        {
            if( SettingDef::Tuple::addTuple)
            {
                if( m_configHolder.GetSample().Contains("Bd2KstGEE"))
                    MessageSvc::Warning("_AddProSamplesToReader", Form("HACK for Conv-Gamma,CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
                else
                    MessageSvc::Error("_AddProSamplesToReader", Form("CreateTupleReader %s, proVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()),"","EXIT_FAILURE");
            }
            else
                MessageSvc::Warning("_AddProSamplesToReader", Form("CreateTupleReader %s, proVer, must have loaded the exact nFiles for polarities and years asked for [addTuple=False, no Error]",m_configHolder.GetSample().Data()));
        }
    }

    return _addList;
}

bool TupleHolder::_AddCreSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities)
{
    auto _tupleName = SettingDef::Tuple::chainexctrg ? _TupleNameForCRE() : m_tupleName;

    if( m_tupleOption.Contains("cre["))
        MessageSvc::Warning("_AddCreSamplesToReader", "creVer not global via Option ", m_tupleOption);                

    int _mustEXIST = _years.size() * _polarities.size();
    if (SettingDef::Tuple::chainexctrg) 
        _mustEXIST *= 2; //If both exclusive categories loaded, factor 2 is needed.

    for (const auto & _year : _years) 
    {
        for (const auto & _polarity : _polarities) 
        {
            m_tupleReader.AddFile(m_tupleDir + "/" + _year + _polarity + "/TupleCreate.root");
            if (_tupleName != m_tupleName) 
                m_tupleReader.AddFile(m_tupleDir + "/" + _year + _polarity + "/TupleCreate.root", _tupleName);
        }
    }
    /*
       Protection for eos. If you deal with TupleCreate / LPT and SignalMC, the whole set of data must be loaded ( 11MD + 11MU + 12MU + 12MD ). 
       I.e avoid this to silently load only a part of what is needed. 
       */
    if ( m_configHolder.GetSample() == "LPT" && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0){
        if( SettingDef::Tuple::addTuple )
            MessageSvc::Fatal("_AddCreSamplesToReader", "LPT, creVer, must have loaded the exact nFiles for polarities and years asked for");
        else
            MessageSvc::Warning("_AddCreSamplesToReader", Form("CreateTupleReader LPT, creVer, must have loaded the exact nFiles for polarities and years asked for [addTuple=False, no Error]"));
    }
    if ( m_configHolder.IsSignalMC() && m_tupleReader.GetNFiles() != _mustEXIST && SettingDef::Tuple::frac < 0)
    {
        if( SettingDef::Tuple::addTuple)
        {          
            if( m_configHolder.GetSample().Contains("Bd2KstGEE") && m_configHolder.GetProject() == Prj::RKst){      
                MessageSvc::Warning(Form("HACK for Conv-Gamma,CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
            }else{
                MessageSvc::Error(Form("CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()),"","EXIT_FAILURE");
            }
        }
        else
            MessageSvc::Warning("_AddCreSamplesToReader", Form("CreateTupleReader %s, creVer, must have loaded the exact nFiles for polarities and years asked for",m_configHolder.GetSample().Data()));
    }

    bool _trowLogicError       = SettingDef::trowLogicError;
    bool _useEOS               = SettingDef::IO::useEOS;
    SettingDef::trowLogicError = true;
    SettingDef::IO::useEOS     = true;
    try 
    {
        m_tupleReader.Init();
    } 
    catch (const exception & e) 
    {
        MessageSvc::Line();
        MessageSvc::Warning("CreateTupleReader", (TString) "Trying TupleProcess", SettingDef::Tuple::proVer, "on EOS");
        MessageSvc::Line();

        if (SettingDef::Tuple::proVer == "") 
            MessageSvc::Fatal("CreateTupleReader", "proVer not defined");

        SettingDef::trowLogicError = _trowLogicError;
        m_tupleOption              = "pro";
        m_tupleName                = "DT";
        Init();
    }
    SettingDef::trowLogicError = _trowLogicError;
    SettingDef::IO::useEOS     = _useEOS;

    return true;
}

bool TupleHolder::_AddSplSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities)
{
    Analysis ana    = m_configHolder.GetAna();
    for (const auto & _year : _years) 
    {
        if (m_configHolder.GetTrigger() != Trigger::All) 
        {
            _sample = "FitHolder_" + m_configHolder.GetKey() + "_SPlot";
            m_tupleReader.AddFile(m_tupleDir + "/" + _year + "/" + _sample + ".root");
            if (m_tupleOption.Contains("splp")) 
            {
                // SPLOT FROM TUPLEPROCESS
                m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/" + _sample + ".root", SettingDef::Tuple::SPT);
            } 
            else 
            {
                // SPLOT FROM TUPLECREATE
                // m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root", SettingDef::Tuple::SPT);
                m_tupleReader.AddFriend(m_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root", SettingDef::Tuple::SPT);
            }
        } 
        else 
        {
            TupleReader _tupleReader = TupleReader(SettingDef::Tuple::SPT);
            for (auto _trigger : GetTriggers(to_string(m_configHolder.GetTrigger()), true)) {
                SettingDef::Config::trigger = _trigger;
                ConfigHolder _co            = ConfigHolder();
                TString      _tupleDir      = IOSvc::GetTupleDir(m_tupleOption, _co);
                _sample                     = "FitHolder_" + _co.GetKey() + "_SPlot";
                if (m_tupleOption.Contains("splp")) {
                    // SPLOT FROM TUPLEPROCESS
                    m_tupleReader.AddFile(_tupleDir + "/" + _year + "/" + _sample + ".root");
                    _tupleReader.AddFile(_tupleDir + "/" + _year + "/" + _sample + ".root");
                } else {
                    // SPLOT FROM TUPLECREATE
                    // m_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root");
                    //_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + _sample + ".root");
                    m_tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root");
                    _tupleReader.AddFile(_tupleDir + "/" + _year + "/TupleSPlot" + SettingDef::separator + to_string(ana) + ".root");
                }
            }
            m_tupleReader.AddFriend(_tupleReader.Tuple());
            SettingDef::Config::trigger = to_string(Trigger::All);
        }
    }

    return true;
}

bool TupleHolder::_MakeReaderFromPaths()
{
    if (m_tupleName == "" || m_fileName == "")
    {
        MessageSvc::Debug("_MakeReaderFromPaths", "Won't make TupleReader from paths: ", m_fileName, ":", m_tupleName);
        return false; 
    }

    MessageSvc::Info("_MakeReaderFromPaths", m_fileName, ":", m_tupleName);
    MessageSvc::Info("_MakeReaderFromPaths",   "AddTuple = ", SettingDef::Tuple::addTuple ? "true" : "false");

    m_tupleReader = TupleReader(m_tupleName, m_fileName);
    m_tupleReader.Init();
    SetBranches();
    SetAliases();
    m_isInitialized = true;

    return true;
}

void TupleHolder::_CreateTupleReader() 
{
    MessageSvc::Info("_CreateTupleReader ", "Making TupleReader");

    auto status = _MakeReaderFromPaths();
    if (status)
        return;

    Year     year                 = m_configHolder.GetYear();
    vector< TString > _years      = GetYears(to_string(year));

    Polarity polarity             = m_configHolder.GetPolarity();
    vector< TString > _polarities = GetPolarities(to_string(polarity));

    TString _type                 = m_configHolder.IsMC() ? "MC" : "CL";

    MessageSvc::Info("CreateTupleReader", "Creating TupleReader from tuple name: \"", m_tupleName, "\"");
    MessageSvc::Debug("Years:      ", to_string(year    ));
    MessageSvc::Debug("Polarities: ", to_string(polarity));

    MessageSvc::Info("CreateTupleReader", "AddTuple =", SettingDef::Tuple::addTuple ? " true" : " false");

    m_tupleReader = TupleReader(m_tupleName);

    auto _sample  = _GetSampleName();
    auto _addList = false;

    if (m_tupleOption.Contains("gng")) 
        _addList = _AddGangaSamplesToReader(_sample, _type, _years, _polarities);

    if ( m_tupleOption.Contains("pro") ) 
        _addList = _AddProSamplesToReader(_sample, _type, _years, _polarities);

    if (m_tupleOption.Contains("cre")) 
        _addList = _AddCreSamplesToReader(_sample, _type, _years, _polarities);

    if (m_tupleOption.Contains("spl")) 
        _addList = _AddSplSamplesToReader(_sample, _type, _years, _polarities);

    if (m_tupleOption.Contains("pap")) 
        _addList = _AddPAPSamplesToReader(_sample, _type, _years, _polarities);

    if (m_tupleOption.Contains("rap")) 
        m_tupleReader.AddFile(m_tupleDir + "/" + _sample + "_tree.root"); 
	
    _CheckAddedList(_addList);

    m_tupleReader.Init();

    SetBranches();
    SetAliases();
    m_isInitialized = true;
}

void TupleHolder::_CheckAddedList(const bool &_addList)
{
    if (_addList || SettingDef::Tuple::datasetCache) 
        return;

    auto is_background_mc = m_configHolder.IsMC() && !m_configHolder.IsSignalMC();
    if (is_background_mc)
        MessageSvc::Warning(      "CreateTupleReader" ,  "AddList incomplete");
    else
        MessageSvc::Fatal(TString("CreateTupleReader"),  "AddList incomplete");
}

void TupleHolder::CreateSubTupleReader(int _iFile) 
{
    if (_iFile < m_tupleReader.GetNFiles()) 
    {
        m_fileName = m_tupleReader.GetFileName(_iFile);

        MessageSvc::Info("TupleHolder", (TString) "CreateSubTupleReader for", m_fileName);

        m_tupleReader = TupleReader(m_tupleName);
        m_tupleReader.AddFile(m_fileName);
        m_tupleReader.Init();

        if (m_tupleReader.IsInitialized()) {
            SetBranches();
            SetAliases();
            m_isInitialized = true;
        }

        SettingDef::Tuple::fileName = m_fileName;
    } else
        MessageSvc::Error("TupleHolder", (TString) "CreateSubTupleReader", to_string(_iFile) + "th", "file does not exist (NFiles =", to_string(m_tupleReader.GetNFiles()) + ")", "EXIT_FAILURE");
    return;
}

vector< TString > TupleHolder::GetBranches(TString _option) 
{
    MessageSvc::Debug("GetBranches", "Getting branches with option : ", _option);

    vector< TString > _branches;

    TString _card = IOSvc::GetDataDir("cards");
    if (_option.Contains("EXC"))
        _card += "/branches" + to_string(m_configHolder.GetProject()) + ".txt";
    else if (_option.Contains("MVA"))
        _card += "/branches" + to_string(m_configHolder.GetProject()) + "MVA.txt";
    else if (_option.Contains("SKIM"))
        _card += "/branchesSKIM.txt";
    else if (_option.Contains("INC"))
        _card += "/branchesINC.txt";

    vector< vector< TString > > _lines = IOSvc::ParseFile(_card, "");
    for (auto & _line : _lines) { _branches.push_back(_line[0]); }
    MessageSvc::Info("GetBranches", _card, to_string(_branches.size()));

    if (_option.Contains("EXC")) {
        _card.ReplaceAll(to_string(m_configHolder.GetProject()), "TRUE");
        vector< vector< TString > > _lines_EXC = IOSvc::ParseFile(_card, "");
        for (auto & _line : _lines_EXC) { _branches.push_back(_line[0]); }
        MessageSvc::Info("GetBranches", _card, to_string(_branches.size()));
    }

    if (m_tupleReader.IsInitialized()) {
        if (_branches.size() != 0) {
            vector< TString > _branchesTmp;
            for (auto * _branch : *GetTuple()->GetListOfBranches()) {
                TString _name = _branch->GetName();
                if (find(_branchesTmp.begin(), _branchesTmp.end(), _name) == _branchesTmp.end()) {
                    for (const auto & _b : _branches) {
                        if (_b.BeginsWith("_") && _name.EndsWith(_b)) _branchesTmp.push_back(_name);
                        if (!_b.BeginsWith("_") && (_name == _b)) _branchesTmp.push_back(_name);
                    }
                }
            }
            RemoveVectorDuplicates(_branchesTmp);
            MessageSvc::Info("GetBranches", GetTuple()->GetName(), to_string(_branches.size()), "->", to_string(_branchesTmp.size()));
            _branches = _branchesTmp;
        }
    }

    auto nbranch = _branches.size();

    MessageSvc::Debug("GetBranches", "Will keep ", std::to_string(nbranch), " branches");

    return _branches;
}

void TupleHolder::SetBranches(vector< TString > _branches) 
{
    if (_branches.size() == 0) 
        _branches = m_branches;

    int _nBranches = _branches.size();
    if (_branches.size() != 0) 
    {
        MessageSvc::Info("SetBranches", (TString) GetTuple()->GetName(), to_string(GetTuple()->GetNbranches()), "->", to_string(_branches.size()));
        m_tupleReader.SetBranches({}, false, "all");
        _nBranches = m_tupleReader.SetBranches(_branches, true);
        m_branches = _branches;
    } 
    else 
        _nBranches = m_tupleReader.SetBranches({}, true, "all");

    if (_nBranches != _branches.size()) 
        MessageSvc::Info("SetBranches", GetTuple()->GetName(), ": ", to_string(GetTuple()->GetNbranches()), "->", to_string(_nBranches));    
}

void TupleHolder::CheckBranches(vector< TString > _branches, bool deep ) {
    if( !deep){
        if (_branches.size() == 0) _branches = m_branches;
        if (_branches.size() != 0) {
            MessageSvc::Info("CheckBranches", (TString) GetTuple()->GetName(), to_string(_branches.size()), "out of", to_string(GetTuple()->GetNbranches()));
            int _count = 0;
            for (auto _branch : _branches) {
                if (!CheckVarInTuple(_branch)) {
                    MessageSvc::Warning("TupleHolder", (TString) "Branch", _branch, "does not exist");
                    _count++;
                }
            }
            MessageSvc::Line();
            if (_count != 0)
                MessageSvc::Warning(GetTuple()->GetName(), to_string(_count), "missing branches");
            else
                MessageSvc::Info(GetTuple()->GetName(), (TString) "All requested branches present");
            MessageSvc::Line();
        }
    }else{
        if( !m_tupleReader.CheckVarsInChain( _branches, m_tupleName)){
            MessageSvc::Error("INCOMPATIBLE BRANCHES ON TCHAINS, CANNOT CUT SAFELY, ABORT AND PLEASE REPORT/FIX IT. THIS IS A DANGEROUS A BUG!", "","EXIT_FAILURE");
        };
    }
        
    return;
}


const bool TupleHolder::IsSampleInCreVer(TString _creVer, TString _prj, TString _ana, TString _q2bin, TString _year , TString _trigger, TString _triggerConf, TString _sample) {
    SettingDef::Tuple::creVer = _creVer;
    TString _tupleDir = IOSvc::GetTupleDir("cre", _prj, _ana, _q2bin, _year, _trigger);
    MessageSvc::Info("IsSampleInCreVer: check for", _sample, "in tuple ", _tupleDir + "/" + _year + "MD/TupleCreate.root");
    TFile _file(_tupleDir + "/" + _year + "MD/TupleCreate.root", to_string(OpenMode::READ));
    _file.GetListOfKeys()->Print();
    TIter   _keyList(_file.GetListOfKeys());
    TKey *  _key;
    while ((_key = (TKey *) _keyList())) {
        if (((TString) _key->ReadObj()->GetName()).Contains(_sample)) return true;
    }
    return false;
};

TString TupleHolder::_GetAliasFile(const TString &_option)
{
    MessageSvc::Debug("GetAliasFile", "Getting alias file with option : ", _option);

    TString _card = IOSvc::GetDataDir("cards");
    if      (_option.Contains("EXC"))
        _card += "/aliases" + to_string(m_configHolder.GetProject()) + ".txt";
    else if (_option.Contains("SKIM"))
        _card += "/aliasesSKIM.txt";
    else if (_option.Contains("INC"))
        _card += "/aliasesINC.txt";

    return _card;
}

bool TupleHolder::_SkipAliasForProject(const TString & _alias, const TString & _expr)
{
    Prj project = m_configHolder.GetProject();
    bool _skip  = false;

    switch (project) 
    {
        case Prj::RK:
            if (_alias.Contains("{HH}") || _alias.Contains("_HH") || _alias.Contains("HH_") || _alias.Contains("H2_") || _alias.Contains("{H2}")) 
                _skip = true;

            if (_expr.Contains("{HH}") || _expr.Contains("_HH") || _expr.Contains("HH_") || _expr.Contains("H2_") || _expr.Contains("{H2}"))      
                _skip = true;
            break;
        case Prj::RKst: break;
        case Prj::RPhi: break;
        case Prj::RL:  break;
        case Prj::RKS: break;
        default: 
            MessageSvc::Fatal("_SkipProject", "Wrong project", to_string(project));
    }

    if (_skip) 
        MessageSvc::Debug("GetAliases", "Invalid Alias definition for", to_string(project), "project", _alias, "->", _expr, "SKIPPING");

    return _skip;
}

bool TupleHolder::_SKipAliasForAnalysis(const TString & _alias, const TString & _expr)
{
    Analysis ana = m_configHolder.GetAna();
    bool _skip   = false;

    switch (ana) 
    {
        case Analysis::MM:
            if (_expr.Contains("M1_TRACK") || _expr.Contains("M2_TRACK")) _skip = true;
            if (_alias.Contains("E1_")     || _alias.Contains("E2_")    ) _skip = true;
            if (_expr.Contains("E1_")      || _expr.Contains("E2_")     ) _skip = true;
            break;
        case Analysis::EE:
            if (_alias.Contains("M1_")     || _alias.Contains("M2_")    ) _skip = true;
            if (_expr.Contains("M1_")      || _expr.Contains("M2_")     ) _skip = true;
            break;
        case Analysis::ME:
            if (_alias.Contains("E1_")     && _alias.Contains("E2_"    )) _skip = true;
            if (_alias.Contains("M1_")     && _alias.Contains("M2_"    )) _skip = true;
            if (_expr.Contains("E1_")      && _expr.Contains("E2_"     )) _skip = true;
            if (_expr.Contains("M1_")      && _expr.Contains("M2_"     )) _skip = true;
            break;
        default: 
            MessageSvc::Error("Wrong analysis", to_string(ana), "EXIT_FAILURE"); break;
    }

    if (_skip) 
        MessageSvc::Debug("GetAliases", (TString) "Invalid Alias definition for", to_string(ana), "analysis", _alias, "->", _expr, "SKIPPING");

    return _skip;
}

vector< pair< TString, TString > > TupleHolder::GetAliases(const TString & _option) 
{
    MessageSvc::Debug("GetAliases", "Getting aliases with option: ", _option);

    auto _card = _GetAliasFile(_option);

    map< TString, TString > _names = m_configHolder.GetParticleNames();

    vector< pair< TString, TString > > _aliases;
    vector< vector< TString > > _lines = IOSvc::ParseFile(_card, " ");
    for (auto & _line : _lines) 
    {
        TString _alias = _line[0];
        TString _expr  = _line[1];

        if (_alias.Contains("_wMVA_") && (SettingDef::Cut::mvaVer == "")) 
            continue;

        if (_alias.Contains("_wMVA_") && _alias.Contains("_wMVA_PR_") && (m_configHolder.GetAna() == Analysis::MM)) 
            continue;

        if ( _SkipAliasForProject(_alias, _expr) )
            continue;

        _alias = ReplaceWildcards(_alias, _names);

        if (_expr.Contains("{TUNE}")) 
        {
            TString _tuneH = GetPIDTune(to_string(m_configHolder.GetYear()), "H");
            TString _tuneL = GetPIDTune(to_string(m_configHolder.GetYear()), "L");
            if (_expr.Contains("{H")) _expr.ReplaceAll("{TUNE}", _tuneH);
            if (_expr.Contains("{L")) _expr.ReplaceAll("{TUNE}", _tuneL);
        }

        _expr = ReplaceWildcards(_expr, _names);

        if ( _SKipAliasForAnalysis(_alias, _expr) )
            continue;

        MessageSvc::Debug("GetAliases", _alias, "->", _expr);
        _aliases.push_back(make_pair(_alias, _expr));
    }

    MessageSvc::Info("GetAliases", _card, to_string(_aliases.size()));

    return _aliases;
}

void TupleHolder::SetAliases(const vector< pair< TString, TString > > &_aliases) 
{
    if (_aliases.size() != 0) 
    {
        MessageSvc::Warning("SetAliases", "Overriding aliases");
        m_aliases = _aliases;
    }   

    int _nAliases = m_aliases.size();
    if (_nAliases == 0) 
    {
        MessageSvc::Info("SetAliases", "No aliases found");
        return;
    }

    MessageSvc::Info("SetAliases", GetTuple()->GetName(), to_string(_nAliases));
    m_tupleReader.SetAliases(m_aliases);
}

TTree * TupleHolder::GetTuple(TCut _cut, TString _tupleName) {
    TTree * _tuple = nullptr;
    if (_tupleName == "") _tupleName = m_tupleName;
    if (SettingDef::Tuple::dataFrame) {
        _tuple = m_tupleReader.SnapshotTuple(_cut, _tupleName, SettingDef::Tuple::frac, false, m_branches, GetVectorFirst(m_aliases));
    } else {
        _tuple = m_tupleReader.CopyTuple(_cut, _tupleName, SettingDef::Tuple::frac, false);
    }
    return _tuple;
}

const Long64_t TupleHolder::TupleEntries(TCut _cut) {
    Long64_t _entries = 0;
    if (IsCut(_cut)) {
        if (HasWeight(_cut)) {
            _entries = round(GetEntries(*GetTuple(), _cut));
        } else {
            // if (SettingDef::Tuple::dataFrame) {
            //    _entries = GetEntriesDF(*GetTuple(), _cut);
            //} else {
            _entries = GetTuple()->GetEntries(_cut);
            //}
        }
    } else {
        _entries = GetTuple()->GetEntriesFast();
    }
    return _entries;
}

pair< double, double > TupleHolder::GetLuminosity() {
    if (!m_configHolder.IsMC() && ((m_tupleName == "LT") || m_tupleName.Contains("Lumi"))) {
        // For errors on luminosity look why we sum it up :
        // https://indico.cern.ch/event/100756/contributions/1296401/attachments/3262/4947/Lumi_Yas.pdf#search=yasmine%20amhis%20luminosity
        // In 2016-2017-2018 data the error is the same as the value, as not calibrated yet....
        MessageSvc::Line();
        MessageSvc::Info("TupleHolder", (TString) "Luminosity");
        MessageSvc::Line();

        double _lumiT  = 0;
        double _lumiTE = 0;

        TTreeReaderValue< double > _lumi  = m_tupleReader.GetValue< double >("IntegratedLuminosity");
        TTreeReaderValue< double > _lumiE = m_tupleReader.GetValue< double >("IntegratedLuminosityErr");

        Long64_t _nEntries = m_tupleReader.GetEntries();
        MessageSvc::Line();
        MessageSvc::Info("TupleHolder", (TString) "Looping ...", to_string(_nEntries));
        Long64_t i = 0;
        while (m_tupleReader.Reader()->Next()) {
            MessageSvc::ShowPercentage(i, _nEntries);
            // m_tupleReader.GetEntry(i);
            _lumiT += *_lumi;
            _lumiTE += *_lumiE;
            i++;
        }

        RooRealVar _iLumi = RooRealVar("IntegratedLuminosity", "IntegratedLuminosity", _lumiT);
        _iLumi.setError(_lumiTE);

        MessageSvc::Line();
        MessageSvc::Info("Luminosity", &_iLumi);
        MessageSvc::Line();

        return make_pair(_iLumi.getValV(), _iLumi.getError());
    }
    return make_pair(0, 0);
}

void TupleHolder::PrintInline() const noexcept 
{
    m_configHolder.PrintInline();

    TString message = fmt::format("TupleOption: \"{0}\"", Option());
    MessageSvc::Info("TupleHolder", message);
}

vector<TString> TupleHolder::GetFileNames() const 
{ 
    return m_tupleReader.GetFileNames();
}; 
