int testAll() {
  const string myname = "testAll: ";
  cout << myname << "Fetching service pointers." << endl;
  cout << myname << "--------------------------" << endl;
  const geo::Geometry* pgeo = art::ServiceHandle<geo::Geometry>().get();
  const lariov::ChannelStatusProvider* pcss = art::ServiceHandle<lariov::ChannelStatusService>().get()->provider();
  const IndexRangeTool* pcrt = DuneToolManager::instance()->getShared<IndexRangeTool>("channelRanges");
  cout << myname << "--------------------------" << endl;
  cout << myname << "      Detector name: " << pgeo->DetectorName() << endl;
  cout << myname << "    # good channels: " << pcss->GoodChannels().size() << endl;
  cout << myname << "     # bad channels: " << pcss->BadChannels().size() << endl;
  cout << myname << "   # noisy channels: " << pcss->NoisyChannels().size() << endl;
  cout << myname << " APA channel ranges: " << endl;
  for ( string name : {"apa1", "apa2", "apa3", "apa4", "apa5", "apa6"} ) {
    cout << myname << "  " << pcrt->get(name) << endl;
  }
  return 0;
}
