int drawChanSum(int runIn=0) {
  string myname = "drawChanSum: ";
  bool doSumPlots = false;
  bool doGainFits =  true;
  int chan1 = 0;
  int chan2 = 800;
  float hchsqMax = 5000.0;   // Pulses with chi-square above this value are not used in fits.
  float rcsqMax = 20.0;      // Limit for plots of chi-square/DOF
  vector<int> runs = {1194, 1195, 1193, 1197, 1192, 1198, 1203};
  vector<int> ipuls = {  1,    2,    3,    4,    5,    6,    7};
  if ( runIn ) {
    for ( unsigned int irun=0; irun<runs.size(); ++irun ) {
      int ipul = 0;
      if ( runs[irun] == runIn ) {
        ipul = ipuls[irun];
        runs.clear();
        runs.push_back(runIn);
        ipuls.clear();
        ipuls.push_back(ipul);
        break;
      }
    }
    if ( runs.size() != 1 ) {
      cout << myname << "Unable to find input run " << runIn << endl;
      return 1;
    }
  }
  vector<TH1*> phhps;
  vector<TH1*> phhns;
  vector<TFile*> pfps;
  vector<TFile*> pfns;
  ostringstream ssout;
  TLatex* ptxt = nullptr;
  TCanvas* pcan = nullptr;
  // Open the files.
  for ( unsigned int irun=0; irun<runs.size(); ++irun ) {
    int run = runs[irun];
    string spn = "pos";
    ssout.str("");
    ssout << "../calib00" << run << "/roichan" << spn << ".root";
    string sfn = ssout.str();
    TFile* pfp = TFile::Open(sfn.c_str(), "READ");
    if ( pfp == nullptr || ! pfp->IsOpen() ) {
      cout << myname << "File not found: " << sfn << endl;
      return 2;
    }
    pfps.push_back(pfp);
    ssout.str("");
    spn = "neg";
    ssout << "../calib00" << run << "/roichan" << spn << ".root";
    sfn = ssout.str();
    TFile* pfn = TFile::Open(sfn.c_str(), "READ");
    if ( pfn == nullptr || ! pfn->IsOpen() ) {
      cout << myname << "File not found: " << sfn << endl;
      return 3;
    }
    pfns.push_back(pfn);
  }
  cout << "Positive ROI file count: " << pfps.size() << endl;
  cout << "Negative ROI file count: " << pfns.size() << endl;
  // Make the channel summary plots.
  if ( doSumPlots ) {
    for ( int irun=0; irun<runs.size(); ++irun ) {
      int run = runs[irun];
      int ipul = ipuls[irun];
      TFile* pfp = pfps[irun];
      TFile* pfn = pfns[irun];
      TH1* phhp = dynamic_cast<TH1*>(pfp->Get("hcsHeight_apa1u"));
      TH1* phwp = dynamic_cast<TH1*>(pfp->Get("hcsShaping_apa1u"));
      TH1* phcp = dynamic_cast<TH1*>(pfp->Get("hcsChiSquare_apa1u"));
      TH1* phrp = dynamic_cast<TH1*>(pfp->Get("hcsCSNormDof_apa1u"));
      TH1* phgp = dynamic_cast<TH1*>(pfp->Get("hcsGain_apa1u"));
      TH1* phhn = dynamic_cast<TH1*>(pfn->Get("hcsHeight_apa1u"));
      TH1* phwn = dynamic_cast<TH1*>(pfn->Get("hcsShaping_apa1u"));
      TH1* phcn = dynamic_cast<TH1*>(pfn->Get("hcsChiSquare_apa1u"));
      TH1* phrn = dynamic_cast<TH1*>(pfn->Get("hcsCSNormDof_apa1u"));
      TH1* phgn = dynamic_cast<TH1*>(pfn->Get("hcsGain_apa1u"));
      double ymin = 0.0;
      double ymax = 600*(ipul - 1);
      phhp->GetYaxis()->SetRangeUser(ymin, ymax);
      phwp->GetYaxis()->SetRangeUser(0.0, 6.0);
      phcp->GetYaxis()->SetRangeUser(100.0, 10000000.0);
      phrp->GetYaxis()->SetRangeUser(0.0, 3.0);
      phgp->GetYaxis()->SetRangeUser(0.0, 30.0);
      vector<TH1*> histps = {phhp, phwp, phcp, phrp, phgp};
      vector<TH1*> histns = {phhn, phwn, phcn, phrn, phgn};
      for ( TH1* ph : histps ) {
        ph->GetXaxis()->SetRangeUser(chan1, chan2);
        ph->SetMarkerStyle(2);
      }
      for ( TH1* ph : histns ) {
        ph->GetXaxis()->SetRangeUser(chan1, chan2);
        ph->SetMarkerStyle(4);
      }
      TCanvas* pcan1 = new TCanvas;
      pcan1->SetWindowSize(1500, 1000);
      pcan1->Divide(1,2);
      TCanvas* pcan2 = new TCanvas;
      pcan2->SetWindowSize(1500, 1000);
      pcan2->Divide(1,2);
      TCanvas* pcan3 = new TCanvas;
      pcan3->SetWindowSize(1500, 1000);
      pcan3->Divide(1,2);
      for ( TCanvas* pcan : {pcan1, pcan2, pcan3} ) {
        for ( int ipad=1; ipad<=2; ++ipad ) {
          TVirtualPad* ppad = pcan->GetPad(ipad);
          ppad->SetLeftMargin(0.06);
          ppad->SetRightMargin(0.03);
          ppad->SetGridx();
          ppad->SetGridy();
        }
      }
      phhp->GetYaxis()->SetTitleOffset(0.80);
      phwp->GetYaxis()->SetTitleOffset(0.80);
      // Draw height.
      pcan1->cd(1);
      phhp->DrawCopy("P");
      phhn->DrawCopy("P same");
      // Draw width.
      pcan1->cd(2);
      phwp->DrawCopy("P");
      phwn->DrawCopy("P same");
      ssout.str("");
      // Draw chi-square
      pcan2->cd(1);
      gPad->SetLogy();
      phcp->DrawCopy("P");
      phcn->DrawCopy("P same");
      // Draw reduced chi-square
      pcan2->cd(2);
      phrp->DrawCopy("P");
      phrn->DrawCopy("P same");
      // Draw height gain
      pcan3->cd(1);
      phgp->DrawCopy("P");
      phgn->DrawCopy("P same");
      vector<TCanvas*> cans = {pcan1, pcan2, pcan3};
      vector<string> labs = {"hw", "cs", "ga"};
      for ( unsigned int ican=0; ican<cans.size(); ++ican ) {
        pcan = cans[ican];
        string slab = labs[ican];
        // Add label.
        pcan->cd(1);
        ssout.str("");
        ssout << "Pulser DAC = " << ipul;
        string txt = ssout.str();
        ptxt = new TLatex(0.85, 0.92, txt.c_str());
        ptxt->SetNDC();
        ptxt->SetTextFont(42);
        ptxt->Draw();
        // Save plot.
        ssout.str("");
        ssout << "hcsum_" << slab << "_dac" << ipul << ".png";
        string fout = ssout.str();
        pcan->Update();
        pcan->Print(fout.c_str());
      }
    }
  }
  // Fit the gains.
  if ( doGainFits ) {
    int icha1 = chan1;
    int icha2 = chan2;
    int ipad = 0;
    int ncan = 0;
    TGraphErrors* pgg = new TGraphErrors(icha2-icha1);
    TGraphErrors* pgo = new TGraphErrors(icha2-icha1);
    TGraph* pgc = new TGraph(icha2-icha1);
    TGraph* pgn = new TGraph(icha2-icha1);
    float chsqMax = 10000.0;
    TH1* phgDist = new TH1F("hgdist", "Fitted gains; Gain [(ADC count)/Q_{step}]; # channels", 50, 0, 0);
    TH1* phoDist = new TH1F("hodist", "Fitted offsets; Offset/V_{step}; # channels", 50, 0, 0);
    TH1* phcDist = new TH1F("hcdist", "Fit qualities; log_{10}(#chi^{2}); # channels", 40, 0, log10(chsqMax));
    TH1* phrDist = new TH1F("hrdist", "Fit qualities; #chi^{2}/DOF; # channels", 40, 0, rcsqMax);
    unsigned int nchaProc = 0;
    unsigned int nchaSkip = 0;
    for ( int icha=icha1; icha<icha2; ++icha ) {
      vector<float> x;
      vector<float> y;
      vector<float> ex;
      vector<float> ey;
      vector<float> xf1;
      vector<float> yf1;
      vector<float> exf1;
      vector<float> eyf1;
      vector<float> xf2;
      vector<float> yf2;
      vector<float> exf2;
      vector<float> eyf2;
      for ( int irun=0; irun<runs.size(); ++irun ) {
        int run = runs[irun];
        int ipul = ipuls[irun];
        // Loop over positive and negative pulses;
        float sign = 1.0;
        for ( TFile* pf : {pfps[irun], pfns[irun]} ) {
          TH1* phh = dynamic_cast<TH1*>(pf->Get("hcsHeight_apa1u"));
          TH1* phc = dynamic_cast<TH1*>(pf->Get("hcsChiSquare_apa1u"));
          int ibin = icha + 1;
          float xval = sign*ipul;
          float yval = sign*phh->GetBinContent(ibin);
          float xerr = 0.5;
          float yerr = phh->GetBinError(ibin);
          float chsq = phc->GetBinContent(ibin);
          bool empty = yval == 0.0 && yerr == 0.0 && chsq == 0.0;
          if ( ! empty ) {   // Otherwise bin is empty
            x.push_back(xval);
            y.push_back(yval);
            ex.push_back(xerr);
            ey.push_back(yerr);
            if ( ipul > 1 && chsq < hchsqMax ) {
              xf1.push_back(xval);
              yf1.push_back(yval);
              exf1.push_back(xerr);
              eyf1.push_back(yerr);
              if ( ipul < 6 ) {
                xf2.push_back(xval);
                yf2.push_back(yval);
                exf2.push_back(xerr);
                eyf2.push_back(yerr);
              }
            }
          }
          sign *= -1.0;
        }
      }
      if ( x.size() < 2 ) {
        cout << myname << "Skipping channel " << icha << " with point count " << x.size() << endl;
        ++nchaSkip;
        continue;
      }
      ++nchaProc;
      string sttl = "Height vs. DAC channel " + std::to_string(icha) + "; DAC count; Pulse height [ADC count]";
      TH1* pax = new TH2F("hax", sttl.c_str(), 10, -8, 8, 10, -2100, 2100);
      pax->SetDirectory(nullptr);
      pax->SetStats(0);
      TGraph* pgh = new TGraphErrors(x.size(), &x[0], &y[0], &ex[0], &ey[0]);
      pgh->SetMarkerStyle(2);
      TGraph* pghf = new TGraphErrors(xf1.size(), &xf1[0], &yf1[0], nullptr, &eyf1[0]);
      pghf->SetMarkerStyle(4);
      //TF1* pf = new TF1("line", "[0]*x", 0, 8);
      //TF1* pf = new TF1("line", "[1]*(x-[0])");
      //TF1* pf = new TF1("line", "x>[0]?[1]*(x-[0]):(x<-[0]?[1]*(x+[0]):0.0)", -8, 8);
      TF1* pf = offsetLineTF1(0.0, 100.0, -8, 8);
      pf->SetParameter(0, 0.2);
      pf->SetParameter(1, 250);
      pghf->Fit(pf);
      float toff = pf->GetParameter(0);
      float dtoff = pf->GetParError(0);
      float gain = pf->GetParameter(1);
      float dgain = pf->GetParError(1);
      float chsq = pf->GetChisquare();
      int ndof = pf->GetNDF();
      float rcsq = ndof ? chsq/ndof : 0.0;
      if ( chsq > chsqMax ) chsq = chsqMax;
      if ( rcsq > rcsqMax ) rcsq = rcsqMax;
      int ipt = icha - icha1;
      pgg->SetPoint(ipt, icha, gain);
      pgg->SetPointError(ipt, 0.5, dgain);
      pgo->SetPoint(ipt, icha, toff);
      pgo->SetPointError(ipt, 0.5, dtoff);
      pgc->SetPoint(ipt, icha, chsq);
      pgn->SetPoint(ipt, icha, rcsq);
      phgDist->Fill(gain);
      phoDist->Fill(toff);
      phcDist->Fill(log10(chsq));
      phrDist->Fill(rcsq);
      ssout.str("");
      ssout.precision(2);
      ssout << "Offset: " << std::fixed << toff << " Q_{step}";
      string slabOffset = ssout.str();
      ssout.str("");
      ssout.precision(1);
      ssout << "Gain: " << std::fixed << gain << " (ADC count)/Q_{step}";
      string slabGain = ssout.str();
      ssout.str("");
      ssout.precision(1);
      ssout << "#chi^{2}: " << std::fixed << pf->GetChisquare();
      string slabChi2 = ssout.str();
      if ( ipad == 0 ) {
        pcan = new TCanvas;
        pcan->SetWindowSize(1500, 1000);
        pcan->Divide(4,4);
      }
      pcan->cd(++ipad);
      gPad->SetGridx();
      gPad->SetGridy();
      pax->DrawCopy();
      delete pax;
      pgh->Draw("p same");
      pghf->Draw("p same");
      pf->Draw("same");
      ptxt = new TLatex(0.15, 0.73, slabOffset.c_str());
      ptxt->SetNDC();
      ptxt->SetTextFont(42);
      ptxt->Draw();
      ptxt = new TLatex(0.15, 0.82, slabGain.c_str());
      ptxt->SetNDC();
      ptxt->SetTextFont(42);
      ptxt->Draw();
      ptxt = new TLatex(0.15, 0.64, slabChi2.c_str());
      ptxt->SetNDC();
      ptxt->SetTextFont(42);
      ptxt->Draw();
      if ( ipad == 16 || icha + 1 == icha2 ) {
        if ( pcan != nullptr ) {
          string fn = "gainfit_";
          ++ncan;
          if ( ncan < 10 ) fn += "0";
          fn += std::to_string(ncan) + ".png";
          pcan->Print(fn.c_str());
          ipad = 0;
          pcan = nullptr;
        }
      }
    }
    if ( nchaProc ) {
      TVirtualPad* ppad = nullptr;
      pgg->SetTitle("Fitted gains");
      pgg->GetYaxis()->SetTitle("Gain [(ADC count)/Q_{step}]");
      pgg->GetYaxis()->SetRangeUser(0, 350);
      pgo->SetTitle("Fitted offsets");
      pgo->GetYaxis()->SetTitle("Offset [DAC count]");
      pgo->GetYaxis()->SetRangeUser(-3.0, 3.0);
      pgc->SetTitle("Gain fit #chi^{2}");
      pgc->GetYaxis()->SetTitle("#chi^{2}");
      pgc->GetYaxis()->SetRangeUser(0.1, chsqMax);
      pgn->SetTitle("Gain fit #chi^{2}/DOF");
      pgn->GetYaxis()->SetTitle("#chi^{2}/DOF");
      pgn->GetYaxis()->SetRangeUser(0, rcsqMax);
      vector<TGraph*> grs = {pgg, pgo, pgc, pgn};
      vector<string> fns = {"fitres_pars.png", "fitres_csqs.png"};
      ipad = 0;
      ncan = 0;
      for ( unsigned int igra=0; igra<grs.size(); ++igra ) {
        TGraph* pg = grs[igra];
        bool hasErrors = dynamic_cast<TGraphErrors*>(pg) != nullptr;
        pg->GetXaxis()->SetTitle("Channel");
        pg->GetXaxis()->SetRangeUser(icha1-0.5, icha2-0.5);
        pg->GetYaxis()->SetTitleOffset(0.80);
        if ( ipad == 0 ) {
          pcan = new TCanvas;
          pcan->SetWindowSize(1500, 1000);
          pcan->Divide(1,2);
        }
        ppad = pcan->cd(++ipad);
        ppad->SetLeftMargin(0.06);
        ppad->SetRightMargin(0.03);
        ppad->SetGridx();
        ppad->SetGridy();
        if ( pg == pgc ) ppad->SetLogy();
        if ( hasErrors ) {
          pg->Draw("APE");
        } else {
          pg->SetMarkerStyle(2);
          pg->Draw("AP");
          if ( pg == pgn ) {
            pg->GetYaxis()->SetRangeUser(0, rcsqMax);
            pg->GetYaxis()->SetTitle("#chi^{2}/DOF");
          }
        }
        if ( ipad == 2 || igra+1 == grs.size() ) {
          pcan->Print(fns[ncan].c_str());
          ++ncan;
          ipad = 0;
        }
      }
      pcan = new TCanvas;
      pcan->SetWindowSize(1500, 1000);
      pcan->Divide(2,2);
      ipad = 0;
      for ( TH1* ph : {phgDist, phoDist, phcDist, phrDist} ) {
        ph->GetYaxis()->SetTitleOffset(0.80);
        ph->SetLineWidth(2);
        pcan->cd(++ipad);
        ppad->SetLeftMargin(0.06);
        ppad->SetRightMargin(0.03);
        if ( ph == phcDist ) ppad->SetLogx();
        ph->Draw();
      }
      pcan->Print("fitdists.png");
    } else {
      cout << myname << "No channels have data." << endl;
    }
  }
  return 0;
}
