int writePedestals(string infil, string hnam, string outfil, string label) {
  const string myname = "writePedestals: ";
  TFile* pfil = TFile::Open(infil.c_str());
  if ( pfil == nullptr  || ! pfil->IsOpen() )  {
    cout << myname << "Unable to open " << infil << endl;
    return 1;
  }
  TH1* ph = dynamic_cast<TH1*>(pfil->Get(hnam.c_str());
  if ( ph == nullptr )  {
    cout << myname << "Histogram not found: " << hnam << endl;
    pfil->ls();
    return 2;
  }
  Index offset = int(ph->GetXaxis()->GetXmin() + 0.1);
  int nbin = ph->GetNbinsX();

  // Create output file.
  ofstream fout(outfil.c_str());
  fout << "tool_type: FclFloatArray\n";
  fout << "LogLevel: 1\n";
  fout << "DefaultValue: 0\n";
  fout << "Label: \"" << label << "\"\n";
  fout << "Unit: \"ADC count\"\n";
  fout << "Offset: " << offset << "\n";
  fout << "Values: [\n";
  fout << "]\n";
  
  cout << myname << "Output file is " << outfil << endl;
}
