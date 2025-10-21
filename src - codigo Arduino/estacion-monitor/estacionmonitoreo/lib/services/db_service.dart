import 'package:path/path.dart';
import 'package:sqflite/sqflite.dart';
import '../models/lectura_model.dart';

class DBService {
  static final DBService _instance = DBService._internal();
  factory DBService() => _instance;
  DBService._internal();

  Database? _db;

  Future<Database> get db async {
    if (_db != null) return _db!;
    _db = await _initDB();
    return _db!;
  }

  Future<Database> _initDB() async {
    final path = await getDatabasesPath();
    final dbPath = join(path, 'estacion.db');
    return await openDatabase(dbPath, version: 1, onCreate: _onCreate);
  }

  Future<void> _onCreate(Database db, int version) async {
    await db.execute('''
      CREATE TABLE lecturas (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        temperatura REAL,
        humedad REAL,
        luz REAL,
        gas REAL,
        fechaHora TEXT
      )
    ''');
  }

  Future<int> insertLectura(Lectura l) async {
    final database = await db;
    return await database.insert('lecturas', l.toMap());
  }

  Future<List<Lectura>> getAllLecturas() async {
    final database = await db;
    final rows = await database.query('lecturas', orderBy: 'fechaHora DESC');
    return rows.map((r) => Lectura.fromMap(r)).toList();
  }

  Future<void> close() async {
    final database = await db;
    await database.close();
    _db = null;
  }
}
