# Estación Meteorológica (Flutter)

Esta app Flutter recibe datos enviados por un Arduino (vía Bluetooth HC-05 o USB serial) y muestra un dashboard en tiempo real, guarda lecturas en `sqflite` y presenta gráficos históricos.

Formato de datos esperado (línea por lectura):

T:24.5;H:45.2;L:300;G:120;D:2025-10-15T12:34:56\n

Donde:
- T: temperatura en °C
- H: humedad relativa en %
- L: intensidad lumínica en lux
- G: nivel de gas/humo en ppm
- D: fecha y hora en formato ISO-8601 (opcional, si no se envía se toma la hora local)

Cómo probar localmente:

1. Abrir el proyecto en VS Code o Android Studio.
2. Ejecutar:

```powershell
flutter pub get
flutter run
```

3. Para pruebas sin hardware real, la clase `BluetoothService` expone `injectLine(String)` para simular la llegada de datos. Puedes accederla a través del provider:

```dart
Provider.of<LecturasProvider>(context, listen:false).bt.injectLine('T:22;H:50;L:200;G:80;D:2025-10-15T12:00:00');
```

Notas y adaptabilidad:
- `BluetoothService` está intencionalmente diseñada como una capa de abstracción: puedes reemplazar su implementación por una que use un plugin de serial USB o un canal de plataforma para SPP en Android/iOS.
- Para cambiar el origen de datos a un servidor HTTP/WS, crea otro servicio que emita líneas compatibles con el formato y úsalo en lugar de `BluetoothService` en `LecturasProvider`.
- La base de datos está en `services/db_service.dart` usando `sqflite` y puede migrarse a cualquier otro almacenamiento local sin tocar la UI.

Próximos pasos posibles:
- Implementar soporte real para Bluetooth clásico SPP (HC-05) con métodos nativos o un plugin adecuado.
- Añadir filtro / deduplicación de lecturas y configuración de retención histórica.
- Añadir exportación de datos (CSV) y sincronización con servidor.

Nota importante sobre compilación en Windows y rutas con caracteres no ASCII:

Si tu proyecto está en una ruta que contiene caracteres no ASCII (por ejemplo espacios o acentos en carpetas superiores), Gradle en Windows puede fallar con un error similar al siguiente:

```
Your project path contains non-ASCII characters. This will most likely cause the build to fail on Windows.
```

He añadido temporalmente la siguiente línea en `android/gradle.properties` para omitir la comprobación de ruta:

```
android.overridePathCheck=true
```

Sin embargo la solución recomendada es mover el proyecto a una ruta sin caracteres no ASCII (por ejemplo `C:\projects\estacion-monitor`). Si puedes moverlo, es la opción más robusta.

