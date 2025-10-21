package com.example.btmonitor

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.example.btmonitor.databinding.ActivityMainBinding
import com.google.android.material.card.MaterialCardView
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var bluetoothAdapter: BluetoothAdapter
    private val bluetoothService = BluetoothService()
    private var connectedDevice: BluetoothDevice? = null

    private val enableBluetoothLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            showPairedDevices()
        } else {
            Toast.makeText(this, R.string.error_bluetooth_disabled, Toast.LENGTH_SHORT).show()
        }
    }

    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val allGranted = permissions.entries.all { it.value }
        if (allGranted) {
            initializeBluetooth()
        } else {
            Toast.makeText(this, R.string.error_bluetooth_disabled, Toast.LENGTH_SHORT).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setSupportActionBar(binding.toolbar)
        checkPermissions()
        setupUI()
    }

    private fun checkPermissions() {
        val permissions = mutableListOf<String>()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permissions.add(Manifest.permission.BLUETOOTH_CONNECT)
            permissions.add(Manifest.permission.BLUETOOTH_SCAN)
        } else {
            permissions.add(Manifest.permission.BLUETOOTH)
            permissions.add(Manifest.permission.BLUETOOTH_ADMIN)
        }

        val notGranted = permissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        if (notGranted.isNotEmpty()) {
            permissionLauncher.launch(notGranted.toTypedArray())
        } else {
            initializeBluetooth()
        }
    }

    private fun initializeBluetooth() {
        val bluetoothManager = getSystemService(BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = bluetoothManager.adapter

        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            enableBluetoothLauncher.launch(enableBtIntent)
        }
    }

    private fun setupUI() {
        binding.btnConnect.setOnClickListener {
            if (connectedDevice == null) {
                showPairedDevices()
            } else {
                disconnectDevice()
            }
        }
    }

    private fun showPairedDevices() {
        if (!::bluetoothAdapter.isInitialized) {
            Toast.makeText(this, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show()
            return
        }

        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.BLUETOOTH_CONNECT
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            checkPermissions()
            return
        }

        val pairedDevices = bluetoothAdapter.bondedDevices.toList()
        if (pairedDevices.isEmpty()) {
            Toast.makeText(this, R.string.no_paired_devices, Toast.LENGTH_SHORT).show()
            return
        }

        val deviceNames = pairedDevices.map { it.name }
        AlertDialog.Builder(this)
            .setTitle(R.string.select_device)
            .setItems(deviceNames.toTypedArray()) { _, which ->
                connectToDevice(pairedDevices[which])
            }
            .show()
    }

    private fun connectToDevice(device: BluetoothDevice) {
        binding.tvConnectionStatus.setText(R.string.status_connecting)
        
        lifecycleScope.launch {
            when (val state = bluetoothService.connect(device)) {
                is BluetoothService.ConnectionState.Connected -> {
                    connectedDevice = device
                    updateUIForConnectedState(device)
                    startListeningForData()
                }
                is BluetoothService.ConnectionState.Error -> {
                    Toast.makeText(this@MainActivity, state.message, Toast.LENGTH_SHORT).show()
                    updateUIForDisconnectedState()
                }
                else -> updateUIForDisconnectedState()
            }
        }
    }

    private fun startListeningForData() {
        lifecycleScope.launch {
            bluetoothService.listenForData().collect { sensorData ->
                withContext(Dispatchers.Main) {
                    updateSensorValues(sensorData)
                }
            }
        }
    }

    private fun updateSensorValues(data: BluetoothService.SensorData) {
        binding.apply {
            tvTemperature.text = "%.1f°C".format(data.temperature)
            tvHumidity.text = "%.1f%%".format(data.humidity)
            tvLight.text = "%.0f LUX".format(data.light)
            tvAirQuality.text = "%.0f PPM".format(data.airQuality)

            // Actualizar color de la tarjeta de calidad del aire
            val cardAirQuality = findViewById<MaterialCardView>(R.id.cardAirQuality)
            when {
                data.airQuality > 300 -> {
                    cardAirQuality.setCardBackgroundColor(
                        ContextCompat.getColor(this@MainActivity, android.R.color.holo_red_light)
                    )
                }
                data.airQuality > 200 -> {
                    cardAirQuality.setCardBackgroundColor(
                        ContextCompat.getColor(this@MainActivity, android.R.color.holo_orange_light)
                    )
                }
                else -> {
                    cardAirQuality.setCardBackgroundColor(
                        ContextCompat.getColor(this@MainActivity, android.R.color.white)
                    )
                }
            }
        }
    }

    private fun updateUIForConnectedState(device: BluetoothDevice) {
        binding.apply {
            tvConnectionStatus.text = getString(R.string.status_connected, device.name)
            btnConnect.setText(R.string.disconnect)
        }
    }

    private fun updateUIForDisconnectedState() {
        binding.apply {
            tvConnectionStatus.setText(R.string.status_disconnected)
            btnConnect.setText(R.string.connect)
            tvTemperature.text = "--.-°C"
            tvHumidity.text = "--%"
            tvLight.text = "--- LUX"
            tvAirQuality.text = "--- PPM"
        }
    }

    private fun disconnectDevice() {
        bluetoothService.disconnect()
        connectedDevice = null
        updateUIForDisconnectedState()
    }

    override fun onDestroy() {
        super.onDestroy()
        bluetoothService.disconnect()
    }
}