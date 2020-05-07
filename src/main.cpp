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

void configModeCallback(WiFiManager *myWiFiManager)
{
    flipdot.clear();
    flipdot.setCursor(0, 5);
    flipdot.print("AP:\n" + myWiFiManager->getConfigPortalSSID());
    flipdot.writeDisplay();
}

void onUdpPacket(AsyncUDPPacket packet)
{

    if (panelOverride) return;

    uint16_t len = (uint16_t) flipdot.getWidth() * flipdot.getHeight() / 8;

    if (packet.length() != len)
        return;

    uint8_t *payload = packet.data();
    memcpy(flipdot.db_buffer, payload, len);

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
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        connectToUdp();
        flipdot.clear();
        flipdot.setCursor(0, 5);
        flipdot.println("Mcast:");
        flipdot.println(multicastGroup);
        flipdot.println("Port:");
        flipdot.println(UDP_PORT);
        flipdot.writeDisplay();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        flipdot.clear();
        flipdot.setCursor(0, 5);
        flipdot.println("WiFi Lost");
        flipdot.writeDisplay();
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
            panelOverride = true;
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "FW";
            } else { // U_FS
            type = "FS";
            }
            flipdot.clear();
            flipdot.setCursor(0, 5);
            flipdot.print("OTA " + type);
            flipdot.writeDisplay();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            flipdot.clear();
            flipdot.setCursor(0, 5);
            flipdot.printf("OTA %u%%", (progress / (total / 100)));
            flipdot.writeDisplay();
        })
        .onEnd([]() {
            flipdot.clear();
            flipdot.setCursor(0, 5);
            flipdot.print("OTA success!");
            flipdot.writeDisplay();
        })
        .onError([](ota_error_t error) {
            flipdot.clear();
            flipdot.setCursor(0, 5);
            flipdot.printf("Err[%u] ", error);
            flipdot.writeDisplay();
            panelOverride = false;
        });

    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();
}
