int testAll() {
  const string myname = "testAll: ";
  cout << myname << "Loading geometry." << endl;
  const geo::Geometry* pgeo = art::ServiceHandle<geo::Geometry>().get();
  cout << myname << "Detector name: " << pgeo->DetectorName() << endl;
  return 0;
}
