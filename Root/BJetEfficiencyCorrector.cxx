/******************************************
 *
 * Interface to CP BJet Efficiency Correction Tool.
 *
 * John Alison (john.alison@cern.ch)
 * Gabriel Facini (gabriel.facini@cern.ch)
 *
 *
 ******************************************/

// c++ include(s):
#include <iostream>

// EL include(s):
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>

// package include(s):
#include "xAODAnaHelpers/HelperFunctions.h"
#include "xAODAnaHelpers/HelperClasses.h"
#include "xAODAnaHelpers/BJetEfficiencyCorrector.h"

#include <xAODAnaHelpers/tools/ReturnCheck.h>

// ROOT include(s):
#include "TEnv.h"
#include "TSystem.h"

using HelperClasses::ToolName;

// this is needed to distribute the algorithm to the workers
ClassImp(BJetEfficiencyCorrector)


BJetEfficiencyCorrector :: BJetEfficiencyCorrector () :
  m_BJetSelectTool(nullptr),
  m_BJetEffSFTool(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

  Info("BJetEfficiencyCorrector()", "Calling constructor");

  // read flags set from .config file
  m_debug                   = false;
  m_inContainerName         = "";
  m_systName                = "";      // default: no syst
  m_outputSystName          = "BJetEfficiency_Algo";

  // configuration of the bjet eff tool
  m_corrFileName           = "$ROOTCOREBIN/data/xAODAnaHelpers/2015-PreRecomm-13TeV-MC12-CDI-May19-v1.root";
  m_jetAuthor              = "AntiKt4EMTopoJets";
  m_taggerName             = "MV2c20";
  m_useDevelopmentFile     = true;
  m_coneFlavourLabel       = true;

  // allowed operating points:
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingCalibrationDataInterface#xAOD_interface
  //For the fixed cut, valid options are: [ "FixedCutBEff_30", "FixedCutBEff_50", "FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_80", "FixedCutBEff_85", "FixedCutBEff_90" ]
  //For the variable cut, valid options are: [ "FlatBEff_30", "FlatBEff_40", "FlatBEff_50", "FlatBEff_60", "FlatBEff_70", "FlatBEff_77", "FlatBEff_85" ]


  // Btag quality
  m_operatingPt             = "";
  m_operatingPtCDI          = "";
  m_getScaleFactors         = false; // will only get scale factors for calibrated working points

  m_decor                   = "BTag";
  m_decorSF                 = ""; // gets set below after configure is called

}


