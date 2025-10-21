import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:fl_chart/fl_chart.dart';
import '../providers/lecturas_provider.dart';

class HistorialScreen extends StatelessWidget {
  const HistorialScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final prov = Provider.of<LecturasProvider>(context);
    final lecturas = prov.historial.reversed.toList(); // oldest -> newest

    List<FlSpot> toSpots(List<double> values) {
      return List.generate(values.length, (i) => FlSpot(i.toDouble(), values[i]));
    }

    final tempValues = lecturas.map((e) => e.temperatura).toList();
    final humValues = lecturas.map((e) => e.humedad).toList();
    final luzValues = lecturas.map((e) => e.luz).toList();
    final gasValues = lecturas.map((e) => e.gas).toList();

    return Scaffold(
      appBar: AppBar(title: const Text('Historial')),
      body: Padding(
        padding: const EdgeInsets.all(12.0),
        child: ListView(
          children: [
            const Text('Temperatura', style: TextStyle(fontWeight: FontWeight.bold)),
            SizedBox(height: 150, child: _buildLineChart(toSpots(tempValues), Colors.deepOrange)),
            const SizedBox(height: 12),
            const Text('Humedad', style: TextStyle(fontWeight: FontWeight.bold)),
            SizedBox(height: 150, child: _buildLineChart(toSpots(humValues), Colors.blue)),
            const SizedBox(height: 12),
            const Text('Luz', style: TextStyle(fontWeight: FontWeight.bold)),
            SizedBox(height: 150, child: _buildLineChart(toSpots(luzValues), Colors.amber)),
            const SizedBox(height: 12),
            const Text('Gas', style: TextStyle(fontWeight: FontWeight.bold)),
            SizedBox(height: 150, child: _buildLineChart(toSpots(gasValues), Colors.grey)),
          ],
        ),
      ),
    );
  }

  Widget _buildLineChart(List<FlSpot> spots, Color color) {
    if (spots.isEmpty) return const Center(child: Text('No hay datos'));
    return LineChart(
      LineChartData(
        gridData: const FlGridData(show: true),
        titlesData: const FlTitlesData(show: false),
        borderData: FlBorderData(show: true),
        lineBarsData: [
          LineChartBarData(spots: spots, isCurved: true, color: color, barWidth: 2)
        ],
      ),
    );
  }
}
