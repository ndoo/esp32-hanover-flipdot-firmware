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

void configModeCallback(WiFiManager *myWiFiManager)
{
    flipdot.clear();
    flipdot.setCursor(0, 5);
    flipdot.print("AP:\n" + myWiFiManager->getConfigPortalSSID());
    flipdot.writeDisplay();
}

void onUdpPacket(AsyncUDPPacket packet)
{
    uint16_t len = (uint16_t) flipdot.getWidth() * flipdot.getHeight() / 8;

    if (packet.length() != len)
        return;

    uint8_t *payload = packet.data();

    for (uint16_t c = 0; c < len; c++)
    {
        char byte = payload[c];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            uint8_t x = (c * 8 + bit) % flipdot.getWidth();
            uint8_t y = (c * 8 + bit) / flipdot.getWidth();
            flipdot.drawPixel(x, y, byte & 1);
            byte = byte >> 1;
        }
    }
    flipdot.writeDisplay();
}

void connectToUdp()
{
    if (udp.listenMulticast(multicastGroup, UDP_PORT))
    {
        // Power optimizations
        WiFi.setSleep(false);
        setCpuFrequencyMhz(80);

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

    flipdot.begin();
    flipdot.setFont(&TomThumb);
    flipdot.setTextSize(1);

    WiFi.onEvent(WiFiEvent);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setHostname("Hanover-Flipdot");
    wifiManager.autoConnect();

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            flipdot.clear();
            flipdot.setCursor(0, 5);
            flipdot.printf("OTA %u%%", (progress / (total / 100)));
            flipdot.writeDisplay();
        });

    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();
}