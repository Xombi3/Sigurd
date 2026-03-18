> **Sigurd** — named after the legendary Norse dragon slayer — is a network scanning and monitoring tool for the **ESP32 Cheap Yellow Display (CYD)**. A portable, pocket-sized security tool that discovers hosts, scans ports, grabs service banners, resolves hostnames, monitors for new devices, detects deauth attacks and evil twins, tracks signal strength history, logs findings to SD card, and serves a live web dashboard — all on a $10 display with a fully animated Viking character.

> ⚠️ **For educational and authorized penetration testing only. Do not use on networks you do not own or have explicit written permission to test.**

---

## Lineage

Sigurd stands on the shoulders of two great projects:

### 🐦‍⬛ Bjorn
Created by [infinition](https://github.com/infinition), **Bjorn** is the original — a Tamagotchi-like autonomous security tool for the Raspberry Pi Zero W. The iconic Viking character, RPG-style stat system, e-Paper UI aesthetic, and the philosophy of making network security feel alive all originate with Bjorn.

👉 [github.com/infinition/Bjorn](https://github.com/infinition/Bjorn)

### ⚡ Ragnar
Created by [PierreGode](https://github.com/PierreGode), **Ragnar** extended Bjorn to run on modern Raspberry Pi, headless Debian servers, and the WiFi Pineapple Pager with a full-color LCD. The Pineapple Pager port — with its color LCD, button controls, and LED indicators — directly inspired Sigurd's display layout.

👉 [github.com/PierreGode/Ragnar](https://github.com/PierreGode/Ragnar)

### ⚔️ Sigurd (this project)
Sigurd ports the spirit of Bjorn and Ragnar to the **ESP32-2432S028R** — the $10 Cheap Yellow Display. Where Bjorn and Ragnar run Linux, Sigurd runs bare-metal Arduino on a microcontroller. Self-contained, pocket-sized, boots in seconds.

---

## Features

### Scanning
| Feature | Description |
|---------|-------------|
| 📡 WiFi Scan | Discovers nearby APs, flags open networks, logs SSIDs and signal strength |
| 🖥️ Host Discovery | TCP probes your subnet to find live hosts |
| 🔓 Port Scan | Connect-scan across 17 common ports |
| 🏷️ Banner Grabbing | Reads service banners from open ports (SSH version, HTTP server, FTP greeting etc.) |
| 🌐 HTTP Title Grab | Pulls the `<title>` tag from web servers so you see "Synology DiskStation" not just "Apache" |
| 🔍 Hostname Resolution | NetBIOS UDP queries to resolve IP → hostname for Windows and NAS devices |

### Monitoring
| Feature | Description |
|---------|-------------|
| 👁️ New Device Alerts | Background sweep every 3 minutes — alerts when a new host appears on the network |
| 👹 Evil Twin Detection | Flags when two APs share the same SSID but different BSSIDs |
| 📶 Signal History | Tracks RSSI of known APs over time, live graph on screen |
| 🕵️ Deauth Detection | Promiscuous 802.11 sniffer catches deauth and disassociation frames with MAC addresses |

### Output
| Feature | Description |
|---------|-------------|
| 🌐 Web Dashboard | Live findings at `http://<device-ip>/` — hosts, ports, banners, deauth log, RSSI history |
| 📋 JSON API | `/api` endpoint returns all findings as JSON |
| 💾 SD Card Logging | Every finding, host, banner, WiFi network and alert written to `sigurd.log` |
| 🚨 Alert Flash | Red border flash + RGB LED pulse on new device or evil twin detection |

### Character
| Feature | Description |
|---------|-------------|
| 🧙 Animated Viking Sprite | Gold helmet with Norse rune engravings, white horns, battle scar, fur collar, axe in hand |
| 😤 State Expressions | Idle, scanning, found, attacking and talking expressions with animated eyes and brows |
| 💬 Original Sayings | 29 original sayings across 4 state-driven pools, Fisher-Yates shuffled — never repeats |
| 🎮 RPG Stats | Targets, vulns, score, level, attack power — the Bjorn/Ragnar tradition lives on |
| 💡 RGB LED | Blue=scanning, Red=port scan/alert, Green=done |

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32-2432S028R ("Cheap Yellow Display") |
| MCU | ESP32-WROOM-32, dual core 240MHz |
| Display | 2.8" ILI9341_2, 240×320 portrait |
| Touch | XPT2046 resistive touchscreen |
| WiFi | ESP32 802.11 b/g/n built-in |
| RGB LED | GPIO 4 (R), 16 (G), 17 (B) |
| MicroSD | GPIO 5 (CS) — optional, for logging |
| Price | ~$10–15 shipped |

---

## Touch Zones

| Zone | Action |
|------|--------|
| Header — left half | Toggle background network monitor on/off |
| Header — right half | Toggle RSSI signal graph (requires WiFi scan first) |
| Stats row 1 — left third | WiFi Scan |
| Stats row 1 — middle third | Host Scan |
| Stats row 1 — right third | Port Scan |
| Stats row 2 — left half | Toggle Deauth Detection |
| Stats row 2 — right half | Port Scan |
| Action row | Random idle saying from Sigurd |
| Bottom panel — left half | Reset score / level / stats |
| Bottom panel — right half | Clear all scan data and findings |

---

## Web Interface

Once connected to WiFi, navigate to the IP shown on the action row:

```
http://192.168.x.x/
```

Shows:
- Live stats (targets, open ports, banners, score, level, deauth count)
- Discovered hosts with resolved hostnames
- Full findings table with IP, port, service badge, and banner/title text
- RSSI history table per tracked AP
- Deauth event log with source/dest MAC and signal strength
- SD card log download link

**Endpoints:**
```
http://<ip>/      — HTML dashboard
http://<ip>/api   — JSON data
http://<ip>/log   — Raw SD log download
```

---

## The Viking

Sigurd's sprite is hand-drawn using TFT primitives across 20+ depth colors:

- **Gold helmet** with three Norse rune engravings (Tiwaz, Algiz, Sowilo)
- **White tapered horns** with ring bands, drawn as gradient scan lines to a single-pixel tip
- **Battle scar** — jagged slash across the left cheek in dark blood red
- **Brown fur collar** — layered tufts sitting above the barrel
- **Axe in hand** — single-bit crescent blade through the right fist, with pommel and knuckle grip detail
- **Spiral eyes** that blink every 3 seconds and look left/right during scans
- **State-driven expressions** — neutral, suspicious, excited, snarling
- **Subtle breathing** — moustache tufts shift 1px on a 2.4 second cycle

---

## Sayings

Sigurd has 29 original sayings across four state-driven pools, shuffled with the ESP32 hardware RNG:

**Idle** — *"Your firewall is a shield made of straw."* / *"SMB wide open. How generous of you."* / *"Oh look. Another default password."*

**Scanning** — *"The ravens are circling. Something is out there."* / *"The longship moves silently through the network."*

**Found** — *"Valhalla! A worthy target reveals itself."* / *"There you are. I knew you were hiding."*

**Attacking** — *"The axe falls. Open ports bleed data."* / *"Telnet in this era? You deserve what follows."*

---

## Installation

### 1. Install Arduino IDE + ESP32 board support

Add to Preferences → Additional Board URLs:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Install **ESP32** via Boards Manager.

### 2. Install libraries

Via Arduino Library Manager:
- **TFT_eSPI** by Bodmer
- **XPT2046_Touchscreen** by Paul Stoffregen

`WebServer`, `SD`, `WiFi`, `FS`, `WiFiUdp` and `esp_random` are all built into the ESP32 Arduino core.

### 3. Configure TFT_eSPI

Replace `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h` with:

```cpp
#define ILI9341_2_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH
#define USE_HSPI_PORT
#define SPI_FREQUENCY        55000000
#define SPI_READ_FREQUENCY   20000000
#define SPI_TOUCH_FREQUENCY   2500000
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
```

### 4. Configure the sketch

Open `sigurd_cyd.ino` and edit the top of the file:

```cpp
#define WIFI_SSID    "YourNetworkName"
#define WIFI_PASS    "YourPassword"
#define SCAN_SUBNET  "192.168.1"   // first three octets of your network
```

### 5. Flash

| Setting | Value |
|---------|-------|
| Board | ESP32 Dev Module |
| Partition Scheme | Default 4MB with SPIFFS |
| Upload Speed | 921600 |
| CPU Frequency | 240MHz |

---

## Ports Scanned

| Port | Service | Port | Service |
|------|---------|------|---------|
| 21 | FTP | 139 | NetBIOS |
| 22 | SSH | 143 | IMAP |
| 23 | Telnet | 443 | HTTPS |
| 25 | SMTP | 445 | SMB |
| 53 | DNS | 3306 | MySQL |
| 80 | HTTP | 3389 | RDP |
| 110 | POP3 | 5900 | VNC |
| 135 | RPC | 8080 | HTTP-Alt |
| | | 8443 | HTTPS-Alt |

---

## Compared to Ragnar / Bjorn

| Feature | Bjorn | Ragnar | Sigurd CYD |
|---------|-------|--------|------------|
| Hardware | RPi Zero W | RPi 4/5 | ESP32 CYD |
| OS | Linux | Linux | Bare metal Arduino |
| Host discovery | ICMP ping | ICMP ping | TCP connect probe |
| Port scanning | nmap | nmap | TCP connect scan |
| Banner grabbing | ✅ | ✅ | ✅ |
| HTTP title grab | ❌ | ❌ | ✅ |
| Hostname resolution | ✅ | ✅ | ✅ NetBIOS |
| New device alerts | ❌ | ❌ | ✅ |
| Evil twin detection | ❌ | ❌ | ✅ |
| Signal history graph | ❌ | ❌ | ✅ |
| Deauth detection | ❌ | ❌ | ✅ 802.11 promiscuous |
| Web interface | ✅ | ✅ Port 8000 | ✅ Port 80 |
| JSON API | ❌ | ✅ | ✅ |
| SD logging | ✅ | ✅ | ✅ |
| Alert flash | ❌ | ❌ | ✅ |
| Brute force | ✅ | ✅ | ❌ ESP32 RAM limit |
| AI analysis | ❌ | ✅ GPT | ❌ |
| Threat intel feeds | ❌ | ✅ CISA/NVD | ❌ |
| Animated sprite | ✅ | ✅ | ✅ Full depth color |
| Original sayings | ✅ | ✅ | ✅ 29 original |
| Cost | ~$50+ | ~$80+ | ~$10–15 |

---

## Troubleshooting

**White screen** — Confirm `ILI9341_2_DRIVER` and `USE_HSPI_PORT` are in `User_Setup.h`. Confirm `tft.invertDisplay(true)` is in `setup()`.

**Touch not responding** — Touch uses VSPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36). Try changing `ts.setRotation(0)` to `ts.setRotation(1)`, `2`, or `3`.

**Touch coordinates wrong** — Cycle through `ts.setRotation()` values 0–3 until taps register correctly for portrait orientation.

**Host scan finds nothing** — Confirm `WIFI_SSID`/`WIFI_PASS` are set and the board has joined the network. Check `SCAN_SUBNET` matches your router (e.g. `"10.0.0"` not `"192.168.1"`).

**Web UI not loading** — Check the IP shown on the action row. Your browser must be on the same WiFi network.

**RSSI graph shows nothing** — Run a WiFi Scan first to populate signal history, then tap the right half of the header.

**SD not detected** — Card must be FAT32 formatted. CS pin is GPIO 5.

**Sayings repeat** — The Fisher-Yates shuffle uses `esp_random()` (hardware RNG). If you see repeats the pool is exhausting and reshuffling — this is normal for small pools.

---

## Credits

| Project | Author | Link |
|---------|--------|------|
| **Bjorn** — the original Viking security tool | [infinition](https://github.com/infinition) | [github.com/infinition/Bjorn](https://github.com/infinition/Bjorn) |
| **Ragnar** — Bjorn's powerful successor | [PierreGode](https://github.com/PierreGode) | [github.com/PierreGode/Ragnar](https://github.com/PierreGode/Ragnar) |
| **TFT_eSPI** | [Bodmer](https://github.com/Bodmer) | [github.com/Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) |
| **XPT2046_Touchscreen** | [Paul Stoffregen](https://github.com/PaulStoffregen) | [github.com/PaulStoffregen/XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) |
| **ESP32 CYD community** | [witnessmenow](https://github.com/witnessmenow) | [github.com/witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) |

---

## License

MIT — same license as Bjorn and Ragnar.

*Sigurd CYD is an independent port and is not affiliated with, endorsed by, or officially connected to the Bjorn or Ragnar projects. All credit for the original concept, character design, and security tool philosophy belongs to infinition and PierreGode.*

---

<p align="center"><i>"Your firewall is a shield made of straw."</i></p>
