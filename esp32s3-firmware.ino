
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences prefs;
WebServer server(80);

// Default configuration values
String ssid = "ESP32_S3_AP";
String password = "12345678";
String cmd_ignition = "halo jarvis";
String cmd_start = "hidupkan mesin";
String cmd_off = "matikan mesin";
String cmd_music_on = "musik on";
String cmd_music_off = "musik off";
String cmd_vol_up = "tambah volume";
String cmd_vol_down = "kurangi volume";

// Pin Definitions
const int PIN_IGNITION = 17;
const int PIN_START = 18;
const int PIN_MUSIK = 19;
const int PIN_VOL_UP = 21;
const int PIN_VOL_DOWN = 22;
const int PIN_ALWAYS = 23;
const int PIN_DFPLAYER = 25;
const int PIN_ALARM = 26;

bool ignitionState = false;
unsigned long lastBlink = 0;

void setup() {
  Serial.begin(115200);
  loadConfig();
  WiFi.softAP(ssid.c_str(), password.c_str());
  setupPins();
  server.on("/cmd", HTTP_POST, handleCmd);
  server.on("/saveconfig", HTTP_POST, handleSaveConfig);
  server.begin();
}

void loop() {
  server.handleClient();
  handleAlarm();
  blinkAlwaysOn();
}

void setupPins() {
  pinMode(PIN_IGNITION, OUTPUT);
  pinMode(PIN_START, OUTPUT);
  pinMode(PIN_MUSIK, OUTPUT);
  pinMode(PIN_VOL_UP, OUTPUT);
  pinMode(PIN_VOL_DOWN, OUTPUT);
  pinMode(PIN_ALWAYS, OUTPUT);
  pinMode(PIN_DFPLAYER, OUTPUT);
  pinMode(PIN_ALARM, OUTPUT);
  resetAll();
}

void resetAll() {
  digitalWrite(PIN_IGNITION, LOW);
  digitalWrite(PIN_START, LOW);
  digitalWrite(PIN_MUSIK, LOW);
  digitalWrite(PIN_VOL_UP, LOW);
  digitalWrite(PIN_VOL_DOWN, LOW);
  digitalWrite(PIN_DFPLAYER, LOW);
  digitalWrite(PIN_ALWAYS, LOW);
  digitalWrite(PIN_ALARM, LOW);
}

void handleCmd() {
  String cmd = server.arg("command");
  cmd.toLowerCase();
  triggerDFPlayer();

  if (cmd == cmd_ignition) {
    digitalWrite(PIN_IGNITION, HIGH);
    ignitionState = true;
  } else if (cmd == cmd_start) {
    digitalWrite(PIN_START, HIGH);
    delay(4000);
    digitalWrite(PIN_START, LOW);
  } else if (cmd == cmd_off) {
    digitalWrite(PIN_IGNITION, LOW);
    ignitionState = false;
  } else if (cmd == cmd_music_on) {
    digitalWrite(PIN_MUSIK, HIGH);
  } else if (cmd == cmd_music_off) {
    digitalWrite(PIN_MUSIK, LOW);
  } else if (cmd == cmd_vol_up) {
    digitalWrite(PIN_VOL_UP, HIGH);
    delay(1000);
    digitalWrite(PIN_VOL_UP, LOW);
  } else if (cmd == cmd_vol_down) {
    digitalWrite(PIN_VOL_DOWN, HIGH);
    delay(1000);
    digitalWrite(PIN_VOL_DOWN, LOW);
  }

  server.send(200, "text/plain", "OK");
}

void triggerDFPlayer() {
  digitalWrite(PIN_DFPLAYER, HIGH);
  blinkAlwaysOnce();
  digitalWrite(PIN_DFPLAYER, LOW);
}

void blinkAlwaysOnce() {
  digitalWrite(PIN_ALWAYS, HIGH);
  delay(200);
  digitalWrite(PIN_ALWAYS, LOW);
}

void blinkAlwaysOn() {
  if (millis() - lastBlink > 1000) {
    lastBlink = millis();
    digitalWrite(PIN_ALWAYS, !digitalRead(PIN_ALWAYS));
  }
}

void handleAlarm() {
  if (WiFi.softAPgetStationNum() == 0) {
    digitalWrite(PIN_ALARM, HIGH);
    digitalWrite(PIN_IGNITION, LOW);
    ignitionState = false;
  } else {
    digitalWrite(PIN_ALARM, LOW);
  }
}

void loadConfig() {
  prefs.begin("config", true);
  ssid = prefs.getString("ssid", "ESP32_S3_AP");
  password = prefs.getString("pass", "12345678");
  cmd_ignition = prefs.getString("cmd_ign", "halo jarvis");
  cmd_start = prefs.getString("cmd_start", "hidupkan mesin");
  cmd_off = prefs.getString("cmd_off", "matikan mesin");
  cmd_music_on = prefs.getString("cmd_mus_on", "musik on");
  cmd_music_off = prefs.getString("cmd_mus_off", "musik off");
  cmd_vol_up = prefs.getString("cmd_vol_up", "tambah volume");
  cmd_vol_down = prefs.getString("cmd_vol_down", "kurangi volume");
  prefs.end();
}

void handleSaveConfig() {
  prefs.begin("config", false);
  prefs.putString("ssid", server.arg("ssid"));
  prefs.putString("pass", server.arg("password"));
  prefs.putString("cmd_ign", server.arg("ignition"));
  prefs.putString("cmd_start", server.arg("start"));
  prefs.putString("cmd_off", server.arg("off"));
  prefs.putString("cmd_mus_on", server.arg("music_on"));
  prefs.putString("cmd_mus_off", server.arg("music_off"));
  prefs.putString("cmd_vol_up", server.arg("vol_up"));
  prefs.putString("cmd_vol_down", server.arg("vol_down"));
  prefs.end();
  server.send(200, "text/plain", "Config saved. Reboot to apply.");
}
