// ProcSet.h
//
// David Adams
// February 2019
//
// A procset is a set of processed samples typically different
// data selections (e.g. different runs) processed in the same way.
// Each sample holds vectors of strings and floats indexed by key names.
// The string values might be file names.

#ifndef ProcSet_H
#define ProcSet_H

#include <map>
#include <memory>

class ProcSet {

public:

  using Name = std::string;
  using NameVector = std::vector<Name>;
  using FloatVector = std::vector<float>;
  using NameMap = std::map<Name, Name>;
  using NameMapMap = std::map<Name, NameMap>;
  using FloatMap = std::map<Name, float>;
  using FloatMapMap = std::map<Name, FloatMap>;
  using ProcSetPtr = std::unique_ptr<ProcSet>;
  using ProcSetMap = std::map<Name, ProcSetPtr>;

  // Fetch an existing procset.
  static ProcSet* getExisting(Name a_name);

  // Create a new procset.
  // Or return the existing one.
  static ProcSet* get(Name a_name);

  // Return the name of this object.
  Name name() const { return m_name; }

  // Return the sample names (e.g. runs).
  const NameVector& sampleNames() const;

  // Return the global string key names (e.g. "channelRange")
  const NameVector& globalStringKeyNames() const;

  // Return a string by sample and key.
  // Return blank if there is no value for the sample and key.
  Name getGlobalString(Name keyName) const;

  // Add a global string for a key.
  // If overwriteFile is true, an existing value may be overwritten.
  int addGlobalString(Name key, Name value, bool overwrite =false);

  // Return the global string key names (e.g. "channelRange")
  const NameVector& globalFloatKeyNames() const;

  // Return a string by sample and key.
  // Return blank if there is no value for the sample and key.
  float getGlobalFloat(Name keyName, float defaultValue =0.0) const;

  // Add a global string for a key.
  // If overwriteFile is true, an existing value may be overwritten.
  int addGlobalFloat(Name key, float value, bool overwrite =false);

  // Return the string key names (e.g. "roiChanSumFile", "channelRange")
  const NameVector& stringKeyNames() const;

  // Return string map for a sample.
  const NameMap& sampleStrings(Name sampleName) const;

  // Return a string by sample and key.
  // Return blank if there is no value for the sample and key.
  Name getString(Name sampleName, Name keyName) const;

  // Add a string for a sample and key.
  // If the sample does not exist and createSample is true,
  // the sample is created.
  // If overwrite is true, an existing value may be overwritten.
  int addString(Name sampleName, Name key, Name value,
                bool createSample =true, bool overwrite =false);

  // Return the float key names (e.g. "run", "runPeriod", "frun")
  const NameVector& floatKeyNames() const;

  // Return float map for a sample.
  const FloatMap& sampleFloats(Name sampleName) const;

  // Return a float by sample and key.
  // Return defaultValue if there is no value for the sample and key.
  float getFloat(Name sampleName, Name keyName, float defaultValue =0.0) const;

  // Add a float for a sample and key.
  // If the sample does not exist and createSample is true,
  // the sample is created.
  // If overwrite is true, an existing value may be overwritten.
  int addFloat(Name sampleName, Name key, float value,
               bool createSample =true, bool overwrite =false);

private:

  Name m_name;
  NameVector m_sampleNames;
  NameVector m_stringKeyNames;
  NameMapMap m_strings;
  NameVector m_floatKeyNames;
  FloatMapMap m_floats;
  NameVector m_globalStringKeyNames;
  NameMap m_globalStrings;
  NameVector m_globalFloatKeyNames;
  FloatMap m_globalFloats;

};

#endif
