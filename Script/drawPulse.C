int drawPulse(int shap =2, float height =275, float t0 =0.0) {
  double shapTick = 2.13*shap;
  TF1* pf = coldelecResponseTF1(height, shapTick, t0);
  int ntick = 3*(shapTick+0.99);
  string sttl = "CE pulse #tau = " + std::to_string(shap) + " #mus; Tick; Signal";
  TH1* ph = new TH1F("hax", sttl.c_str(), ntick, 0, ntick);
  ph->SetStats(0);
  TCanvas* pcan = new TCanvas;
  pcan->SetGridx();
  pcan->SetGridy();
  pcan->SetTickx();
  pcan->SetTicky();
  ph->Draw();
  ph->SetMaximum(1.03*height);
  if ( 0 ) {
    ph->SetMinimum(-0.003*height);
    TLine* plin = new TLine(0, 0, ntick, 0);
    plin->Draw();
  }
  pf->Draw("same");
  if ( 1 ) {
    TGraph* pg = new TGraph();
    for ( int itck=0; itck<ntick; ++itck ) {
      double x = itck;
      double y = pf->Eval(x);
      pg->SetPoint(itck, x,y);
    }
    pg->SetMarkerStyle(4);
    pg->Draw("same P");
  }
  return 0;
}
