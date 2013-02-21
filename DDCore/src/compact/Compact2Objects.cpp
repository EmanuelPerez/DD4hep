// $Id:$
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================

// Framework includes
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/IDDescriptor.h"
#include "DD4hep/FieldTypes.h"
#include "XML/DocumentHandler.h"
#include "XML/Conversions.h"

// Root/TGeo include files
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "Reflex/PluginService.h"


#include <climits>
#include <iostream>
#include <iomanip>
#include <set>

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

namespace DD4hep {
  namespace Geometry {
    struct Compact;
    struct Includes;
    struct GdmlFile;
    struct Property;
    struct AlignmentFile;
  }
  template <> void Converter<Constant>::operator()(const xml_h& e)  const;
  template <> void Converter<Material>::operator()(const xml_h& e)  const;
  template <> void Converter<Atom>::operator()(const xml_h& e)  const;
  template <> void Converter<VisAttr>::operator()(const xml_h& e)  const;
  template <> void Converter<AlignmentEntry>::operator()(const xml_h& e)  const;
  template <> void Converter<Region>::operator()(const xml_h& e)  const;
  template <> void Converter<Readout>::operator()(const xml_h& e)  const;
  template <> void Converter<LimitSet>::operator()(const xml_h& e)  const;
  template <> void Converter<Property>::operator()(const xml_h& e)  const;
  template <> void Converter<CartesianField>::operator()(const xml_h& e)  const;
  template <> void Converter<SensitiveDetector>::operator()(const xml_h& element)  const;
  template <> void Converter<DetElement>::operator()(const xml_h& element)  const;
  template <> void Converter<GdmlFile>::operator()(const xml_h& element)  const;
  template <> void Converter<AlignmentFile>::operator()(const xml_h& element)  const;
  template <> void Converter<Header>::operator()(const xml_h& element)  const;
  template <> void Converter<Compact>::operator()(const xml_h& element)  const;
}

namespace {
  static UInt_t unique_mat_id = 0xAFFEFEED;
}

static Ref_t create_GridXYZ(lcdd_t& /* lcdd */, const xml_h& e)  {
  GridXYZ obj;
  if ( e.hasAttr(_A(gridSizeX)) ) obj.setGridSizeX(e.attr<float>(_A(gridSizeX)));
  if ( e.hasAttr(_A(gridSizeY)) ) obj.setGridSizeY(e.attr<float>(_A(gridSizeY)));
  if ( e.hasAttr(_A(gridSizeZ)) ) obj.setGridSizeZ(e.attr<float>(_A(gridSizeZ)));
  return obj;
}
DECLARE_XMLELEMENT(GridXYZ,create_GridXYZ);

namespace DD4hep { namespace Geometry { typedef GridXYZ RegularNgonCartesianGridXY; }}
DECLARE_XMLELEMENT(RegularNgonCartesianGridXY,create_GridXYZ);

static Ref_t create_GlobalGridXY(lcdd_t& /* lcdd */, const xml_h& e)  {
  GlobalGridXY obj;
  if ( e.hasAttr(_A(gridSizeX)) ) obj.setGridSizeX(e.attr<float>(_A(gridSizeX)));
  if ( e.hasAttr(_A(gridSizeY)) ) obj.setGridSizeY(e.attr<float>(_A(gridSizeY)));
  return obj;
}
DECLARE_XMLELEMENT(GlobalGridXY,create_GlobalGridXY);
  
static Ref_t create_CartesianGridXY(lcdd_t& /* lcdd */, const xml_h& e)  {
  CartesianGridXY obj;
  if ( e.hasAttr(_A(gridSizeX)) ) obj.setGridSizeX(e.attr<double>(_A(gridSizeX)));
  if ( e.hasAttr(_A(gridSizeY)) ) obj.setGridSizeY(e.attr<double>(_A(gridSizeY)));
  return obj;
}
DECLARE_XMLELEMENT(CartesianGridXY,create_CartesianGridXY);

namespace DD4hep { namespace Geometry { typedef CartesianGridXY EcalBarrelCartesianGridXY; }}
DECLARE_XMLELEMENT(EcalBarrelCartesianGridXY,create_CartesianGridXY);
  
