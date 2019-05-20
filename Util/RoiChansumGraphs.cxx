// RoiChansumGraphs.cxx

#include "RoiChansumGraphs.h"
#include "dune/DuneCommon/LineColors.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "TFile.h"
#include "TH1.h"
#include "TGraph.h"
#include <iostream>
#include <algorithm>
#include <sstream>

using std::cout;
using std::endl;

using Name = RoiChansumGraphs::Name;
using GraphPtr = RoiChansumGraphs::GraphPtr;
using PadPtr = RoiChansumGraphs::PadPtr;

//**********************************************************************

RoiChansumGraphs::RoiChansumGraphs(Name procSetName, Index ichaBegin, Index ichaEnd)
: m_procsetVarNames({"run", "runPeriod", "frun"}),
  m_histVarNames({"Count", "Area", "Time"}),
  m_pprocSet(ProcSet::getExisting(procSetName)),
  m_ichaBegin(ichaBegin),
  m_ichaEnd(ichaEnd) {
  Name myname = "RoiChansumGraphs::ctor: ";
  if ( m_pprocSet == nullptr ) {
    cout << myname << "ProcSet not found: " << procSetName << endl;
  }
  m_pChannelStatusProvider = art::ServiceHandle<lariov::ChannelStatusService>()->provider();
}

//**********************************************************************

int RoiChansumGraphs::fillValuesForSample(Name varName, Name sampleName, float defaultValue) {
  Name myname = "RoiChansumGraphs::fillValues: ";
  if ( m_pprocSet == nullptr ) return 1;
  if ( m_histVarNames.find(varName) == m_histVarNames.end() ) return 2;
  Name fileName = m_pprocSet->getString(sampleName, "roiChanSumFile");
  Name crName = m_pprocSet->getGlobalString("channelRange");
  if ( fileName.size() == 0 ) return 3;
  FloatVector& vals = m_vals[varName][sampleName];
  FloatVector& cnts = m_vals["Count"][sampleName];
  FloatVector& errs = m_errs[varName][sampleName];
  using FilePtr = std::unique_ptr<TFile>;
  FilePtr pfil(TFile::Open(fileName.c_str()));
  if ( ! pfil || ! pfil->IsOpen() ) {
    cout << myname << "Unable to open file " << fileName << endl;
    //if ( m_allowMissingRuns ) continue;
    return 4;
  }
  Name histName = "hcs" + varName + "_" + crName;
  TH1* ph = dynamic_cast<TH1*>(pfil->Get(histName.c_str()));
  if ( ph == nullptr ) {
    cout << myname << "Unable to find histogram " << histName  << " in " << fileName << endl;
    //pfil->ls();
    return 5;
  }
  int nbin = ph->GetNbinsX();
  Index ichaMin = Index(ph->GetBinLowEdge(1) + 0.01);
  Index ichaMax = Index(ph->GetBinLowEdge(nbin) + 0.01);
  if ( ichaMax + 1 != ichaMin + nbin ) {
    cout << myname << "Histogram " << histName << " does not have single channel binning." << endl;
    return 6;
  }
  Index icha1 = m_ichaBegin;
  Index icha2 = m_ichaEnd;
  Index ncha = icha2 - icha1;
  if ( icha1 > ichaMax || icha2 <= ichaMin ) {
    cout << myname << "Range for histogram " << ph->GetName()
         << "[" << ichaMin << ", " << ichaMax << "]"
         << " does not include requested range [" << icha1 << ", " << icha2 << ")." << endl;
    return 7;
  }
  vals.resize(ncha, defaultValue);
  errs.resize(ncha, defaultValue);
  bool doErr = varName != "Count";
  for ( Index icha=icha1; icha<icha2; ++icha ) {
    Index ibin = icha - ichaMin + 1;
    Index ival = icha - icha1;
    float val = ph->GetBinContent(ibin);
    float err = 0.0;
    if ( doErr && cnts.size() > ival ) {
      float rms = ph->GetBinError(ibin);
      float cnt = cnts[ival];
      err = cnt > 0.0 ? rms/sqrt(cnt) : 0.0;
    }
    vals[ival] = val;
    errs[ival] = err;
  }
  // Fetch/check label.
  Name& varLabel = m_varLabs[varName];
  Name hstLabel = ph->GetYaxis()->GetTitle();
  if ( varLabel.size() == 0 )  {
    varLabel = hstLabel;
  } else {
    if ( hstLabel != varLabel ) {
      cout << myname << "WARNING: histogram has inconsistent label: "
           << hstLabel << " != " << varLabel << endl;
    }
  }
  return 0;
}
  
