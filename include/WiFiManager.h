#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

class WiFiManager {
public:
  struct Config {
    String ssid;
    String password;
    String userToken;

    bool isValid() const {
      return ssid.length() > 0 && password.length() > 0;
    }
  };

  WiFiManager(uint8_t maxReconnectAttempts = 5,
              unsigned long reconnectBackoffMs = 5000UL,
              unsigned long reconnectResetMs = 60000UL);

  void begin();
  void loop();
  bool isConnected() const;
  bool portalActive() const;
  const Config& getConfig() const;

private:
  static WiFiManager* _instance;
  static void onWiFiEventStatic(WiFiEvent_t event);

  void onWiFiEvent(WiFiEvent_t event);
  void loadConfig();
  void saveConfig(const String& ssid, const String& password, const String& userToken);
  void startStation();
  void attemptReconnect();
  void startCaptivePortal();
  void stopCaptivePortal();
  String captivePortalPage(const String& message = String()) const;
  void handleRoot();
  void handleSave();
  void handleNotFound();

  Config _config;
  Preferences _preferences;
  WebServer _server;
  DNSServer _dnsServer;
  bool _connected;
  bool _portalStarted;
  bool _reconnectPending;
  uint8_t _reconnectAttempts;
  unsigned long _lastActionMillis;
  unsigned long _lastStatusMillis;
  const uint8_t _maxReconnect;
  const unsigned long _reconnectBackoffMs;
  const unsigned long _reconnectResetMs;
};

#endif // WIFI_MANAGER_H
