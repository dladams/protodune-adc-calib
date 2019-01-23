// AdcCalibData.cxx

# include "AdcCalibData.h"
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setw;

//**********************************************************************

AdcCalibData* AdcCalibData::create(Name dstName, Name crName) {
  Spec spc(dstName, crName);
  if ( objects().find(spc) != objects().end() ) return nullptr;
  AdcCalibData* pobj = new AdcCalibData(dstName, crName);
  objects()[spc].reset(pobj);
  return pobj;
}

//**********************************************************************

const AdcCalibData* AdcCalibData::get(Name dstName, Name crName) {
  Spec spc(dstName, crName);
  ObjectMap::const_iterator iobj = objects().find(spc);
  if ( iobj == objects().end() ) return nullptr;
  return iobj->second.get();
}

//**********************************************************************

AdcCalibData::AdcCalibData(Name dstName, Name crName)
: m_dstName(dstName), m_crName(crName) { }

//**********************************************************************

int AdcCalibData::add(int pulser, Index run, Name fileName) {
  Entry ent;
  ent.pulser = pulser;
  ent.run = run;
  ent.fileName = fileName;
  m_ents.push_back(ent);
  return 0;
}

//**********************************************************************

void AdcCalibData::print() const {
  cout << "AdcCalibData " << dstName() << " " << crName() << endl;
  for ( const Entry& ent : data() ) {
    cout << setw(6) << ent.pulser
         << setw(8) << ent.run
         << "  " << ent.fileName << endl;
  }
}

//**********************************************************************

AdcCalibData::ObjectMap& AdcCalibData::objects() {
  static ObjectMap objs;
  return objs;
}

//**********************************************************************
