//==========================================================================
//  AIDA Detector description implementation for LCD
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
#include "DD4hep/LCDD.h"
#include "DD4hep/Plugins.h"
#include "DD4hep/Printout.h"
#include "DD4hep/Conditions.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/ConditionsPrinter.h"

#include "DDCond/ConditionsManager.h"
#include "DDCond/ConditionsIOVPool.h"
#include "DDCond/ConditionsInterna.h"
#include "DDCond/ConditionsPool.h"
#include "DDCond/ConditionsInterna.h"
#include "DDCond/ConditionsRepository.h"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Conditions;
using Geometry::DetElement;
using Geometry::PlacedVolume;

/// Plugin function: Install the alignment manager as an extension to the central LCDD object
/**
 *  Factory: DD4hep_ConditionsManagerInstaller
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static int ddcond_install_cond_mgr (LCDD& lcdd, int argc, char** argv)  {
  Handle<ConditionsManagerObject> mgr(lcdd.extension<ConditionsManagerObject>(false));
  if ( !mgr.isValid() )  {
    ConditionsManager mgr_handle(lcdd);
    lcdd.addExtension<ConditionsManagerObject>(mgr_handle.ptr());
    printout(INFO,"ConditionsManager",
             "+++ Successfully installed conditions manager instance to LCDD.");
    mgr = mgr_handle;
  }
  if ( argc == 2 )  {
    if ( ::strncmp(argv[0],"-handle",7)==0 )  {
      Handle<NamedObject>* h = (Handle<NamedObject>*)argv[1];
      *h = mgr;
    }
  }
  return 1;
}
DECLARE_APPLY(DD4hep_ConditionsManagerInstaller,ddcond_install_cond_mgr)

// ======================================================================================
static ConditionsProcessor* create_processor(lcdd_t& lcdd, int argc, char** argv)  {
  class Processor {public:    virtual ~Processor() {}  };
  ConditionsProcessor* processor = 0;
  if ( argc < 2 )   {
    except("CondPoolProcessor","++ No processor creator name given!");
  }
  for(int i=0; i<argc; ++i)  {
    if ( 0 == ::strncmp(argv[i],"-processor",3) )  {
      vector<char*> args;
      for(int j=2; j<argc && argv[j]; ++j) args.push_back(argv[j]);
      args.push_back(0);
      string fac = argv[++i];
      Condition::Processor* p = (Condition::Processor*)
        PluginService::Create<void*>(fac,&lcdd,int(args.size()),&args[0]);
      processor = dynamic_cast<ConditionsProcessor*>(p);
      break;
    }
  }
  if ( !processor )  {
    except("CondPoolProcessor",
           "++ Found arguments in plugin call, but could not make any sense of them....");
  }
  return processor;
}

// ======================================================================================
/// Plugin function: Dump of all Conditions pool with or without conditions
/**
 *  Factory: DD4hep_ConditionsPoolDump: Dump pools only
 *  Factory: DD4hep_ConditionsDump: Dump pools and conditions
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static int ddcond_conditions_pool_processor(lcdd_t& lcdd, bool process_pool, bool process_conditions, int argc, char** argv)   {
  ConditionsProcessor* processor = create_processor(lcdd,argc,argv);
  typedef std::vector<const IOVType*> _T;
  typedef ConditionsIOVPool::Elements _E;
  typedef RangeConditions _R;
  dd4hep_ptr<Condition::Processor> proc(processor);
  ConditionsManager manager = ConditionsManager::from(lcdd);
  const _T types = manager.iovTypesUsed();
  for( _T::const_iterator i = types.begin(); i != types.end(); ++i )    {
    const IOVType* type = *i;
    if ( type )   {
      ConditionsIOVPool* pool = manager.iovPool(*type);
      if ( pool )  {
        const _E& e = pool->elements;
        if ( process_pool )  {
          printout(INFO,"CondPoolProcessor","+++ ConditionsIOVPool for type %s  [%d IOV element%s]",
                   type->str().c_str(), int(e.size()),e.size()==1 ? "" : "s");
        }
        for (_E::const_iterator j=e.begin(); j != e.end(); ++j)  {
          ConditionsPool* cp = (*j).second;
          if ( process_pool )  {
            cp->print("");
          }
          if ( process_conditions )   {
            _R rc;
            cp->select_all(rc);
            for(_R::const_iterator ic=rc.begin(); ic!=rc.end(); ++ic)  {
              if ( proc.get() )  {  (*proc)(*ic); }
            }
          }
        }
      }
    }
  }
  return 1;
}
static int ddcond_conditions_pool_process(LCDD& lcdd, int argc, char** argv)   {
  return ddcond_conditions_pool_processor(lcdd, false, true, argc, argv);
}
DECLARE_APPLY(DD4hep_ConditionsPoolProcessor,ddcond_conditions_pool_process)

// ======================================================================================
/// Plugin function: Dump of all Conditions pool with or without conditions
/**
 *  Factory: DD4hep_ConditionsPoolDump: Dump pools only
 *  Factory: DD4hep_ConditionsDump: Dump pools and conditions
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static int ddcond_conditions_pool_print(lcdd_t& lcdd, bool print_conditions, int argc, char** argv)   {
  if ( argc > 0 )   {
    for(int i=0; i<argc; ++i)  {
      if ( 0 == ::strncmp(argv[i],"-processor",3) )  {
        vector<char*> args;
        for(int j=i; j<argc && argv[j]; ++j) args.push_back(argv[j]);
        args.push_back(0);
        return ddcond_conditions_pool_processor(lcdd,true,print_conditions,int(args.size()-1),&args[0]);
      }
    }
    printout(WARNING,"DDCondProcessor","++ Found arguments in plugin call, "
             "but could not make any sense of them....");
  }
  const void* args[] = { "-processor", "DD4hepConditionsPrinter", 0};
  return ddcond_conditions_pool_processor(lcdd,true,print_conditions,2,(char**)args);
}

static int ddcond_dump_pools(LCDD& lcdd, int argc, char** argv)   {
  return ddcond_conditions_pool_print(lcdd, false, argc, argv);
}
static int ddcond_dump_conditions(LCDD& lcdd, int argc, char** argv)   {
  return ddcond_conditions_pool_print(lcdd, true, argc, argv);
}
DECLARE_APPLY(DD4hep_ConditionsPoolDump,ddcond_dump_pools)
DECLARE_APPLY(DD4hep_ConditionsDump,ddcond_dump_conditions)

// ======================================================================================
/// Plugin function: Dump of all Conditions associated to the detector elements
/**
 *  Factory: DD4hep_DetElementConditionsDump
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static int ddcond_detelement_dump(LCDD& lcdd, int /* argc */, char** /* argv */)   {

  struct Actor {
    ConditionsManager     manager;
    ConditionsPrinter     printer;
    dd4hep_ptr<UserPool>  user_pool;
    const IOVType*        iov_type;

    /// Standard constructor
    Actor(ConditionsManager m)  : manager(m) {
      iov_type = manager.iovType("run");
      IOV  iov(iov_type);
      iov.set(1500);
      long num_updated = manager.prepare(iov, user_pool);
      printout(INFO,"Conditions",
               "+++ ConditionsUpdate: Updated %ld conditions of type %s.",
               num_updated, iov_type ? iov_type->str().c_str() : "???");
      user_pool->print("User pool");
      printer.setPool(user_pool.get());
    }
    /// Default destructor
    ~Actor()   {
      manager.clean(iov_type, 20);
      user_pool->clear();
      user_pool.release();
    }
    /// Dump method.
    long dump(DetElement de,int level)   {
      const DetElement::Children& children = de.children();
      
      PlacedVolume place = de.placement();
      char sens = place.volume().isSensitive() ? 'S' : ' ';
      char fmt[128], tmp[32];
      ::snprintf(tmp,sizeof(tmp),"%03d/",level+1);
      ::snprintf(fmt,sizeof(fmt),"%03d %%-%ds %%s #Dau:%%d VolID:%%08X %%c",level+1,2*level+1);
      printout(INFO,"DetectorDump",fmt,"",de.path().c_str(),int(children.size()),
               (unsigned long)de.volumeID(), sens);
      printer.setName(string(tmp)+de.name());
      if ( de.hasConditions() )  {
        (printer)(de);
      }
      for (const auto& c : de.children() )
        dump(c.second,level+1);
      return 1;
    }
  };
  return Actor(ConditionsManager::from(lcdd)).dump(lcdd.world(),0);
}
DECLARE_APPLY(DD4hep_DetElementConditionsDump,ddcond_detelement_dump)
  
