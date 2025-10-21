package com.example.btmonitor

import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlinx.coroutines.withContext
import java.io.IOException
import java.util.*

class BluetoothService {
    private var socket: BluetoothSocket? = null
    private val SPP_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    sealed class ConnectionState {
        object Connected : ConnectionState()
        object Disconnected : ConnectionState()
        data class Error(val message: String) : ConnectionState()
    }

    data class SensorData(
        val temperature: Float,
        val humidity: Float,
        val light: Float,
        val airQuality: Float
    )

    suspend fun connect(device: BluetoothDevice): ConnectionState = withContext(Dispatchers.IO) {
        try {
            socket?.close()
            socket = device.createRfcommSocketToServiceRecord(SPP_UUID)
            socket?.connect()
            ConnectionState.Connected
        } catch (e: IOException) {
            Log.e("BluetoothService", "Error connecting", e)
            ConnectionState.Error(e.message ?: "Error de conexión")
        }
    }

    fun listenForData(): Flow<SensorData> = flow {
        val buffer = ByteArray(1024)
        var accumulatedData = ""

        while (true) {
            try {
                val bytesRead = socket?.inputStream?.read(buffer) ?: -1
                if (bytesRead == -1) throw IOException("Conexión cerrada")

                val data = String(buffer, 0, bytesRead)
                accumulatedData += data

                while (accumulatedData.contains("\n")) {
                    val newlineIndex = accumulatedData.indexOf("\n")
                    val line = accumulatedData.substring(0, newlineIndex)
                    accumulatedData = accumulatedData.substring(newlineIndex + 1)

                    val values = line.split(",")
                    if (values.size == 4) {
                        try {
                            val sensorData = SensorData(
                                temperature = values[0].toFloat(),
                                humidity = values[1].toFloat(),
                                light = values[2].toFloat(),
                                airQuality = values[3].toFloat()
                            )
                            emit(sensorData)
                        } catch (e: NumberFormatException) {
                            Log.e("BluetoothService", "Error parsing data: $line", e)
                        }
                    }
                }
            } catch (e: IOException) {
                Log.e("BluetoothService", "Error reading data", e)
                break
            }
        }
    }.flowOn(Dispatchers.IO)

    fun disconnect() {
        try {
            socket?.close()
        } catch (e: IOException) {
            Log.e("BluetoothService", "Error closing socket", e)
        }
    }
}