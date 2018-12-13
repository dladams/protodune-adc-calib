int testAll() {
  const string myname = "testAll: ";
  cout << myname << "Fetching service pointers." << endl;
  const geo::Geometry* pgeo = art::ServiceHandle<geo::Geometry>().get();
  const lariov::ChannelStatusProvider* pcss = art::ServiceHandle<lariov::ChannelStatusService>().get()->provider();
  cout << myname << "Detector name: " << pgeo->DetectorName() << endl;
  cout << myname << "    # good channels: " << pcss->GoodChannels().size() << endl;
  cout << myname << "     # bad channels: " << pcss->BadChannels().size() << endl;
  cout << myname << "   # noisy channels: " << pcss->NoisyChannels().size() << endl;
  return 0;
}
