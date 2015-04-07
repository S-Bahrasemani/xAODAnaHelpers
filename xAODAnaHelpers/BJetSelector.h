#ifndef xAODAnaHelpers_BJetSelector_H
#define xAODAnaHelpers_BJetSelector_H

// EL include(s):
#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// EDM include(s):
#include "xAODJet/JetContainer.h"

// ROOT include(s):
#include "TH1D.h"

class BJetSelector : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

  xAOD::TEvent *m_event;  //!
  xAOD::TStore *m_store;  //!
  int m_numEvent;         //!
  int m_numObject;        //!
  int m_numEventPass;     //!
  int m_numObjectPass;    //!

  std::string m_name;
  std::string m_configName;

  bool m_debug;                  //!

  bool m_isEMjet;                //!
  bool m_isLCjet;                //!

  // cutflow
  bool m_useCutFlow;            //!
  TH1D* m_cutflowHist;          //!
  TH1D* m_cutflowHistW;         //!
  int   m_cutflow_bin;          //!

private:

  // configuration variables
  std::string m_inContainerName; //!      // input container name
  std::string m_outContainerName; //!     // output container name
  bool m_decorateSelectedObjects; //! // decorate selected objects? defaul passSel
  bool m_createSelectedContainer; //! // fill using SG::VIEW_ELEMENTS to be light weight
  int m_nToProcess; //!               // look at n objects
  bool m_cleanJets; //!               // require cleanJet decoration to not be set and false
  int m_pass_min; //!                 // minimum number of objects passing cuts
  int m_pass_max; //!                 // maximum number of objects passing cuts
  float m_pT_max; //!                 // require pT < pt_max
  float m_pT_min; //!                 // require pT > pt_max
  float m_eta_max; //!                // require eta < eta_max
  float m_eta_min; //!                // require eta > eta_max
  float m_mass_max; //!               // require mass < mass_max
  float m_mass_min; //!               // require mass > mass_max
  float m_rapidity_max; //!           // require rapidity < rapidity_max
  bool m_veryloose; //!
  bool m_loose; //!
  bool m_medium; //!
  bool m_tight; //!
  float m_btagCut; //!
  std::string m_decor; //!

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!



  // this is a standard constructor
  BJetSelector ();
  BJetSelector (std::string name, std::string configName);

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // these are the functions not inherited from Algorithm
  virtual EL::StatusCode configure ();
  virtual EL::StatusCode executeConst( const xAOD::JetContainer* inJets, float mcEvtWeight );

  // added functions not from Algorithm
  // why does this need to be virtual?
  virtual int PassCuts( const xAOD::Jet* jet );
  // this is needed to distribute the algorithm to the workers
  ClassDef(BJetSelector, 1);
};

#endif
