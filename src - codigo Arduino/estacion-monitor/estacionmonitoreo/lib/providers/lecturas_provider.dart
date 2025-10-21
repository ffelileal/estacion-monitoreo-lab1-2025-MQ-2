import 'dart:async';
import 'package:flutter/material.dart';
import '../models/lectura_model.dart';
import '../services/db_service.dart';
import '../services/bluetooth_service.dart';

class LecturasProvider extends ChangeNotifier {
  final DBService _db = DBService();
  final BluetoothService bt = BluetoothService();

  Lectura? ultima;
  List<Lectura> historial = [];

  StreamSubscription<String>? _sub;
  StreamSubscription<ConnectionStateBT>? _stateSub;
  ConnectionStateBT get connectionState => bt.connectionState;

  LecturasProvider() {
    // Listen to incoming lines and try to parse
    _sub = bt.lines.listen(_onLine);
    // Listen to bluetooth connection state changes and notify UI
    _stateSub = bt.stateChanges.listen((_) {
      notifyListeners();
    });
    _loadHistorial();
  }

  Future<void> _loadHistorial() async {
    historial = await _db.getAllLecturas();
    if (historial.isNotEmpty) ultima = historial.first;
    notifyListeners();
  }

  /// Public refresh method for UI widgets (e.g. RefreshIndicator)
  Future<void> refresh() async {
    await _loadHistorial();
  }

  void _onLine(String line) async {
    // Expected format (example):
    // T:24.5;H:45.2;L:300;G:120;D:2025-10-15T12:34:56
    try {
      final parts = line.split(RegExp(r'[;\n]'));
      final map = <String, String>{};
      for (var p in parts) {
        if (p.trim().isEmpty) continue;
        final kv = p.split(':');
        if (kv.length < 2) continue;
        map[kv[0]] = kv.sublist(1).join(':');
      }
      final lectura = Lectura(
        temperatura: double.parse(map['T'] ?? '0'),
        humedad: double.parse(map['H'] ?? '0'),
        luz: double.parse(map['L'] ?? '0'),
        gas: double.parse(map['G'] ?? '0'),
        fechaHora: DateTime.parse(map['D'] ?? DateTime.now().toIso8601String()),
      );

      ultima = lectura;
      await _db.insertLectura(lectura);
      historial.insert(0, lectura);
      notifyListeners();
    } catch (e) {
      // ignore parse errors for now
    }
  }

  Future<void> connect(String deviceId) async {
    try {
      await bt.connect(deviceId);
      notifyListeners();
    } catch (e) {
      // propagate as error state if needed
      rethrow;
    }
  }

  Future<void> disconnect() async {
    await bt.disconnect();
    notifyListeners();
  }

  @override
  void dispose() {
    _sub?.cancel();
    _stateSub?.cancel();
    bt.dispose();
    super.dispose();
  }
}