// ======================================================================================
/// Plugin function: Dump of all Conditions associated to the detector elements
/**
 *  Factory: DD4hep_DetElementConditionsDump
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static void* ddcond_prepare(LCDD& lcdd, int argc, char** argv)   {
  long              iov_val = -1;
  const IOVType*    iov_typ = 0;
  ConditionsManager manager = ConditionsManager::from(lcdd);

  for(int i=0; i<argc; ++i)  {
    if ( ::strncmp(argv[i],"-iov_type",7) == 0 )
      iov_typ = manager.iovType(argv[++i]);
    else if ( ::strncmp(argv[i],"-iov_value",7) == 0 )
      iov_val = ::atol(argv[++i]);
  }
  if ( 0 == iov_typ )  {
    except("ConditionsPrepare","++ Unknown IOV type supplied.");
  }
  if ( 0 > iov_val )  {
    except("ConditionsPrepare",
           "++ Unknown IOV value supplied for iov type %s.",iov_typ->str().c_str());
  }
  dd4hep_ptr<UserPool>  user_pool;
  IOV  iov(iov_typ);
  iov.set(iov_val);
  long num_updated = manager.prepare(iov, user_pool);
  printout(DEBUG,"Conditions",
           "+++ ConditionsUpdate: Updated %ld conditions of type %s.",
           num_updated, iov_typ ? iov_typ->str().c_str() : "???");
  return user_pool.get() && user_pool->count() > 0 ? user_pool.release() : 0;
}
DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsPrepare,ddcond_prepare)
  
// ======================================================================================
/// Plugin function: Dump of all Conditions associated to the detector elements
/**
 *  Factory: DD4hep_DetElementConditionsDump
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static int ddcond_detelement_processor(LCDD& lcdd, int argc, char** argv)   {

  struct Actor {
    ConditionsManager     manager;
    ConditionsProcessor*  processor;
    dd4hep_ptr<UserPool>  user_pool;
    const IOVType*        iov_type;

    /// Standard constructor
    Actor(ConditionsProcessor* p, ConditionsManager m)  : manager(m), processor(p)  {
      iov_type = manager.iovType("run");
      IOV  iov(iov_type);
      iov.set(1500);
      long num_updated = manager.prepare(iov, user_pool);
      printout(INFO,"Conditions",
               "+++ ConditionsUpdate: Updated %ld conditions of type %s.",
               num_updated, iov_type ? iov_type->str().c_str() : "???");
      user_pool->print("User pool");
      processor->setPool(user_pool.get());
    }

    /// Default destructor
    ~Actor()   {
      manager.clean(iov_type, 20);
      user_pool->clear();
      user_pool.release();
    }
    /// Dump method.
    long dump(DetElement de)   {
      if ( de.hasConditions() )  {
        (*processor)(de);
      }
      for (const auto& c : de.children() )
        dump(c.second);
      return 1;
    }
  };
  ConditionsProcessor* processor = 0;
  if ( argc > 0 )   {
    processor = create_processor(lcdd, argc, argv);
  }
  else  {
    const void* args[] = { "-processor", "DD4hepConditionsPrinter", 0};
    processor = create_processor(lcdd, 2, (char**)args);
  }
  return Actor(processor,ConditionsManager::from(lcdd)).dump(lcdd.world());
}
DECLARE_APPLY(DD4hep_DetElementConditionsProcessor,ddcond_detelement_processor)

// ======================================================================================
/// Plugin entry point: Synchronize conditions according to new IOV value
/**
 *  Factory: DD4hep_ConditionsSynchronize
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static long ddcond_synchronize_conditions(lcdd_t& lcdd, int argc, char** argv) {
  if ( argc > 0 )   {
    string iov_type = argv[0];
    IOV::Key::first_type iov_key = *(IOV::Key::first_type*)argv[1];
    ConditionsManager    manager = ConditionsManager::from(lcdd);
    const IOVType*       epoch   = manager.iovType(iov_type);
    dd4hep_ptr<UserPool> user_pool;
    IOV  iov(epoch);

    iov.set(iov_key);
    long num_updated = manager.prepare(iov, user_pool);
    if ( iov_type == "epoch" )  {
      char   c_evt[64];
      struct tm evt;
      ::gmtime_r(&iov_key, &evt);
      ::strftime(c_evt,sizeof(c_evt),"%T %F",&evt);
      printout(INFO,"Conditions",
               "+++ ConditionsUpdate: Updated %ld conditions... event time: %s",
               num_updated, c_evt);
    }
    else  {
      printout(INFO,"Conditions",
               "+++ ConditionsUpdate: Updated %ld conditions... key[%s]: %ld",
               num_updated, iov_type.c_str(), iov_key);
    }
    user_pool->print("User pool");
    manager.clean(epoch, 20);
    user_pool->clear();
    return 1;
  }
  except("Conditions","+++ Failed update conditions. No event time argument given!");
  return 0;
}
DECLARE_APPLY(DD4hep_ConditionsSynchronize,ddcond_synchronize_conditions)

// ======================================================================================
/// Plugin entry point: Clean conditions reposiory according to maximum age
/**
 *  Factory: DD4hep_ConditionsClean
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static long ddcond_clean_conditions(lcdd_t& lcdd, int argc, char** argv) {
  if ( argc > 0 )   {
    string iov_type = argv[0];
    int max_age = *(int*)argv[1];
    printout(INFO,"Conditions",
             "+++ ConditionsUpdate: Cleaning conditions... type:%s max age:%d",
             iov_type.c_str(), max_age);
    ConditionsManager manager = ConditionsManager::from(lcdd);
    const IOVType* epoch = manager.iovType(iov_type);
    manager.clean(epoch, max_age);
    return 1;
  }
  except("Conditions","+++ Failed cleaning conditions. Insufficient arguments!");
  return 0;
}
DECLARE_APPLY(DD4hep_ConditionsClean,ddcond_clean_conditions)

// ======================================================================================
/// Plugin entry point: Create repository csv file from loaded conditions
/**
 *  Factory: DD4hep_ConditionsCreateRepository
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static long ddcond_create_repository(lcdd_t& lcdd, int argc, char** argv) {
  if ( argc > 0 )   {
    string output = argv[0];
    printout(INFO,"Conditions",
             "+++ ConditionsRepository: Creating %s",output.c_str());
    ConditionsManager manager = ConditionsManager::from(lcdd);
    ConditionsRepository().save(manager,output);
    return 1;
  }
  except("Conditions","+++ Failed creating conditions repository. Insufficient arguments!");
  return 0;
}
DECLARE_APPLY(DD4hep_ConditionsCreateRepository,ddcond_create_repository)

// ======================================================================================
/// Plugin entry point: Dump conditions repository csv file
/**
 *  Factory: DD4hep_ConditionsDumpRepository
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static long ddcond_dump_repository(lcdd_t& lcdd, int argc, char** argv) {
  if ( argc > 0 )   {
    typedef ConditionsRepository::Data Data;
    Data data;
    string input = argv[0];
    printout(INFO,"Conditions",
             "+++ ConditionsRepository: Dumping %s",input.c_str());
    ConditionsManager manager = ConditionsManager::from(lcdd);
    if ( ConditionsRepository().load(input, data) )  {
      printout(INFO,"Repository","%-8s  %-60s %-60s","Key","Name","Address");
      for(Data::const_iterator i=data.begin(); i!=data.end(); ++i)  {
        const ConditionsRepository::Entry& e = *i;
        string add = e.address;
        if ( add.length() > 80 ) add = e.address.substr(0,60) + "...";
        printout(INFO,"Repository","%08X  %s",e.key,e.name.c_str());
        printout(INFO,"Repository","          -> %s",e.address.c_str());
      }
    }
    return 1;
  }
  except("Conditions","+++ Failed dumping conditions repository. Insufficient arguments!");
  return 0;
}
DECLARE_APPLY(DD4hep_ConditionsDumpRepository,ddcond_dump_repository)

// ======================================================================================
/// Plugin entry point: Load conditions repository csv file into conditions manager
/**
 *  Factory: DD4hep_ConditionsDumpRepository
 *

TO BE DONE!!!

 *  \author  M.Frank
 *  \version 1.0
 *  \date    01/04/2016
 */
static long ddcond_load_repository(lcdd_t& lcdd, int argc, char** argv) {
  if ( argc > 0 )   {
    string input = argv[0];
    printout(INFO,"Conditions",
             "+++ ConditionsRepository: Loading %s",input.c_str());
    ConditionsManager manager = ConditionsManager::from(lcdd);
    ConditionsRepository::Data data;
    ConditionsRepository().load(input, data);
    return 1;
  }
  except("Conditions","+++ Failed loading conditions repository. Insufficient arguments!");
  return 0;
}
DECLARE_APPLY(DD4hep_ConditionsLoadRepository,ddcond_load_repository)
// ======================================================================================