EL::StatusCode  BJetEfficiencyCorrector :: configure ()
{
  if(!getConfig().empty()){
    Info("configure()", "Configuing BJetEfficiencyCorrector Interface. User configuration read from : %s ", getConfig().c_str());

    TEnv* config = new TEnv(getConfig(true).c_str());

    //
    // read flags set from .config file
    //
    m_debug                   = config->GetValue("Debug" , m_debug );
    m_inContainerName         = config->GetValue("InputContainer", m_inContainerName.c_str());
    m_systName                = config->GetValue("SystName" ,       m_systName.c_str() );      // default: no syst
    m_outputSystName          = config->GetValue("OutputSystName",  m_outputSystName.c_str() );

    //
    // configuration of the bjet eff tool
    //
    m_corrFileName           = config->GetValue("CorrectionFileName", m_corrFileName.c_str() );
    m_jetAuthor              = config->GetValue("JetAuthor",          m_jetAuthor.c_str() );
    m_taggerName             = config->GetValue("TaggerName",         m_taggerName.c_str() );
    m_useDevelopmentFile     = config->GetValue("UseDevelopmentFile", m_useDevelopmentFile);
    m_coneFlavourLabel       = config->GetValue("ConeFlavourLabel",   m_coneFlavourLabel);

    //
    // Btag quality
    //
    m_operatingPt             = config->GetValue("OperatingPoint",  m_operatingPt.c_str());

    m_decor                   = config->GetValue("DecorationName", m_decor.c_str());
    m_decorSF                 = m_decor + "_SF";

    config->Print();
    Info("configure()", "BJetEfficiencyCorrector Interface succesfully configured! ");

    delete config;
  }

  bool allOK(false);
  m_getScaleFactors = false;
  // not calibrated yet
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalib2015#Pre_Recommendation_August_2015
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingBenchmarks#MV2c20_tagger_AntiKt4EMTopoJets
  if (m_operatingPt == "FixedCutBEff_30") { allOK = true; }
  if (m_operatingPt == "FixedCutBEff_50") { allOK = true; }
  if (m_operatingPt == "FixedCutBEff_80") { allOK = true; }
  if (m_operatingPt == "FixedCutBEff_90") { allOK = true; }
  // these are the only calibrated working points
  if (m_operatingPt == "FixedCutBEff_60") { allOK = true; m_getScaleFactors =  true; }
  if (m_operatingPt == "FixedCutBEff_70") { allOK = true; m_getScaleFactors =  true; }
  if (m_operatingPt == "FixedCutBEff_77") { allOK = true; m_getScaleFactors =  true; }
  if (m_operatingPt == "FixedCutBEff_85") { allOK = true; m_getScaleFactors =  true; }
  //
  if (m_operatingPt == "FlatCutBEff_30") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_40") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_50") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_60") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_70") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_77") { allOK = true; }
  if (m_operatingPt == "FlatCutBEff_85") { allOK = true; }

  if( !allOK ) {
    Error("configure()", "Requested operating point is not known to xAH...out of date? %s", m_operatingPt.c_str());
    return EL::StatusCode::FAILURE;
  }

  // make unique name
  m_decor           += "_" + m_operatingPt;
  m_decorSF         += "_" + m_operatingPt;
  m_outputSystName  += "_" + m_operatingPt;

  // now take this name and convert it to the cut value for the CDI file
  // if using the fixed efficiency points
  m_operatingPtCDI = m_operatingPt;
  if( m_operatingPtCDI.find("FixedCutBEff") != std::string::npos) {
    m_operatingPtCDI.erase(0,13); // remove FixedCutBEff_
    std::cout << "Get OperatingPoint for CDI BTag Efficiency using eff = " << m_operatingPtCDI << std::endl;
    m_operatingPtCDI = HelperFunctions::GetBTagMV2c20_CutStr( atoi( m_operatingPtCDI.c_str() ) );
  }


  m_runAllSyst = (m_systName.find("All") != std::string::npos);

  if( m_inContainerName.empty() ) {
    Error("configure()", "InputContainer is empty!");
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode BJetEfficiencyCorrector :: setupJob (EL::Job& job)
{
  Info("setupJob()", "Calling setupJob");

  job.useXAOD ();
  xAOD::Init( "BJetEfficiencyCorrector" ).ignore(); // call before opening first file

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode BJetEfficiencyCorrector :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode BJetEfficiencyCorrector :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode BJetEfficiencyCorrector :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode BJetEfficiencyCorrector :: initialize ()
{
  Info("initialize()", "Initializing BJetEfficiencyCorrector Interface... ");

  m_event = wk()->xaodEvent();
  m_store = wk()->xaodStore();

  const xAOD::EventInfo* eventInfo(nullptr);
  RETURN_CHECK("BJetEfficiencyCorrector::execute()", HelperFunctions::retrieve(eventInfo, m_eventInfoContainerName, m_event, m_store, m_verbose) ,"");
  m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) );

  if (!m_isMC){
    Warning("initialize()", "Attempting to run BTagging Jet Scale Factors on data.  Turning tool off." );
    return EL::StatusCode::SUCCESS;
  }


  Info("initialize()", "Number of events in file: %lld ", m_event->getEntries() );

  if ( this->configure() == EL::StatusCode::FAILURE ) {
    Error("initialize()", "Failed to properly configure. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //
  // initialize the BJetSelectionTool
  //
  std::string sel_tool_name = std::string("BJetSelectionTool_") + m_name;
  m_BJetSelectTool= new BTaggingSelectionTool( sel_tool_name );
  m_BJetSelectTool->msg().setLevel( MSG::DEBUG ); // DEBUG, VERBOSE, INFO, ERROR

  //
  //  Configure the BJetSelectionTool
  //
  // A few which are not configurable as of yet....
  // is there a reason to have this configurable here??...I think no (GF to self)
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("MaxEta",2.5),"Failed to set property:MaxEta");
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("MinPt",20000.),"Failed to set property:MinPt");
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("FlvTagCutDefinitionsFileName","$ROOTCOREBIN/data/xAODBTaggingEfficiency/cutprofiles_22072015.root"),"Failed to set property:FlvTagCutDefinitionsFileName");
  // configurable parameters
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("TaggerName",          m_taggerName),"Failed to set property: TaggerName");
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("OperatingPoint",      m_operatingPt),"Failed to set property: OperatingPoint");
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->setProperty("JetAuthor",           m_jetAuthor),"Failed to set property: JetAuthor");
  RETURN_CHECK( "BJetSelection::initialize()", m_BJetSelectTool->initialize(), "Failed to properly initialize the BJetSelectionTool");



  //
  // initialize the BJetEfficiencyCorrectionTool
  //
  std::string sf_tool_name = std::string("BJetEfficiencyCorrectionTool_") + m_name;
  m_BJetEffSFTool = new BTaggingEfficiencyTool( sf_tool_name );
  m_BJetEffSFTool->msg().setLevel( MSG::DEBUG ); // DEBUG, VERBOSE, INFO, ERROR


  //
  //  Configure the BJetEfficiencyCorrectionTool
  //
  if( m_getScaleFactors ) {
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("TaggerName",          m_taggerName),"Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("OperatingPoint",      m_operatingPtCDI),"Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("JetAuthor",           m_jetAuthor),"Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("ScaleFactorFileName", m_corrFileName),"Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("UseDevelopmentFile",  m_useDevelopmentFile), "Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->setProperty("ConeFlavourLabel",    m_coneFlavourLabel), "Failed to set property");
    RETURN_CHECK( "BJetEfficiencyCorrector::initialize()", m_BJetEffSFTool->initialize(), "Failed to properly initialize the BJetEfficiencyCorrectionTool");
  } else {
    Warning( "initialize()", "Input operating point is not calibrated - no SFs will be obtained");
  }

  //
  // Print out
  //
  if( !m_systName.empty() && m_debug && m_getScaleFactors ) {
    std::cout << "-----------------------------------------------------" << std::endl;
    const std::map<CP::SystematicVariation, std::vector<std::string> > allowed_variations = m_BJetEffSFTool->listSystematics();
    std::cout << "Allowed systematics variations for tool " << m_BJetEffSFTool->name() << ":" << std::endl;
    for (auto var : allowed_variations) {
      std::cout << std::setw(40) << std::left << var.first.name() << ":";
      for (auto flv : var.second) std::cout << " " << flv;
      std::cout << std::endl;
    }
    std::cout << "-----------------------------------------------------" << std::endl;
  }


  // Get a list of affecting systematics
  if( m_getScaleFactors ) {
    CP::SystematicSet affectSysts = m_BJetEffSFTool->affectingSystematics();
    // Convert into a simple list
    std::vector<CP::SystematicSet> affectSystList = CP::make_systematics_vector(affectSysts);
    if( !m_systName.empty() && m_debug ) {
      for ( const auto& syst_it : affectSystList ){
        Info("initialize()"," tool can be affected by systematic: %s", (syst_it.name()).c_str());
      }
    }

    // Get a list of recommended systematics
    CP::SystematicSet recSysts = m_BJetEffSFTool->recommendedSystematics();
    // Convert into a simple list -- nominal is included already here!!
    m_systList = CP::make_systematics_vector(recSysts);
    if( !m_systName.empty() ) {
      for ( const auto& syst_it : m_systList ){
        Info("initialize()"," available recommended systematic: %s", (syst_it.name()).c_str());
      }
    } else { // remove all but the nominal
      std::vector<CP::SystematicSet>::iterator syst_it = m_systList.begin();
      while( syst_it != m_systList.end() ) {
        if( syst_it->name().empty() ) { syst_it++; }
        syst_it = m_systList.erase(syst_it);
      }
    }

    if( m_systName.empty() ){
      Info("initialize()"," Running w/ nominal configuration!");
    }

  }

  if( m_runAllSyst ){
    Info("initialize()"," Running w/ All systematics");
  }

  Info("initialize()", "BJetEfficiencyCorrector Interface succesfully initialized!" );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode BJetEfficiencyCorrector :: execute ()
{

  if (!m_isMC)
    return EL::StatusCode::SUCCESS;

  if(m_debug) Info("execute()", "Applying BJet Efficiency Correction... ");

  // get the collection from TEvent or TStore
  const xAOD::JetContainer* correctedJets(nullptr);
  RETURN_CHECK("BJetEfficiencyCorrector::execute()", HelperFunctions::retrieve(correctedJets, m_inContainerName, m_event, m_store, m_verbose) ,"");



  //
  // loop over available systematics
  //
  std::vector< std::string >* sysVariationNames = new std::vector< std::string >;
  for(const auto& syst_it : m_systList){

    //
    // if not running systematics, only compulte weight for specified systematic (m_systName)
    //    default is nominal (i.e., "")
    //
    if(!m_runAllSyst){
      if( syst_it.name() != m_systName ) {
        if(m_debug) Info("execute()","Not running systematics only apply nominal SF");
        continue;
      }
    }

    //
    // Create the name of the weight
    //   template:  SYSNAME
    //
    if(m_debug) Info("execute()", "SF decoration name is: %s", syst_it.name().c_str());
    sysVariationNames->push_back(syst_it.name());

    //
    // configure tool with syst variation
    //
    if (m_getScaleFactors ) {
      if (m_BJetEffSFTool->applySystematicVariation(syst_it) != CP::SystematicCode::Ok) {
        Error("initialize()", "Failed to configure BJetEfficiencyCorrections for systematic %s.", syst_it.name().c_str());
        return EL::StatusCode::FAILURE;
      }
      if(m_debug) Info("execute()", "Successfully applied systematic: %s.", syst_it.name().c_str());
    }

    bool tagged(false);
    //
    // and now apply data-driven efficiency and efficiency SF!
    //
    for( auto jet_itr : *(correctedJets)) {

      if(m_debug){
        Info( "execute", "New Jet eta = %f",  jet_itr->eta());
      }

      //
      // Add decorator for decision
      // 
      SG::AuxElement::Decorator< int > isBTag( m_decor );
      if( !isBTag.isAvailable( *jet_itr ) ) {
        if( m_BJetSelectTool->accept( *jet_itr ) ) { 
          isBTag( *jet_itr ) = 1; 
          tagged = true;
        }
        else { 
          isBTag( *jet_itr ) = 0; 
          tagged = false;
        }
      }

      // if only decorator with decision because OP is not calibrated, go to next jet
      if(!m_getScaleFactors) { continue; }

      //
      //  If btagging vector doesnt exist create it
      //
      SG::AuxElement::Decorator< std::vector<float> > sfVec( m_decorSF );
      if(!sfVec.isAvailable(*jet_itr)) {
        sfVec(*jet_itr) = std::vector<float>();
      }

      //
      // obtain efficiency SF
      //
      float SF(0.0);
      if ( fabs(jet_itr->eta()) < 2.5 ) {

        CP::CorrectionCode BJetEffCode;
        // if passes cut take the efficiency scale factor
        // if failed cut take the inefficiency scale factor
        if( tagged ) {
          BJetEffCode = m_BJetEffSFTool->getScaleFactor( *jet_itr, SF );
        } else {
          BJetEffCode = m_BJetEffSFTool->getInefficiencyScaleFactor( *jet_itr, SF );
        }
        if (BJetEffCode == CP::CorrectionCode::Error){
          Warning( "execute()", "Error in getEfficiencyScaleFactor");
          SF = -2;
          //return EL::StatusCode::FAILURE;
        }
        // if it is out of validity range (jet pt > 1200 GeV), the tools just applies the SF at 200 GeV
        //if (BJetEffCode == CP::CorrectionCode::OutOfValidityRange)
      } else {
        SF = -1;
      }
      if ( m_debug ) { Info( "execute()", "\t SF = %g", SF ); }

      //
      // Add it to vector
      //
      sfVec(*jet_itr).push_back(SF);

      if(m_debug){
        //
        // directly obtain reco efficiency
        //
        float eff(0.0);
        if( (fabs(jet_itr->eta()) < 2.5) &&
            m_BJetEffSFTool->getEfficiency( *jet_itr, eff ) != CP::CorrectionCode::Ok){
          Error( "execute()", "Problem in getRecoEfficiency");
          //return EL::StatusCode::FAILURE;
        }
        Info( "execute()", "\t reco efficiency = %g", eff );
      }

      if(m_debug){
        Info( "execute", "===>>> Resulting SF (%s) (from tool) %f, (from object) %f",  m_decor.c_str(), SF, sfVec(*jet_itr).back());
      }

    } // close jet loop

  } // close loop on systematics

  //
  // add list of sys names to TStore
  //
  RETURN_CHECK( "BJetEfficiencyCorrector::execute()", m_store->record( sysVariationNames, m_outputSystName), "Failed to record vector of systematic names.");

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode BJetEfficiencyCorrector :: postExecute ()
{
  if(m_debug) Info("postExecute()", "Calling postExecute");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode BJetEfficiencyCorrector :: finalize ()
{
  Info("finalize()", "Deleting tool instances...");
  if(m_BJetEffSFTool){
    delete m_BJetEffSFTool; m_BJetEffSFTool = nullptr;
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode BJetEfficiencyCorrector :: histFinalize ()
{
  Info("histFinalize()", "Calling histFinalize");
  return EL::StatusCode::SUCCESS;
}

