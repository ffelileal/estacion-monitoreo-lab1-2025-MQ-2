import 'bluetooth_platform.dart';

Future<List<BondedDevice>> getBondedDevices() async {
  // Stub: return empty list on unsupported platforms (web/desktop)
  return <BondedDevice>[];
}
