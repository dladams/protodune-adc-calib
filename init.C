// init.C
//
// David Adams
// June 2018
//
// Root initialization file for protodune-adc-calib.
//
//   pkgDir - Directory where this package resides, i.e. where to find the macros.
//   buildDir - Directory for building and installing
//
// The latter may (and probably should) include "$DUNETPC_VERSION" which is
// replaced with that vlaue of that env variable.

int init(string pkgDir =".", string buildDirIn =".aclic/$DUNETPC_VERSION") {
  const string myname = "init: ";
  cout << "Welcome to the protoDUNE ADC calibration package." << endl;

  // Check that the package directory exists.
  if ( pkgDir.size() && gSystem->AccessPathName(pkgDir.c_str()) ) {
    cout << myname << "Unable to find package directory: " << pkgDir << endl;
    return 1;
  }

  string buildDir = buildDirIn;
  if ( buildDir.size() == 0 ) {
    cout << myname << "Build directory not specified--using package directory." << endl;
    buildDir = pkgDir + "/.aclic/$DUNETPC_VERSION";
  }
  string::size_type ipos = buildDir.find("$DUNETPC_VERSION");
  if ( ipos != string::npos ) {
    const char* pch = gSystem->Getenv("DUNETPC_VERSION");
    if ( pch == nullptr ) {
      cout << myname << "DUNETPC_VERSION must be defined" << endl;
      cout << myname << "Please setup dunetpc" << endl;
      return 2;
    }
    buildDir.replace(ipos, 16, pch);
  }
  cout << "ACLiC build dir: " << buildDir << endl;
  string utilLibDir = buildDir + "/lib";
  string toolLibDir = utilLibDir;
  cout << "Util library dir: " << utilLibDir << endl;
  cout << "Tool library dir: " << toolLibDir << endl;
  gSystem->SetBuildDir(buildDir.c_str());
  string dtinc = gSystem->Getenv("DUNETPC_INC");
  cout << "Dunetpc include dir: " << dtinc << endl;
  string sarg = "-I" + dtinc;
  string drinc = gSystem->Getenv("DUNE_RAW_DATA_INC");
  cout << "Dune_raw data include dir: " << drinc << endl;
  sarg = "-I" + drinc;
  gSystem->AddIncludePath(sarg.c_str());

  // Append this package directory to the macro search path.
  // Needed if this script is run from a different directory.
  string oldMacroPath = TROOT::GetMacroPath();
  string newMacroPath = ".:" + pkgDir;
  if ( oldMacroPath.size() ) newMacroPath += ":" + oldMacroPath;
  gROOT->SetMacroPath(newMacroPath.c_str());

  // Load the dunetpc and other supporting libraries.
  vector<string> libs;
  libs.push_back("$FHICLCPP_LIB/libfhiclcpp");
  libs.push_back("$DUNETPC_LIB/libdune_ArtSupport");
  libs.push_back("$DUNETPC_LIB/libdune_DuneServiceAccess");
  libs.push_back("$DUNETPC_LIB/libdune_DuneCommon");
  libs.push_back("$DUNETPC_LIB/libdune_Geometry");
  libs.push_back("$DUNETPC_LIB/libdune_DataPrep_Utility");
  string libext = "so";
  string arch = gSystem->GetBuildArch();
  if ( arch.substr(0,3) == "mac" ) libext = "dylib";
  for ( string lib : libs ) {
    string libpath = lib + "." + libext;
    string libres = gSystem->ExpandPathName(libpath.c_str());
    if ( 1 ) cout << "AddLinkedLibs: " << libres << endl;
    gSystem->AddLinkedLibs(libres.c_str());
  }

  // Load the dunetpc classes we would like available on the command line.
  cout << "Loading dunetpc classes." << endl;
  gROOT->ProcessLine(".L $ART_INC/art/Framework/Services/Registry/ServiceHandle.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/DuneToolManager.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/ArtServiceHelper.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/TPadManipulator.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/RootPalette.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/AdcChannelData.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/IndexRangeTool.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/IndexMapTool.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/AdcChannelTool.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/ArtServiceHelper.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneServiceAccess/DuneServiceAccess.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/coldelecResponse.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/offsetLine.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/gausTF1.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/Geometry/ChannelGeo.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/Utilities/SignalShapingServiceDUNE.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/DataPrep/Utility/TickModTreeData.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DataPrep/Utility/ParabolicInterpolator.h+");
  gROOT->ProcessLine(".L $DUNE_RAW_DATA_INC/dune-raw-data/Services/ChannelMap/PdspChannelMapService.h+");
  gROOT->ProcessLine(".L $LARCORE_INC/larcore/Geometry/Geometry.h+");
  gROOT->ProcessLine(".L $LAREVT_INC/larevt/CalibrationDBI/Interface/ChannelStatusService.h+");
  gROOT->ProcessLine(".L $LARDATA_INC/lardata/Utilities/LArFFT.h+");

  // Build scripts to be used only from the Root command line.
  cout << "Loading local libraries." << endl;
  gROOT->ProcessLine(".L Script/drawRois.C");
  gROOT->ProcessLine(".L Script/drawSums.C");
  gROOT->ProcessLine(".L Script/drawChanSum.C");
  gROOT->ProcessLine(".L Script/testAll.C");

  // Build utilities, i.e. classes, functions, etc. whose libraries
  // are installed at buildDir/lib and so visible to clients.
  // The source for utility UNAME is taken from Util/UNAME.cxx and
  // the library is libUNAME.so 
  vector<string> unames = {"AdcCalibData", "AdcCalibGraphs"};
  string oldBuildDir = gSystem->GetBuildDir();
  gSystem->SetBuildDir(utilLibDir.c_str());
  cout << "Building utilities." << endl;
  for ( string uname : unames ) {
    string sname = pkgDir + "/Util/" + uname + ".cxx";
    //string lname = "lib" + uname;   // Root adds the extension.
    //gSystem->CompileMacro(sname.c_str(), "-ck", lname.c_str());
    string line = ".L " + sname + "+";
    gROOT->ProcessLine(line.c_str());
  }
  gSystem->SetBuildDir(oldBuildDir.c_str());
 
  // Build local tools.
  // These are art tools--configured with fcl and discovered automatically.
  // The source for tool TNAME is taken from Tool/TNAME_tool.cc.
  vector<string> tnames = { };
  cout << "Building tools." << endl;
  oldBuildDir = gSystem->GetBuildDir();
  gSystem->SetBuildDir(toolLibDir.c_str());
  for ( string tname : tnames ) {
    string sname = "Tool/" + tname + "_tool.cc";
    string lname = "lib" + tname + "_tool";   // Root adds the extension.
    gSystem->CompileMacro(sname.c_str(), "-ck", lname.c_str());
  }
  gSystem->SetBuildDir(oldBuildDir.c_str());

  string sfile = "startup.C";
  if ( gSystem->AccessPathName(sfile.c_str()) == 0 ){
    cout << "Executing local startup: " << sfile << endl;
    string sline = ".X " + sfile;
    gROOT->ProcessLine(sline.c_str());
  } else {
    cout << "Local startup file not found: " << sfile << endl;
  }

  cout << "Finished loading." << endl;
  return 0;
}