static Ref_t create_ProjectiveCylinder(lcdd_t& /* lcdd */, const xml_h& e)  {
  ProjectiveCylinder obj;
  if ( e.hasAttr(_A(phiBins))   ) obj.setPhiBins(e.attr<int>(_A(phiBins)));
  if ( e.hasAttr(_A(thetaBins)) ) obj.setThetaBins(e.attr<int>(_A(thetaBins)));
  return obj;
}
DECLARE_XMLELEMENT(ProjectiveCylinder,create_ProjectiveCylinder);
  
static Ref_t create_NonProjectiveCylinder(lcdd_t& /* lcdd */, const xml_h& e)  {
  NonProjectiveCylinder obj;
  if ( e.hasAttr(_A(gridSizePhi)) ) obj.setThetaBinSize(e.attr<double>(_A(gridSizePhi)));
  if ( e.hasAttr(_A(gridSizeZ))   ) obj.setPhiBinSize(e.attr<double>(_A(gridSizeZ)));
  return obj;
}
DECLARE_XMLELEMENT(NonProjectiveCylinder,create_NonProjectiveCylinder);
  
static Ref_t create_ProjectiveZPlane(lcdd_t& /* lcdd */, const xml_h& e)  {
  ProjectiveZPlane obj;
  if ( e.hasAttr(_A(phiBins))   ) obj.setThetaBins(e.attr<int>(_A(phiBins)));
  if ( e.hasAttr(_A(thetaBins)) ) obj.setPhiBins(e.attr<int>(_A(thetaBins)));
  return obj;
}
DECLARE_XMLELEMENT(ProjectiveZPlane,create_ProjectiveZPlane);



static Ref_t create_ConstantField(lcdd_t& /* lcdd */, const xml_h& e)  {
  CartesianField obj;
  xml_comp_t field(e), strength(e.child(_X(strength)));
  string t = e.attr<string>(_A(field));
  Value<TNamed,ConstantField>* ptr = new Value<TNamed,ConstantField>();
  ptr->type = ::toupper(t[0])=='E' ? CartesianField::ELECTRIC : CartesianField::MAGNETIC;
  ptr->direction.SetX(strength.x());
  ptr->direction.SetY(strength.y());
  ptr->direction.SetZ(strength.z());
  obj.assign(ptr,field.nameStr(),field.typeStr());
  return obj;
}
DECLARE_XMLELEMENT(ConstantField,create_ConstantField);


static Ref_t create_SolenoidField(lcdd_t& lcdd, const xml_h& e)  {
  xml_comp_t c(e);
  CartesianField obj;
  Value<TNamed,SolenoidField>* ptr = new Value<TNamed,SolenoidField>();
  if ( c.hasAttr(_A(inner_radius)) ) ptr->innerRadius = c.attr<double>(_A(inner_radius));
  else ptr->innerRadius = 0.0;
  if ( c.hasAttr(_A(outer_radius)) ) ptr->outerRadius = c.attr<double>(_A(outer_radius));
  else ptr->outerRadius = lcdd.constant<double>("world_side");
  if ( c.hasAttr(_A(inner_field))  ) ptr->innerField  = c.attr<double>(_A(inner_field));
  if ( c.hasAttr(_A(outer_field))  ) ptr->outerField  = c.attr<double>(_A(outer_field));
  if ( c.hasAttr(_A(zmax))         ) ptr->maxZ        = c.attr<double>(_A(zmax));
  else ptr->maxZ = lcdd.constant<double>("world_side");
  if ( c.hasAttr(_A(zmin))         ) ptr->minZ        = c.attr<double>(_A(zmin));
  else                               ptr->minZ        = - ptr->maxZ;
  obj.assign(ptr,c.nameStr(),c.typeStr());
  return obj;
}
DECLARE_XMLELEMENT(SolenoidMagnet,create_SolenoidField);

static Ref_t create_DipoleField(lcdd_t& /* lcdd */, const xml_h& e)  {
  xml_comp_t c(e);
  CartesianField obj;
  Value<TNamed,DipoleField>* ptr = new Value<TNamed,DipoleField>();
  double val, lunit = c.attr<double>(_A(lunit)), funit = c.attr<double>(_A(funit));

  if ( c.hasAttr(_A(zmin))  ) ptr->zmin  = _multiply<double>(c.attr<string>(_A(zmin)),lunit);
  if ( c.hasAttr(_A(zmax))  ) ptr->zmax  = _multiply<double>(c.attr<string>(_A(zmax)),lunit);
  if ( c.hasAttr(_A(rmax))  ) ptr->rmax  = _multiply<double>(c.attr<string>(_A(rmax)),lunit);
  for( xml_coll_t coll(c,_X(dipole_coeff)); coll; ++coll)   {
    val = funit/pow(lunit,(int)ptr->coefficents.size());
    val = _multiply<double>(coll.value(),val);
    ptr->coefficents.push_back(val);
  }
  obj.assign(ptr,c.nameStr(),c.typeStr());
  return obj;
} 
DECLARE_XMLELEMENT(DipoleMagnet,create_DipoleField);

