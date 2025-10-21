class Lectura {
  final int? id;
  final double temperatura;
  final double humedad;
  final double luz;
  final double gas;
  final DateTime fechaHora;

  Lectura({
    this.id,
    required this.temperatura,
    required this.humedad,
    required this.luz,
    required this.gas,
    required this.fechaHora,
  });

  factory Lectura.fromMap(Map<String, dynamic> m) => Lectura(
        id: m['id'] as int?,
        temperatura: (m['temperatura'] as num).toDouble(),
        humedad: (m['humedad'] as num).toDouble(),
        luz: (m['luz'] as num).toDouble(),
        gas: (m['gas'] as num).toDouble(),
        fechaHora: DateTime.parse(m['fechaHora'] as String),
      );

  Map<String, dynamic> toMap() => {
        'id': id,
        'temperatura': temperatura,
        'humedad': humedad,
        'luz': luz,
        'gas': gas,
        'fechaHora': fechaHora.toIso8601String(),
      };
}
