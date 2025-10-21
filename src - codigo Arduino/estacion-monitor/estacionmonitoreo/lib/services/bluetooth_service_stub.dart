// Stub implementation for platforms where flutter_bluetooth_serial isn't available.
import 'dart:async';

enum ConnectionStateBT { disconnected, connecting, connected }

class BluetoothService {
  // Minimal stub: expose the same API surface used by the app but do nothing.
  final _linesController = StreamController<String>.broadcast();
  Stream<String> get lines => _linesController.stream;
  ConnectionStateBT state = ConnectionStateBT.disconnected;
  final _stateController = StreamController<ConnectionStateBT>.broadcast();
  Stream<ConnectionStateBT> get stateChanges => _stateController.stream;
  String? lastDeviceId;

  Future<void> connectToDevice(String deviceId) async {
    // Not supported on this platform
    throw UnsupportedError('Bluetooth not supported on this platform');
  }

  Future<void> disconnect() async {}

  void injectLine(String line) {
    _linesController.add(line);
  }

  void dispose() {
    _linesController.close();
    _stateController.close();
  }
}