static long create_Compact(lcdd_t& lcdd, const xml_h& element) {
  Converter<Compact> converter(lcdd);
  converter(element);
  return 1;
}
DECLARE_XML_DOC_READER(lccdd,create_Compact);

/** Convert compact constant objects (defines)
 *
 *
 */
template <> void Converter<Constant>::operator()(const xml_h& e)  const  {
  xml_ref_t    constant(e);
  TNamed*      obj = new TNamed(constant.attr<string>(_A(name)).c_str(),
				constant.attr<string>(_A(value)).c_str()); 
  Ref_t        cons(obj);
  _toDictionary(obj->GetName(),obj->GetTitle());
  lcdd.addConstant(cons);
}

/** Convert compact constant objects (defines)
 *
 *
 */
template <> void Converter<Header>::operator()(const xml_h& e)  const  {
  xml_comp_t c(e);
  Header h(e.attr<string>(_A(name)),e.attr<string>(_A(title)));
  h.setUrl(e.attr<string>(_A(url)));
  h.setAuthor(e.attr<string>(_A(author)));
  h.setStatus(e.attr<string>(_A(status)));
  h.setVersion(e.attr<string>(_A(version)));
  h.setComment(e.child(_X(comment)).text());
  lcdd.setHeader(h);
}

/** Convert compact material/element description objects
 *
 *  Materials:
 *  <material name="Air">
 *    <D type="density" unit="g/cm3" value="0.0012"/>
 *    <fraction n="0.754" ref="N"/>
 *    <fraction n="0.234" ref="O"/>
 *    <fraction n="0.012" ref="Ar"/>
 *  </material>
 *
 *  Elements:
 *  <element Z="29" formula="Cu" name="Cu" >
 *    <atom type="A" unit="g/mol" value="63.5456" />
 *  </element>
 *
 */
