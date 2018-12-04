//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

// Framework include files
#include "run_plugin.h"

//______________________________________________________________________________
int main(int argc,char** argv)  {
  std::vector<const char*> av;
  std::string level, visopt, opt;
  for(int i=0; i<argc; ++i)  {
    if      ( strncmp(argv[i],"-visopt",4) == 0 )  visopt = argv[i];
    else if ( strncmp(argv[i],"-level", 4)  == 0 ) level  = argv[i];
    else if ( strncmp(argv[i],"-option",4) == 0 )  opt    = argv[i];
    else av.push_back(argv[i]);
  }
  av.push_back("-plugin");
  av.push_back("-DD4hep_GeometryDisplay");
  if ( !opt.empty()    ) av.push_back("-opt"),    av.push_back(opt.c_str());
  if ( !level.empty()  ) av.push_back("-level"),  av.push_back(level.c_str());
  if ( !visopt.empty() ) av.push_back("-visopt"), av.push_back(visopt.c_str());
  return dd4hep::execute::main_plugins("DD4hep_GeometryDisplay", av.size(), (char**)&av[0]);
}