//**********************************************************************

int RoiChansumGraphs::fillValues(Name varName, float defaultValue) {
  if ( m_pprocSet == nullptr ) return 1;
  int rstat = 0;
  for ( Name sampleName : m_pprocSet->sampleNames() ) {
    rstat += fillValuesForSample(varName, sampleName, defaultValue);
  }
  return rstat;
}

//**********************************************************************

float RoiChansumGraphs::value(Name varName, Name sampleName, Index icha, float defaultValue) {
  // Handle special variables.
  if ( varName == "CountPerEvent" ) {
    float eventCount = m_pprocSet->getFloat(sampleName, "eventCount", 1);
    if ( eventCount < 1.01 ) eventCount = 1.0;
    return value("Count", sampleName, icha, defaultValue)/eventCount;
  }
  // First check if this is a procset variable.
  if ( m_procsetVarNames.find(varName) != m_procsetVarNames.end() ) {
    return m_pprocSet->getFloat(sampleName, varName, defaultValue);
  }
  // Otherwise see if this is an assigned value.
  FloatVectorMapMap::const_iterator ival = m_vals.find(varName);
  if ( ival == m_vals.end() ) return defaultValue;
  const FloatVectorMap& vecs = ival->second;
  FloatVectorMap::const_iterator ivec = vecs.find(sampleName);
  if ( ivec == vecs.end() ) return defaultValue;
  const FloatVector& flts = ivec->second;
  if ( icha < m_ichaBegin ) return defaultValue;
  Index iflt = icha - m_ichaBegin;
  if ( iflt >= flts.size() ) return defaultValue;
  return flts[iflt];
}

//**********************************************************************

float RoiChansumGraphs::error(Name varName, Name sampleName, Index icha, float defaultValue) {
  FloatVectorMapMap::const_iterator ival = m_errs.find(varName);
  if ( ival == m_errs.end() ) return defaultValue;
  const FloatVectorMap& vecs = ival->second;
  FloatVectorMap::const_iterator ivec = vecs.find(sampleName);
  if ( ivec == vecs.end() ) return defaultValue;
  const FloatVector& flts = ivec->second;
  if ( icha < m_ichaBegin ) return defaultValue;
  Index iflt = icha - m_ichaBegin;
  if ( iflt >= flts.size() ) return defaultValue;
  return flts[iflt];
}

//**********************************************************************

Name RoiChansumGraphs::graphName(Name varNameY, Name varNameX, Index icha) {
  Name scha = std::to_string(icha);
  while ( scha.size() < 6 ) scha = "0" + scha;
  return varNameY + "_" + varNameX + "_" + scha;
}

//**********************************************************************

Name RoiChansumGraphs::sampleGraphName(Name varNameY, Name sampleName) {
  return varNameY + "_channel_" + sampleName;
}

//**********************************************************************

GraphPtr RoiChansumGraphs::graph(Name varNameY, Name varNameX, Index icha) {
  Name graName = graphName(varNameY, varNameX, icha);
  GraphMap::const_iterator igra = m_gras.find(graName);
  if ( igra != m_gras.end() ) return igra->second;
  GraphPtr pgra;
  if ( m_pprocSet == nullptr ) return pgra;
  pgra.reset(new TGraphErrors());
  Index ipt = 0;
  bool haveErr = false;
  for ( Name sampleName : m_pprocSet->sampleNames() ) {
    float x = value(varNameX, sampleName, icha);
    float y = value(varNameY, sampleName, icha);
    float ex = error(varNameX, sampleName, icha);
    float ey = error(varNameY, sampleName, icha);
    if ( ex > 0.0 || ey > 0.0 ) haveErr = true;
    pgra->SetPoint(ipt, x, y);
    pgra->SetPointError(ipt, ex, ey);
    ++ipt;
  }
  if ( ! haveErr ) pgra->SetMarkerStyle(2);
  pgra->GetXaxis()->SetTitle(m_varLabs[varNameX].c_str());
  pgra->GetYaxis()->SetTitle(m_varLabs[varNameY].c_str());
  string sttl = "ROI " + varNameY + " vs. " + varNameX + " for channel " + std::to_string(icha);
  pgra->SetTitle(sttl.c_str());
  m_gras[graName] = pgra;
  return pgra;
}

