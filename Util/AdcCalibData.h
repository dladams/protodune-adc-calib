// AdcCalibData.h
//
// David Adams
// January 2019
//
// Class that describes files holding channel summary histograms
// from DUNE ROI fitting of pulser data.
//
// Note that pulser indices are signed with negative values indicating
// that negative pulses were processed.
//
// The name attached to the data should specify the input data and ROI
// processing but be concise enough to appear in a plot label.

#ifndef AdcCalibData_H
#define AdcCalibData_H

#include <memory>
#include <map>

class AdcCalibData {

public:

  using Index = unsigned int;
  using Name = std::string;
  using ObjectPtr = std::unique_ptr<AdcCalibData>;
  using ObjectMap = std::map<Name, ObjectPtr>;

  class Entry {
  public:
    int pulser =0;
    Index run =0;
    Name fileName;
  };

  using EntryVector = std::vector<Entry>;

  // Create an empty object, insert it in the global map
  // and return it.
  // Fails (returns null) if an object with the name already exists.
  static AdcCalibData* create(Name name);

  // Return a named object from the global map.
  // Returns null if no object has the name.
  static const AdcCalibData* get(Name name);

  // Ctor.
  AdcCalibData(Name name);

  // Add an entry.
  int add(int pulser, Index run, Name fileName);

  // Getters.
  Name name() const { return m_name; }
  const EntryVector& data() const { return m_ents; }

  // Show the data.
  void print() const;

private:

  Name m_name;
  EntryVector m_ents;

  static ObjectMap& objects();

};

#endif
