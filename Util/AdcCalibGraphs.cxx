// AdcCalibGraphs.cxx

#include "AdcCalibGraphs.h"
#include "AdcCalibData.h"
#include "dune/DuneCommon/offsetLine.h"
#include "dune/DuneCommon/StringManipulator.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <iostream>
#include <iomanip>
#include "TFile.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TLegend.h"

using std::cout;
using std::endl;
using std::ofstream;
using Index = AdcCalibGraphs::Index;
using PadPtr = AdcCalibGraphs::PadPtr;
using GraphPtr = AdcCalibGraphs::GraphPtr;
using Name = AdcCalibGraphs::Name;
using NameVector = std::vector<Name>;
using FilePtr = std::shared_ptr<TFile>;
using FloatVector = std::vector<float>;

//**********************************************************************

AdcCalibGraphs::AdcCalibGraphs(
  Name a_dstName,
  Name a_crName)
: m_dstName(a_dstName),
  m_crName(a_crName),
  m_allowMissingRuns(false),
  m_errorScaling(1),
  m_doGainResiduals(false),
  m_xgmin(0.0), m_xgmax(0.0),
  m_chargeUnit("Q_{step}"),
  m_graphChannelBegin(0),
  m_graphChannelEnd(0) {
  m_fits["Height"] = {offsetLineFullTF1(0, 140, 0,  1, -4, 12, "HeightFit")};
  m_fits["Area"] =   {offsetLineFullTF1(0, 900, 0,  1, -4, 12, "AreaFit")};
  m_fits["Shaping"] = {offsetLinePedTF1(0, 0, 4,  0, 12, "shapingFitPlus"),
                       offsetLinePedTF1(0, 0, 4, -4,  0, "shapingFitMinus")};
  m_fits["ChiSquare"] = {offsetLineFullTF1(0, 900, 0,  1, -4, 12, "ChiSquareFit")};
  m_fits["CSNormDof"] = {};
  for ( TF1* pfit : m_fits["Shaping"] ) {
    pfit->FixParameter(0, 0.0);
    pfit->FixParameter(1, 0.0);
    pfit->FixParameter(3, 0.0);
  }
  for ( TF1* pfit : m_fits["ChiSquare"] ) {
    pfit->FixParameter(0, 0.0);
    pfit->FixParameter(1, 0.0);
    pfit->FixParameter(3, 0.0);
  }
  m_varNames.push_back("Height");
  m_varNames.push_back("Area");
  m_varNames.push_back("Shaping");
  m_varNames.push_back("ChiSquare");
  m_varNames.push_back("CSNormDof");
  m_varNames.push_back("Count");
  m_varNames.push_back("FitCount");
  m_fitNames.push_back("Height");
  m_fitNames.push_back("Area");
  m_fitNames.push_back("Shaping");
  m_resNames.push_back("Height");
  m_resNames.push_back("Area");
  Name label = m_dstName + " " + m_crName;
  for ( Name varName : m_varNames ) {
    m_labels[varName] = m_dstName + " " + m_crName;
  }
  m_labels["Ratio"] = m_dstName + " " + m_crName;
  m_pChannelStatusProvider = art::ServiceHandle<lariov::ChannelStatusService>()->provider();
}

//**********************************************************************

AdcCalibGraphs::~AdcCalibGraphs() {
  string myname = "AdcCalibGraphs::dtor: ";
  cout << myname << "Deleting object at " << this << endl;
}

//**********************************************************************

