// rootlogon.C

{
  if ( gSystem->AccessPathName("pkginit.C") ) {
    cout << "Unable to find pkginit.C" << endl;
    cout << "Create with . setup.sh" << endl;
  }
  gROOT->ProcessLine(".x pkginit.C");
}