//**********************************************************************

PadPtr RoiChansumGraphs::pad(Name varNameY, Name varNameX, Index icha, float ymin, float ymax) {
  Name graName = graphName(varNameY, varNameX, icha);
  PadMap::const_iterator ipad = m_pads.find(graName);
  if ( ipad != m_pads.end() ) return ipad->second;
  GraphPtr pgra = graph(varNameY, varNameX, icha);
  PadPtr ppad;
  if ( ! pgra ) return ppad;
  ppad.reset(new TPadManipulator);
  ppad->add(pgra.get(), "P");
  ppad->addAxis();
  ppad->setRangeY(ymin, ymax);
  ppad->setGridY();
  ppad->showGraphOverflow("BT");
  m_pads[graName] = ppad;
  return ppad;
}

//**********************************************************************

PadPtr RoiChansumGraphs::draw(Name varNameY, Name varNameX, Index icha, float ymin, float ymax) {
  PadPtr ppad = pad(varNameY, varNameX, icha, ymin, ymax);
  if ( ! ppad ) return ppad;
  ppad->setRangeY(ymin, ymax);
  ppad->draw();
  return ppad;
}

//**********************************************************************

PadPtr RoiChansumGraphs::print(Name varNameY, Name varNameX, Index icha, float ymin, float ymax) {
  PadPtr ppad = pad(varNameY, varNameX, icha, ymin, ymax);
  if ( ! ppad ) return ppad;
  ppad->setRangeY(ymin, ymax);
  Name graName = graphName(varNameY, varNameX, icha);
  Name fnam = "rcg_" + graName + ".png";
  ppad->print(fnam);
  return ppad;
}

//**********************************************************************

GraphPtr RoiChansumGraphs::sampleGraph(Name varNameY, Name sampleName) {
  const Name myname = "RoiChansumGraphs::sampleGraph: ";
  Name graName = sampleGraphName(varNameY, sampleName);
  GraphMap::const_iterator igra = m_gras.find(graName);
  if ( igra != m_gras.end() ) return igra->second;
  GraphPtr pgra;
  if ( m_pprocSet == nullptr ) return pgra;
  const ProcSet::NameVector& sampleNames = m_pprocSet->sampleNames();
  if ( std::find(sampleNames.begin(), sampleNames.end(), sampleName) == sampleNames.end() ) {
    cout << myname << "Sample not found: " << sampleName << endl;
    return pgra;
  }
  pgra.reset(new TGraphErrors());
  GraphPtr pgraBad(new TGraphErrors());
  GraphPtr pgraNoisy(new TGraphErrors());
  int colBad = LineColors::red();
  int colNoisy = LineColors::brown();
  int mrkBad = 24;
  int mrkNoisy = 24;
  pgraBad->SetLineColor(colBad);
  pgraBad->SetMarkerColor(colBad);
  pgraBad->SetMarkerStyle(mrkBad);
  pgraNoisy->SetLineColor(colNoisy);
  pgraNoisy->SetMarkerColor(colNoisy);
  pgraNoisy->SetMarkerStyle(mrkNoisy);
  Index ipt = 0;
  bool haveErr = false;
  for ( Index icha=ichaBegin(); icha<ichaEnd(); ++icha ) {
    float x = icha;
    float y = value(varNameY, sampleName, icha);
    float ex = 0.25;
    float ey = error(varNameY, sampleName, icha);
    if ( ey > 0.0 ) haveErr = true;
    pgra->SetPoint(ipt, x, y);
    pgra->SetPointError(ipt, ex, ey);
    bool isBad   = m_pChannelStatusProvider != nullptr ? m_pChannelStatusProvider->IsBad(icha) : false;
    bool isNoisy = m_pChannelStatusProvider != nullptr ? m_pChannelStatusProvider->IsNoisy(icha) : false;
    if ( isBad ) {
      pgraBad->SetPoint(ipt, x, y);
      pgraBad->SetPointError(ipt, ex, ey);
    } else if ( isNoisy ) {
      pgraNoisy->SetPoint(ipt, x, y);
      pgraNoisy->SetPointError(ipt, ex, ey);
    }
    ++ipt;
  }
  if ( ! haveErr ) pgra->SetMarkerStyle(2);
  pgra->GetXaxis()->SetTitle("Channel");
  pgra->GetYaxis()->SetTitle(m_varLabs[varNameY].c_str());
  string sttl = "ROI " + varNameY + " vs. channel for sample " + sampleName;
  string crName = m_pprocSet->getGlobalString("channelRange");
  if ( crName.size() ) sttl += " " + crName;
  ostringstream ssout;
  ssout.precision(1);
  ssout.setf(ios::fixed);
  ssout << " at hour " << value("Time", sampleName, ichaBegin());
  sttl += ssout.str();
  pgra->SetTitle(sttl.c_str());
  m_gras[graName] = pgra;
  m_grasBad[graName] = pgraBad;
  m_grasNoisy[graName] = pgraNoisy;
  return pgra;
}

