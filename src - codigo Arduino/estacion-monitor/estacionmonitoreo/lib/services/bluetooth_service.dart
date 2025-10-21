import 'dart:async';
import 'dart:convert';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:shared_preferences/shared_preferences.dart';

// Estado de la conexión
enum ConnectionStateBT { disconnected, connecting, connected }

class BluetoothService {
  // Stream para emitir las líneas de datos recibidas
  final StreamController<String> _linesController = StreamController.broadcast();
  Stream<String> get lines => _linesController.stream;

  // Stream para emitir los cambios de estado de la conexión
  final StreamController<ConnectionStateBT> _stateController = StreamController.broadcast();
  Stream<ConnectionStateBT> get stateChanges => _stateController.stream;

  ConnectionStateBT _state = ConnectionStateBT.disconnected;
  ConnectionStateBT get connectionState => _state;

  BluetoothDevice? _connectedDevice;
  StreamSubscription<List<int>>? _valueSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionStateSubscription;

  String? lastDeviceId;
  static const _prefsKey = 'last_bt_device';

  BluetoothService() {
    _loadLastDevice();
  }

  // Carga el ID del último dispositivo conectado desde SharedPreferences
  Future<void> _loadLastDevice() async {
    final prefs = await SharedPreferences.getInstance();
    lastDeviceId = prefs.getString(_prefsKey);
  }

  // Guarda el ID del dispositivo actual
  Future<void> _saveLastDevice(String id) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(_prefsKey, id);
    lastDeviceId = id;
  }
  
  // Función para conectar al dispositivo
  Future<void> connect(String deviceId) async {
    // 1. Pedir permisos
    if (!(await _requestPermissions())) {
      throw Exception('Permisos Bluetooth no concedidos');
    }

    _updateState(ConnectionStateBT.connecting);

    try {
      // Usamos el ID de dispositivo que viene de un escaneo o guardado previamente
  // Try to find device among scanned/bonded
  final device = BluetoothDevice(remoteId: DeviceIdentifier(deviceId));

  // 2. Conectar al dispositivo
  await device.connect(timeout: const Duration(seconds: 15));
  _connectedDevice = device;
      await _saveLastDevice(deviceId);

      // 3. Escuchar cambios de estado de la conexión
      _connectionStateSubscription = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _updateState(ConnectionStateBT.disconnected);
        } else if (state == BluetoothConnectionState.connected) {
           _updateState(ConnectionStateBT.connected);
        }
      });
      
      _updateState(ConnectionStateBT.connected);

      // 4. Descubrir servicios y características
  await _discoverServices();

    } catch (e) {
      _updateState(ConnectionStateBT.disconnected);
      rethrow; // Propaga el error para que la UI pueda manejarlo
    }
  }

  // Descubre los servicios y se suscribe a la característica correcta
  Future<void> _discoverServices() async {
    if (_connectedDevice == null) return;
    var services = await _connectedDevice!.discoverServices();
    for (var svc in services) {
      for (var characteristic in svc.characteristics) {
        if (characteristic.properties.notify) {
          await characteristic.setNotifyValue(true);
          _valueSubscription = characteristic.value.listen((value) {
            String line = utf8.decode(value, allowMalformed: true).trim();
            if (line.isNotEmpty) _linesController.add(line);
          });
          return;
        }
      }
    }
  }
  
  // Desconectar del dispositivo
  Future<void> disconnect() async {
    await _valueSubscription?.cancel();
    _valueSubscription = null;
    await _connectionStateSubscription?.cancel();
    _connectionStateSubscription = null;
    try {
      await _connectedDevice?.disconnect();
    } catch (_) {}
    _connectedDevice = null;
    _updateState(ConnectionStateBT.disconnected);
  }

  // Helper para actualizar el estado y notificar a los listeners
  void _updateState(ConnectionStateBT newState) {
    _state = newState;
    _stateController.add(_state);
  }

  // Pide los permisos necesarios para Bluetooth en Android
  Future<bool> _requestPermissions() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse, // A veces necesario
    ].request();

    return statuses[Permission.bluetoothScan]!.isGranted &&
           statuses[Permission.bluetoothConnect]!.isGranted;
  }

  // Para pruebas: inyecta una línea de datos simulada
  void injectLine(String line) {
    _linesController.add(line);
  }

  // Limpia los StreamControllers cuando ya no se necesiten
  void dispose() {
    disconnect(); // Asegurarse de desconectar al cerrar
    _linesController.close();
    _stateController.close();
  }
}
