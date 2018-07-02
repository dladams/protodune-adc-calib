void pkginit() {
  gROOT->ProcessLine(
    ".x /home/dladams/dudev/dudev01/pkgs/protodune-adc-calib/init.C("
      "\"/home/dladams/dudev/dudev01/pkgs/protodune-adc-calib\","
      "\"/home/dladams/dudev/dudev01/pkgs/protodune-adc-calib/.aclic/$DUNETPC_VERSION\""
    ")"
  );
}
