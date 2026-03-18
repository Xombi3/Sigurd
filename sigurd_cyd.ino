/*
 * SIGURD CYD — Port of PierreGode/Ragnar for ESP32 Cheap Yellow Display
 * Hardware: ESP32-2432S028R
 * FOR EDUCATIONAL / AUTHORIZED TESTING ONLY
 *
 * Libraries needed (Arduino Library Manager):
 *   - TFT_eSPI by Bodmer
 *   - XPT2046_Touchscreen by Paul Stoffregen
 *   - WebServer (built-in, no install needed)
 *
 * User_Setup.h must have:
 *   #define ILI9341_2_DRIVER
 *   #define USE_HSPI_PORT
 *   #define TFT_MISO 12  #define TFT_MOSI 13  #define TFT_SCLK 14
 *   #define TFT_CS 15    #define TFT_DC 2     #define TFT_RST -1
 *   #define TFT_BL 21    #define TFT_BACKLIGHT_ON HIGH
 *   #define SPI_FREQUENCY 55000000
 *   #define SPI_READ_FREQUENCY 20000000
 *   #define SPI_TOUCH_FREQUENCY 2500000
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include "esp_random.h"
#include <WiFiUdp.h>
#include <FS.h>
#include <SD.h>
#include "esp_wifi.h"
#include <WebServer.h>

// ═══════════════════════════════════════════════════════════
//  USER CONFIG
// ═══════════════════════════════════════════════════════════
#define WIFI_SSID    "TellMyWifiLoveHer"           // your WiFi SSID
#define WIFI_PASS    "Oracle12!"           // your WiFi password
#define SCAN_SUBNET  "192.168.1"  // adjust to your network
#define SCAN_START   1
#define SCAN_END     254

// ═══════════════════════════════════════════════════════════
//  PINS
// ═══════════════════════════════════════════════════════════
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33
#define SD_CS         5
#define LED_R         4
#define LED_G        16
#define LED_B        17

// ═══════════════════════════════════════════════════════════
//  SCREEN
// ═══════════════════════════════════════════════════════════
#define SCREEN_W  240
#define SCREEN_H  320
#define TS_MINX   200
#define TS_MAXX  3900
#define TS_MINY   200
#define TS_MAXY  3900

// ═══════════════════════════════════════════════════════════
//  COLORS
// ═══════════════════════════════════════════════════════════
#define C_BG      TFT_BLACK
#define C_FG      TFT_WHITE
#define C_GREY    0x7BEF
#define C_DKGREY  0x39E7
#define C_ACCENT  0xFD20
#define C_RED     TFT_RED
#define C_GREEN   TFT_GREEN
#define C_YELLOW  TFT_YELLOW
#define C_CYAN    TFT_CYAN
// Depth palette for sprite
#define C_SHADOW  0x18C3   // very dark blue-grey (deep shadow)
#define C_MID     0x4A69   // mid grey-blue (base fill)
#define C_LIGHT   0x9CF3   // light blue-grey (lit surfaces)
#define C_HILITE  0xCE79   // near-white highlight
#define C_SKIN    0x9C73   // warm skin tone for face
#define C_SKIND   0x6B4A   // darker skin shadow
#define C_SKINH   0xCE96   // skin highlight
#define C_HORNC   0xFFFF   // horn white
#define C_HORND   0xB5B6   // horn shadow grey
#define C_BEARDS  0x528A   // beard steel-blue mid
#define C_BEARDD  0x2965   // beard shadow
#define C_BEARDH  0xAD75   // beard highlight
#define C_BARREL  0x4208   // dark wood barrel
#define C_BARRELH 0x8410   // barrel highlight plank
#define C_METAL   0xC540   // helmet gold mid
#define C_METALD  0x8400   // helmet gold dark
#define C_METALH  0xFF80   // helmet gold highlight
#define C_EYEWHT  0xFFFF   // eye white
#define C_EYEDRK  0x0841   // eye dark iris
#define C_EYEACC  0xFD20   // eye orange accent ring
#define C_WARPNT  0x9800   // war paint / scar — dark blood red
#define C_RUNE    0x528A   // rune engraving on helmet — darker than metal
#define C_FUR     0x6200   // fur brown mid
#define C_FURH    0xA540   // fur brown highlight
#define C_FURD    0x3000   // fur brown shadow
#define C_AXE     0x4A69   // axe head metal — matches armour
#define C_WARPNT  0x8800   // war paint / scar dark red
#define C_WARPNH  0xC8A2   // war paint highlight edge
#define C_FUR     0x6B4A   // fur collar dark base
#define C_FURH    0xAD96   // fur collar highlight
#define C_FURL    0xCEB6   // fur collar light tips
#define C_RUNEC   0x528A   // helmet rune engraving colour
#define C_AXEH    0xB5B6   // axe blade highlight
#define C_AXED    0x2104   // axe blade shadow
#define C_AXEW    0x6B4A   // axe haft (wood handle)

// ═══════════════════════════════════════════════════════════
//  LAYOUT
// ═══════════════════════════════════════════════════════════
#define HDR_Y      0
#define HDR_H     28
#define STATS1_Y  28
#define STATS1_H  34
#define STATS2_Y  62
#define STATS2_H  34
#define ACTION_Y  96
#define ACTION_H  36
#define COMMENT_Y 132
#define COMMENT_H  42
#define DIV_Y     174
#define DIV_H       6
#define BOT_Y     180
#define BOT_H     140
#define FCX  (SCREEN_W / 2)
#define FCY  (BOT_Y + 72)

// ═══════════════════════════════════════════════════════════
//  PORTS
// ═══════════════════════════════════════════════════════════
const uint16_t PORTS[] = {
  21,22,23,25,53,80,110,135,139,143,443,445,3306,3389,5900,8080,8443
};
const char* PORT_NAMES[] = {
  "FTP","SSH","Telnet","SMTP","DNS","HTTP","POP3","RPC",
  "NetBIOS","IMAP","HTTPS","SMB","MySQL","RDP","VNC","HTTP-Alt","HTTPS-Alt"
};
#define NUM_PORTS (sizeof(PORTS)/sizeof(PORTS[0]))
#define CONNECT_TIMEOUT_MS 150
#define BANNER_TIMEOUT_MS  400

// ═══════════════════════════════════════════════════════════
//  OBJECTS
// ═══════════════════════════════════════════════════════════
TFT_eSPI            tft      = TFT_eSPI();
TFT_eSprite         sprite   = TFT_eSprite(&tft);
SPIClass            touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
WebServer           webServer(80);
WiFiUDP             udp;

// ═══════════════════════════════════════════════════════════
//  FINDINGS — stored in RAM, served via web UI, logged to SD
// ═══════════════════════════════════════════════════════════
#define MAX_FINDINGS 128
struct Finding {
  char ip[16];
  char hostname[32];
  uint16_t port;
  char service[16];
  char banner[64];
  unsigned long ts_ms;
};
Finding findings[MAX_FINDINGS];
int findingCount = 0;

// ═══════════════════════════════════════════════════════════
//  GAME / SCAN STATE
// ═══════════════════════════════════════════════════════════
int  statTargets  = 0;
int  statVulns    = 0;
int  statCreds    = 0;
int  statAttacks  = 0;
int  statZombies  = 0;   // deauth events detected
int  statData     = 0;   // banners grabbed
int  statScore    = 0;
int  statLevel    = 1;
int  statAlive    = 0;
int  statDefense  = 0;
char currentAction[32] = "Idle";
char wifiName[32]      = "Not connected";
int  wifiSignal        = 0;
char comment[96]       = "Sigurd awakens. Fear the network.";

#define MAX_HOSTS 32
char hostList[MAX_HOSTS][16];
char hostnames[MAX_HOSTS][32];
int  hostCount = 0;

bool sdAvailable   = false;
bool deauthRunning = false;
int  deauthCount   = 0;
bool          monitorRunning   = false;
unsigned long lastMonitorMs    = 0;
#define MONITOR_INTERVAL_MS 180000
int           alertFlashCount  = 0;
unsigned long lastAlertFlashMs = 0;
#define RSSI_HISTORY_LEN 30
#define RSSI_AP_MAX       4
struct RssiTrack {
  char   ssid[33];
  int8_t history[RSSI_HISTORY_LEN];
  int    head;
  int    count;
};
RssiTrack rssiTracks[RSSI_AP_MAX];
int       rssiTrackCount = 0;
bool      showRssiGraph  = false;
#define KNOWN_MAX 64
char knownHosts[KNOWN_MAX][16];
int  knownCount = 0;

// ═══════════════════════════════════════════════════════════
//  ANIMATION
// ═══════════════════════════════════════════════════════════
enum AnimState { ANIM_IDLE, ANIM_SCAN, ANIM_FOUND, ANIM_ATTACK, ANIM_TALK };
AnimState     animState      = ANIM_IDLE;
AnimState     lastDrawnState = ANIM_IDLE;
bool          eyeOpen        = true;
int           eyeOffsetX     = 0;
int           breathOffset   = 0;   // 0 or 1 — subtle moustache breathing
int           animFrame      = 0;
unsigned long lastBlinkMs    = 0;
unsigned long lastAnimMs     = 0;
unsigned long lastTouchMs    = 0;
unsigned long lastUpdateMs   = 0;
unsigned long lastCommentMs  = 0;
#define COMMENT_INTERVAL_MS 14000

#define SPR_W  120
#define SPR_H  130
#define SPR_X  (FCX - SPR_W/2)
#define SPR_Y  (BOT_Y + 10)

// ═══════════════════════════════════════════════════════════
//  SD LOGGING
// ═══════════════════════════════════════════════════════════
void sdLog(const char* line) {
  if (!sdAvailable) return;
  File f = SD.open("/sigurd.log", FILE_APPEND);
  if (!f) return;
  f.println(line);
  f.close();
}

void sdLogFinding(const Finding& f) {
  if (!sdAvailable) return;
  char buf[160];
  snprintf(buf, sizeof(buf), "%lu,%s,%s,%d,%s,%s",
    f.ts_ms, f.ip, f.hostname, f.port, f.service, f.banner);
  sdLog(buf);
}

void addFinding(const char* ip, const char* hostname,
                uint16_t port, const char* service, const char* banner) {
  if (findingCount >= MAX_FINDINGS) return;
  Finding& f   = findings[findingCount++];
  strncpy(f.ip,       ip,       15);
  strncpy(f.hostname, hostname, 31);
  f.port = port;
  strncpy(f.service,  service,  15);
  strncpy(f.banner,   banner,   63);
  f.ts_ms = millis();
  sdLogFinding(f);
  statVulns++;
  statScore += 15;
  statAlive += 2;
  statLevel  = 1 + (statScore / 100);
}

// ═══════════════════════════════════════════════════════════
//  BANNER GRABBING
// ═══════════════════════════════════════════════════════════
// Connects to port, reads first response line as banner
String grabBanner(const char* ip, uint16_t port) {
  WiFiClient c;
  c.setTimeout(BANNER_TIMEOUT_MS);
  if (!c.connect(ip, port)) return "";

  // For HTTP ports grab title instead of banner
  if (port == 80 || port == 8080) {
    c.stop();
    String title = grabHttpTitle(ip, port);
    if (title.length() > 0) return title;
    // fall through to banner grab if no title
    if (!c.connect(ip, port)) return "";
  }
  if (port == 80 || port == 8080 || port == 443 || port == 8443) {
    c.print("HEAD / HTTP/1.0\r\nHost: ");
    c.print(ip);
    c.print("\r\n\r\n");
  }

  unsigned long t = millis();
  while (!c.available() && millis() - t < BANNER_TIMEOUT_MS) delay(10);

  String banner = "";
  while (c.available() && banner.length() < 63) {
    char ch = c.read();
    if (ch == '\r' || ch == '\n') break;
    banner += ch;
  }
  c.stop();
  banner.trim();
  return banner;
}

// ═══════════════════════════════════════════════════════════
//  mDNS / NetBIOS HOSTNAME RESOLUTION
// ═══════════════════════════════════════════════════════════
// NetBIOS name query — UDP port 137
// Sends a broadcast name query and listens for response
String resolveHostname(const char* ip) {
  // NetBIOS node status request
  uint8_t nbns[] = {
    0xA2,0x48,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x20,0x43,0x4B,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x00,
    0x00,0x21,0x00,0x01
  };

  udp.begin(13700);
  IPAddress target;
  target.fromString(ip);
  udp.beginPacket(target, 137);
  udp.write(nbns, sizeof(nbns));
  udp.endPacket();

  unsigned long t = millis();
  while (millis() - t < 500) {
    int len = udp.parsePacket();
    if (len > 56) {
      uint8_t buf[256];
      udp.read(buf, min(len, 255));
      udp.stop();
      // Name starts at offset 57, 15 chars, space-padded
      char name[16] = {0};
      for (int i = 0; i < 15; i++) {
        char ch = (char)buf[57 + i];
        if (ch == ' ' || ch == '\0') break;
        name[i] = ch;
      }
      if (strlen(name) > 0) return String(name);
    }
    delay(10);
  }
  udp.stop();

  // Fallback: try mDNS .local reverse lookup via DNS
  // Build reverse lookup name e.g. 1.1.168.192.in-addr.arpa
  IPAddress addr; addr.fromString(ip);
  char rev[64];
  snprintf(rev, sizeof(rev), "%d.%d.%d.%d.in-addr.arpa",
    addr[3], addr[2], addr[1], addr[0]);
  // Simple DNS PTR query would need a full resolver — skip for now
  // Return empty if nothing found
  return "";
}

// ═══════════════════════════════════════════════════════════
//  DEAUTH DETECTION — promiscuous mode 802.11 sniffer
// ═══════════════════════════════════════════════════════════
#define DEAUTH_LOG_MAX 32
struct DeauthEvent {
  uint8_t src[6];
  uint8_t dst[6];
  int8_t  rssi;
  unsigned long ts_ms;
};
DeauthEvent deauthLog[DEAUTH_LOG_MAX];
int deauthLogCount = 0;

// Promiscuous callback — runs in WiFi task context
void IRAM_ATTR snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  uint8_t* payload = pkt->payload;
  uint8_t  frameType = payload[0];
  // Deauth = 0xC0, Disassoc = 0xA0
  if (frameType != 0xC0 && frameType != 0xA0) return;

  deauthCount++;
  if (deauthLogCount < DEAUTH_LOG_MAX) {
    DeauthEvent& e = deauthLog[deauthLogCount++];
    memcpy(e.dst, payload + 4,  6);
    memcpy(e.src, payload + 10, 6);
    e.rssi   = pkt->rx_ctrl.rssi;
    e.ts_ms  = millis();
    statZombies = deauthCount;
    statScore  += 5;
  }
}

void startDeauthDetection() {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);
  deauthRunning = true;
  char buf[48];
  snprintf(buf, sizeof(buf), "Deauth detector active.");
  strncpy(comment, buf, 95);
  sdLog("# Deauth detection started");
}

void stopDeauthDetection() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
  deauthRunning = false;
  char buf[64];
  snprintf(buf, sizeof(buf), "Deauth stopped. %d events.", deauthCount);
  strncpy(comment, buf, 95);
}

// ═══════════════════════════════════════════════════════════
//  WEB UI
// ═══════════════════════════════════════════════════════════
// Builds and serves the findings page at http://<ip>/

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════
void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_R, r ? LOW : HIGH);
  digitalWrite(LED_G, g ? LOW : HIGH);
  digitalWrite(LED_B, b ? LOW : HIGH);
}

void updateWifiInfo() {
  if (WiFi.status() == WL_CONNECTED) {
    strncpy(wifiName, WiFi.SSID().c_str(), 31);
    wifiSignal = WiFi.RSSI();
  } else {
    strncpy(wifiName, "Not connected", 31);
    wifiSignal = 0;
  }
}

// ═══════════════════════════════════════════════════════════
//  SPRITE — face drawn into offscreen buffer, pushed once
// ═══════════════════════════════════════════════════════════
void drawFaceSprite() {
  int cx = SPR_W / 2;   // 60
  int cy = 68;          // face centre in sprite
  sprite.fillSprite(C_BG);

  // (axe drawn in-hand with right arm below)

  // ── FUR COLLAR — sits above barrel, below face ────────────
  // Shadow base — wide ellipse
  sprite.fillEllipse(cx, cy+20, 38, 10, C_FURD);
  // Main fur mass
  sprite.fillEllipse(cx, cy+18, 36,  9, C_FUR);
  // Fur tufts — series of small bumps along top edge
  for(int f=-32;f<=32;f+=8){
    int fh = 5 - abs(f)/12;  // tufts shorter at edges
    sprite.fillEllipse(cx+f, cy+13, 4, fh+2, C_FUR);
    sprite.fillEllipse(cx+f, cy+13, 3, fh,   C_FURH);
  }
  // Highlight across top of collar
  sprite.fillEllipse(cx-4, cy+14, 18, 4, C_FURH);
  // Inner shadow where fur meets body
  sprite.drawEllipse(cx, cy+18, 36, 9, C_FURD);

  // ── BARREL — dark wood with depth ─────────────────────────
  sprite.fillRoundRect(cx-27,cy+20,54,34,5,C_SHADOW);
  sprite.fillRoundRect(cx-26,cy+19,52,34,5,C_BARREL);
  for(int p=-18;p<=18;p+=9)
    sprite.drawLine(cx+p, cy+22, cx+p, cy+51, C_BARRELH);
  sprite.drawRoundRect(cx-26,cy+19,52,34,5,C_METALH);
  sprite.fillRect(cx-26,cy+26,52,4,C_MID);
  sprite.drawFastHLine(cx-26,cy+26,52,C_METALH);
  sprite.drawFastHLine(cx-26,cy+29,52,C_METALD);
  sprite.fillRect(cx-26,cy+42,52,4,C_MID);
  sprite.drawFastHLine(cx-26,cy+42,52,C_METALH);
  sprite.drawFastHLine(cx-26,cy+45,52,C_METALD);
  sprite.drawFastVLine(cx-26,cy+20,33,C_METALH);

  // ── BEARD ─────────────────────────────────────────────────
  sprite.fillEllipse(cx,   cy+17, 26, 18, C_BEARDD);
  sprite.fillEllipse(cx-10,cy+21, 10,  9, C_BEARDD);
  sprite.fillEllipse(cx+10,cy+21, 10,  9, C_BEARDD);
  sprite.fillEllipse(cx,   cy+15, 24, 17, C_BEARDS);
  sprite.fillEllipse(cx-9, cy+19,  9,  8, C_BEARDS);
  sprite.fillEllipse(cx+9, cy+19,  9,  8, C_BEARDS);
  sprite.fillEllipse(cx,   cy+12, 12,  7, C_BEARDH);
  for(int b=-18;b<=18;b+=5)
    sprite.drawLine(cx+b, cy+8, cx+b+(b/3), cy+30, C_BEARDD);
  sprite.drawEllipse(cx, cy+15, 24, 17, C_METALD);

  // ── FACE ──────────────────────────────────────────────────
  sprite.fillCircle(cx, cy, 26, C_SKIND);
  sprite.fillCircle(cx, cy, 24, C_SKIN);
  sprite.fillCircle(cx-6, cy-8, 12, C_SKINH);
  sprite.fillCircle(cx+2, cy+2, 16, C_SKIN);
  sprite.drawCircle(cx, cy, 25, C_SKIND);
  // Cheeks
  sprite.fillCircle(cx-16, cy+6, 8, C_SKIN);
  sprite.fillCircle(cx+16, cy+6, 8, C_SKIN);
  sprite.fillCircle(cx-18, cy+4, 4, C_SKINH);
  sprite.fillCircle(cx+18, cy+4, 4, C_SKINH);
  sprite.drawCircle(cx-16, cy+6, 8, C_SKIND);
  sprite.drawCircle(cx+16, cy+6, 8, C_SKIND);

  // ── SCAR — jagged slash across left cheek ─────────────────
  // Three-segment jagged line in dark blood red
  // Starts above left cheek, zigs down through it
  sprite.drawLine(cx-20, cy-2,  cx-14, cy+2,  C_WARPNT);
  sprite.drawLine(cx-14, cy+2,  cx-17, cy+5,  C_WARPNT);
  sprite.drawLine(cx-17, cy+5,  cx-11, cy+10, C_WARPNT);
  // Slightly lighter second pass for depth
  sprite.drawLine(cx-19, cy-2,  cx-13, cy+2,  C_SKIND);
  sprite.drawLine(cx-16, cy+5,  cx-10, cy+10, C_SKIND);

  // ── NOSE ──────────────────────────────────────────────────
  sprite.fillCircle(cx, cy+5, 5, C_SKIND);
  sprite.fillCircle(cx, cy+4, 4, C_SKIN);
  sprite.fillCircle(cx-1,cy+3, 2, C_SKINH);
  sprite.fillCircle(cx-2,cy+6, 1, C_SHADOW);
  sprite.fillCircle(cx+2,cy+6, 1, C_SHADOW);

  // ── MOUTH ─────────────────────────────────────────────────
  int mo = breathOffset;
  if(animState==ANIM_ATTACK){
    sprite.drawLine(cx-8,cy+13,cx-2,cy+10,C_SKIND);
    sprite.drawLine(cx+8,cy+13,cx+2,cy+10,C_SKIND);
    sprite.drawLine(cx-2,cy+10,cx+2,cy+10,C_SKIND);
    sprite.fillRect(cx-5,cy+9,10,3,C_EYEWHT);
    for(int t=-4;t<=4;t+=4)
      sprite.drawLine(cx+t,cy+10,cx+t,cy+13,C_SKIND);
  } else if(animState==ANIM_FOUND){
    sprite.drawLine(cx-8,cy+12,cx-2,cy+16,C_SKIND);
    sprite.drawLine(cx+8,cy+12,cx+2,cy+16,C_SKIND);
    sprite.drawLine(cx-2,cy+16,cx+2,cy+16,C_SKIND);
  } else {
    sprite.fillEllipse(cx-8, cy+11+mo, 7, 4, C_BEARDD);
    sprite.fillEllipse(cx+8, cy+11+mo, 7, 4, C_BEARDD);
    sprite.fillEllipse(cx-8, cy+10+mo, 6, 3, C_BEARDS);
    sprite.fillEllipse(cx+8, cy+10+mo, 6, 3, C_BEARDS);
  }

  // ── EYES ──────────────────────────────────────────────────
  int lx=cx-11+eyeOffsetX, rx=cx+11+eyeOffsetX, ey=cy-7;
  if(!eyeOpen){
    sprite.fillEllipse(lx, ey, 9, 5, C_SKIN);
    sprite.drawFastHLine(lx-8, ey,   16, C_SKIND);
    sprite.drawFastHLine(lx-7, ey+1, 14, C_SKIND);
    sprite.fillEllipse(rx, ey, 9, 5, C_SKIN);
    sprite.drawFastHLine(rx-8, ey,   16, C_SKIND);
    sprite.drawFastHLine(rx-7, ey+1, 14, C_SKIND);
  } else {
    // Left
    sprite.fillCircle(lx, ey, 9, C_SHADOW);
    sprite.fillCircle(lx, ey, 8, C_EYEWHT);
    sprite.fillCircle(lx, ey, 6, C_MID);
    sprite.fillCircle(lx, ey, 5, C_EYEDRK);
    sprite.drawCircle(lx, ey, 5, C_EYEACC);
    sprite.fillCircle(lx, ey, 3, C_EYEDRK);
    sprite.drawCircle(lx, ey, 3, C_LIGHT);
    sprite.drawCircle(lx, ey, 1, C_LIGHT);
    sprite.fillCircle(lx, ey, 1, C_SHADOW);
    sprite.fillCircle(lx-3, ey-3, 2, C_HILITE);
    sprite.drawCircle(lx, ey, 8, C_SKIND);
    // Right
    sprite.fillCircle(rx, ey, 9, C_SHADOW);
    sprite.fillCircle(rx, ey, 8, C_EYEWHT);
    sprite.fillCircle(rx, ey, 6, C_MID);
    sprite.fillCircle(rx, ey, 5, C_EYEDRK);
    sprite.drawCircle(rx, ey, 5, C_EYEACC);
    sprite.fillCircle(rx, ey, 3, C_EYEDRK);
    sprite.drawCircle(rx, ey, 3, C_LIGHT);
    sprite.drawCircle(rx, ey, 1, C_LIGHT);
    sprite.fillCircle(rx, ey, 1, C_SHADOW);
    sprite.fillCircle(rx-3, ey-3, 2, C_HILITE);
    sprite.drawCircle(rx, ey, 8, C_SKIND);
  }

  // ── EYEBROWS ──────────────────────────────────────────────
  if(animState==ANIM_ATTACK){
    sprite.fillTriangle(cx-20,cy-14, cx-8,cy-19, cx-8,cy-14, C_BEARDD);
    sprite.fillTriangle(cx+20,cy-14, cx+8,cy-19, cx+8,cy-14, C_BEARDD);
    sprite.drawLine(cx-20,cy-14, cx-8,cy-19, C_BEARDS);
    sprite.drawLine(cx+20,cy-14, cx+8,cy-19, C_BEARDS);
  } else if(animState==ANIM_FOUND){
    sprite.fillRect(cx-20,cy-23,13,5,C_BEARDD);
    sprite.fillRect(cx+7, cy-23,13,5,C_BEARDD);
    sprite.drawLine(cx-20,cy-23,cx-8,cy-25,C_BEARDS);
    sprite.drawLine(cx+20,cy-23,cx+8,cy-25,C_BEARDS);
  } else if(animState==ANIM_SCAN){
    sprite.fillRect(cx-20,cy-23,13,5,C_BEARDD);
    sprite.fillRect(cx+7, cy-17,13,5,C_BEARDD);
    sprite.drawLine(cx-20,cy-23,cx-8,cy-21,C_BEARDS);
    sprite.drawLine(cx+20,cy-17,cx+8,cy-19,C_BEARDS);
  } else {
    sprite.fillRect(cx-20,cy-19,13,5,C_BEARDD);
    sprite.fillRect(cx+7, cy-19,13,5,C_BEARDD);
    sprite.drawLine(cx-20,cy-18,cx-8,cy-20,C_BEARDS);
    sprite.drawLine(cx+20,cy-18,cx+8,cy-20,C_BEARDS);
  }

  // ── HELMET ────────────────────────────────────────────────
  // Shadow base dome
  sprite.fillEllipse(cx, cy-21, 25, 15, C_METALD);
  // Main dome
  sprite.fillEllipse(cx, cy-21, 23, 14, C_METAL);
  // Highlight band
  sprite.fillEllipse(cx-4, cy-26, 12, 5, C_METALH);
  sprite.drawEllipse(cx, cy-21, 23, 14, C_METALD);

  // ── RUNE ENGRAVINGS on dome ────────────────────────────────
  // Three rune-like symbols etched into the helmet — darker than metal
  // Rune 1 (left of centre) — Tiwaz ↑
  sprite.drawFastVLine(cx-10, cy-30, 8, C_RUNE);
  sprite.drawLine(cx-10, cy-30, cx-13, cy-26, C_RUNE);
  sprite.drawLine(cx-10, cy-30, cx-7,  cy-26, C_RUNE);
  // Rune 2 (centre) — Algiz ψ
  sprite.drawFastVLine(cx, cy-31, 8, C_RUNE);
  sprite.drawLine(cx, cy-31, cx-3, cy-27, C_RUNE);
  sprite.drawLine(cx, cy-31, cx+3, cy-27, C_RUNE);
  sprite.drawLine(cx, cy-27, cx-3, cy-23, C_RUNE);
  sprite.drawLine(cx, cy-27, cx+3, cy-23, C_RUNE);
  // Rune 3 (right of centre) — Sowilo zig
  sprite.drawLine(cx+8,  cy-30, cx+11, cy-27, C_RUNE);
  sprite.drawLine(cx+11, cy-27, cx+8,  cy-24, C_RUNE);
  sprite.drawLine(cx+8,  cy-24, cx+11, cy-21, C_RUNE);

  // Brim
  sprite.fillRect(cx-27,cy-14,54,7,C_METALD);
  sprite.fillRect(cx-26,cy-14,52,6,C_METAL);
  sprite.drawFastHLine(cx-26,cy-14,52,C_METALH);
  sprite.drawFastHLine(cx-26,cy-9, 52,C_METALD);
  // Rivets
  for(int r=-20;r<=20;r+=10){
    sprite.fillCircle(cx+r, cy-11, 3, C_METALD);
    sprite.fillCircle(cx+r, cy-11, 2, C_METALH);
  }
  // Nasal bar
  sprite.fillRect(cx-3,cy-14,6,15,C_METALD);
  sprite.fillRect(cx-2,cy-14,4,15,C_METAL);
  sprite.drawFastVLine(cx-2,cy-14,15,C_METALH);

  // ── HORNS — tapered to a point ────────────────────────────
  uint16_t hornFill = C_HORNC;
  uint16_t hornDark = C_HORND;
  uint16_t hornHigh = C_METALH;
  // Left horn
  for(int i=0;i<=20;i++){
    float t = i / 20.0f;
    int bw = (int)(12*(1.0f-t));
    int hx = cx-20 + (int)((-30)*t);
    int hy = cy-18 + (int)((-38)*t);
    if(bw<=0){ sprite.drawPixel(hx, hy, hornHigh); }
    else {
      uint16_t c = (i<5)?hornDark:(i>16)?hornHigh:hornFill;
      sprite.drawFastHLine(hx-bw/2, hy, bw, c);
      sprite.drawPixel(hx+bw/2, hy, hornDark);
      sprite.drawPixel(hx-bw/2, hy, hornHigh);
    }
  }
  sprite.drawLine(cx-14,cy-16,cx-50,cy-56,hornDark);
  sprite.drawLine(cx-26,cy-16,cx-50,cy-56,hornDark);
  // Right horn
  for(int i=0;i<=20;i++){
    float t = i / 20.0f;
    int bw = (int)(12*(1.0f-t));
    int hx = cx+20 + (int)(30*t);
    int hy = cy-18 + (int)((-38)*t);
    if(bw<=0){ sprite.drawPixel(hx, hy, hornHigh); }
    else {
      uint16_t c = (i<5)?hornDark:(i>16)?hornHigh:hornFill;
      sprite.drawFastHLine(hx-bw/2, hy, bw, c);
      sprite.drawPixel(hx+bw/2, hy, hornDark);
      sprite.drawPixel(hx-bw/2, hy, hornHigh);
    }
  }
  sprite.drawLine(cx+14,cy-16,cx+50,cy-56,hornDark);
  sprite.drawLine(cx+26,cy-16,cx+50,cy-56,hornDark);
  // Horn rings
  sprite.drawLine(cx-28,cy-28,cx-36,cy-28,C_METAL);
  sprite.drawLine(cx-29,cy-29,cx-37,cy-29,C_METALH);
  sprite.drawLine(cx+28,cy-28,cx+36,cy-28,C_METAL);
  sprite.drawLine(cx+29,cy-29,cx+37,cy-29,C_METALH);

  // ── ARMS — armoured sleeves (drawn over fur) ───────────────
  int armY=(animState==ANIM_ATTACK)?cy+10:cy+18;
  // Left arm
  sprite.fillRoundRect(cx-44,armY+2,15,22,4,C_SHADOW);
  sprite.fillRoundRect(cx-43,armY,  14,22,4,C_MID);
  sprite.drawFastVLine(cx-43,armY+2,18,C_LIGHT);
  // Bracer bands on left arm
  sprite.drawFastHLine(cx-43,armY+6, 14,C_METALH);
  sprite.drawFastHLine(cx-43,armY+14,14,C_METALH);
  sprite.drawRoundRect(cx-43,armY,14,22,4,C_METALD);
  // Left fist
  sprite.fillCircle(cx-36,armY+25,7,C_SKIND);
  sprite.fillCircle(cx-37,armY+24,6,C_SKIN);
  // Knuckle lines
  for(int k=-3;k<=3;k+=2)
    sprite.drawLine(cx-36+k,armY+20,cx-36+k,armY+23,C_SKIND);
  sprite.fillCircle(cx-39,armY+23,2,C_SKINH);
  sprite.drawCircle(cx-36,armY+25,7,C_METALD);
  // ── AXEHEAD — drawn before arm so arm overlaps it ──────────
  // Fist centre: (cx+36, armY+25)
  // Haft runs vertically through fist, blade sits above fist
  // Haft — short vertical segment above fist, gripped at fist level
  int hx = cx+36;           // haft x centre (aligns with fist)
  int haftTop  = armY+4;    // top of visible haft above fist
  int haftGrip = armY+22;   // where fist grips it
  // Haft body — two-tone wood
  sprite.fillRect(hx-2, haftTop, 4, haftGrip-haftTop+6, C_AXEW);
  sprite.drawFastVLine(hx-2, haftTop, haftGrip-haftTop+6, C_METALH); // left highlight
  sprite.drawFastVLine(hx+1, haftTop, haftGrip-haftTop+6, C_METALD); // right shadow
  // Axe head — single-bit crescent blade above the fist, pointing right
  // Blade socket at haft top
  int bx = hx;
  int by = haftTop;
  // Shadow behind blade
  sprite.fillTriangle(bx,   by,    bx+16, by-14, bx+16, by+8,  C_AXED);
  // Main blade fill
  sprite.fillTriangle(bx,   by,    bx+14, by-12, bx+14, by+7,  C_AXE);
  // Blade face highlight — the ground edge
  sprite.drawLine(bx+14, by-12, bx+14, by+7, C_AXEH);
  // Upper and lower blade curves
  sprite.drawLine(bx,    by,    bx+14, by-12, C_METALD);
  sprite.drawLine(bx,    by,    bx+14, by+7,  C_METALD);
  // Cheek of axe — circular detail on flat
  sprite.fillCircle(bx+5, by-2, 4, C_MID);
  sprite.drawCircle(bx+5, by-2, 4, C_AXED);
  sprite.fillCircle(bx+5, by-2, 2, C_METALH); // eye/rivet

  // ── RIGHT ARM ────────────────────────────────────────────
  sprite.fillRoundRect(cx+29,armY+2,15,22,4,C_SHADOW);
  sprite.fillRoundRect(cx+29,armY,  14,22,4,C_MID);
  sprite.drawFastVLine(cx+29,armY+2,18,C_LIGHT);
  // Bracer bands
  sprite.drawFastHLine(cx+29,armY+6, 14,C_METALH);
  sprite.drawFastHLine(cx+29,armY+14,14,C_METALH);
  sprite.drawRoundRect(cx+29,armY,14,22,4,C_METALD);
  // Right fist — gripping the haft
  sprite.fillCircle(cx+36,armY+25,7,C_SKIND);
  sprite.fillCircle(cx+35,armY+24,6,C_SKIN);
  // Finger wrap lines around the haft
  for(int k=-3;k<=3;k+=2)
    sprite.drawLine(cx+36+k,armY+20,cx+36+k,armY+25,C_SKIND);
  // Thumb over haft
  sprite.fillEllipse(cx+30,armY+23,4,3,C_SKIN);
  sprite.drawEllipse(cx+30,armY+23,4,3,C_SKIND);
  sprite.fillCircle(cx+33,armY+23,2,C_SKINH);
  sprite.drawCircle(cx+36,armY+25,7,C_METALD);
  // Haft visible below fist (pommel end)
  sprite.fillRect(hx-2, armY+28, 4, 6, C_AXEW);
  sprite.fillCircle(hx, armY+34, 3, C_METALD); // pommel cap
  sprite.fillCircle(hx, armY+34, 2, C_METALH);

  // ── Push to screen in one DMA transfer ────────────────────
  sprite.pushSprite(SPR_X, SPR_Y);
}

// ═══════════════════════════════════════════════════════════
//  ICONS
// ═══════════════════════════════════════════════════════════
void drawIconTarget(int x,int y){tft.drawCircle(x+8,y+8,8,C_FG);tft.drawCircle(x+8,y+8,4,C_FG);tft.drawFastHLine(x,y+8,16,C_FG);tft.drawFastVLine(x+8,y,16,C_FG);}
void drawIconShield(int x,int y){tft.fillTriangle(x+8,y+16,x,y,x+16,y,C_DKGREY);tft.drawTriangle(x+8,y+16,x,y,x+16,y,C_FG);tft.fillRect(x+2,y,12,10,C_DKGREY);tft.drawRect(x+2,y,12,10,C_FG);tft.drawLine(x+8,y+2,x+8,y+14,C_FG);tft.drawLine(x+2,y+6,x+14,y+6,C_FG);}
void drawIconKey(int x,int y){tft.drawCircle(x+5,y+5,5,C_FG);tft.drawLine(x+9,y+9,x+16,y+16,C_FG);tft.drawLine(x+13,y+13,x+16,y+10,C_FG);}
void drawIconSword(int x,int y){tft.drawLine(x+2,y+14,x+14,y+2,C_FG);tft.drawLine(x+3,y+14,x+14,y+3,C_FG);tft.drawLine(x+8,y+10,x+12,y+14,C_FG);tft.drawLine(x+8,y+10,x+4,y+14,C_FG);tft.fillCircle(x+3,y+13,2,C_FG);}
void drawIconSkull(int x,int y){tft.fillCircle(x+8,y+7,6,C_DKGREY);tft.drawCircle(x+8,y+7,6,C_FG);tft.fillRect(x+4,y+11,8,5,C_DKGREY);tft.drawRect(x+4,y+11,8,5,C_FG);tft.fillCircle(x+6,y+7,2,C_BG);tft.fillCircle(x+10,y+7,2,C_BG);tft.drawLine(x+6,y+13,x+6,y+16,C_FG);tft.drawLine(x+10,y+13,x+10,y+16,C_FG);tft.drawLine(x+8,y+13,x+8,y+16,C_FG);}
void drawIconData(int x,int y){tft.fillEllipse(x+8,y+3,7,3,C_DKGREY);tft.drawEllipse(x+8,y+3,7,3,C_FG);tft.fillRect(x+1,y+3,14,8,C_DKGREY);tft.drawLine(x+1,y+11,x+1,y+3,C_FG);tft.drawLine(x+15,y+11,x+15,y+3,C_FG);tft.fillEllipse(x+8,y+11,7,3,C_DKGREY);tft.drawEllipse(x+8,y+11,7,3,C_FG);tft.fillEllipse(x+8,y+7,7,3,C_DKGREY);tft.drawEllipse(x+8,y+7,7,3,C_FG);}
void drawIconCoin(int x,int y){tft.fillCircle(x+8,y+8,8,C_YELLOW);tft.drawCircle(x+8,y+8,8,C_FG);tft.setTextColor(C_BG,C_YELLOW);tft.setTextSize(1);tft.setTextDatum(MC_DATUM);tft.drawString("$",x+8,y+8);tft.setTextDatum(TL_DATUM);}
void drawIconAttack(int x,int y){tft.drawCircle(x+8,y+5,4,C_FG);tft.fillCircle(x+6,y+4,1,C_FG);tft.fillCircle(x+10,y+4,1,C_FG);tft.drawLine(x+2,y+10,x+14,y+16,C_FG);tft.drawLine(x+14,y+10,x+2,y+16,C_FG);tft.fillCircle(x+2,y+10,2,C_FG);tft.fillCircle(x+14,y+16,2,C_FG);tft.fillCircle(x+14,y+10,2,C_FG);tft.fillCircle(x+2,y+16,2,C_FG);}
void drawIconAxes(int x,int y){tft.drawLine(x+2,y+14,x+14,y+2,C_FG);tft.fillTriangle(x+2,y+2,x+8,y+2,x+2,y+8,C_FG);tft.drawLine(x+14,y+14,x+2,y+2,C_GREY);tft.fillTriangle(x+14,y+14,x+8,y+14,x+14,y+8,C_GREY);}
void drawVikingChar(int x,int y){tft.fillRect(x+4,y+12,12,14,C_GREY);tft.drawRect(x+4,y+12,12,14,C_FG);tft.fillCircle(x+10,y+8,7,C_DKGREY);tft.drawCircle(x+10,y+8,7,C_FG);tft.fillRect(x+4,y+2,12,6,C_GREY);tft.drawRect(x+4,y+2,12,6,C_FG);tft.drawLine(x+4,y+2,x,y-4,C_FG);tft.drawLine(x+16,y+2,x+20,y-4,C_FG);tft.fillCircle(x+8,y+8,1,C_FG);tft.fillCircle(x+12,y+8,1,C_FG);tft.fillRect(x-6,y+12,8,10,C_DKGREY);tft.drawRect(x-6,y+12,8,10,C_FG);tft.drawLine(x+20,y+10,x+20,y+26,C_FG);tft.drawLine(x+17,y+14,x+23,y+14,C_FG);tft.fillRect(x+4,y+26,4,6,C_GREY);tft.fillRect(x+12,y+26,4,6,C_GREY);}
void drawWifiIcon(int x,int y,int rssi){int bars=rssi==0?0:rssi>-55?3:rssi>-70?2:1;tft.drawCircle(x+8,y+14,12,bars>=1?C_FG:C_GREY);tft.drawCircle(x+8,y+14,8,bars>=2?C_FG:C_GREY);tft.drawCircle(x+8,y+14,4,bars>=3?C_FG:C_GREY);tft.fillRect(x,y+14,16,8,C_BG);tft.fillCircle(x+8,y+14,2,bars>0?C_FG:C_GREY);}

// ═══════════════════════════════════════════════════════════
//  DRAW SCREEN
// ═══════════════════════════════════════════════════════════
void drawScreen() {
  tft.fillScreen(C_BG);

  // Header
  tft.fillRect(0,HDR_Y,SCREEN_W,HDR_H,C_BG);
  tft.drawFastHLine(0,HDR_H,SCREEN_W,C_GREY);
  // WiFi icon left
  drawWifiIcon(2,4,wifiSignal);
  char sig[8]; snprintf(sig,sizeof(sig),"%d",wifiSignal==0?0:abs(wifiSignal));
  tft.setTextColor(C_GREY,C_BG); tft.setTextSize(1); tft.drawString(sig,20,18);
  // SIGURD centred
  tft.setTextDatum(MC_DATUM); tft.setTextColor(C_FG,C_BG); tft.setTextSize(2);
  tft.drawString("SIGURD",SCREEN_W/2,HDR_H/2);
  // SD indicator right
  tft.setTextSize(1); tft.setTextDatum(MR_DATUM);
  tft.setTextColor(sdAvailable?C_GREEN:C_DKGREY,C_BG);
  tft.drawString(sdAvailable?"SD":"--",SCREEN_W-4,HDR_H/2);
  tft.setTextDatum(TL_DATUM);

  // Stats row 1
  tft.drawFastHLine(0,STATS1_Y+STATS1_H,SCREEN_W,C_GREY);
  int col=SCREEN_W/3;
  drawIconTarget(8,STATS1_Y+8);
  tft.setTextColor(C_FG,C_BG); tft.setTextSize(2); tft.setTextDatum(ML_DATUM);
  tft.drawString(String(statTargets).c_str(),28,STATS1_Y+18);
  tft.drawFastVLine(col,STATS1_Y,STATS1_H,C_GREY);
  drawIconShield(col+8,STATS1_Y+8);
  tft.drawString(String(statVulns).c_str(),col+28,STATS1_Y+18);
  tft.drawFastVLine(col*2,STATS1_Y,STATS1_H,C_GREY);
  drawIconKey(col*2+8,STATS1_Y+8);
  tft.drawString(String(statCreds).c_str(),col*2+28,STATS1_Y+18);
  tft.setTextDatum(TL_DATUM);

  // Stats row 2
  tft.drawFastHLine(0,STATS2_Y+STATS2_H,SCREEN_W,C_GREY);
  drawIconSword(8,STATS2_Y+8);
  tft.setTextColor(C_FG,C_BG); tft.setTextSize(2); tft.setTextDatum(ML_DATUM);
  tft.drawString(String(statAttacks).c_str(),28,STATS2_Y+18);
  tft.drawFastVLine(col,STATS2_Y,STATS2_H,C_GREY);
  drawIconSkull(col+6,STATS2_Y+7);
  tft.drawString(String(statZombies).c_str(),col+28,STATS2_Y+18);
  tft.drawFastVLine(col*2,STATS2_Y,STATS2_H,C_GREY);
  drawIconData(col*2+6,STATS2_Y+7);
  tft.drawString(String(statData).c_str(),col*2+28,STATS2_Y+18);
  tft.setTextDatum(TL_DATUM);

  // Action row
  tft.drawFastHLine(0,ACTION_Y+ACTION_H,SCREEN_W,C_GREY);
  drawVikingChar(4,ACTION_Y+2);
  tft.setTextColor(C_FG,C_BG); tft.setTextSize(1);
  tft.drawString(currentAction,34,ACTION_Y+4);
  tft.setTextColor(C_GREY,C_BG);
  char wbuf[48]; snprintf(wbuf,sizeof(wbuf),"WiFi: %.18s",wifiName);
  tft.drawString(wbuf,34,ACTION_Y+17);
  // IP address right-aligned, small
  if(WiFi.status()==WL_CONNECTED){
    tft.setTextColor(C_CYAN,C_BG); tft.setTextDatum(MR_DATUM);
    tft.drawString(WiFi.localIP().toString().c_str(),SCREEN_W-4,ACTION_Y+ACTION_H/2);
    tft.setTextDatum(TL_DATUM);
  }
  // Status indicators
  if(deauthRunning&&monitorRunning){
    tft.setTextColor(C_RED,C_BG); tft.drawString("DEAUTH+MON",34,ACTION_Y+28);
  } else if(deauthRunning){
    tft.setTextColor(C_RED,C_BG); tft.drawString("DEAUTH WATCH",34,ACTION_Y+28);
  } else if(monitorRunning){
    tft.setTextColor(C_GREEN,C_BG); tft.drawString("MONITORING",34,ACTION_Y+28);
  }

  // Comment area — use drawComment() so no fillRect flash
  // If RSSI graph is toggled on AND we have data, show graph instead
  tft.fillRect(0,COMMENT_Y,SCREEN_W,COMMENT_H,C_BG);  // one-time clear on full redraw only
  if(showRssiGraph && rssiTrackCount > 0){
    drawRssiGraph();
  } else {
    showRssiGraph = false;  // auto-off if no data
    drawComment();
  }

  // Divider
  tft.fillRect(0,DIV_Y,SCREEN_W,DIV_H,C_DKGREY);
  for(int x=0;x<SCREEN_W;x+=8){tft.fillRect(x,DIV_Y,4,4,C_FG);tft.fillRect(x+4,DIV_Y+4,4,4,C_FG);}

  // Bottom panel static — BOT_H=140, centred items
  // Left column: coin score (top) + level (bottom)
  drawIconCoin(6,BOT_Y+8);
  tft.setTextColor(C_YELLOW,C_BG); tft.setTextSize(2); tft.setTextDatum(MC_DATUM);
  tft.drawString(String(statScore).c_str(),18,BOT_Y+38);
  tft.setTextColor(C_FG,C_BG); tft.setTextSize(1);
  tft.drawString("LVL",14,BOT_Y+95);
  tft.setTextSize(2);
  tft.drawString(String(statLevel).c_str(),14,BOT_Y+110);
  // Right column: attack (top) + defense (bottom)
  drawIconAttack(SCREEN_W-34,BOT_Y+8);
  tft.setTextColor(C_FG,C_BG); tft.setTextSize(1);
  tft.setTextDatum(MR_DATUM);
  tft.drawString("A",SCREEN_W-4,BOT_Y+8);
  tft.setTextSize(2);
  tft.drawString(String(statAlive).c_str(),SCREEN_W-6,BOT_Y+38);
  drawIconAxes(SCREEN_W-34,BOT_Y+90);
  tft.setTextSize(2);
  tft.drawString(String(statDefense).c_str(),SCREEN_W-6,BOT_Y+112);
  tft.setTextDatum(TL_DATUM);

  drawFaceSprite();
}

// ═══════════════════════════════════════════════════════════
//  RANDOM COMMENT SYSTEM — state-driven pools
// ═══════════════════════════════════════════════════════════

// Idle — wit, personality, menace
const char* POOL_IDLE[] = {
  "Your firewall is a shield made of straw.",
  "I have pillaged stronger networks than yours.",
  "Odin sees all. So does Sigurd.",
  "The ravens have spotted your open ports.",
  "Oh look. Another default password.",
  "Silence is not safety. It is just a closed curtain.",
  "I do not knock. I walk straight through.",
  "Every network has a crack. I find them all.",
  "Your router called. It said goodbye.",
  "SMB wide open. How generous of you.",
};
#define POOL_IDLE_N 10

// Scanning — focused, hunting
const char* POOL_SCAN[] = {
  "Sweeping the seas. Nothing hides from Sigurd.",
  "The ravens are circling. Something is out there.",
  "Casting the net wide. Let us see what we catch.",
  "Probing the darkness. One port at a time.",
  "The longship moves silently through the network.",
  "No noise. No mercy. Just results.",
};
#define POOL_SCAN_N 6

// Found something — celebratory, cocky
const char* POOL_FOUND[] = {
  "Valhalla! A worthy target reveals itself.",
  "There you are. I knew you were hiding.",
  "The shield wall has a gap. I found it.",
  "Another head for the collection.",
  "Your secrets are mine now, little network.",
  "Even the mighty fall when Sigurd knocks.",
};
#define POOL_FOUND_N 6

// Attacking / port scan — aggressive, brutal
const char* POOL_ATTACK[] = {
  "The axe falls. Open ports bleed data.",
  "No port is safe. No service is sacred.",
  "I am finding your open doors. All of them.",
  "Your default credentials are an insult to Odin.",
  "Banner grabbed. Your secrets whisper to me.",
  "Finding your services one banner at a time.",
  "Telnet in this era? You deserve what follows.",
};
#define POOL_ATTACK_N 7

// Pick a random comment from a pool
// Per-pool cycle state — tracks position in each shuffled pool
// 4 pools: 0=IDLE, 1=SCAN, 2=FOUND, 3=ATTACK
#define NUM_POOLS 4
#define MAX_POOL_N 10  // largest pool size
int poolOrder[NUM_POOLS][MAX_POOL_N];  // shuffled index order per pool
int poolPos[NUM_POOLS]   = {0,0,0,0};  // current position in each pool
bool poolInit[NUM_POOLS] = {false,false,false,false};

// Fisher-Yates shuffle using ESP32 hardware RNG
void shufflePool(int pid, int n) {
  for(int i=0;i<n;i++) poolOrder[pid][i]=i;
  for(int i=n-1;i>0;i--){
    int j = esp_random() % (i+1);
    int tmp = poolOrder[pid][i];
    poolOrder[pid][i] = poolOrder[pid][j];
    poolOrder[pid][j] = tmp;
  }
  poolPos[pid] = 0;
}

void setRandomComment(const char** pool, int n, int pid) {
  if(!poolInit[pid]){
    shufflePool(pid, n);
    poolInit[pid] = true;
  }
  // Get next comment in shuffled order
  int idx = poolOrder[pid][poolPos[pid]];
  strncpy(comment, pool[idx], 95);
  poolPos[pid]++;
  // When we reach the end, reshuffle so we never repeat the last
  // comment as the first of the next cycle
  if(poolPos[pid] >= n){
    int last = poolOrder[pid][n-1];
    shufflePool(pid, n);
    // Make sure first of new cycle isn't same as last of previous
    if(poolOrder[pid][0] == last && n > 1){
      int swap = 1 + (esp_random() % (n-1));
      int tmp = poolOrder[pid][0];
      poolOrder[pid][0] = poolOrder[pid][swap];
      poolOrder[pid][swap] = tmp;
    }
  }
}

// Called from loop — fires a new idle comment periodically
void drawComment() {
  // Draw comment text with C_BG background — overwrites old text cleanly
  // without a visible fillRect flash. Each character cell is cleared by
  // the background colour of drawString itself.
  tft.setTextColor(C_FG, C_BG); tft.setTextSize(1);
  String cmnt = String(comment);
  int s1 = min((int)cmnt.length(), 30);
  if((int)cmnt.length() > 30) {
    int sp = cmnt.lastIndexOf(' ', 30);
    if(sp > 0) s1 = sp;
  }
  // Pad each line to 30 chars so old longer text gets overwritten
  String line1 = cmnt.substring(0, s1);
  while((int)line1.length() < 30) line1 += ' ';
  String line2 = (cmnt.length() > (unsigned)s1) ? cmnt.substring(s1+1, s1+31) : "";
  while((int)line2.length() < 30) line2 += ' ';
  tft.drawString(line1.c_str(), 4, COMMENT_Y + 4);
  tft.drawString(line2.c_str(), 4, COMMENT_Y + 18);
}

void tickComment() {
  if(millis() - lastCommentMs < COMMENT_INTERVAL_MS) return;
  lastCommentMs = millis();
  if(animState == ANIM_IDLE || animState == ANIM_TALK) {
    setRandomComment(POOL_IDLE, POOL_IDLE_N, 0);
    animState = ANIM_TALK;
    drawComment();
  }
}

// ═══════════════════════════════════════════════════════════
//  ANIMATION TICK
// ═══════════════════════════════════════════════════════════
void animateTick() {
  unsigned long now = millis();
  bool changed = false;

  // Blink
  if(eyeOpen && now-lastBlinkMs>3000){eyeOpen=false;lastBlinkMs=now;changed=true;}
  else if(!eyeOpen && now-lastBlinkMs>150){eyeOpen=true;changed=true;}

  // Frame tick every 600ms
  if(now-lastAnimMs>600){
    lastAnimMs=now;
    animFrame=(animFrame+1)%4;
    if(animState==ANIM_SCAN){
      const int looks[]={-2,0,2,0};
      int nx=looks[animFrame];
      if(nx!=eyeOffsetX){eyeOffsetX=nx;changed=true;}
    } else if(eyeOffsetX!=0){eyeOffsetX=0;changed=true;}
    // Breathing — slow 1px shift every other frame (0->1->0->1)
    // Uses animFrame which ticks every 600ms so breath cycle = ~2.4s
    int nb = (animFrame % 2 == 0) ? 0 : 1;
    if(nb != breathOffset){ breathOffset = nb; changed = true; }
    if(animState!=lastDrawnState){lastDrawnState=animState;changed=true;}
  }

  if(changed) drawFaceSprite();
}

// ═══════════════════════════════════════════════════════════
//  SCANNING
// ═══════════════════════════════════════════════════════════
bool tcpProbe(const char* ip, uint16_t port) {
  WiFiClient c; c.setTimeout(CONNECT_TIMEOUT_MS);
  bool ok=c.connect(ip,port); c.stop(); return ok;
}

bool hostAlive(const char* ip) {
  const uint16_t p[]={80,443,22,23,8080,445};
  for(int i=0;i<6;i++){if(tcpProbe(ip,p[i]))return true;yield();}
  return false;
}

void runWifiScan() {
  animState=ANIM_SCAN;
  strncpy(currentAction,"WiFi Scanning",31);
  setRandomComment(POOL_SCAN, POOL_SCAN_N, 1);
  setLED(false,false,true); drawScreen();
  int n=WiFi.scanNetworks(false,true);
  char b[64];
  if(n>0){
    statTargets+=n; statScore+=n*5;
    snprintf(b,sizeof(b),"Found %d network(s)! Valhalla calls.",n);
    strncpy(comment,b,95);
    // Log open networks to SD
    for(int i=0;i<n;i++){
      bool open=(WiFi.encryptionType(i)==WIFI_AUTH_OPEN);
      snprintf(b,sizeof(b),"WIFI,%s,%d,%s",
        WiFi.SSID(i).c_str(),WiFi.RSSI(i),open?"OPEN":"ENC");
      sdLog(b);
      if(open){statCreds++;statScore+=20;}
    }
  } else { strncpy(comment,"No networks found. Seas are quiet.",95); animState=ANIM_IDLE; }
  WiFi.scanDelete();
  animState=ANIM_FOUND;
  strncpy(currentAction,"WiFi Scan done",31);
  statAttacks++; setLED(false,true,false); drawScreen();
}

void runHostScan() {
  if(WiFi.status()!=WL_CONNECTED){
    strncpy(comment,"Not connected to WiFi! Set SSID/PASS.",95);
    strncpy(currentAction,"Error",31); drawScreen(); return;
  }
  animState=ANIM_SCAN; hostCount=0;
  char buf[64];
  snprintf(buf,sizeof(buf),"Scanning %s.0/24...",SCAN_SUBNET);
  strncpy(currentAction,"Host Scanning",31); setRandomComment(POOL_SCAN, POOL_SCAN_N, 1);
  setLED(false,false,true); drawScreen();
  sdLog("# Host scan started");

  for(int i=SCAN_START;i<=SCAN_END&&hostCount<MAX_HOSTS;i++){
    char ip[20]; snprintf(ip,sizeof(ip),"%s.%d",SCAN_SUBNET,i);
    animateTick();
    if(hostAlive(ip)){
      strncpy(hostList[hostCount],ip,15);
      // Resolve hostname
      String hn=resolveHostname(ip);
      strncpy(hostnames[hostCount],hn.c_str(),31);
      hostCount++;
      statTargets++; statScore+=10; statLevel=1+(statScore/100);
      animState=ANIM_FOUND;
      if(hn.length()>0)
        snprintf(buf,sizeof(buf),"Found: %s (%s)",ip,hn.c_str());
      else
        snprintf(buf,sizeof(buf),"Found host: %s",ip);
      setRandomComment(POOL_FOUND, POOL_FOUND_N, 2);
      strncpy(currentAction,"Host found!",31);
      snprintf(buf,sizeof(buf),"HOST,%s,%s",ip,hn.c_str());
      sdLog(buf);
      drawScreen();
    }
    yield();
  }
  snprintf(buf,sizeof(buf),"%d host(s) found in the domain.",hostCount);
  strncpy(comment,buf,95); strncpy(currentAction,"Host Scan done",31);
  animState=ANIM_FOUND;
  animState=ANIM_IDLE; statAttacks++;
  setLED(false,true,false); drawScreen();
}

void runPortScan() {
  if(hostCount==0){
    strncpy(comment,"No hosts yet. Run Host Scan first.",95);
    drawScreen(); return;
  }
  animState=ANIM_ATTACK;
  strncpy(currentAction,"Port Scanning",31);
  setRandomComment(POOL_ATTACK, POOL_ATTACK_N, 3);
  setLED(true,false,false);
  char buf[96];

  for(int h=0;h<hostCount;h++){
    snprintf(buf,sizeof(buf),"Probing %s...",hostList[h]);
    strncpy(comment,buf,95); drawScreen();

    for(int p=0;p<(int)NUM_PORTS;p++){
      animateTick();
      if(tcpProbe(hostList[h],PORTS[p])){
        // Grab banner
        String banner=grabBanner(hostList[h],PORTS[p]);
        addFinding(hostList[h], hostnames[h],
                   PORTS[p], PORT_NAMES[p], banner.c_str());
        statData++;  // count banner grabs
        if(banner.length()>0){
          snprintf(buf,sizeof(buf),"%s:%d %s | %s",
            hostList[h],PORTS[p],PORT_NAMES[p],banner.c_str());
        } else {
          snprintf(buf,sizeof(buf),"%s:%d %s OPEN",
            hostList[h],PORTS[p],PORT_NAMES[p]);
        }
        // Interleave attack comments with actual findings
        if(statVulns % 3 == 0) setRandomComment(POOL_ATTACK, POOL_ATTACK_N, 3);
        else strncpy(comment,buf,95);
        drawScreen();
      }
      yield();
    }
  }
  snprintf(buf,sizeof(buf),"%d open ports. %d banners grabbed.",statVulns,statData);
  strncpy(comment,buf,95); strncpy(currentAction,"Port Scan done",31);
  animState=ANIM_IDLE; statAttacks++;
  setLED(false,true,false); drawScreen();
}


// ═══════════════════════════════════════════════════════════
//  WEB UI  (built-in WebServer, no extra libraries needed)
// ═══════════════════════════════════════════════════════════
String buildPage() {
  String html = "<!DOCTYPE html><html><head>"
    "<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Sigurd CYD</title><style>"
    "body{background:#111;color:#eee;font-family:monospace;padding:16px;}"
    "h1{color:#fd6000;}h2{color:#aaa;}"
    "table{width:100%;border-collapse:collapse;font-size:13px;}"
    "th{background:#222;color:#fd6000;padding:6px;text-align:left;}"
    "td{padding:5px 6px;border-bottom:1px solid #333;}"
    ".badge{background:#fd6000;color:#000;padding:2px 6px;border-radius:3px;font-size:11px;}"
    ".g{color:#0f0;}.o{color:#fd6000;}.c{color:#0cf;}"
    "</style></head><body>";
  html += "<h1>&#9876; SIGURD CYD</h1><p>Authorized use only</p>";
  html += "<table><tr><th>Targets</th><th>Ports</th><th>Banners</th>"
          "<th>Score</th><th>Level</th><th>Deauth</th></tr><tr>";
  html += "<td class='g'>"  + String(statTargets) + "</td>";
  html += "<td class='g'>"  + String(statVulns)   + "</td>";
  html += "<td class='c'>"  + String(statData)    + "</td>";
  html += "<td class='o'>"  + String(statScore)   + "</td>";
  html += "<td class='o'>"  + String(statLevel)   + "</td>";
  html += "<td class='g'>"  + String(deauthCount) + "</td>";
  html += "</tr></table><br>";
  html += "<h2>Hosts</h2><table><tr><th>IP</th><th>Hostname</th></tr>";
  for (int i = 0; i < hostCount; i++)
    html += "<tr><td>" + String(hostList[i]) + "</td><td>" +
            String(hostnames[i][0] ? hostnames[i] : "-") + "</td></tr>";
  html += "</table><br>";
  html += "<h2>Open Ports &amp; Banners</h2>"
          "<table><tr><th>IP</th><th>Host</th><th>Port</th><th>Service</th><th>Banner</th></tr>";
  for (int i = 0; i < findingCount; i++) {
    Finding& f = findings[i];
    html += "<tr><td>" + String(f.ip) + "</td><td>" +
            String(f.hostname[0] ? f.hostname : "-") + "</td>";
    html += "<td class='g'>" + String(f.port) + "</td>";
    html += "<td><span class='badge'>" + String(f.service) + "</span></td>";
    html += "<td class='c'>" + String(f.banner[0] ? f.banner : "-") + "</td></tr>";
  }
  html += "</table><br>";
  if (deauthLogCount > 0) {
    html += "<h2>Deauth Events</h2>"
            "<table><tr><th>#</th><th>Src MAC</th><th>Dst MAC</th><th>RSSI</th><th>ms</th></tr>";
    for (int i = 0; i < deauthLogCount; i++) {
      DeauthEvent& e = deauthLog[i];
      char s[20], d[20];
      snprintf(s,sizeof(s),"%02X:%02X:%02X:%02X:%02X:%02X",
        e.src[0],e.src[1],e.src[2],e.src[3],e.src[4],e.src[5]);
      snprintf(d,sizeof(d),"%02X:%02X:%02X:%02X:%02X:%02X",
        e.dst[0],e.dst[1],e.dst[2],e.dst[3],e.dst[4],e.dst[5]);
      html += "<tr><td>" + String(i+1) + "</td><td class='o'>" + String(s) +
              "</td><td>" + String(d) + "</td><td>" + String(e.rssi) +
              "dBm</td><td>" + String(e.ts_ms) + "</td></tr>";
    }
    html += "</table><br>";
  }
  // RSSI history table
  if (rssiTrackCount > 0) {
    html += "<h2>Signal Strength History</h2>"
            "<table><tr><th>SSID</th><th>Latest RSSI</th><th>Samples</th></tr>";
    for (int i = 0; i < rssiTrackCount; i++) {
      RssiTrack& t = rssiTracks[i];
      int latest = t.history[(t.head - 1 + RSSI_HISTORY_LEN) % RSSI_HISTORY_LEN];
      html += "<tr><td>" + String(t.ssid) + "</td>";
      html += "<td class='g'>" + String(latest) + " dBm</td>";
      html += "<td>" + String(t.count) + "</td></tr>";
    }
    html += "</table><br>";
  }
  if (sdAvailable)
    html += "<p><a href='/log' style='color:#fd6000'>&#8595; Download sigurd.log</a></p>";
  html += "<p style='color:#555;font-size:11px'>Sigurd CYD &mdash; Port of PierreGode/Ragnar</p>";
  html += "</body></html>";
  return html;
}

String buildJson() {
  String j = "{";
  j += "\"targets\":" + String(statTargets);
  j += ",\"vulns\":"  + String(statVulns);
  j += ",\"score\":"  + String(statScore);
  j += ",\"level\":"  + String(statLevel);
  j += ",\"deauth\":" + String(deauthCount);
  j += ",\"hosts\":[";
  for (int i = 0; i < hostCount; i++) {
    if (i) j += ",";
    j += "{\"ip\":\"" + String(hostList[i]) + "\",\"hostname\":\"" + String(hostnames[i]) + "\"}";
  }
  j += "],\"findings\":[";
  for (int i = 0; i < findingCount; i++) {
    if (i) j += ",";
    Finding& f = findings[i];
    j += "{\"ip\":\"" + String(f.ip) + "\"";
    j += ",\"port\":"     + String(f.port);
    j += ",\"service\":\"" + String(f.service) + "\"";
    j += ",\"banner\":\"" + String(f.banner) + "\"}";
  }
  j += "]}";
  return j;
}

void handleWebRoot() { webServer.send(200, "text/html",       buildPage()); }
void handleWebApi()  { webServer.send(200, "application/json",buildJson()); }
void handleWebLog()  {
  if (!sdAvailable) { webServer.send(404,"text/plain","SD not available"); return; }
  File f = SD.open("/sigurd.log");
  if (!f)           { webServer.send(404,"text/plain","Log not found");    return; }
  webServer.streamFile(f, "text/plain");
  f.close();
}

void setupWebServer() {
  webServer.on("/",    handleWebRoot);
  webServer.on("/api", handleWebApi);
  webServer.on("/log", handleWebLog);
  webServer.begin();
}


// ═══════════════════════════════════════════════════════════
//  ALERT FLASH — red border + LED blink
// ═══════════════════════════════════════════════════════════
void triggerAlert(const char* msg) {
  strncpy(comment, msg, 95);
  animState = ANIM_FOUND;
  alertFlashCount = 6;  // 3 flashes (on+off = 2 per flash)
  statScore += 25;
  statLevel = 1 + (statScore / 100);
  char logbuf[96];
  snprintf(logbuf, sizeof(logbuf), "ALERT,%s", msg);
  sdLog(logbuf);
  drawScreen();
}

void tickAlertFlash() {
  if (alertFlashCount <= 0) return;
  if (millis() - lastAlertFlashMs < 150) return;
  lastAlertFlashMs = millis();
  bool on = (alertFlashCount % 2 == 0);

  // Flash only inside the bottom panel border — safe zone with no text near edges
  // BOT_Y is below the divider, nothing outside the sprite area gets touched
  uint16_t col = on ? C_RED : C_BG;
  tft.drawRect(0,       BOT_Y,     SCREEN_W,   BOT_H,   col);
  tft.drawRect(1,       BOT_Y+1,   SCREEN_W-2, BOT_H-2, col);
  // RGB LED only — no screen drawing outside bottom panel
  setLED(on, false, false);

  alertFlashCount--;
}

// ═══════════════════════════════════════════════════════════
//  HTTP TITLE GRABBER
//  Fetches the <title> tag from port 80/443 on a host
// ═══════════════════════════════════════════════════════════
String grabHttpTitle(const char* ip, uint16_t port) {
  WiFiClient c;
  c.setTimeout(600);
  if (!c.connect(ip, port)) return "";
  c.printf("GET / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", ip);
  unsigned long t = millis();
  String body = "";
  while (millis() - t < 1500) {
    while (c.available()) {
      body += (char)c.read();
      if (body.length() > 2048) goto done;  // limit read
    }
    if (!c.connected()) break;
    delay(10);
  }
  done:
  c.stop();
  // Extract <title>...</title>
  body.toLowerCase();
  int ts = body.indexOf("<title>");
  int te = body.indexOf("</title>");
  if (ts < 0 || te < 0 || te <= ts) return "";
  String title = body.substring(ts + 7, te);
  title.trim();
  if (title.length() > 48) title = title.substring(0, 48);
  return title;
}

// ═══════════════════════════════════════════════════════════
//  RSSI HISTORY — track signal strength of known APs
// ═══════════════════════════════════════════════════════════
void rssiAddSample(const char* ssid, int8_t rssi) {
  // Find existing track
  for (int i = 0; i < rssiTrackCount; i++) {
    if (strcmp(rssiTracks[i].ssid, ssid) == 0) {
      rssiTracks[i].history[rssiTracks[i].head] = rssi;
      rssiTracks[i].head = (rssiTracks[i].head + 1) % RSSI_HISTORY_LEN;
      if (rssiTracks[i].count < RSSI_HISTORY_LEN) rssiTracks[i].count++;
      return;
    }
  }
  // New AP
  if (rssiTrackCount >= RSSI_AP_MAX) return;
  RssiTrack& t = rssiTracks[rssiTrackCount++];
  strncpy(t.ssid, ssid, 32);
  memset(t.history, 0, sizeof(t.history));
  t.history[0] = rssi;
  t.head  = 1;
  t.count = 1;
}

// Draw signal strength graph in the comment/bottom area
void drawRssiGraph() {
  int gx = 4, gy = COMMENT_Y + 2;
  int gw = SCREEN_W - 8, gh = COMMENT_H - 4;
  tft.fillRect(gx, gy, gw, gh, C_BG);
  tft.drawRect(gx, gy, gw, gh, C_GREY);
  // Grid lines at -50, -70, -90 dBm
  // Map rssi -100..0 to gh..0
  auto rssiY = [&](int r) { return gy + gh - (int)((r + 100) * gh / 100); };
  tft.drawFastHLine(gx, rssiY(-50), gw, C_DKGREY);
  tft.drawFastHLine(gx, rssiY(-70), gw, C_DKGREY);
  tft.setTextColor(C_DKGREY, C_BG); tft.setTextSize(1);
  tft.drawString("-50", gx+2, rssiY(-50)-8);
  tft.drawString("-70", gx+2, rssiY(-70)-8);

  uint16_t colors[] = {C_GREEN, C_CYAN, C_YELLOW, C_ACCENT};
  int slotW = gw / RSSI_HISTORY_LEN;

  for (int t = 0; t < rssiTrackCount; t++) {
    RssiTrack& tr = rssiTracks[t];
    // SSID label
    tft.setTextColor(colors[t % 4], C_BG);
    tft.drawString(String(tr.ssid).substring(0,12).c_str(),
                   gx + 2, gy + 2 + t * 10);
    // Plot line
    int prev_x = -1, prev_y = -1;
    for (int i = 0; i < tr.count; i++) {
      int idx = (tr.head - tr.count + i + RSSI_HISTORY_LEN) % RSSI_HISTORY_LEN;
      int rx = gx + i * (gw / max(tr.count, 1));
      int ry = rssiY(tr.history[idx]);
      ry = constrain(ry, gy+1, gy+gh-1);
      if (prev_x >= 0)
        tft.drawLine(prev_x, prev_y, rx, ry, colors[t % 4]);
      else
        tft.drawPixel(rx, ry, colors[t % 4]);
      prev_x = rx; prev_y = ry;
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  EVIL TWIN DETECTION
//  Scans for APs sharing an SSID but with different BSSIDs
// ═══════════════════════════════════════════════════════════
void checkEvilTwin() {
  int n = WiFi.scanNetworks(false, true);
  if (n <= 0) { WiFi.scanDelete(); return; }

  // Add RSSI samples for tracked APs while we have scan data
  for (int i = 0; i < n; i++) {
    rssiAddSample(WiFi.SSID(i).c_str(), (int8_t)WiFi.RSSI(i));
  }

  // Check for duplicate SSIDs with different BSSIDs
  for (int i = 0; i < n; i++) {
    String ssidA = WiFi.SSID(i);
    if (ssidA.length() == 0) continue;
    for (int j = i+1; j < n; j++) {
      if (WiFi.SSID(j) == ssidA) {
        String macA = WiFi.BSSIDstr(i);
        String macB = WiFi.BSSIDstr(j);
        if (macA != macB) {
          char alert[95];
          snprintf(alert, sizeof(alert),
            "EVIL TWIN! %s seen on %s and %s",
            ssidA.substring(0,16).c_str(),
            macA.substring(0,8).c_str(),
            macB.substring(0,8).c_str());
          triggerAlert(alert);
          statZombies++;
          WiFi.scanDelete();
          return;
        }
      }
    }
  }
  WiFi.scanDelete();
}

// ═══════════════════════════════════════════════════════════
//  NEW DEVICE MONITOR
//  Background sweep — fires triggerAlert on new hosts
// ═══════════════════════════════════════════════════════════
// Persistent known-hosts list (survives manual host scan resets)

bool isKnown(const char* ip) {
  for (int i = 0; i < knownCount; i++)
    if (strcmp(knownHosts[i], ip) == 0) return true;
  return false;
}

void addKnown(const char* ip) {
  if (knownCount >= KNOWN_MAX) return;
  strncpy(knownHosts[knownCount++], ip, 15);
}

// Lightweight single-host probe — faster than full hostAlive
bool quickProbe(const char* ip) {
  WiFiClient c; c.setTimeout(80);
  bool ok = c.connect(ip, 80);
  if (!ok) { c.stop(); c.setTimeout(80); ok = c.connect(ip, 443); }
  if (!ok) { c.stop(); c.setTimeout(80); ok = c.connect(ip, 22);  }
  c.stop();
  return ok;
}

void tickMonitor() {
  if (!monitorRunning) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMonitorMs < MONITOR_INTERVAL_MS) return;
  lastMonitorMs = millis();

  // Sweep subnet quietly in background
  for (int i = SCAN_START; i <= SCAN_END; i++) {
    char ip[20];
    snprintf(ip, sizeof(ip), "%s.%d", SCAN_SUBNET, i);
    animateTick();
    webServer.handleClient();
    if (quickProbe(ip) && !isKnown(ip)) {
      addKnown(ip);
      // Also add to main host list if room
      if (hostCount < MAX_HOSTS) {
        strncpy(hostList[hostCount], ip, 15);
        hostnames[hostCount][0] = '\0';
        hostCount++;
      }
      statTargets++;
      char alert[64];
      snprintf(alert, sizeof(alert), "New device! %s just appeared.", ip);
      triggerAlert(alert);
      return;  // one alert at a time — next sweep catches more
    }
    yield();
  }
  // Also check for evil twins and update RSSI on each monitor cycle
  checkEvilTwin();
}

void startMonitor() {
  // Seed known hosts from current host list
  for (int i = 0; i < hostCount; i++)
    if (!isKnown(hostList[i])) addKnown(hostList[i]);
  monitorRunning  = true;
  lastMonitorMs   = millis() - MONITOR_INTERVAL_MS + 10000; // first sweep in 10s
  char buf[64];
  snprintf(buf, sizeof(buf), "Monitor active. Sweep every 3 min.");
  strncpy(comment, buf, 95);
  sdLog("# Monitor started");
}

void stopMonitor() {
  monitorRunning = false;
  strncpy(comment, "Monitor stopped.", 95);
  sdLog("# Monitor stopped");
}

// ═══════════════════════════════════════════════════════════
//  WIFI CONNECT
// ═══════════════════════════════════════════════════════════
void wifiConnect() {
  if(strlen(WIFI_SSID)==0){
    strncpy(comment,"No SSID set. WiFi scan still works.",95); return;
  }
  char buf[64]; snprintf(buf,sizeof(buf),"Joining %s...",WIFI_SSID);
  strncpy(comment,buf,95); strncpy(currentAction,"Connecting...",31); drawScreen();
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASS);
  for(int i=0;i<20&&WiFi.status()!=WL_CONNECTED;i++){setLED(false,false,i%2);delay(500);}
  updateWifiInfo();
  if(WiFi.status()==WL_CONNECTED){
    snprintf(buf,sizeof(buf),"Connected! %s",WiFi.localIP().toString().c_str());
    strncpy(comment,buf,95); strncpy(currentAction,"NetworkScanner",31);
    statScore+=20;
    setupWebServer();
    char logbuf[64];
    snprintf(logbuf,sizeof(logbuf),"# Connected to %s IP %s",
      WIFI_SSID,WiFi.localIP().toString().c_str());
    sdLog(logbuf);
  } else {
    strncpy(comment,"WiFi join failed. Scan still works.",95);
    strncpy(currentAction,"Idle",31);
  }
  setLED(false,false,false);
}

// ═══════════════════════════════════════════════════════════
//  TOUCH
// ═══════════════════════════════════════════════════════════
bool getTouch(int& tx, int& ty) {
  if(!ts.tirqTouched()||!ts.touched()) return false;
  TS_Point p=ts.getPoint();
  if(p.z<100) return false;
  tx=map(p.y,TS_MINY,TS_MAXY,0,SCREEN_W);
  ty=map(p.x,TS_MINX,TS_MAXX,0,SCREEN_H);
  tx=constrain(tx,0,SCREEN_W-1);
  ty=constrain(ty,0,SCREEN_H-1);
  return true;
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  pinMode(LED_R,OUTPUT); pinMode(LED_G,OUTPUT); pinMode(LED_B,OUTPUT);
  setLED(false,false,false);

  // Display
  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(2);
  tft.fillScreen(C_BG);

  // Sprite — created once, never recreated
  sprite.createSprite(SPR_W,SPR_H);
  sprite.setColorDepth(16);

  // Touch
  touchSPI.begin(XPT2046_CLK,XPT2046_MISO,XPT2046_MOSI,XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(0);

  // SD card
  if(SD.begin(SD_CS)){
    sdAvailable=true;
    sdLog("# Sigurd CYD boot");
  } else {
    sdAvailable=false;
  }

  WiFi.mode(WIFI_STA);
  drawScreen();
  delay(500);
  wifiConnect();
  updateWifiInfo();
  drawScreen();
}

// ═══════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  animateTick();
  webServer.handleClient();
  tickComment();
  tickMonitor();
  tickAlertFlash();

  // WiFi signal refresh every 15s
  if(millis()-lastUpdateMs>15000){
    lastUpdateMs=millis();
    updateWifiInfo();
    drawWifiIcon(4,2,wifiSignal);
    char sig[8]; snprintf(sig,sizeof(sig),"%d",wifiSignal==0?0:abs(wifiSignal));
    tft.setTextColor(C_FG,C_BG); tft.setTextSize(1); tft.drawString(sig,22,14);
  }

  int tx,ty;
  if(!getTouch(tx,ty)) return;
  unsigned long now=millis();
  if(now-lastTouchMs<400) return;
  lastTouchMs=now;

  // Header tap: toggle RSSI graph / monitor
  if(ty>=HDR_Y&&ty<HDR_H){
    if(tx < SCREEN_W/2){
      // Left half — toggle monitor
      if(monitorRunning){ stopMonitor(); animState=ANIM_IDLE; }
      else              { startMonitor(); animState=ANIM_SCAN; }
      drawScreen();
    } else {
      // Right half — toggle RSSI graph
      if(rssiTrackCount > 0){
        showRssiGraph = !showRssiGraph;
        tft.fillRect(0,COMMENT_Y,SCREEN_W,COMMENT_H,C_BG);
        if(showRssiGraph) drawRssiGraph();
        else drawComment();
      } else {
        // No data yet — inform user
        strncpy(comment,"No signal data yet. Run WiFi Scan first.",95);
        drawComment();
      }
    }
  }
  // Stats row 1: WiFi | Host | Port scan
  else if(ty>=STATS1_Y&&ty<STATS1_Y+STATS1_H){
    int t=SCREEN_W/3;
    if(tx<t) runWifiScan();
    else if(tx<t*2) runHostScan();
    else runPortScan();
  }
  // Stats row 2: left=toggle deauth, right=port scan
  else if(ty>=STATS2_Y&&ty<STATS2_Y+STATS2_H){
    int t=SCREEN_W/2;
    if(tx<t){
      // Toggle deauth detection
      if(deauthRunning){ stopDeauthDetection(); animState=ANIM_IDLE; }
      else             { startDeauthDetection(); animState=ANIM_SCAN; }
      drawScreen();
    } else {
      runPortScan();
    }
  }
  // Action row: tap for a random comment from the idle pool
  else if(ty>=ACTION_Y&&ty<ACTION_Y+ACTION_H){
    setRandomComment(POOL_IDLE, POOL_IDLE_N, 0);
    lastCommentMs = millis();
    animState=ANIM_TALK;
    drawComment();
  }
  // Bottom panel: left=reset, right=clear
  else if(ty>=BOT_Y){
    if(tx<SCREEN_W/2){
      statScore=0;statLevel=1;statAlive=0;statDefense=0;
      strncpy(comment,"Stats reset. Begin anew, warrior.",95);
    } else {
      hostCount=0;statTargets=0;statVulns=0;statAttacks=0;
      statZombies=0;statData=0;statCreds=0;
      findingCount=0;deauthCount=0;deauthLogCount=0;
      strncpy(comment,"All data purged. The slate is clean.",95);
      sdLog("# Data cleared by user");
    }
    animState=ANIM_IDLE;
    drawScreen();
  }
}