template <> void Converter<Material>::operator()(const xml_h& e)  const  {
  xml_ref_t         m(e);
  TGeoManager*      mgr      = gGeoManager;
  xml_tag_t         mname    = m.name();
  const char*       matname  = mname.c_str();
  TGeoElementTable* table    = mgr->GetElementTable();
  TGeoMaterial*     mat      = mgr->GetMaterial(matname);
  TGeoMixture*      mix      = dynamic_cast<TGeoMixture*>(mat);
  xml_coll_t        fractions(m,_X(fraction));
  xml_coll_t        composites(m,_X(composite));
  bool has_density = true;
  bool mat_created = false;
  set<string> elts;

  if ( 0 == mat )  {
    xml_h  radlen     = m.child(_X(RL),false);
    xml_h  intlen     = m.child(_X(NIL),false);
    xml_h  density    = m.child(_X(D),false);
    double radlen_val = radlen.ptr() ? radlen.attr<double>(_A(value)) : 0.0;
    double intlen_val = intlen.ptr() ? intlen.attr<double>(_A(value)) : 0.0;
    double dens_val   = density.ptr() ? density.attr<double>(_A(value)) : 0.0;
    if ( 0 == mat && !density.ptr() ) {
      cout << "Compact2Objects[WARNING]: Material:" << matname << " with NO density." << endl;
      has_density = false;
    }
    if ( mat == 0  ) mat = mix = new TGeoMixture(matname,composites.size(),dens_val);
    mat->SetRadLen(radlen_val,intlen_val);
    mat_created = true;
    //cout << "Compact2Objects[INFO]: Creating material:" << matname << " composites:" << composites.size()+fractions.size() << endl;
  }
  if ( mat_created ) {
    if ( mix )  {
      for(Int_t i=0, n=mix->GetNelements(); i<n; ++i)
	elts.insert(mix->GetElement(i)->GetName());
    }
    for(; composites; ++composites)  {
      std::string nam = composites.attr<string>(_X(ref));
      TGeoMaterial*  comp_mat;
      TGeoElement*   element;
      if ( elts.find(nam) == elts.end() )  {
	double fraction = composites.attr<double>(_X(n));
	if ( 0 != (comp_mat=mgr->GetMaterial(nam.c_str())) ) {
	  mix->AddElement(comp_mat,fraction);
	}
	else if ( 0 != (element=table->FindElement(nam.c_str())) ) {
	  mix->AddElement(element,fraction);
	}
	else  {
	  string msg = "Compact2Objects[ERROR]: Creating material:"+mname+" Element missing: "+nam;
	  cout << msg << endl;
	  throw runtime_error(msg);
	}
      }
    }
    for(; fractions; ++fractions)  {
      std::string nam = fractions.attr<string>(_X(ref));
      TGeoMaterial*  comp_mat;
      TGeoElement*   element;
      if ( elts.find(nam) == elts.end() )  {
	double fraction = fractions.attr<double>(_X(n));
	if ( 0 != (comp_mat=mgr->GetMaterial(nam.c_str())) )
	  mix->AddElement(comp_mat,fraction);
	else if ( 0 != (element=table->FindElement(nam.c_str())) )
	  mix->AddElement(element,fraction);
	else  {
	  string msg = "Compact2Objects[ERROR]: Creating material:"+mname+" Element missing: "+nam;
	  cout << msg << endl;
	  throw runtime_error(msg);
	}
      }
    }
    // Update estimated density if not provided.
    if ( !has_density && mix && 0 == mix->GetDensity() ) {
      double dens = 0.0;
      for(composites.reset(); composites; ++composites)  {
	std::string nam = composites.attr<string>(_X(ref));
	double fraction = composites.attr<double>(_X(n));
	TGeoMaterial*  comp_mat = mgr->GetMaterial(nam.c_str());
	dens += fraction*comp_mat->GetDensity();
      }
      for(fractions.reset(); fractions; ++fractions)  {
	std::string nam = fractions.attr<string>(_X(ref));
	double fraction = fractions.attr<double>(_X(n));
	TGeoMaterial*  comp_mat = mgr->GetMaterial(nam.c_str());
	dens += fraction*comp_mat->GetDensity();
      }
      cout << "Compact2Objects[WARNING]: Material" << matname << " Set density:" << dens << " g/cm**3 " << endl;
      mix->SetDensity(dens);
    }
  }
  TGeoMedium* medium = mgr->GetMedium(matname);
  if ( 0 == medium )  {
    --unique_mat_id;
    medium = new TGeoMedium(matname,unique_mat_id,mat);
    medium->SetTitle("material");
    medium->SetUniqueID(unique_mat_id);
  }
  lcdd.addMaterial(Ref_t(medium));

  // TGeo has no notion of a material "formula"
  // Hence, treat the formula the same way as the material itself
  if ( m.hasAttr(_A(formula)) ) {
    string form = m.attr<string>(_A(formula));
    if ( form != matname ) {
      LCDD::HandleMap::const_iterator im=lcdd.materials().find(form);
      if ( im == lcdd.materials().end() ) {
	medium = mgr->GetMedium(form.c_str());
	if ( 0 == medium ) {
	  --unique_mat_id;
	  medium = new TGeoMedium(form.c_str(),unique_mat_id,mat);
	  medium->SetTitle("material");
	  medium->SetUniqueID(unique_mat_id);      
	}
	lcdd.addMaterial(Ref_t(medium));
      }
    }
  }
}

/** Convert compact atom objects
 *
 *   <element Z="29" formula="Cu" name="Cu" >
 */
template <> void Converter<Atom>::operator()(const xml_h& e)  const  {
  xml_ref_t    elem(e);
  xml_tag_t    eltname  = elem.name();
  TGeoManager* mgr      = gGeoManager;
  TGeoElementTable* tab = mgr->GetElementTable();
  TGeoElement*  element = tab->FindElement(eltname.c_str());
  if ( !element )  {
    xml_ref_t atom(elem.child(_X(atom)));
    tab->AddElement(elem.attr<string>(_A(name)).c_str(),
		    elem.attr<string>(_A(formula)).c_str(),
		    elem.attr<int>(_A(Z)),
		    atom.attr<int>(_A(value))
		    );
    element = tab->FindElement(eltname.c_str());
  }
}

/** Convert compact visualization attribute to LCDD visualization attribute
 *
 *  <vis name="SiVertexBarrelModuleVis" 
 *       alpha="1.0" r="1.0" g="0.75" b="0.76" 
 *       drawingStyle="wireframe" 
 *       showDaughters="false" 
 *       visible="true"/>
 */
