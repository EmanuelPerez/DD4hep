// $Id: Geant4Converter.cpp 603 2013-06-13 21:15:14Z markus.frank $
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
// @author  P.Kostka (main author)
// @author  M.Frank  (code reshuffeling into new DDG4 scheme)
//
//====================================================================

// Framework include files
#include "LCIOEventReader.h"
#include "DD4hep/Printout.h"
#include "DDG4/Geant4Primary.h"
#include "DDG4/Geant4Context.h"
#include "DDG4/Factories.h"

#include "G4ParticleTable.hh"
#include "EVENT/MCParticle.h"
#include "EVENT/LCCollection.h"

#include "G4Event.hh"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Simulation;
typedef DD4hep::ReferenceBitMask<int> PropertyMask;

// Neede for backwards compatibility:
namespace DD4hep{namespace Simulation{typedef Geant4InputAction LCIOInputAction;}}
DECLARE_GEANT4ACTION(LCIOInputAction)

namespace {
  inline int GET_ENTRY(const map<EVENT::MCParticle*,int>& mcparts, EVENT::MCParticle* part)  {
    map<EVENT::MCParticle*,int>::const_iterator ip=mcparts.find(part);
    if ( ip == mcparts.end() )  {
      throw runtime_error("Unknown particle identifier look-up!");
    }
    return (*ip).second;
  }
}


/// Initializing constructor
LCIOEventReader::LCIOEventReader(const string& nam)
  : Geant4EventReader(nam), m_numEvent(0)
{
}

/// Default destructor
LCIOEventReader::~LCIOEventReader()   {
}

/// Read an event and fill a vector of MCParticles.
LCIOEventReader::EventReaderStatus
LCIOEventReader::readParticles(int event_number, vector<Particle*>& particles)   {
  EVENT::LCCollection*        primaries = 0;
  map<EVENT::MCParticle*,int> mcparts;
  vector<EVENT::MCParticle*>  mcpcoll;
  EventReaderStatus ret = EVENT_READER_OK;

  if ( hasDirectAccess() )  
    ret = readParticles(event_number,&primaries);
  else if ( m_numEvent == event_number )
    ret = readParticles(event_number,&primaries);
  else
    ret = EVENT_READER_NO_DIRECT;

  ++m_numEvent;
  if ( ret != EVENT_READER_OK ) return ret;

  int NHEP = primaries->getNumberOfElements();
  // check if there is at least one particle
  if ( NHEP == 0 ) return EVENT_READER_NO_PRIMARIES;

  mcpcoll.resize(NHEP,0);
  for(int i=0; i<NHEP; ++i ) {
    EVENT::MCParticle* p=dynamic_cast<EVENT::MCParticle*>(primaries->getElementAt(i));
    mcparts[p] = i;
    mcpcoll[i] = p;
  }

  // build collection of MCParticles
  G4ParticleTable*      tab = G4ParticleTable::GetParticleTable();
  for(int i=0; i<NHEP; ++i )   {
    EVENT::MCParticle* mcp = mcpcoll[i];
    Geant4ParticleHandle p(new Particle(i));
    const double *mom   = mcp->getMomentum();
    const double *vsx   = mcp->getVertex();
    const double *vex   = mcp->getEndpoint();
    const float  *spin  = mcp->getSpin();
    const int    *color = mcp->getColorFlow();
    G4ParticleDefinition* def = tab->FindParticle(mcp->getPDG());
    PropertyMask status(p->status);
    p->pdgID        = mcp->getPDG();
    p->charge       = int(mcp->getCharge()*3.0);
    p->psx          = mom[0]*GeV;
    p->psy          = mom[1]*GeV;
    p->psz          = mom[2]*GeV;
    p->time         = mcp->getTime()*ns;
    p->properTime   = mcp->getTime()*ns;
    p->vsx          = vsx[0]*mm;
    p->vsy          = vsx[1]*mm;
    p->vsz          = vsx[2]*mm;
    p->vex          = vex[0]*mm;
    p->vey          = vex[1]*mm;
    p->vez          = vex[2]*mm;
    p->definition   = def;
    p->process      = 0;
    p->spin[0]      = spin[0];
    p->spin[1]      = spin[1];
    p->spin[2]      = spin[2];
    p->colorFlow[0] = color[0];
    p->colorFlow[0] = color[1];
    p->mass         = mcp->getMass()*GeV;
    const EVENT::MCParticleVec &par = mcp->getParents(), &dau=mcp->getDaughters();
    for(int num=dau.size(),k=0; k<num; ++k)
      p->daughters.insert(GET_ENTRY(mcparts,dau[k]));
    for(int num=par.size(),k=0; k<num; ++k)
      p->parents.insert(GET_ENTRY(mcparts,par[k]));

    int genStatus = mcp->getGeneratorStatus();
    if ( genStatus == 0 ) status.set(G4PARTICLE_GEN_EMPTY);
    else if ( genStatus == 1 ) status.set(G4PARTICLE_GEN_STABLE); 
    else if ( genStatus == 2 ) status.set(G4PARTICLE_GEN_DECAYED);
    else if ( genStatus == 3 ) status.set(G4PARTICLE_GEN_DOCUMENTATION);
    else { 
      cout << " #### WARNING - LCIOInputAction : unknown generator status : " 
	   << genStatus << " -> ignored ! " << endl;
    }
    if ( mcp->isCreatedInSimulation() )       status.set(G4PARTICLE_SIM_CREATED);
    if ( mcp->isBackscatter() )               status.set(G4PARTICLE_SIM_BACKSCATTER);
    if ( mcp->vertexIsNotEndpointOfParent() ) status.set(G4PARTICLE_SIM_PARENT_RADIATED);
    if ( mcp->isDecayedInTracker() )          status.set(G4PARTICLE_SIM_DECAY_TRACKER);
    if ( mcp->isDecayedInCalorimeter() )      status.set(G4PARTICLE_SIM_DECAY_CALO);
    if ( mcp->hasLeftDetector() )             status.set(G4PARTICLE_SIM_LEFT_DETECTOR);
    if ( mcp->isStopped() )                   status.set(G4PARTICLE_SIM_STOPPED);
    if ( mcp->isOverlay() )                   status.set(G4PARTICLE_SIM_OVERLAY);
    particles.push_back(p);
  }
  return EVENT_READER_OK;
}
