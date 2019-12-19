// postinit.C
//
// David Adams
// November 2019
//
// Root post-initialization file for protodune-adc-calib.
//

int postinit() {
  const string myname = "postinit: ";
  cout << myname << "Welcome to the protoDUNE ADC post-initialization." << endl;

  cout << myname << "Skipping postinit." << endl;
  return 0;

  // Load the dunetpc and other supporting libraries.
  vector<string> libs;
  libs.push_back("$FHICLCPP_LIB/libfhiclcpp");
  //dla libs.push_back("$DUNETPC_LIB/libdune_ArtSupport");
  //dla libs.push_back("$DUNETPC_LIB/libdune_DuneServiceAccess");
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
  //dla gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/DuneToolManager.h+");
  //dla gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/ArtServiceHelper.h+");
  //dict gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/TPadManipulator.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/LineColors.h+");
  //dla gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/RootPalette.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/AdcChannelData.h+");
  //dla gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/IndexRangeTool.h+");
  //dla gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/IndexMapTool.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/AdcChannelTool.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneInterface/Tool/FloatArrayTool.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/ArtSupport/ArtServiceHelper.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneServiceAccess/DuneServiceAccess.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/coldelecResponse.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/cePulser.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/cePulserToAdc.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/offsetLine.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DuneCommon/gausTF1.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/Geometry/ChannelGeo.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/Utilities/SignalShapingServiceDUNE.h+");
  //gROOT->ProcessLine(".L $DUNETPC_INC/dune/DataPrep/Utility/TickModTreeData.h+");
  gROOT->ProcessLine(".L $DUNETPC_INC/dune/DataPrep/Utility/ParabolicInterpolator.h+");
  gROOT->ProcessLine(".L $DUNE_RAW_DATA_INC/dune-raw-data/Services/ChannelMap/PdspChannelMapService.h+");
  gROOT->ProcessLine(".L $LARCORE_INC/larcore/Geometry/Geometry.h+");
  //dla gROOT->ProcessLine(".L $LAREVT_INC/larevt/CalibrationDBI/Interface/ChannelStatusService.h+");
  gROOT->ProcessLine(".L $LARDATA_INC/lardata/Utilities/LArFFT.h+");

  // Build scripts to be used only from the Root command line.
  cout << "Loading local libraries." << endl;
  gROOT->ProcessLine(".L Script/drawRois.C");
  gROOT->ProcessLine(".L Script/drawSums.C");
  gROOT->ProcessLine(".L Script/drawChanSum.C");
  //dla gROOT->ProcessLine(".L Script/testAll.C");

  // Build utilities, i.e. classes, functions, etc. whose libraries
  // are installed at buildDir/lib and so visible to clients.
  // The source for utility UNAME is taken from Util/UNAME.cxx and
  // the library is libUNAME.so 
  vector<string> unames = {"ProcSet", "RoiChansumGraphs",  "AdcCalibData", "AdcCalibGraphs"};
  string oldBuildDir = gSystem->GetBuildDir();
  string utilLibDir = oldBuildDir + "/lib";
  string toolLibDir = utilLibDir;
  gSystem->SetBuildDir(utilLibDir.c_str());
  cout << "Building utilities." << endl;
  for ( string uname : unames ) {
    //dla string sname = pkgDir + "/Util/" + uname + ".cxx";
    string sname = "Util/" + uname + ".cxx";
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

  cout << "Finished postinit" << endl;
  return 0;
}
