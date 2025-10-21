import 'dart:async';
import 'dart:typed_data';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:permission_handler/permission_handler.dart';

// Note: HC-05 uses Bluetooth Classic SPP. To connect for real you must implement
// a platform-specific adapter (Android) that opens a BluetoothSocket and forwards
// incoming lines into this service. This class provides a consistent API and
// simple simulated connect for development.

enum ConnectionStateBT { disconnected, connecting, connected }

class BluetoothService {
  // Stream of raw lines received from the serial/bluetooth connection
  final StreamController<String> _linesController = StreamController.broadcast();
  Stream<String> get lines => _linesController.stream;
  ConnectionStateBT state = ConnectionStateBT.disconnected;
  // StreamController to broadcast state changes so other parts can listen
  final StreamController<ConnectionStateBT> _stateController = StreamController.broadcast();
  Stream<ConnectionStateBT> get stateChanges => _stateController.stream;
  String? lastDeviceId;
  BluetoothConnection? _connection;
  final _inputBuffer = <int>[];
  StreamSubscription<Uint8List>? _inputSub;

  static const _prefsKey = 'last_bt_device';

  BluetoothService() {
    _loadLastDevice();
  }

  Future<void> _loadLastDevice() async {
    final prefs = await SharedPreferences.getInstance();
    lastDeviceId = prefs.getString(_prefsKey);
  }

  Future<void> _saveLastDevice(String id) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(_prefsKey, id);
  }

  Future<void> connectToDevice(String deviceId) async {
    // Request necessary permissions
    await Permission.bluetooth.request();
    await Permission.bluetoothConnect.request();
    await Permission.bluetoothScan.request();
    await Permission.locationWhenInUse.request();

    // Check statuses
    final status = await Permission.bluetooth.status;
    final statusConnect = await Permission.bluetoothConnect.status;
    if (!status.isGranted && !statusConnect.isGranted) {
      throw Exception('Permisos Bluetooth no concedidos');
    }

  if (state == ConnectionStateBT.connected) return;

  state = ConnectionStateBT.connecting;
    _stateController.add(state);

    try {
      // `deviceId` expected to be MAC address (e.g., "00:11:22:33:44:55")
      _connection = await BluetoothConnection.toAddress(deviceId);
      lastDeviceId = deviceId;
      await _saveLastDevice(deviceId);
      state = ConnectionStateBT.connected;
      _stateController.add(state);

      // Listen to incoming bytes and keep subscription to cancel later
      _inputSub = _connection!.input?.listen((Uint8List data) {
        _handleIncomingData(data);
      });

      // handle connection closure/errors
      _inputSub?.onDone(() {
        _inputSub = null;
        try {
          _connection?.finish();
        } catch (_) {}
        _connection = null;
        state = ConnectionStateBT.disconnected;
        _stateController.add(state);
      });
      _inputSub?.onError((_) {
        _inputSub = null;
        try {
          _connection?.finish();
        } catch (_) {}
        _connection = null;
        state = ConnectionStateBT.disconnected;
        _stateController.add(state);
      });
    } catch (e) {
      // Ensure partial connection is closed
      try {
        await _connection?.finish();
      } catch (_) {}
      _connection = null;
      _inputSub = null;
      state = ConnectionStateBT.disconnected;
      _stateController.add(state);
      rethrow;
    }
  }

  Future<void> disconnect() async {
    try {
      await _inputSub?.cancel();
    } catch (_) {}
    _inputSub = null;

    try {
      await _connection?.finish();
    } catch (_) {}
    _connection = null;

    state = ConnectionStateBT.disconnected;
    _stateController.add(state);
  }

  void _handleIncomingData(Uint8List data) {
    // Buffer bytes until newline and then emit line(s)
    for (var b in data) {
      if (b == 10 || b == 13) {
        if (_inputBuffer.isNotEmpty) {
          final line = String.fromCharCodes(_inputBuffer).trim();
          if (line.isNotEmpty) _linesController.add(line);
          _inputBuffer.clear();
        }
      } else {
        _inputBuffer.add(b);
      }
    }
  }

  // state changes are emitted through [_stateController]

  // For testing: inject a line
  void injectLine(String line) {
    _linesController.add(line);
  }

  void dispose() {
    try {
      _inputSub?.cancel();
    } catch (_) {}
    try {
      _connection?.finish();
    } catch (_) {}

    _linesController.close();
    _stateController.close();
  }
}
