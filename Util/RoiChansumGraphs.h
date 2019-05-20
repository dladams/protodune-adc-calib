// RoiChansumGraphs.h
//
// DavidAdams
// February 2019
//
// Make graphs from ROI channel summary histograms.
//
// Supported variable names:
//   Area - ROI area mean
//   AreaSigma - ROI area sigma
//   Time - ROI mean time

#ifndef RoiChansumGraphs_H
#define RoiChansumGraphs_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "ProcSet.h"
#include "TGraphErrors.h"
#include "dune/DuneCommon/TPadManipulator.h"

namespace lariov {
  class ChannelStatusProvider;
}

class RoiChansumGraphs {

public:

  using Index = unsigned int;
  using Name = std::string;
  using NameVector = std::vector<Name>;
  using NameSet = std::set<Name>;
  using NameMap = std::map<Name, Name>;
  using Graph = TGraphErrors;
  using GraphPtr = std::shared_ptr<Graph>;
  using GraphMap = std::map<Name, GraphPtr>;
  using PadPtr = std::shared_ptr<TPadManipulator>;
  using PadMap = std::map<Name, PadPtr>;
  using FloatVector = std::vector<float>;
  using FloatVectorMap = std::map<Name, FloatVector>;
  using FloatVectorMapMap = std::map<Name, FloatVectorMap>;

  // Ctor from a procset.
  // ROI channel summary file is type "roiChanSumFile"
  // Channel range type "channelRange"
  RoiChansumGraphs(Name procSetName, Index ichaBegin, Index ichaEnd);

  // Fill values for variable and sample.
  int fillValuesForSample(Name varName, Name sampleName, float defaultValue =0.0);

  // Fill values for variable for all samples.
  int fillValues(Name varName, float defaultValue =0.0);

  // Getters.
  Index ichaBegin() const { return m_ichaBegin; }
  Index ichaEnd() const { return m_ichaEnd; }

  // Return a variable value (or error) for a sample and channel.
  // If needed, caches the value for all channels.
  // Returns defaultValue if the data is not available.
  float value(Name varName, Name sampleName, Index icha, float defaultValue =0.0);
  float error(Name varName, Name sampleName, Index icha, float defaultValue =0.0);

  // Return the name for a graph.
  Name graphName(Name vary, Name varx, Index icha);
  Name sampleGraphName(Name vary, Name sampleName);

  // Return a graph for channel.
  GraphPtr graph(Name vary, Name varx, Index icha);

  // Return a pad for a channel.
  PadPtr pad(Name vary, Name varx, Index icha, float ymin, float ymax);
  PadPtr draw(Name vary, Name varx, Index icha, float ymin, float ymax);
  PadPtr print(Name vary, Name varx, Index icha, float ymin, float ymax);

  // Return a graph vs. channel for a sample.
  GraphPtr sampleGraph(Name vary, Name sampleName);
  GraphPtr sampleGraphBad(Name vary, Name sampleName);
  GraphPtr sampleGraphNoisy(Name vary, Name sampleName);

  // Return a pad for a sample.
  PadPtr samplePad(Name vary, Name sampleName, float ymin, float ymax);
  PadPtr drawSample(Name vary, Name sampleName, float ymin, float ymax);
  PadPtr printSample(Name vary, Name sampleName, float ymin, float ymax);

private:

  NameSet m_procsetVarNames;
  NameSet m_histVarNames;
  const ProcSet* m_pprocSet;
  Index m_ichaBegin;
  Index m_ichaEnd;
  FloatVectorMapMap m_vals;     // [varname][sampleName][icha]
  FloatVectorMapMap m_errs;     // Errors for vals
  NameMap m_varLabs;            // [varname] - Label for each variable.
  GraphMap m_gras;
  GraphMap m_grasBad;
  GraphMap m_grasNoisy;
  PadMap m_pads;
  const lariov::ChannelStatusProvider* m_pChannelStatusProvider;

};

#endif