template <> void Converter<VisAttr>::operator()(const xml_h& e)  const  {
  VisAttr attr(e.attr<string>(_A(name)));
  float r = e.hasAttr(_A(r)) ? e.attr<float>(_A(r)) : 1.0f;
  float g = e.hasAttr(_A(g)) ? e.attr<float>(_A(g)) : 1.0f;
  float b = e.hasAttr(_A(b)) ? e.attr<float>(_A(b)) : 1.0f;
  attr.setColor(r,g,b);
  if ( e.hasAttr(_A(alpha))         ) attr.setAlpha(e.attr<float>(_A(alpha)));
  if ( e.hasAttr(_A(visible))       ) attr.setVisible(e.attr<bool>(_A(visible)));
  if ( e.hasAttr(_A(lineStyle))     )   {
    string ls = e.attr<string>(_A(lineStyle));
    if ( ls == "unbroken" ) attr.setLineStyle(VisAttr::SOLID);
    if ( ls == "broken" ) attr.setLineStyle(VisAttr::DASHED);
  }
  else  {
    attr.setLineStyle(VisAttr::SOLID);
  }
  if ( e.hasAttr(_A(drawingStyle))  )   {
    string ds = e.attr<string>(_A(drawingStyle));
    if ( ds == "wireframe" ) attr.setDrawingStyle(VisAttr::WIREFRAME);
  }
  else  {
    attr.setDrawingStyle(VisAttr::WIREFRAME);
  }
  if ( e.hasAttr(_A(showDaughters)) ) attr.setShowDaughters(e.attr<bool>(_A(showDaughters)));
  lcdd.addVisAttribute(attr);
}

/** Specialized converter for compact AlignmentEntry objects.
 *
 *  <alignment name="<path/to/object>"  shortcut="short_cut_name">
 *    <position x="x-value" y="y-value" z="z-value"/>
 *    <rotation theta="theta-value" phi="phi-value" psi="psi-value"/>
 *  </alignment>
 */
template <> void Converter<AlignmentEntry>::operator()(const xml_h& e)  const  {
  xml_comp_t child(e);
  string  path = e.attr<string>(_A(name));
  bool check   = e.hasAttr(_A(check));
  bool overlap = e.hasAttr(_A(overlap));
  AlignmentEntry alignment(path);
  Position pos;
  Rotation rot;
  if ( (child=e.child(_X(position),false)) )  { // Position is not mandatory!
    pos.SetXYZ(child.x(),child.y(),child.z());
  }
  if ( (child=e.child(_X(rotation),false)) )  {  // Rotation is not mandatory
    rot.SetComponents(child.z(),child.y(),child.x());
  }
  if ( overlap ) {
    double ovl = e.attr<double>(_A(overlap));
    alignment.align(pos,rot,check,ovl);
  }
  else {
    alignment.align(pos,rot,check);
  }
  lcdd.addAlignment(alignment);
}

/** Specialized converter for compact region objects.
 *
 */
template <> void Converter<Region>::operator()(const xml_h& e)  const {
  Region region(e.attr<string>(_A(name)));
  vector<string>& limits = region.limits();
  string ene = e.attr<string>(_A(eunit)), len = e.attr<string>(_A(lunit));    

  region.setEnergyUnit(ene);
  region.setLengthUnit(len);
  region.setCut(_multiply<double>(e.attr<string>(_A(cut)),len));
  region.setThreshold(_multiply<double>(e.attr<string>(_A(threshold)),ene));
  region.setStoreSecondaries(e.attr<bool>(_A(store_secondaries)));
  for(xml_coll_t user_limits(e,_X(limitsetref)); user_limits; ++user_limits)
    limits.push_back(user_limits.attr<string>(_A(name)));
  lcdd.addRegion(region);
}

/** Specialized converter for compact readout objects.
 *
 * <readout name="HcalBarrelHits">
 *  <segmentation type="RegularNgonCartesianGridXY" gridSizeX="3.0*cm" gridSizeY="3.0*cm" />
 *  <id>system:6,barrel:3,module:4,layer:8,slice:5,x:32:-16,y:-16</id>
 *  </readout>
 */
