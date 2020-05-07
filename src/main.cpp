#include <main.h>

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
}

#include <Hanover_Flipdot.h>
#include <Adafruit_GFX.h>
#include <Fonts/TomThumb.h>

#include "AsyncUDP.h"

Hanover_Flipdot flipdot = Hanover_Flipdot();
AsyncUDP udp;
IPAddress multicastGroup = IPAddress(239, 1, 2, 3);

bool panelOverride = false;

void debugText(String text, uint8_t seconds = 0)
{
    if (seconds)
        flipdot.backup();
    flipdot.clear();
    flipdot.setCursor(0, 5);
    flipdot.print(text);
    flipdot.writeDisplay();
    if (seconds)
    {
        sleep(seconds);
        flipdot.restore();
        flipdot.writeDisplay();
    }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
    debugText("Setup: " + myWiFiManager->getConfigPortalSSID());
}

void onUdpPacket(AsyncUDPPacket packet)
{
    if (panelOverride)
        return;

    uint16_t len = (uint16_t)flipdot.getWidth() * flipdot.getHeight() / 8;
    if (packet.length() != len)
        return;

    memcpy(flipdot.db_buffer, packet.data(), len);
    flipdot.writeDisplay();
}

void connectToUdp()
{
    if (udp.listenMulticast(multicastGroup, UDP_PORT))
    {
        // Power optimizations
        setCpuFrequencyMhz(80);

        // Uncomment for better receive performance, e.g. for videos
        // ESP32 may get hot!
        WiFi.setSleep(false);

        // UDP handler function
        udp.onPacket(onUdpPacket);
    }
}

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    String ip_msg = "IP: ";
    ip_msg += multicastGroup.toString() + "\nPort: " + UDP_PORT;
    switch (event)
    {
    case SYSTEM_EVENT_STA_START:
        debugText("Wi-Fi connecting", 3);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        connectToUdp();
        debugText(ip_msg);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        debugText("Wi-Fi lost", 3);
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        debugText("Wi-Fi lost IP", 3);
        break;
    }
}
void setup()
{

    Serial.begin(115200);

    flipdot.begin();
    flipdot.setFont(&TomThumb);
    flipdot.setTextSize(1);

    WiFi.onEvent(WiFiEvent);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setHostname("Hanover-Flipdot");
    wifiManager.autoConnect();

    ArduinoOTA.onStart([]() {
                  // Stash flipdot contents and lock display to prevent slowdown of OTA by synchronous display updates from UDP
                  flipdot.backup();
                  panelOverride = true;
                  String type;
                  if (ArduinoOTA.getCommand() == U_FLASH)
                  {
                      type = "FW";
                  }
                  else
                  { // U_FS
                      type = "FS";
                  }
                  debugText("OTA " + type);
              })
        .onProgress([](unsigned int progress, unsigned int total) {
            String ota_msg = "OTA ";
            ota_msg += String(progress / (total / 100)) + "%";
            debugText(ota_msg);
        })
        .onEnd([]() {
            debugText("OTA success!");
        })
        .onError([](ota_error_t error) {
            String ota_msg = "OTA error " + String(error) + ":\n";
            switch (error)
            {
            case OTA_AUTH_ERROR:
                ota_msg += "Auth Failed";
                break;
            case OTA_BEGIN_ERROR:
                ota_msg += "Begin Failed";
                break;
            case OTA_CONNECT_ERROR:
                ota_msg += "Connect Failed";
                break;
            case OTA_RECEIVE_ERROR:
                ota_msg += "Receive Failed";
                break;
            case OTA_END_ERROR:
                ota_msg += "End Failed";
                break;
            }
            debugText(ota_msg);
            sleep(3);
            panelOverride = false;
            flipdot.restore();
            flipdot.writeDisplay();
        });

    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();
}
