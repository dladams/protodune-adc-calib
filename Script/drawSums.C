int drawSums(int run=1193, int icha=500) {
  string myname = "drawSums: ";
  ostringstream ssout;
  ssout << "../calib00" << run << "/roisumpos.root";
  string sfp = ssout.str();
  ssout.str("");
  ssout << "../calib00" << run << "/roisumneg.root";
  string sfn = ssout.str();
  TFile* pfp = TFile::Open(sfp.c_str(), "READ");
  if ( ! pfp->IsOpen() ) {
    cout << myname << "Unable to open file " << sfp << endl;
    return 1;
  }
  TFile* pfn = TFile::Open(sfn.c_str(), "READ");
  if ( ! pfn->IsOpen() ) {
    cout << myname << "Unable to open file " << sfn << endl;
    return 2;
  }
  TCanvas* pcan1 = new TCanvas;
  pcan1->SetWindowSize(1500, 1000);
  pcan1->Divide(2,2);
  TCanvas* pcan2 = new TCanvas;
  pcan2->SetWindowSize(1500, 1000);
  pcan2->Divide(2,2);
  vector<string> hpres = {"hfhw", "hfww", "hfcsw", "hfcsndw"};
  vector<TCanvas*> pcans = {pcan1, pcan1, pcan2, pcan2};
  vector<int> ipads = {1, 2, 1, 2, 1};
  for ( int ihst=0; ihst<hpres.size(); ++ihst ) {
    ssout.str("");
    ssout << hpres[ihst] << "_ch";
    if ( icha < 10000 ) ssout << "0";
    if ( icha < 1000 ) ssout << "0";
    if ( icha < 100 ) ssout << "0";
    if ( icha < 10 ) ssout << "0";
    ssout << icha;
    string hnam = ssout.str();
    TCanvas* pcan = pcans[ihst];
    int ipad = ipads[ihst];
    TH1* php = dynamic_cast<TH1*>(pfp->Get(hnam.c_str()));
    if ( php == nullptr ) {
      cout << myname << "Unable to find histogram " << hnam << endl;
    } else {
      pcan->cd(ipad);
      php->Draw();
    }
    TH1* phn = dynamic_cast<TH1*>(pfn->Get(hnam.c_str()));
    if ( phn == nullptr ) {
      cout << myname << "Unable to find histogram " << hnam << endl;
    } else {
      pcan->cd(ipad+2);
      phn->Draw();
    }
  }
  pcan1->Update();
  pcan2->Update();
  ssout.str("");
  ssout << "hsum1_run" << run << "_chan" << icha << ".png";
  string fno1 = ssout.str();
  ssout.str("");
  ssout << "hsum2_run" << run << "_chan" << icha << ".png";
  string fno2 = ssout.str();
  ssout.str("");
  ssout << "hsum3_run" << run << "_chan" << icha << ".png";
  string fno3 = ssout.str();
  pcan1->Print(fno1.c_str());
  pcan2->Print(fno2.c_str());
  return 0;
}