template <> void Converter<Readout>::operator()(const xml_h& e)  const {
  const xml_h  id = e.child(_X(id));
  const xml_h seg = e.child(_X(segmentation),false);
  string  name = e.attr<string>(_A(name));
  Readout ro(name);

  if ( seg )  { // Segmentation is not mandatory!
    string type = seg.attr<string>(_A(type));
    Ref_t segment(ROOT::Reflex::PluginService::Create<TNamed*>(type,&lcdd,&seg));
    if ( !segment.isValid() ) throw runtime_error("FAILED to create segmentation:"+type);
    ro.setSegmentation(segment);
  }
  if ( id )  {
    //  <id>system:6,barrel:3,module:4,layer:8,slice:5,x:32:-16,y:-16</id> 
    Ref_t idSpec = IDDescriptor(id.text());
    idSpec->SetName(ro.name());
    ro.setIDDescriptor(idSpec);
    lcdd.addIDSpecification(idSpec);
  }
  lcdd.addReadout(ro);
}

/** Specialized converter for compact LimitSet objects.
 *
 *      <limitset name="...."> 
 *        <limit name="step_length_max" particles="*" value="5.0" unit="mm" />
 *  ... </limitset>
 */
template <> void Converter<LimitSet>::operator()(const xml_h& e)  const {
  LimitSet ls(e.attr<string>(_A(name)));
  for (xml_coll_t c(e,XML::Tag_limit); c; ++c) {
    Limit limit;
    limit.particles = c.attr<string>(_A(particles));
    limit.name      = c.attr<string>(_A(name));
    limit.content   = c.attr<string>(_A(value));
    limit.unit      = c.attr<string>(_A(unit));
    limit.value     = _multiply<double>(limit.content,limit.unit);
    ls.addLimit(limit);
  }
  lcdd.addLimitSet(ls);
}

/** Specialized converter for generic LCDD properties
 *
 *      <properties>
 *        <attributes name="key" type="" .... />
 *  ... </properties>
 */
template <> void Converter<Property>::operator()(const xml_h& e)  const {
  string name = e.attr<string>(_A(name));
  LCDD::Properties& prp = lcdd.properties();
  if ( name.empty() ) {
    throw runtime_error("Failed to convert properties. No name given!");
  }
  vector<xml_attr_t> a = e.attributes();
  if ( prp.find(name) == prp.end() ) {
    prp.insert(make_pair(name,LCDD::PropertyValues()));
  }
  for( vector<xml_attr_t>::iterator i=a.begin(); i != a.end(); ++i)  {
    pair<string,string> val(xml_tag_t(e.attr_name(*i)),e.attr<string>(*i));
    prp[name].insert(val);
  }
}

/** Specialized converter for electric and magnetic fields
 *
 *  Uses internally a plugin to allow flexible field descriptions.
 * 
 *     <field type="ConstantField" name="Myfield" field="electric">
 *       <strength x="0" y="0" z="5"/>
 *     </field>
 */
template <> void Converter<CartesianField>::operator()(const xml_h& e)  const  {
  string msg = "updated";
  string name = e.attr<string>(_A(name));
  string type = e.attr<string>(_A(type));
  CartesianField field = lcdd.field(name);
  if ( !field.isValid() ) {
    // The field is not present: We create it and add it to LCDD
    field = Ref_t(ROOT::Reflex::PluginService::Create<TNamed*>(type,&lcdd,&e));
    if ( !field.isValid() ) {
      throw runtime_error("Failed to create field object of type "+type);
    }
    lcdd.addField(field);
    msg = "created";
  }
  type = field.type();
  // Now update the field structure with the generic part ie. set it's properties
  CartesianField::Properties& prp = field.properties();
  for( xml_coll_t c(e,_X(properties)); c; ++c)   {
    string    props_name = c.attr<string>(_A(name));
    vector<xml_attr_t> a = c.attributes();
    if ( prp.find(props_name) == prp.end() ) {
      prp.insert(make_pair(props_name,CartesianField::PropertyValues()));
    }
    for( vector<xml_attr_t>::iterator i=a.begin(); i != a.end(); ++i)  {
      pair<string,string> val(xml_tag_t(c.attr_name(*i)),c.attr<string>(*i));
      prp[props_name].insert(val);
    }
    if ( c.hasAttr(_A(global)) && c.attr<bool>(_A(global)) ) {
      lcdd.field().properties() = prp;
    }
  }
  cout << "Converted field: Successfully " << msg << " field " << name << " [" << type << "]" << endl;
}

