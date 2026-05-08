#pragma once

#include <Arduino.h>
#include <IPAddress.h>

namespace WifiAp {

bool begin();
IPAddress ip();
String ssid();
uint8_t connectedClients();

}  // namespace WifiAp
