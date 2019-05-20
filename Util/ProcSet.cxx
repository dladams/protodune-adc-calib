// ProcSet.cxx

#include "ProcSet.h"
#include <algorithm>

using Name = ProcSet::Name;

//*********************************************************************

namespace {
ProcSet* procSetGet(Name name, bool doCreate) {
  static ProcSet::ProcSetMap pss;
  ProcSet::ProcSetMap::iterator ips = pss.find(name);
  if ( ips != pss.end() ) return ips->second.get();
  if ( ! doCreate ) return nullptr;
  ProcSet* pps = new ProcSet;
  pss[name].reset(pps);
  return pps;
}
}

//*********************************************************************

ProcSet* ProcSet::getExisting(Name name) {
  return procSetGet(name, false);
}

//*********************************************************************

ProcSet* ProcSet::get(Name name) {
  ProcSet* pps = procSetGet(name, false);
  if ( pps != nullptr ) return pps;
  pps = procSetGet(name, true);
  pps->m_name = name;
  return pps;
}

//*********************************************************************

const ProcSet::NameVector& ProcSet::sampleNames() const {
  return m_sampleNames;
}

//*********************************************************************

const ProcSet::NameVector& ProcSet::globalStringKeyNames() const {
  return m_globalStringKeyNames;
}

//*********************************************************************

ProcSet::Name ProcSet::getGlobalString(Name keyName) const {
  const NameMap& strings = m_globalStrings;
  NameMap::const_iterator istr = strings.find(keyName);
  if ( istr == strings.end() ) return "";
  return istr->second;
}

//*********************************************************************

int ProcSet::addGlobalString(Name keyName, Name value, bool overwrite) {
  NameMap& keyValues = m_globalStrings;
  if ( keyValues.find(keyName) != keyValues.end() && !overwrite ) return 2;
  if ( std::find(m_globalStringKeyNames.begin(), m_globalStringKeyNames.end(), keyName) == m_globalStringKeyNames.end() ) {
    m_globalStringKeyNames.push_back(keyName);
  }
  keyValues[keyName] = value;
  return 0;
}

//*********************************************************************

const ProcSet::NameVector& ProcSet::globalFloatKeyNames() const {
  return m_globalFloatKeyNames;
}

//*********************************************************************

float ProcSet::getGlobalFloat(Name keyName, float defaultValue) const {
  const FloatMap& floats = m_globalFloats;
  FloatMap::const_iterator ival = floats.find(keyName);
  if ( ival == floats.end() ) return defaultValue;
  return ival->second;
}

//*********************************************************************

int ProcSet::addGlobalFloat(Name keyName, float value, bool overwrite) {
  FloatMap& keyValues = m_globalFloats;
  if ( keyValues.find(keyName) != keyValues.end() && !overwrite ) return 2;
  if ( std::find(m_globalFloatKeyNames.begin(), m_globalFloatKeyNames.end(), keyName) == m_globalFloatKeyNames.end() ) {
    m_globalFloatKeyNames.push_back(keyName);
  }
  keyValues[keyName] = value;
  return 0;
}

//*********************************************************************

const ProcSet::NameVector& ProcSet::stringKeyNames() const {
  return m_stringKeyNames;
}

//*********************************************************************

const ProcSet::NameMap& ProcSet::sampleStrings(Name sampleName) const {
  NameMapMap::const_iterator iprs = m_strings.find(sampleName);
  static NameMap empty;
  if ( iprs == m_strings.end() ) return empty;
  return iprs->second;
}

//*********************************************************************

ProcSet::Name ProcSet::getString(Name sampleName, Name keyName) const {
  const NameMap& strings = sampleStrings(sampleName);
  NameMap::const_iterator istr = strings.find(keyName);
  if ( istr == strings.end() ) return "";
  return istr->second;
}

//*********************************************************************

int ProcSet::addString(Name sampleName, Name keyName, Name value,
                       bool createSample, bool overwrite) {
  NameMapMap::const_iterator istr = m_strings.find(sampleName);
  if ( istr == m_strings.end() ) {
    if ( ! createSample ) return 1;
  }
  NameMap& keyValues = m_strings[sampleName];
  if ( keyValues.find(keyName) != keyValues.end() && !overwrite ) return 2;
  if ( std::find(m_sampleNames.begin(), m_sampleNames.end(), sampleName) == m_sampleNames.end() ) {
    m_sampleNames.push_back(sampleName);
  }
  if ( std::find(m_stringKeyNames.begin(), m_stringKeyNames.end(), keyName) == m_stringKeyNames.end() ) {
    m_stringKeyNames.push_back(keyName);
  }
  keyValues[keyName] = value;
  return 0;
}

//*********************************************************************

const ProcSet::NameVector& ProcSet::floatKeyNames() const {
  return m_floatKeyNames;
}

//*********************************************************************

const ProcSet::FloatMap& ProcSet::sampleFloats(Name sampleName) const {
  FloatMapMap::const_iterator iprf = m_floats.find(sampleName);
  static FloatMap empty;
  if ( iprf == m_floats.end() ) return empty;
  return iprf->second;
}

//*********************************************************************

float ProcSet::getFloat(Name sampleName, Name keyName, float defaultValue) const {
  const FloatMap& floats = sampleFloats(sampleName);
  FloatMap::const_iterator iflt = floats.find(keyName);
  if ( iflt == floats.end() ) return defaultValue;
  return iflt->second;
}

//*********************************************************************

int ProcSet::addFloat(Name sampleName, Name keyName, float value,
                      bool createSample, bool overwrite) {
  FloatMapMap::const_iterator iflt = m_floats.find(sampleName);
  if ( iflt == m_floats.end() ) {
    if ( ! createSample ) return 1;
  }
  FloatMap& keyValues = m_floats[sampleName];
  if ( keyValues.find(keyName) != keyValues.end() && !overwrite ) return 2;
  if ( std::find(m_sampleNames.begin(), m_sampleNames.end(), sampleName) == m_sampleNames.end() ) {
    m_sampleNames.push_back(sampleName);
  }
  if ( std::find(m_floatKeyNames.begin(), m_floatKeyNames.end(), keyName) == m_floatKeyNames.end() ) {
    m_floatKeyNames.push_back(keyName);
  }
  keyValues[keyName] = value;
  return 0;
}

//*********************************************************************