void AdcCalibGraphs::fixHeightSlope(double val) {
  m_fits["Height"][0]->FixParameter(1, val);
  ostringstream sssuf;
  sssuf << " fixH" << val;
  m_labels["Height"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixAreaSlope(double val) {
  m_fits["Area"][0]->FixParameter(1, val);
  ostringstream sssuf;
  sssuf << " fixA" << val;
  m_labels["Area"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixHeightOffset(double val) {
  m_fits["Height"][0]->FixParameter(0, val);
  ostringstream sssuf;
  sssuf << " fixO" << val;
  m_labels["Height"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixAreaOffset(double val) {
  m_fits["Area"][0]->FixParameter(0, val);
  ostringstream sssuf;
  sssuf << " fixO" << val;
  m_labels["Area"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixHeightPedestal(double val) {
  m_fits["Height"][0]->FixParameter(2, val);
  ostringstream sssuf;
  sssuf << " fixP" << val;
  m_labels["Height"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixAreaPedestal(double val) {
  m_fits["Area"][0]->FixParameter(2, val);
  ostringstream sssuf;
  sssuf << " fixP" << val;
  m_labels["Area"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixHeightNegScale(double val) {
  m_fits["Height"][0]->FixParameter(3, val);
  ostringstream sssuf;
  sssuf << " fixN" << val;
  m_labels["Height"] += sssuf.str();
}

//**********************************************************************

void AdcCalibGraphs::fixAreaNegScale(double val) {
  m_fits["Area"][0]->FixParameter(3, val);
  ostringstream sssuf;
  sssuf << " fixN" << val;
  m_labels["Area"] += sssuf.str();
}

//**********************************************************************

int AdcCalibGraphs::setFitFunction(string varName, TF1* pfit, Index ifun) {
  const string myname = "AdcCalibGraphs::setFitFunction: ";
  NameVector::const_iterator ivna = find(m_varNames.begin(), m_varNames.end(), varName);
  if ( ivna == m_varNames.end() ) {
    cout << myname << "Unknown variable: " << varName << endl;
    return 1;
  }
  m_fits[varName][ifun] = pfit;
  return 0;
}

//**********************************************************************

Index AdcCalibGraphs::makeGraphs(Index icha1, Index ncha) {
  // Make the graphs.
  Index rstat = makeGraphsInternal("", icha1, ncha, m_gras, 1, 0);
  if ( rstat ) return rstat;
  // Find plot range.
  const AdcCalibData* padt = AdcCalibData::get(m_dstName, m_crName);
  double amin = 0.0;
  double amax = 0.0;
  Index count = 0;
  for ( const AdcCalibData::Entry& adt: padt->data() ) {
    double a = adt.pulser;
    if ( count == 0 || a < amin ) amin = a;
    if ( count == 0 || a > amax ) amax = a;
    ++count;
  }
  double da = 0.01*(amax - amin);
  m_xgmin = amin - da;
  m_xgmax = amax + da;
  return 0;
}

//**********************************************************************

Index AdcCalibGraphs::makeExtraGraphs(Name dstName, Index icha1, Index ncha, int icol, int imrk) {
  const Name myname = "AdcCalibGraphs::makeExtraGraphs: ";
  if ( find(m_extraDsts.begin(), m_extraDsts.end(), dstName) != m_extraDsts.end() ) {
    cout << myname << "Extra dataset has already been addded: " << dstName << endl;
    return 1;
  }
  m_extraDsts.push_back(dstName);
  GraphMapMap& gras = m_extraGras[dstName];
  return makeGraphsInternal(dstName, icha1, ncha, gras, icol, imrk);
}

//**********************************************************************

Index AdcCalibGraphs::
makeGraphsInternal(Name dstNameIn, Index icha1, Index ncha,
                   GraphMapMap& grasOut, int icol, int imrk) {
  string myname = "AdcCalibGraphs::makeGraphsInternal: ";
  Index icha2 = icha1 + ncha;
  Name dstName = dstNameIn.size() ? dstNameIn : m_dstName;
  for ( Name varName : m_varNames ) {
    for ( Index icha=icha1; icha<icha2; ++icha ) {
      if ( graph(varName, icha, dstNameIn) != nullptr ) {
        cout << myname << "ERROR: Graph " << varName << " for channel "
             << icha << " already exists." << endl;
        return 1;
      }
    }
  }
  GraphMapMap gras;
  const AdcCalibData* padt = AdcCalibData::get(dstName, m_crName);
  if ( padt == nullptr ) {
    cout << myname << "ERROR: AdcCalibData does not have dataset/CR "
         << dstName << "/" << m_crName << endl;
    return 2;
  }
  if ( padt->data().size() == 0 ) {
    cout << myname << "ERROR: AdcCalibData dataset " << dstName << "/"
         << m_crName << " is empty." << endl;
    return 3;
  }
  for ( Name varName : m_varNames ) {
    string sttl;
    string sttlx = "Pulser amplitude";
    string sttly;
    string sunity;
    int erropt = m_errorScaling;
    bool isFit = false;
    if ( varName == "Height" ) {
      sttl = "Fitted height";
      sttly = "Height [ADC count]";
      isFit = true;
    } else if ( varName == "Area" ) {
      sttl = "ROI area";
      sttly = "Area [(ADC count)#timestick]";
      isFit = true;
    } else if ( varName == "ChiSquare" ) {
      sttl = "Fit chi-square";
      sttly = "#chi^{2}";
      isFit = true;
      erropt = 0;
    } else if ( varName == "CSNormDof" ) {
      sttl = "Normalized fit chi-square/DOF";
      sttly = "#chi^{2}/DOF";
      isFit = true;
      erropt = 0;
    } else if ( varName == "Shaping" ) {
      sttl = "Fit shaping time";
      sttly = "Shaping time";
      sunity = "Tick";
      isFit = true;
    } else if ( varName == "Count" ) {
      sttl = "ROI count";
      sttly = "Count";
      erropt = 0;
    } else if ( varName == "FitCount" ) {
      sttl = "Good fit count";
      sttly = "Count";
      erropt = 0;
    } else {
      cout << myname << "WARNING: Title not set for variable " << varName << endl;
    }
    if ( varName == "Height" ) sttly = "Height [ADC counts]";
    for ( Index icha=icha1; icha<icha2; ++icha ) {
      ostringstream sscha;
      sscha << icha;
      Name scha = sscha.str();
      Name sttlChan = sttl + " channel " + scha;
      TGraphErrors* pg = new TGraphErrors;
      pg->SetMarkerColor(icol);
      pg->SetLineColor(icol);
      pg->SetMarkerStyle(imrk);
      pg->SetTitle(sttlChan.c_str());
      pg->GetXaxis()->SetTitle(sttlx.c_str());
      pg->GetYaxis()->SetTitle(sttly.c_str());
      int imrk = erropt == 0 ? 2 : 1;
      if ( erropt == 0 && pg->GetMarkerStyle() < 2 ) pg->SetMarkerStyle(imrk);
      gras[varName][icha].reset(pg);
    }
    for ( const AdcCalibData::Entry& adt: padt->data() ) {
      int erroptEntry = erropt;
      double a = adt.pulser;
      Index run = adt.run;
      Name fnam = adt.fileName;
      double sign = 1.0;
      if ( varName == "Height" ) sign = a/fabs(a);
      if ( varName == "Area" ) sign = a/fabs(a);
      float xval = a;
      float xerr = 0.0;
      FilePtr pfil(TFile::Open(fnam.c_str()));
      if ( ! pfil || ! pfil->IsOpen() ) {
        cout << myname << "Unable to open file " << fnam << endl;
        if ( m_allowMissingRuns ) continue;
        return 4;
      }
      //pfil->ls();
      Name histName = "hcs" + varName + "_" + m_crName;
      TH1* ph = dynamic_cast<TH1*>(pfil->Get(histName.c_str()));
      if ( ph == nullptr ) {
        cout << myname << "Unable to find histogram " << histName  << " in " << fnam << endl;
        return 5;
      }
      int nbin = ph->GetNbinsX();
      Index ichaMin = Index(ph->GetBinLowEdge(1) + 0.01);
      Index ichaMax = Index(ph->GetBinLowEdge(nbin) + 0.01);
      if ( ichaMax + 1 != ichaMin + nbin ) {
        cout << myname << "Histogram " << histName << " does not have single channel binning." << endl;
        return 6;
      }
      if ( icha1 > ichaMax || icha2 <= ichaMin ) {
        cout << myname << "Range for histogram " << ph->GetName()
             << "[" << ichaMin << ", " << ichaMax << "]"
             << " does not include requested range [" << icha1 << ", " << icha2 << ")." << endl;
        return 7;
      }
      // If needed get corresponding count histogram.
      TH1* phCount = nullptr;
      if ( erroptEntry == 2 ) {
        Name cvarName = isFit ? "FitCount" : "Count";
        Name chistName = "hcs" + cvarName + "_" + m_crName;
        phCount = dynamic_cast<TH1*>(pfil->Get(chistName.c_str()));
        if ( phCount == nullptr ) {
          cout << myname << "WARNING: Unable to find count histogram " << chistName  << " in " << fnam << endl;
          cout << myname << "WARNING: Errors will not be assigned." << endl;
          erroptEntry = 0;
        }
      }
      for ( int ibin=1; ibin<=nbin; ++ibin ) {
        double yval = sign*ph->GetBinContent(ibin);
        double yrms = ph->GetBinError(ibin);
        double yerr = yrms;
        if ( erroptEntry == 0 ) yerr = 0.0;
        if ( erroptEntry == 2 ) {
          double cnt = phCount->GetBinContent(ibin);
          double sfac = cnt > 1 ? 1/sqrt(cnt) : 0.0;
          yerr *= sfac;
        }
        Index icha = Index(ph->GetBinLowEdge(ibin) + 0.01);
        if ( icha < icha1 ) continue;
        if ( icha >= icha2 ) break;
        //cout << myname << "  " << icha << ": " << yval << " +/- " << yerr << endl;
        GraphPtr pg = gras[varName][icha];
        if ( ! pg ) {
          cout << myname << "ERROR: Unexpected null graph." << endl;
          return 8;
        }
        int ipt = pg->GetN();
        pg->SetPoint(ipt, xval, yval);
        pg->SetPointError(ipt, xerr, yerr);
      }
    }
  }
  for ( Name varName : m_varNames ) {
    for ( Index icha=icha1; icha<icha2; ++icha ) {
      GraphPtr pg = gras[varName][icha];
      pg->GetXaxis()->SetRangeUser(m_xgmin, m_xgmax);
      grasOut[varName][icha] = pg;
    }
  }
  if ( graphChannelEnd() <= graphChannelBegin() ) {
    m_graphChannelBegin = icha1;
    m_graphChannelEnd = icha2;
  } else {
    if ( icha1 < graphChannelBegin() ) m_graphChannelBegin = icha1;
    if ( icha2 < graphChannelEnd() ) m_graphChannelEnd = icha2;
  }
  return 0;
}

//**********************************************************************

Index AdcCalibGraphs::fitGraphs(Index icha1, Index ncha, double xmin0, double xmax0, Name fopt0) {
  Name myname = "AdcCalibGraphs::fitGraphs: ";
  //makeGraphs(icha1, ncha);
  for ( Name varName : m_varNames ) {
    for ( Index icha=icha1; icha<icha1+ncha; ++icha ) {
      GraphPtr pgra = graph(varName, icha);
      if ( ! pgra ) {
        cout << myname << "WARNING: Unable to find graph for channel " << icha << endl;
        continue;
      }
      const FunVector& fits = m_fits[varName];
      Name fopt = fopt0;
      Index nfit = 0;
      for ( TF1* pfit : m_fits[varName] ) {
        double xmin = xmin0;
        double xmax = xmax0;
        if ( fits.size() == 2 ) {
          if ( nfit == 0 ) xmin = 0.0;
          if ( nfit == 1 ) xmax = 0.0;
        }
        if ( xmax > xmin ) {
          pgra->Fit(pfit, fopt.c_str(), "", xmin, xmax);
          fopt = fopt0 + "+";
          ++nfit;
        }
      }
    }
    ostringstream sslab;
    sslab << " fit:(" << xmin0 << "," << xmax0 << ")";
    m_labels[varName] += sslab.str();
  }
  return 0;
}

//**********************************************************************

Index AdcCalibGraphs::resGraphs(Index icha1, Index ncha) {
  Name myname = "AdcCalibGraphs::fitGraphs: ";
  Name calUnit = "e";
  double qpulser = 21400;
  for ( Name varName : m_resNames ) {
    Name gresName = "gres" + varName;
    Name fresName = "fres" + varName;
    Name calName = "calres" + varName;
    for ( Index icha=icha1; icha<icha1+ncha; ++icha ) {
      if ( graph(fresName, icha) ) {
        cout << myname << "Residual graph " << fresName << " for channel " << icha
             << " already exists." << endl;
        continue;
      }
      GraphPtr pgfit = graph(varName, icha);
      if ( ! pgfit ) {
        cout << myname << "Graph " << varName << " not found for channel " << icha << endl;
        continue;
      }
      Name fnam = m_fits[varName][0]->GetName();
      TF1* pfit = pgfit->GetFunction(fnam.c_str());
      if ( pfit == nullptr ) {
        cout << myname << "Graph " << varName << " for channel " << icha 
             << " does not have a fit." << endl;
        //continue;
      }
      string slpName = "Slope";
      double slope = 0.0;
      if ( pfit != nullptr ) {
        int iparSlope = pfit->GetParameter(slpName.c_str());
        if ( iparSlope < 0 ) {
          cout << myname << "WARNING: Unable to find fit parameter " << slpName
               << " for variable " << varName << endl;
        } else {
          slope = pfit->GetParameter(iparSlope);
        }
      }
      double calfac = slope == 0.0 ? 0.0 : qpulser/slope;
      int imrk = pgfit->GetMarkerStyle();
      GraphPtr pggres;
      GraphPtr pgcal;
      GraphPtr pgfres(new TGraphErrors);
      pgfres->SetMarkerStyle(imrk);
      Name sttl0 = pgfit->GetTitle();
      Name::size_type ipos = sttl0.find("channel");
      if ( ipos != Name::npos ) {
        Name sttl = sttl0;
        sttl.insert(ipos, "full residual ");
        pgfres->SetTitle(sttl.c_str());
      } else {
        cout << myname << "Graph " << varName << " for channel " << icha 
             << "does not have \"channel\" in its title: " << sttl0 << endl;
      }
      Name sttlx = pgfit->GetXaxis()->GetTitle();
      Name sttly = "#Delta";
      sttly += pgfit->GetYaxis()->GetTitle();
      pgfres->GetXaxis()->SetTitle(sttlx.c_str());
      pgfres->GetYaxis()->SetTitle(sttly.c_str());
      if ( m_doGainResiduals ) {
        GraphPtr pggres(new TGraphErrors);
        pggres->SetMarkerStyle(imrk);
        GraphPtr pgcal(new TGraphErrors);
        pgcal->SetMarkerStyle(imrk);
        if ( ipos != Name::npos ) {
          Name sttl = sttl0;
          sttl.insert(ipos, "gain residual ");
          pggres->SetTitle(sttl.c_str());
          sttl.insert(ipos, "calibrated ");
          pgcal->SetTitle(sttl.c_str());
        }
        pggres->GetXaxis()->SetTitle(sttlx.c_str());
        pgcal->GetXaxis()->SetTitle(sttlx.c_str());
        pggres->GetYaxis()->SetTitle(sttly.c_str());
        sttly = "#Delta" + varName + " [e]";
        pgcal->GetYaxis()->SetTitle(sttly.c_str());
      }
      // Loop over primary and extra datasets.
      NameVector dstNames;
      for ( const GraphMapMapMap::value_type& igmm : m_extraGras ) dstNames.push_back(igmm.first);
      dstNames.push_back("");
      for ( Name dstName : dstNames ) {
        GraphPtr pgdst = graph(varName, icha, dstName);
        if ( ! pgdst ) continue;
        int npt = pgdst->GetN();
        const double* xfs = pgdst->GetX();
        const double* yfs = pgdst->GetY();
        const double* efs = pgdst->GetEY();
        GraphPtr pgfresFill(dynamic_cast<Graph*>(pgfres->Clone()));
        GraphPtr pggresFill;
        GraphPtr pgcalFill;
        std::vector<GraphPtr> pgs(1, pgfresFill);
        if ( m_doGainResiduals ) {
          pggresFill.reset(dynamic_cast<Graph*>(pggres->Clone()));
          pgcalFill.reset(dynamic_cast<Graph*>(pgcal->Clone()));
          pgs.push_back(pggresFill);
          pgs.push_back(pgcalFill);
        }
        for ( GraphPtr pg : pgs ) {
          pg->SetLineColor(pgdst->GetLineColor());
          pg->SetMarkerColor(pgdst->GetMarkerColor());
          pg->SetMarkerStyle(pgdst->GetMarkerStyle());
        }
        for ( int ipt=0; ipt<npt; ++ipt ) {
          double xdat = xfs[ipt];
          double ydat = yfs[ipt];
          double edat = efs[ipt];
          double ygcal = slope*xdat;
          double eres = edat;
          double ecalres = calfac*eres;
          if ( m_doGainResiduals ) {
            double ygres = ydat - ygcal;
            double ycalres = calfac*ygres;
            pggresFill->SetPoint(ipt, xdat, ygres);
            pggresFill->SetPointError(ipt, 0.0, eres);
            pgcalFill->SetPoint(ipt, xdat, ycalres);
            pgcalFill->SetPointError(ipt, 0.0, ecalres);
          }
          double yfcal = pfit == nullptr ? 0.0 : pfit->Eval(xdat);
          double yfres = ydat - yfcal;
          pgfresFill->SetPoint(ipt, xdat, yfres);
          pgfresFill->SetPointError(ipt, 0.0, eres);
        }
        for ( const GraphPtr& pg : pgs ) {
          pg->GetXaxis()->SetRangeUser(m_xgmin, m_xgmax);
        }
        GraphMapMap& gras = dstName.size() ? m_extraGras[dstName] : m_gras;
        gras[fresName][icha] = pgfresFill;
        if ( m_doGainResiduals ) {
          gras[gresName][icha] = pggresFill;
          gras[calName][icha] = pgcalFill;
        }
      }
    }
  }
  return 0;
}
    
//**********************************************************************

PadPtr AdcCalibGraphs::pad(Name graName, Index icha, Index nx, Index ny) {
  Name myname = "AdcCalibGraphs::pad: ";
  Name varName = graName;
  bool isCalres = graName.substr(0, 6) == "calres";
  bool isRes = graName.substr(1, 3) == "res";
  if ( isCalres ) varName = graName.substr(6);
  if ( isRes ) varName = graName.substr(4);
  PadPtr& ppad = m_pads[graName][icha];
  if ( ppad ) return ppad;
  GraphPtr pgraPad = graph(graName, icha);
  if ( ! pgraPad ) {
    cout << myname << "Graph " << graName << " not found for channel " << icha << endl;
    return ppad;
  }
  GraphPtr pgraFit = graph(varName, icha);
  NameVector labs;
  NameVector labs2;
  float chisq = 0.0;
  if ( pgraFit->GetListOfFunctions()->GetEntries() ) {
    ostringstream sslab;
    sslab.setf(std::ios_base::fixed);
    float chisq = 0.0;
    bool isPulserFit = pgraFit->GetFunction("adcpulser") != nullptr;
    Name fitName = isPulserFit ? "adcpulser" : varName + "Fit";
    if ( varName == "Shaping" ) {
      TF1* pfit = pgraFit->GetFunction("shapingFitMinus");
      bool haveMinus = pfit != nullptr;
      if ( haveMinus ) {
        float shap = pfit->GetParameter("Pedestal");
        sslab.precision(2);
        sslab << "Minus shaping time: " << shap << " tick";
        labs.push_back(sslab.str());
        chisq = pfit->GetChisquare();
        sslab.str("");
        sslab << "Chi-square: " << chisq;
        labs.push_back(sslab.str());
        int ndof = pfit->GetNDF();
        if ( ndof > 0 ) {
          sslab.str("");
          sslab << "CS/DoF: " << chisq/ndof;
          labs.push_back(sslab.str());
        }
      }
      pfit = pgraFit->GetFunction("shapingFitPlus");
      if ( pfit != nullptr ) {
        NameVector& labsp = haveMinus ? labs2 : labs;
        float shap = pfit->GetParameter("Pedestal");
        sslab.str("");
        sslab.precision(2);
        sslab << "Plus shaping time: " << shap << " tick";
        labsp.push_back(sslab.str());
        chisq = pfit->GetChisquare();
        sslab.str("");
        sslab << "Chi-square: " << chisq;
        labsp.push_back(sslab.str());
        int ndof = pfit->GetNDF();
        if ( ndof > 0 ) {
          sslab.str("");
          sslab << "CS/DoF: " << chisq/ndof;
          labsp.push_back(sslab.str());
        }
      }
    } else {
      NameVector parNamesOld = {"Slope", "Offset", "Pedestal", "NegScale"};
      FloatVector parPrecsOld = {2, 3, 1, 3};
      NameVector parNamesPul = {"AdcScale", "Pedestal", "NegScale", "R0", "R1", "R2", "R3", "R4", "R5", "R6", "QVscale"};
      FloatVector parPrecsPul(20, 4);
      parPrecsPul[0] = 1;
      parPrecsPul[1] = 2;
      parPrecsPul[2] = 3;
      NameVector parNames = isPulserFit ? parNamesPul : parNamesOld;
      TF1* pfit = pgraFit->GetFunction(fitName.c_str());
      FloatVector parPrecs = isPulserFit ? parPrecsPul : parPrecsOld;
      if ( pfit == nullptr ) {
        cout << myname << "Fit " << fitName << " not found for graph " << graName << endl;
      } else {
        for ( Index ipar=0; ipar<parNames.size(); ++ipar ) {
          Name parName = parNames[ipar];
          float prec = parPrecs[ipar];
          int kpar = pfit->GetParNumber(parName.c_str());
          if ( kpar < 0 ) continue;
          float val = pfit->GetParameter(kpar);
          float err = pfit->GetParError(kpar);
          sslab.precision(prec);
          sslab.str("");
          sslab << parName << ": " << val;
          if ( err > 0.0 ) sslab << " #pm " << err;
          labs.push_back(sslab.str());
        }
        chisq = pfit->GetChisquare();
        int ndig = log10(chisq/2.0);
        int prec = ndig < 3 ? 3 - ndig : 0;
        sslab.precision(prec);
        sslab.str("");
        sslab << "Chi-square: " << chisq;
        labs.push_back(sslab.str());
        int ndof = pfit->GetNDF();
        if ( ndof > 0 ) {
          sslab.str("");
          sslab << "CS/DoF: " << chisq/ndof;
          labs.push_back(sslab.str());
        }
      }
    }
  }
  if ( m_pChannelStatusProvider->IsBad(icha) ) labs.push_back("Bad channel.");
  else if ( m_pChannelStatusProvider->IsNoisy(icha) ) labs.push_back("Noisy channel.");
  ppad.reset(new Pad(nx, ny));
  ppad->add(pgraPad.get(), "P");
  float xleg1 = 0.14;
  float xleg2 = 0.45;
  float yleg1 = 0.13;
  float yleg2 = yleg1 + 0.046*m_extraGras.size();
  TLegend* pleg = ppad->addLegend(xleg1, yleg1, xleg2, yleg2);
  pleg->SetMargin(0.15);   // Width allocated for the symbol
  Index nextra = 0;
  for ( Name dstName : m_extraDsts ) {
    GraphPtr pgdst = graph(graName, icha, dstName);
    if ( pgdst ) {
      ppad->add(pgdst.get(), "P");
      pleg->AddEntry(ppad->objects().back().get(), dstName.c_str(), "p");
      ++nextra;
    }
  }
  if ( nextra ) ppad->add(pgraPad.get(), "P");
  ppad->addAxis();
  ppad->setGrid();
  ppad->setMarginLeft(0.120);
  double xlab = 0.16;
  double ylab = 0.85;
  double dylab = 0.045;
  double xlab2 = 0.60;
  for ( Name lab : labs ) {
    TLatex* pptl = nullptr;
    pptl = new TLatex(xlab, ylab, lab.c_str());
    pptl->SetNDC();
    pptl->SetTextFont(42);
    pptl->SetTextSize(0.85*dylab);
    ppad->add(pptl);
    ylab -= dylab;
  }
  xlab = 0.50;
  ylab = 0.85;
  for ( Name lab : labs2 ) {
    TLatex* pptl = nullptr;
    pptl = new TLatex(xlab, ylab, lab.c_str());
    pptl->SetNDC();
    pptl->SetTextFont(42);
    pptl->SetTextSize(0.85*dylab);
    ppad->add(pptl);
    ylab -= dylab;
  }
  if ( graName == "Height" ) ppad->setRangeY(-1500, 3500);
  if ( graName == "Area" ) ppad->setRangeY(-10000, 25000);
  if ( graName == "Shaping" ) ppad->setRangeY(4.0, 4.8);
  if ( graName.substr(1) == "resHeight" ) {
    ppad->setRangeY(-50, 50);
    ppad->addHorizontalLine();
    ppad->addVerticalLine();
  }
  if ( graName.substr(1) == "resArea" ) {
    ppad->setRangeY(-300, 300);
    ppad->addHorizontalLine();
    ppad->addVerticalLine();
  }
  if ( graName.substr(0, 6) == "calres" ) {
    ppad->setRangeY(-5000, 5000);
    ppad->addHorizontalLine();
    ppad->addVerticalLine();
  }
  // 1% lines.
  if ( graName.substr(1) == "resHeight" || graName.substr(1) == "resArea" ) {
    bool isPulserFit = pgraFit->GetFunction("adcpulser") != nullptr;
    Name funName = isPulserFit ? "adcpulser" : varName + "Fit";
    TF1* pfun = graph(varName, icha)->GetFunction(funName.c_str());
    double slop = 0.0;
    if ( pfun == nullptr ) {
      cout << myname << "Fit " << funName << " not found for graph " << graName << endl;
    } else if ( isPulserFit ) {
      double x = 10.0;
      double y1 = pfun->Eval(0.0);
      double y2 = pfun->Eval(x);
      slop = 0.01*(y2 - y1)/x;
    } else {
      slop = 0.01*pfun->GetParameter("Slope");
    }
    if ( slop > 1.e-6 ) {
      ppad->addSlopedLine(slop, 0.0, 2);
      ppad->addSlopedLine(-slop, 0.0, 2);
    }
  }
  ppad->setLabel(label(graName));
  return ppad;
}

//**********************************************************************

PadPtr AdcCalibGraphs::draw(Name gnam, Index icha) {
  PadPtr ppad = pad(gnam, icha);
  if ( ppad ) ppad->draw();
  return ppad;
}

//**********************************************************************

PadPtr AdcCalibGraphs::
multiChannelPad(Name gnam, Index icha1, Index ncha, Index npadx, Index npady, Index nx, Index ny) {
  PadPtr ppad;
  if ( npadx*npady < ncha ) return ppad;
  ppad.reset(new TPadManipulator(nx, ny, npadx, npady));
  Index ipad = 0;
  for ( Index icha=icha1; icha<icha1+ncha; ++icha, ++ipad ) {
    PadPtr ppadIn = pad(gnam, icha);
    if ( ppadIn ) *(ppad->man(ipad)) = *ppadIn;
  }
  ostringstream ssnam;
  string scha1 = to_string(icha1);
  while ( scha1.size() < 5 ) scha1 = "0" + scha1;
  ssnam << gnam << "_" << scha1 << "_" << ncha;
  Name snam = ssnam.str();
  m_multiPads[snam] = ppad;
  return ppad;
}
 
//**********************************************************************

void AdcCalibGraphs::makeMultiChannelPads(Name gnam, Index npadx, Index npady, Index nx, Index ny) {
  Index npadxy = npadx*npady;
  for ( Index icha1=graphChannelBegin(); icha1<graphChannelEnd(); icha1 += npadxy ) {
    PadPtr ppad = multiChannelPad(gnam, icha1, npadxy, npadx, npady, nx, ny);
    if ( ! ppad ) break;
  }
}

//**********************************************************************

int AdcCalibGraphs::printMultiChannelPads() const {
  Name myname = "AdcCalibGraphs::printMultiChannelPads: ";
  for ( PadNameMap::value_type ipad : m_multiPads ) {
    Name pnam = ipad.first;
    PadPtr ppad = ipad.second;
    Name fnam = pnam + ".png";
    cout << myname << "Printing " << fnam << endl;
    ppad->print(fnam);
  }
  return 0;
}

//**********************************************************************

GraphPtr AdcCalibGraphs::graph(Name gnam, Index icha, Name dstName) {
  if ( dstName.size() ) {
    if ( m_extraGras.find(dstName) == m_extraGras.end() ) {
      return GraphPtr();
    }
  }
  GraphMapMap& gras = dstName.size() ? m_extraGras[dstName] : m_gras;
  GraphMapMap::const_iterator igra = gras.find(gnam);
  if ( igra == gras.end() ) return GraphPtr();
  const GraphMap& gmap = igra->second;
  GraphMap::const_iterator igma = gmap.find(icha);
  if ( igma == gmap.end() ) return GraphPtr();
  return igma->second;
}

//**********************************************************************

GraphPtr AdcCalibGraphs::channelSummaryGraph(Name varName, Name parName, Name fitName0, Name* pgraName) {
  Name myname = "AdcCalibGraphs::channelSummaryGraph: ";
  if ( varName == "Ratio" ) {
    if ( pgraName != nullptr ) *pgraName = "Ratio_" + parName;
    return channelRatioGraph(parName);
  }
  Name fitName = fitName0.size() ? fitName0 : varName + "Fit";
  Name graName = fitName + "_" + parName;
  if ( pgraName != nullptr ) *pgraName = graName;
  GraphNameMap::const_iterator igm = m_chsgras.find(graName);
  if ( igm != m_chsgras.end() ) return igm->second;
  GraphPtr pgraSum;
  GraphPtr pgraSumBad;
  GraphPtr pgraSumNoisy;
  GraphMapMap::const_iterator igmm = m_gras.find(varName);
  if ( igmm == m_gras.end() ) {
    cout << myname << "Variable " << varName << " not found." << endl;
    return pgraSum;
  }
  float xmin = 0.0;
  float xmax = 0.0;
  int ipt = 0;
  const GraphMap& grfs = igmm->second;
  for ( GraphMap::value_type igrf : grfs ) {
    int icha = igrf.first;
    bool isBad = m_pChannelStatusProvider->IsBad(icha);
    bool isNoisy = m_pChannelStatusProvider->IsNoisy(icha);
    float xval = icha;
    if ( ! pgraSum ) {
      pgraSum.reset(new TGraphErrors);
      pgraSumBad.reset(new TGraphErrors);
      pgraSumNoisy.reset(new TGraphErrors);
      pgraSumBad->SetMarkerStyle(24);
      pgraSumNoisy->SetMarkerStyle(26);
      string sttl = "Channel summary for " + varName + " " + parName;
      pgraSum->GetXaxis()->SetTitle("Channel");
      pgraSum->SetTitle(sttl.c_str());
      pgraSum->GetYaxis()->SetTitle(parName.c_str());
      //pgraSum->SetMarkerStyle(2);
      xmin = xval - 0.5;
    }
    xmax = xval + 0.5;
    float yval = 0.0;
    float eval = 0.0;
    GraphPtr& pgra = igrf.second;
    TF1* pfit = pgra->GetFunction(fitName.c_str());
    if ( pfit == nullptr ) {
      cout << myname << "WARNING: Variable " << varName << " channel " << icha
           << " is not fit with function " << fitName << "." << endl;
    } else {
      int ipar = pfit->GetParNumber(parName.c_str());
      if ( ipar < 0 ) {
        cout << myname << "WARNING: Variable " << varName << " channel " << icha << " function "
             << fitName << " does not have parameter " << parName << "." << endl;
      } else {
        yval = pfit->GetParameter(ipar);
        eval = pfit->GetParError(ipar);
      }
    }
    pgraSum->SetPoint(ipt, icha, yval);
    pgraSum->SetPointError(ipt, 0.25, eval);
    if ( isBad ) pgraSumBad->SetPoint(pgraSumBad->GetN(), icha, yval);
    if ( isNoisy ) pgraSumNoisy->SetPoint(pgraSumBad->GetN(), icha, yval);
    ++ipt;
  }
  if ( ! pgraSum ) {
    cout << myname << "No entries found for graph " << graName << "." <<  endl;
    return pgraSum;
  }
  if ( xmax > xmin ) pgraSum->GetXaxis()->SetRangeUser(xmin, xmax);
  m_chsgras[graName] = pgraSum;
  m_chsgrasBad[graName] = pgraSumBad;
  m_chsgrasNoisy[graName] = pgraSumNoisy;
  return pgraSum;
}
    
//**********************************************************************

PadPtr AdcCalibGraphs::
channelSummaryPad(Name varName, Name parName, Name fitName0,
                  double ymin, double ymax, Index xw, Index yw) {
  Name myname = "AdcCalibGraphs::channelSummaryGraph: ";
  Name fitName = fitName0.size() ? fitName0 : varName + "Fit";
  if ( varName == "Ratio" ) fitName = "Ratio";
  Name padName = fitName + "_" + parName;
  PadNameMap::const_iterator ipad = m_chspads.find(padName);
  if ( ipad != m_chspads.end() ) return ipad->second;
  PadPtr ppad;
  Name graName;
  GraphPtr pgra = channelSummaryGraph(varName, parName, fitName, &graName);
  if ( ! pgra ) {
    cout << "Unable to find graph " << graName << endl;
    return ppad;
  }
  GraphPtr pgraBad = m_chsgrasBad[graName];
  if ( ! pgraBad ) {
    cout << "Unable to find bad graph " << graName << endl;
    return ppad;
  }
  GraphPtr pgraNoisy = m_chsgrasNoisy[graName];
  if ( ! pgraNoisy ) {
    cout << "Unable to find noisy graph " << graName << endl;
    return ppad;
  }
  double x0 = pgra->GetN() ? pgra->GetX()[0] : 0.0;
  ppad.reset(new TPadManipulator(xw, yw));
  ppad->add(pgra.get(), "P");
  if ( pgraNoisy->GetN() ) ppad->add(pgraNoisy.get(), "P");
  if ( pgraBad->GetN() ) ppad->add(pgraBad.get(), "P");
  ppad->showGraphOverflow("BTLR", 41);
  ppad->setGridY();
  ppad->addAxis();
  ppad->addVerticalModLines(48, x0 - 0.5);
  ppad->addHorizontalLine(0.0);
  ppad->setLabel(label(varName));
  if ( ymax > ymin ) ppad->setRangeY(ymin, ymax);
  m_chspads[graName] = ppad;
  return ppad;
}

//**********************************************************************

PadPtr AdcCalibGraphs::
drawChannelSummaryPad(Name varName, Name parName, Name fitName0,
                      double ymin, double ymax, Index xw, Index yw) {
  Name myname = "AdcCalibGraphs::channelSummaryGraph: ";
  PadPtr ppad = channelSummaryPad(varName, parName, fitName0, ymin, ymax, xw, yw);
  if ( ppad ) ppad->draw();
  return ppad;
}

//**********************************************************************

GraphPtr AdcCalibGraphs::channelRatioGraph(Name ratName) {
  Name myname = "AdcCalibGraphs::channelRatioGraph: ";
  Name graName = "Ratio_" + ratName;
  if ( m_chsgras.find(graName) != m_chsgras.end() ) return m_chsgras[graName];
  GraphPtr pgraRat;
  GraphPtr pgraRatBad;
  GraphPtr pgraRatNoisy;
  int npt = 0;
  if ( ratName == "AHS" ) {
    Name graNameArea;
    GraphPtr pgraArea = channelSummaryGraph("Area", "Slope");
    npt = pgraArea->GetN();
    GraphPtr pgraHeig = channelSummaryGraph("Height", "Slope");
    GraphPtr pgraShap = channelSummaryGraph("Shaping", "Pedestal", "shapingFitPlus");
    pgraRat.reset(new TGraphErrors);
    pgraRatBad.reset(new TGraphErrors);
    pgraRatBad->SetMarkerStyle(24);
    pgraRatNoisy.reset(new TGraphErrors);
    pgraRatNoisy->SetMarkerStyle(26);
    for ( int ipt=0; ipt<npt; ++ipt ) {
      double xa = pgraArea->GetX()[ipt];
      double xh = pgraHeig->GetX()[ipt];
      double xs = pgraShap->GetX()[ipt];
      Index icha = Index(xa + 0.1);
      Index ichaHeig = Index(xh + 0.1);
      Index ichaShap = Index(xs + 0.1);
      if ( ichaHeig != icha ) {
        cout << myname << "Area/Height channel mismatch: " << icha << " != " << ichaHeig << endl;
        break;
      }
      if ( ichaShap != icha ) {
        cout << myname << "Area/Shaping channel mismatch: " << icha << " != " << ichaShap << endl;
        break;
      }
      double area = pgraArea->GetY()[ipt];
      double heig = pgraHeig->GetY()[ipt];
      double shap = pgraShap->GetY()[ipt];
      double darea = pgraArea->GetEY()[ipt];
      double dheig = pgraHeig->GetEY()[ipt];
      double dshap = pgraShap->GetEY()[ipt];
      double hs = heig*shap;
      double rat = 0;
      double drat = 0;
      if ( hs > 0.0 && area > 0.0 ) {
        rat = area/hs;
        double fda = darea/area;
        double fdh = dheig/heig;
        double fds = dshap/shap;
        double fdr = sqrt(fda*fda + fdh*fdh + fds*fds);
        drat = fdr*rat;
      }
      pgraRat->SetPoint(ipt, xa, rat);
      pgraRat->SetPointError(ipt, 0.25, drat);
      if ( m_pChannelStatusProvider->IsBad(icha) ) pgraRatBad->SetPoint(pgraRatBad->GetN(), xa, rat);
      if ( m_pChannelStatusProvider->IsNoisy(icha) ) pgraRatNoisy->SetPoint(pgraRatNoisy->GetN(), xa, rat);
    }
  } else {
    cout << myname << "Invalid ratio :" << ratName << endl;
    return GraphPtr();
  }
  if ( npt ) {
    double xmin = pgraRat->GetX()[0] - 0.5;
    double xmax = pgraRat->GetX()[npt-1] + 0.5;
    pgraRat->GetXaxis()->SetRangeUser(xmin, xmax);
  }
  m_chsgras[graName] = pgraRat;
  m_chsgrasBad[graName] = pgraRatBad;
  m_chsgrasNoisy[graName] = pgraRatNoisy;
  return pgraRat;
}

//**********************************************************************

Name AdcCalibGraphs::variableName(Name graName) const {
  if ( graName.substr(0,4) == "gres" ) return graName.substr(4);
  if ( graName.substr(0,4) == "fres" ) return graName.substr(4);
  if ( graName.substr(0,6) == "calres" ) return graName.substr(6);
  return graName;
}

//**********************************************************************

Name AdcCalibGraphs::label(Name graName) const {
  Name myname = "AdcCalibGraphs::label: ";
  Name varName = variableName(graName);
  NameMap::const_iterator ilab = m_labels.find(varName);
  if ( ilab == m_labels.end() ) {
    cout << myname << "No label found for variable " << varName << endl;
    return "VariableNotFound";
  }
  return ilab->second;
}

//**********************************************************************

int AdcCalibGraphs::writeFcl(Name calName, Name filNameIn) {
  Name myname = "AdcCalibGraphs::writeFcl: ";
  Name gainUnit = chargeUnit() + "/(ADC-count)/tick";
  //const double qstepFC = 0.01875 * 183.0;
  const double qstepFC = 3.448;   // Nominal resistors
  const double qstepE = 6241.509*qstepFC;
  double gainFac = 0.0;
  if ( chargeUnit() == "Q_{step}" ) gainFac = 1.0;
  else if ( chargeUnit() == "fC" ) gainFac = qstepFC;
  else if ( chargeUnit() == "e" ) gainFac = qstepE;
  else if ( chargeUnit() == "ke" ) gainFac = 0.001*qstepE;
  else {
    cout << myname << "Invalid charge unit: " << chargeUnit() << endl;
    return 1;
  }
  bool doGain = false;
  Name varName = calName;
  Name sumPar;
  Name fitName;
  if ( calName == "AreaGain" ) {
    doGain = true;
    varName = "Area";
    sumPar = "Slope";
  } else if ( calName == "HeightGain" ) {
    doGain = true;
    varName = "Height";
    sumPar = "Slope";
  } else if ( calName == "Shaping" ) {
    sumPar = "Pedestal";
    fitName = "shapingFitPlus";
  } else {
    cout << myname << "Invalid calibration variable: " << calName << endl;
    return 1;
  }
  GraphPtr pgsum = channelSummaryGraph(varName, sumPar, fitName);
  if ( ! pgsum ) {
    cout << myname << "Summary graph not found." << endl;
    return 2;
  }
  Index ncha = pgsum->GetN();
  if ( ncha == 0 ) {
    cout << myname << "Summary graph is empty." << endl;
    return 3;
  }
  const double* xvals = pgsum->GetX();
  const double* yvals = pgsum->GetY();
  const double* yerrs = pgsum->GetEY();
  Name slab = calName + "_" + label(varName) + "$"; 
  StringManipulator sman (slab);
  sman.replace(":(", "-");
  sman.replace(")$", "");
  sman.replace("$", "");
  sman.replace("(", "-");
  sman.replace(")", "-");
  sman.replace(",", "-");
  sman.replace(".", "p");
  slab = sman.string();
  //for ( Index ich=0; ich<slab.size(); ++ich ) if ( slab[ich] == ' ' ) slab[ich] = '_';
  for ( char& ch : slab ) if ( ch == ' ' ) ch = '_';
  string filName = filNameIn;
  if ( filName.size() == 0 ) filName = slab + ".fcl";
  ofstream fout(filName);
  fout << "tool_type: FclFloatArray";
  fout << "\nLogLevel: 1";
  fout << "\nDefaultValue: 0";
  fout << "\nLabel: \"" << slab << "\"";
  fout << "\nUnit: \"" << (doGain ? gainUnit : "Tick") << "\"";
  fout.setf(ios::fixed);
  int prec = doGain ? 7 : 4;
  if ( doGain && chargeUnit() == "e" ) prec = 4;
  fout.precision(prec);
  Index npt = pgsum->GetN();
  for ( Index ipt=0; ipt<npt; ++ipt ) {
    Index icha = xvals[ipt] + 0.01;
    float val = yvals[ipt];
    if ( doGain ) {
      double slop = val;
      double slopErr = yerrs[ipt];
      if ( slop <= 0.0 || slopErr/slop > 0.30 ) {
        slop = 0.0;
        slopErr = 0.0;
      }
      double gain = 0.0;
      double gainErr = 0.0;
      if ( slop != 0.0 ) {
        gain = gainFac/slop;
        gainErr = gain*slopErr/slop;
      }
      val = gain;
    }
    if ( ipt == 0 ) {
      fout << "\nOffset: " << icha;
      fout << "\nValues: [";
    }
    if ( ipt ) fout << ",";
    if ( 10*(ipt/10) == ipt ) fout << "\n ";
    fout << setw(10) << val;
  }
  fout << "\n]" << endl;
  cout << myname << "Wrote " << filName << endl;
  return 0;
}

//**********************************************************************