/** Update sensitive detectors from group tags.
 *
 *  Handle xml sections of the type:
 *  <sd name="MuonBarrel"
 *      type="Geant4Calorimeter" 
 *      ecut="100.0*MeV" 
 *      verbose="true" 
 *      hit_aggregation="position"
 *      limits="limit-set-reference"
 *      region="region-name-reference">
 *  </sd>
 *
 */
template <> void Converter<SensitiveDetector>::operator()(const xml_h& element)  const {
  string name = element.attr<string>(_A(name));
  try {
    DetElement        det = lcdd.detector(name);
    SensitiveDetector sd  = lcdd.sensitiveDetector(name);

    xml_attr_t type  = element.attr_nothrow(_A(type));
    if ( type ) {
      sd.setType(element.attr<string>(type));
    }
    xml_attr_t verbose = element.attr_nothrow(_A(verbose));
    if ( verbose ) {
      sd.setVerbose(element.attr<bool>(verbose));
    }
    xml_attr_t combine = element.attr_nothrow(_A(combine_hits));
    if ( combine ) {
      sd.setCombineHits(element.attr<bool>(combine));
    }
    xml_attr_t limits  = element.attr_nothrow(_A(limits));
    if ( limits ) {
      string   l  = element.attr<string>(limits);
      LimitSet ls = lcdd.limitSet(l);
      if ( !ls.isValid() )  {
	throw runtime_error("Converter<SensitiveDetector>: Request for non-existing limitset:"+l);
      }
      sd.setLimitSet(ls);
    }
    xml_attr_t region  = element.attr_nothrow(_A(region));
    if ( region ) {
      string r   = element.attr<string>(region);
      Region reg = lcdd.region(r);
      if ( !reg.isValid() )  {
	throw runtime_error("Converter<SensitiveDetector>: Request for non-existing region:"+r);
      }
      sd.setRegion(reg);
    }
    xml_attr_t hits    = element.attr_nothrow(_A(hits_collection));
    if ( hits ) {
      sd.setHitsCollection(element.attr<string>(hits));
    }
    xml_attr_t ecut  = element.attr_nothrow(_A(ecut));
    xml_attr_t eunit = element.attr_nothrow(_A(eunit));
    if ( ecut && eunit ) {
      double value = _multiply<double>(_toString(ecut),_toString(eunit));
      sd.setEnergyCutoff(value);
    }
    else if ( ecut ) { // If no unit is given , we assume the correct Geant4 unit is used!
      sd.setEnergyCutoff(element.attr<double>(ecut));
    }
    if ( sd.verbose() ) {
      cout << "SensitiveDetector-update:" << setw(18) << left << sd.name() 
	   << setw(24)  << left << " ["+sd.type()+"] " 
	   << "Hits:"   << setw(24) << left << sd.hitsCollection()
	   << "Cutoff:" << sd.energyCutoff()
	   << endl;
    }
  }
  catch(const exception& e) {
    cout << "FAILED    to convert sensitive detector:" << name << ": " << e.what() << endl;
  }
  catch(...) {
    cout << "FAILED    to convert sensitive detector:" << name << ": UNKNONW Exception" << endl;
  }
}

void setChildTitles(const pair<string,DetElement>& e) {
  DetElement parent = e.second.parent();
  const DetElement::Children& children = e.second.children();
  if ( ::strlen(e.second->GetTitle()) == 0 ) {
    e.second->SetTitle(parent.isValid() ? parent.type().c_str() : e.first.c_str());
  }
  for_each(children.begin(),children.end(),setChildTitles);
}

