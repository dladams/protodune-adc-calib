// AdcCalibData.cxx

# include "AdcCalibData.h"
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setw;

//**********************************************************************

AdcCalibData* AdcCalibData::create(Name name) {
  if ( objects().find(name) != objects().end() ) return nullptr;
  AdcCalibData* pobj = new AdcCalibData(name);
  objects()[name].reset(pobj);
  return pobj;
}

//**********************************************************************

const AdcCalibData* AdcCalibData::get(Name name) {
  ObjectMap::const_iterator iobj = objects().find(name);
  if ( iobj == objects().end() ) return nullptr;
  return iobj->second.get();
}

//**********************************************************************

AdcCalibData::AdcCalibData(Name name)
: m_name(name) { }

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
  cout << "AdcCalibData " << name() << endl;
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
