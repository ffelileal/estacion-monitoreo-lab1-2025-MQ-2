import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'screens/dashboard_screen.dart';
import 'screens/historial_screen.dart';
import 'providers/lecturas_provider.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const EstacionMeteorologicaApp());
}

class EstacionMeteorologicaApp extends StatelessWidget {
  const EstacionMeteorologicaApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => LecturasProvider(),
      child: MaterialApp(
        title: 'Estación Meteorológica',
        theme: ThemeData(useMaterial3: true, colorSchemeSeed: Colors.teal),
        darkTheme: ThemeData.dark(useMaterial3: true),
        themeMode: ThemeMode.system,
        initialRoute: '/',
        routes: {
          '/': (_) => const DashboardScreen(),
          '/historial': (_) => const HistorialScreen(),
        },
      ),
    );
  }
}
