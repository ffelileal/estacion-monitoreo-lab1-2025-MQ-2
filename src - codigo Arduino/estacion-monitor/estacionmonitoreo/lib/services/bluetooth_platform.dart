// Conditional wrapper to access bonded devices without importing plugin directly
import 'bluetooth_platform_io.dart'
    if (dart.library.html) 'bluetooth_platform_stub.dart' as impl;

class BondedDevice {
  final String? name;
  final String address;
  BondedDevice(this.name, this.address);
}

Future<List<BondedDevice>> getBondedDevices() => impl.getBondedDevices();
