int drawRois(int run=1193, double ymax =0.0, int evt=4, int icha=500, int nroi=8, int iroiBegin =0) {
  string myname = "drawRois: ";
  int ncan = (nroi+1)/8;
  ostringstream ssout;
  ssout << "../calib00" << run << "/roiroipos.root";
  string sfp = ssout.str();
  ssout.str("");
  ssout << "../calib00" << run << "/roiroineg.root";
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
  vector<int> ipads = {0, 1, 2, 3, 8, 9, 10, 11};
  int iroiEnd = iroiBegin + nroi;
  int iroi = iroiBegin;
  bool done = iroi >= iroiEnd;
  while ( ! done ) {
    if ( done ) break;
    TPadManipulator man(1500, 1000, 4, 4);
    //TCanvas* pcan = new TCanvas;
    //pcan->SetWindowSize(1500, 1000);
    //pcan->Divide(4,4);
    int iroi1 = -1;
    int iroi2 = -1;
    int npad = 0;
    for ( int ipad : ipads ) {
      ssout.str("");
      ssout << "hroi_evt" << evt << "_chan" << icha << "_roi";
      if ( iroi < 100 ) ssout << "0";
      if ( iroi < 10 ) ssout << "0";
      ssout << iroi;
      string hnam = ssout.str();
      TH1* php = dynamic_cast<TH1*>(pfp->Get(hnam.c_str()));
      if ( php == nullptr ) {
        cout << myname << "Unable to find histogram " << hnam << endl;
        done = true;
        break;
      } else {
        //pcan->cd(ipad);
        if ( ymax > 0.0 ) php->GetYaxis()->SetRangeUser(-ymax, ymax);
        //php->DrawCopy();
        man.add(ipad, php);
        if ( npad == 0 ) iroi1 = iroi;
        iroi2 = iroi;
        ++npad;
      }
      TH1* phn = dynamic_cast<TH1*>(pfn->Get(hnam.c_str()));
      if ( phn == nullptr ) {
        cout << myname << "Unable to find histogram " << hnam << endl;
        done = true;
        break;
      } else {
        //pcan->cd(ipad+4);
        if ( ymax > 0.0 ) phn->GetYaxis()->SetRangeUser(-ymax, ymax);
        man.add(ipad+4, phn);
        //phn->DrawCopy();
        if ( npad == 0 ) iroi1 = iroi;
        iroi2 = iroi;
        ++npad;
      }
      ++iroi;
    }
    if ( npad > 0 ) {
      ssout.str("");
      ssout << "hroi_run" << run << "_evt" << evt << "_chan" << icha << "_roi" << iroi1 << "-" << iroi2 << ".png";
      string fno = ssout.str();
      man.print(fno);
      //pcan->Print(fno.c_str());
    }
  }
  return 0;
}

