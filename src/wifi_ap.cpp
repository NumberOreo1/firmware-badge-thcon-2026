#include "wifi_ap.h"

#include <WiFi.h>

#include "board_config.h"

namespace WifiAp {

bool begin() {
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);

  const bool ok = WiFi.softAP(
      BoardConfig::AP_SSID,
      BoardConfig::AP_PASSWORD,
      BoardConfig::AP_CHANNEL,
      false,
      BoardConfig::AP_MAX_CONNECTIONS);

  return ok;
}

IPAddress ip() {
  return WiFi.softAPIP();
}

String ssid() {
  return BoardConfig::AP_SSID;
}

uint8_t connectedClients() {
  return WiFi.softAPgetStationNum();
}

}  // namespace WifiAp
