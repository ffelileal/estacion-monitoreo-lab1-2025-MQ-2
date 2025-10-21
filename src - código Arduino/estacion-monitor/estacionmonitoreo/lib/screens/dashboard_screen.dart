import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:provider/provider.dart';
import '../providers/lecturas_provider.dart';
import '../widgets/sensor_card.dart';
import '../services/bluetooth_service.dart';
import 'package:intl/intl.dart';

class DashboardScreen extends StatelessWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
  final prov = Provider.of<LecturasProvider>(context);
  final connState = prov.connectionState;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Estación Meteorológica'),
        actions: [
          IconButton(
            icon: const Icon(Icons.show_chart),
            onPressed: () => Navigator.pushNamed(context, '/historial'),
          )
        ],
      ),
      floatingActionButton: kDebugMode
          ? FloatingActionButton.extended(
              onPressed: () {
                // inject sample line
                prov.bt.injectLine('T:23.5;H:48.2;L:420;G:110;D:${DateTime.now().toIso8601String()}');
              },
              label: const Text('Simular'),
              icon: const Icon(Icons.bug_report),
            )
          : null,
      body: RefreshIndicator(
        onRefresh: () async {},
        child: ListView(
          padding: const EdgeInsets.all(12),
          children: [
            Row(
              children: [
                Expanded(
                  child: SensorCard(
                    title: 'Temperatura',
                    value: prov.ultima?.temperatura != null
                        ? '${prov.ultima!.temperatura.toStringAsFixed(1)} °C'
                        : '--',
                    color: Colors.deepOrange,
                    icon: Icons.thermostat,
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: SensorCard(
                    title: 'Humedad',
                    value: prov.ultima?.humedad != null
                        ? '${prov.ultima!.humedad.toStringAsFixed(1)} %'
                        : '--',
                    color: Colors.blue,
                    icon: Icons.water_drop,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 8),
            Row(
              children: [
                Expanded(
                  child: SensorCard(
                    title: 'Luz',
                    value: prov.ultima?.luz != null
                        ? '${prov.ultima!.luz.toStringAsFixed(0)} lx'
                        : '--',
                    color: Colors.amber,
                    icon: Icons.wb_sunny,
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: SensorCard(
                    title: 'Gas/Humo',
                    value: prov.ultima?.gas != null
                        ? '${prov.ultima!.gas.toStringAsFixed(0)} ppm'
                        : '--',
                    color: Colors.grey,
                    icon: Icons.air,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            SensorCard(
              title: 'Fecha / Hora',
              value: prov.ultima != null
                  ? DateFormat('yyyy-MM-dd HH:mm:ss').format(prov.ultima!.fechaHora)
                  : '--',
              color: Colors.green,
              icon: Icons.access_time,
            ),
            const SizedBox(height: 16),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Conexión', style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
                    const SizedBox(height: 8),
                    Text('Último dispositivo: ${prov.bt.lastDeviceId ?? 'Ninguno'}'),
                    const SizedBox(height: 8),
                    Row(
                      children: [
                        ElevatedButton.icon(
                          onPressed: connState == ConnectionStateBT.connected || connState == ConnectionStateBT.connecting
                              ? null
                              : () async {
                                  final id = await _askDeviceId(context);
                                  if (id != null) await prov.connect(id);
                                },
                          icon: const Icon(Icons.bluetooth),
                          label: const Text('Conectar'),
                        ),
                        const SizedBox(width: 8),
                        ElevatedButton.icon(
                          onPressed: connState == ConnectionStateBT.connected ? prov.disconnect : null,
                          icon: const Icon(Icons.close),
                          label: const Text('Desconectar'),
                        ),
                        const SizedBox(width: 8),
                        ElevatedButton(
                          onPressed: prov.bt.lastDeviceId == null
                              ? null
                              : () {
                                  // Retry: try to reconnect to last device
                                  prov.connect(prov.bt.lastDeviceId!);
                                },
                          child: const Text('Reintentar conexión'),
                        )
                      ],
                    ),
                    const SizedBox(height: 8),
                    Row(
                      children: [
                        const Text('Estado: ', style: TextStyle(fontWeight: FontWeight.w600)),
                        if (connState == ConnectionStateBT.connecting) const Text('Conectando...'),
                        if (connState == ConnectionStateBT.connected) const Text('Conectado', style: TextStyle(color: Colors.green)),
                        if (connState == ConnectionStateBT.disconnected) const Text('Desconectado', style: TextStyle(color: Colors.red)),
                      ],
                    )
                  ],
                ),
              ),
            )
          ],
        ),
      ),
    );
  }

  Future<String?> _askDeviceId(BuildContext context) async {
    final controller = TextEditingController();
    return showDialog<String>(
      context: context,
      builder: (_) => AlertDialog(
        title: const Text('ID dispositivo (ej: HC-05)'),
        content: TextField(controller: controller),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('Cancelar')),
          TextButton(onPressed: () => Navigator.pop(context, controller.text.trim()), child: const Text('Conectar')),
        ],
      ),
    );
  }
}