template <> void Converter<DetElement>::operator()(const xml_h& element)  const {
  static const char* req_dets = ::getenv("REQUIRED_DETECTORS");
  static const char* req_typs = ::getenv("REQUIRED_DETECTOR_TYPES");
  static const char* ign_dets = ::getenv("IGNORED_DETECTORS");
  static const char* ign_typs = ::getenv("IGNORED_DETECTOR_TYPES");
  string           type = element.attr<string>(_A(type));
  string           name = element.attr<string>(_A(name));
  string           name_match = ":"+name+":";
  string           type_match = ":"+type+":";
  if ( req_dets && !strstr(req_dets,name_match.c_str()) ) return;
  if ( req_typs && !strstr(req_typs,type_match.c_str()) ) return;
  if ( ign_dets &&  strstr(ign_dets,name_match.c_str()) ) return;
  if ( ign_typs &&  strstr(ign_typs,type_match.c_str()) ) return;
  try {
    xml_attr_t attr_ro = element.attr_nothrow(_A(readout));
    SensitiveDetector sd;
    if ( attr_ro )  {
      Readout ro = lcdd.readout(element.attr<string>(attr_ro));
      sd = SensitiveDetector(name,"sensitive");
      sd.setHitsCollection(ro.name());
      sd.setReadout(ro);
      lcdd.addSensitiveDetector(sd);
    }
    DetElement det(Ref_t(ROOT::Reflex::PluginService::Create<TNamed*>(type,&lcdd,&element,&sd)));
    if ( det.isValid() )  {
      setChildTitles(make_pair(name,det));
      if ( attr_ro )  {
	det.setReadout(sd.readout());
      }
    }
    cout << (det.isValid() ? "Converted" : "FAILED    ")
	 << " subdetector:" << name << " of type " << type;
    if ( sd.isValid() ) cout << " [" << sd.type() << "]";
    cout << endl;
    lcdd.addDetector(det);
  }
  catch(const exception& e) {
    cout << "FAILED    to convert subdetector:" << name << " of type " << type << ": " << e.what() << endl;
  }
  catch(...) {
    cout << "FAILED    to convert subdetector:" << name << " of type " << type << ": UNKNONW Exception" << endl;
  }
}
  
/// Read material entries from a seperate file in one of the include sections of the geometry
template <> void Converter<GdmlFile>::operator()(const xml_h& element)  const  {
  xml_h materials = XML::DocumentHandler().load(element,element.attr_value(_A(ref))).root();
  xml_coll_t(materials,_X(element) ).for_each(Converter<Atom>(this->lcdd));
  xml_coll_t(materials,_X(material)).for_each(Converter<Material>(this->lcdd));
}

/// Read alignment entries from a seperate file in one of the include sections of the geometry
template <> void Converter<AlignmentFile>::operator()(const xml_h& element)  const  {
  xml_h alignments = XML::DocumentHandler().load(element,element.attr_value(_A(ref))).root();
  xml_coll_t(alignments,_X(alignment)).for_each(Converter<AlignmentEntry>(this->lcdd));
}

template <> void Converter<Compact>::operator()(const xml_h& element)  const  {
  char text[32];
  xml_elt_t compact(element);
  xml_coll_t(compact,_X(includes)    ).for_each(_X(gdmlFile), Converter<GdmlFile>(lcdd));
  if ( element.hasChild(_X(info)) )
    (Converter<Header>(lcdd))(xml_h(compact.child(_X(info))));
  xml_coll_t(compact,_X(define)      ).for_each(_X(constant),  Converter<Constant>(lcdd));
  xml_coll_t(compact,_X(materials)   ).for_each(_X(element),   Converter<Atom>(lcdd));
  xml_coll_t(compact,_X(materials)   ).for_each(_X(material),  Converter<Material>(lcdd));
  xml_coll_t(compact,_X(properties)  ).for_each(_X(attributes),Converter<Property>(lcdd));
  lcdd.init();
  xml_coll_t(compact,_X(limits)      ).for_each(_X(limitset), Converter<LimitSet>(lcdd));
  xml_coll_t(compact,_X(display)     ).for_each(_X(vis),      Converter<VisAttr>(lcdd));
  xml_coll_t(compact,_X(readouts)    ).for_each(_X(readout),  Converter<Readout>(lcdd));
  xml_coll_t(compact,_X(detectors)   ).for_each(_X(detector), Converter<DetElement>(lcdd));
  xml_coll_t(compact,_X(includes)    ).for_each(_X(alignment),Converter<AlignmentFile>(lcdd));
  xml_coll_t(compact,_X(alignments)  ).for_each(_X(alignment),Converter<AlignmentEntry>(lcdd));
  xml_coll_t(compact,_X(fields)      ).for_each(_X(field),    Converter<CartesianField>(lcdd));
  xml_coll_t(compact,_X(sensitive_detectors)).for_each(_X(sd),Converter<SensitiveDetector>(lcdd));
  ::sprintf(text,"%u",xml_h(element).checksum(0));
  lcdd.addConstant(Constant("compact_checksum",text));
  lcdd.endDocument();
}


#ifdef _WIN32
template Converter<Atom>;
template Converter<Compact>;
template Converter<Readout>;
template Converter<VisAttr>;
template Converter<Constant>;
template Converter<LimitSet>;
template Converter<Material>;
template Converter<DetElement>;
template Converter<AlignmentEntry>;
template Converter<SensitiveDetector>;
template Converter<CartesianField>;
#endif
