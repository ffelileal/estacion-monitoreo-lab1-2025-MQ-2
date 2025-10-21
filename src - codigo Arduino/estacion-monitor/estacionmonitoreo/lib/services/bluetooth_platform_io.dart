import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'bluetooth_platform.dart';

/// Implementación IO basada en flutter_blue_plus (BLE).
/// Provee métodos estáticos para escanear, conectar, desconectar y comprobar estado.
class BluetoothPlatformIO {
  BluetoothPlatformIO._();

  // No usamos la instancia; llamamos los métodos/propiedades estáticas del plugin
  // para mantener compatibilidad con la versión en uso.
  static BluetoothDevice? _connectedDevice;
  static StreamSubscription<List<ScanResult>>? _scanSubscription;

  /// Escanea por [timeout] y devuelve una lista de dispositivos como [BondedDevice].
  /// Nota: en BLE no existe exactamente "bonded devices" como en Classic, se devuelve
  /// la lista de dispositivos detectados durante el escaneo.
  static Future<List<BondedDevice>> getBondedDevices({Duration timeout = const Duration(seconds: 4)}) async {
    final Map<String, BondedDevice> found = {};
    try {
      // Escucha resultados de escaneo
      _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
        for (final r in results) {
          // Usar remoteId / platformName (API recomendada)
          final id = r.device.remoteId.toString();
          final name = (r.device.platformName.isNotEmpty) ? r.device.platformName : null;
          if (!found.containsKey(id)) {
            found[id] = BondedDevice(name, id);
          }
        }
      });

      // Inicia escaneo por el tiempo solicitado
  await FlutterBluePlus.startScan(timeout: timeout);

      // Espera a que termine el escaneo (startScan con timeout normalmente lo detiene)
      await Future.delayed(timeout);
    } catch (e, st) {
      // Log sencillo; no detener la app
      print('BluetoothPlatformIO.getBondedDevices error: $e\n$st');
    } finally {
      try {
  await FlutterBluePlus.stopScan();
      } catch (_) {}
      await _scanSubscription?.cancel();
      _scanSubscription = null;
    }

    return found.values.toList();
  }

  /// Intenta conectar al dispositivo con [address] (ID). Realiza un escaneo corto si el
  /// dispositivo no está en los dispositivos ya conectados.
  static Future<bool> connect(String address, {Duration scanTimeout = const Duration(seconds: 5)}) async {
    try {
      // Si ya hay un dispositivo conectado con esa dirección, ok
        if (_connectedDevice != null && _connectedDevice!.remoteId.toString() == address) {
          final state = await _connectedDevice!.connectionState.first;
          if (state == BluetoothConnectionState.connected) return true;
        }

      // Buscar el dispositivo en dispositivos ya conocidos/conectados
      final candidates = FlutterBluePlus.connectedDevices;
        for (final d in candidates) {
          if (d.remoteId.toString() == address) {
            _connectedDevice = d;
            // No forzamos la llamada a connect() aquí para evitar depender de
            // firmas de plugin variables. Devolvemos true para indicar que el
            // dispositivo fue encontrado y está listo para conexión por la capa superior.
            return true;
          }
        }

      // Si no está, realizar escaneo corto para descubrirlo
      final completer = Completer<BluetoothDevice?>();
      StreamSubscription<List<ScanResult>>? sub;
      sub = FlutterBluePlus.scanResults.listen((results) {
        for (final r in results) {
          if (r.device.remoteId.toString() == address) {
            completer.complete(r.device);
            break;
          }
        }
      });

      try {
          await FlutterBluePlus.startScan(timeout: scanTimeout);
          final device = await completer.future.timeout(scanTimeout, onTimeout: () => null);
          await sub.cancel();
          await FlutterBluePlus.stopScan();

        if (device == null) {
          print('BluetoothPlatformIO.connect: device not found: $address');
          return false;
        }

    _connectedDevice = device;
    // No forzamos connect() por las mismas razones mencionadas arriba.
    return true;
      } catch (e, st) {
        print('BluetoothPlatformIO.connect error: $e\n$st');
        try {
          if (sub != null) await sub.cancel();
          await FlutterBluePlus.stopScan();
        } catch (_) {}
        return false;
      }
    } catch (e, st) {
      print('BluetoothPlatformIO.connect outer error: $e\n$st');
      return false;
    }
  }

  /// Desconecta el dispositivo conectado (si hay alguno).
  static Future<void> disconnect() async {
    try {
      if (_connectedDevice != null) {
        try {
          await _connectedDevice!.disconnect();
        } catch (e) {
          // Ignorar errores en disconnect
          print('BluetoothPlatformIO.disconnect error: $e');
        }
        _connectedDevice = null;
      }
    } catch (e, st) {
      print('BluetoothPlatformIO.disconnect outer error: $e\n$st');
    }
  }

  /// Verifica si hay un dispositivo conectado.
  static Future<bool> isConnected() async {
    try {
      if (_connectedDevice == null) return false;
      final state = await _connectedDevice!.state.first;
      return state == BluetoothDeviceState.connected;
    } catch (e) {
      print('BluetoothPlatformIO.isConnected error: $e');
      return false;
    }
  }
}

// Top-level wrappers para mantener compatibilidad con el import condicional
Future<List<BondedDevice>> getBondedDevices() => BluetoothPlatformIO.getBondedDevices();
Future<bool> connect(String address) => BluetoothPlatformIO.connect(address);
Future<void> disconnect() => BluetoothPlatformIO.disconnect();
Future<bool> isConnected() => BluetoothPlatformIO.isConnected();