//**********************************************************************

GraphPtr RoiChansumGraphs::sampleGraphBad(Name varNameY, Name sampleName) {
  const Name myname = "RoiChansumGraphs::sampleGraphBad: ";
  Name graName = sampleGraphName(varNameY, sampleName);
  GraphMap::const_iterator igra = m_grasBad.find(graName);
  if ( igra != m_grasBad.end() ) return igra->second;
  return GraphPtr();
}

//**********************************************************************

GraphPtr RoiChansumGraphs::sampleGraphNoisy(Name varNameY, Name sampleName) {
  const Name myname = "RoiChansumGraphs::sampleGraphNoisy: ";
  Name graName = sampleGraphName(varNameY, sampleName);
  GraphMap::const_iterator igra = m_grasNoisy.find(graName);
  if ( igra != m_grasNoisy.end() ) return igra->second;
  return GraphPtr();
}

//**********************************************************************

PadPtr RoiChansumGraphs::samplePad(Name varNameY, Name sampleName, float ymin, float ymax) {
  Name graName = sampleGraphName(varNameY, sampleName);
  PadMap::const_iterator ipad = m_pads.find(graName);
  if ( ipad != m_pads.end() ) return ipad->second;
  GraphPtr pgra = sampleGraph(varNameY, sampleName);
  PadPtr ppad;
  if ( ! pgra ) return ppad;
  ppad.reset(new TPadManipulator(1400, 500));
  ppad->add(pgra.get(), "P");
  ppad->addAxis();
  Index icha1 = ichaBegin();
  Index icha2 = ichaEnd();
  if ( icha2 > icha1 ) {
    double xmin = icha1 - 0.5;
    double xmax = icha2 - 0.5;
    ppad->setRangeX(xmin, xmax);
  }
  ppad->setRangeY(ymin, ymax);
  ppad->setGridY();
  ppad->showGraphOverflow("BT");
  int fembWidth = m_pprocSet->getGlobalFloat("fembWidth", 0.0);
  if ( fembWidth > 0.0 ) {
    ppad->addVerticalModLines(fembWidth, ichaBegin()-0.5);
  }
  GraphPtr pgraBad = sampleGraphBad(varNameY, sampleName);
  if ( pgraBad ) ppad->add(pgraBad.get(), "P");
  GraphPtr pgraNoisy = sampleGraphNoisy(varNameY, sampleName);
  if ( pgraNoisy ) ppad->add(pgraNoisy.get(), "P");
  m_pads[graName] = ppad;
  return ppad;
}

//**********************************************************************

PadPtr RoiChansumGraphs::drawSample(Name varNameY, Name sampleName, float ymin, float ymax) {
  PadPtr ppad = samplePad(varNameY, sampleName, ymin, ymax);
  if ( ! ppad ) return ppad;
  ppad->setRangeY(ymin, ymax);
  ppad->draw();
  return ppad;
}

//**********************************************************************

PadPtr RoiChansumGraphs::printSample(Name varNameY, Name sampleName, float ymin, float ymax) {
  PadPtr ppad = samplePad(varNameY, sampleName, ymin, ymax);
  if ( ! ppad ) return ppad;
  ppad->setRangeY(ymin, ymax);
  Name graName = sampleGraphName(varNameY, sampleName);
  Name fnam = "rcg_" + graName + ".png";
  ppad->print(fnam);
  return ppad;
}

//**********************************************************************
