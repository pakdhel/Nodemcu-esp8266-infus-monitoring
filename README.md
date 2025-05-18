# Monitoring Infus IoT dengan ESP8266

Sistem monitoring infus berbasis Internet of Things (IoT) menggunakan board **NodeMCU ESP8266**, yang berfungsi untuk memantau *drip rate* (tetesan per menit) dan berat cairan infus yang tersisa secara real-time, lalu mengirimkan data tersebut ke **Firebase Realtime Database**.

> Proyek ini merupakan prototipe yang dibuat untuk pembelajaran dan pengembangan lebih lanjut dalam dunia medis berbasis IoT.

## ✨ Fitur

- Mengukur **drip rate** infus secara real-time berdasarkan sensor tetesan.
- Menghitung **berat cairan infus yang tersisa** menggunakan data awal berat infus.
- Mengirimkan data ke **Firebase Realtime Database** secara periodik.
- Menampilkan hasil monitoring pada serial monitor (dapat dikembangkan ke dashboard atau aplikasi).

## ⚙️ Hardware yang Digunakan
```
| Komponen                  | Jumlah     |
|---------------------------|--------    |
| NodeMCU ESP8266           | 1          |
| Sensor Tetesan Air (IR)   | 1          |
| Load Cell + HX711 Module  | *Opsional* |
| Breadboard + Jumper Wire  | -          |
| Sumber Daya 5V (USB)      | -          |
```
## 🔌 Skema Wiring

### Sensor Tetesan IR
```
| Sensor IR     | NodeMCU (ESP8266) |
|---------------|-------------------|
| VCC           | 3V3               |
| GND           | G                 |
| OUT           | D2 (GPIO4)        |
```

> **Catatan**: Proyek ini belum menghubungkan Load Cell secara aktual, tetapi logika berat infus disimulasikan berdasarkan volume tetesan (dengan asumsinya 13 tetes = 1 mL).

## 🔐 Keamanan Data Rahasia

Untuk menjaga kerahasiaan data seperti SSID WiFi, Password, dan API Firebase, gunakan file konfigurasi terpisah:

### 1. Buat file baru: `include/secrets.h`
```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
#define API_KEY "your_firebase_api_key"
#define DATABASE_URL "your_firebase_database_url"
```
### 2. Di file main.cpp, import:
```cpp
#include "secrets.h"
```

### 3. Tambahkan file secrets.h ke .gitignore
```
/include/secrets.h
```

## 🛠️ Cara Menjalankan Proyek
### 1. Persiapan
1. Install PlatformIO di VS Code.
2. Clone repo ini ke komputer lokal.

### 2. Sesuaikan konfigurasi platformio.ini:
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps = 
	mobizt/Firebase Arduino Client Library for ESP8266 and ESP32@^4.4.14
monitor_speed = 115200
```

### 3. Upload Kode

## 📊 Struktur Data di Firebase
```
pasien/
 └── 1/
      ├── drip_rate: <float>
      ├── infusion_weight: <float>
      ├── time: <timestamp>
      └── initial_infusion_weight: <float>
```

## 📌 Catatan Tambahan
- Sistem ini dapat dikembangkan lebih lanjut menggunakan sensor berat aktual seperti Load Cell + HX711.
- Dapat diintegrasikan ke aplikasi mobile/web dashboard untuk tampilan visualisasi data real-time.
- Proyek ini bertujuan edukatif dan bukan untuk digunakan langsung dalam lingkungan medis tanpa validasi profesional.