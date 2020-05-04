#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include <Hanover_Flipdot.h>
#include <Adafruit_GFX.h>
#include <Fonts/Picopixel.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#define MQTT_HOST IPAddress(192, 168, 20, 10)
#define MQTT_PORT 1883

Hanover_Flipdot flipdot = Hanover_Flipdot();
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

void configModeCallback(WiFiManager *myWiFiManager)
{
    flipdot.print("Config: " + myWiFiManager->getConfigPortalSSID());
    flipdot.writeDisplay();
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        break;
    }
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    mqttClient.subscribe("flipdot/text", 0);
    mqttClient.subscribe("flipdot/bitmap", 0);
    mqttClient.subscribe("flipdot/clear", 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");
    if (WiFi.isConnected())
        xTimerStart(mqttReconnectTimer, 0);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{

    if (strcmp(topic, "flipdot/text") == 0)
    {
        payload[len] = '\0';
        flipdot.clear();
        flipdot.setCursor(0, 0);
        flipdot.setTextSize(1);
        flipdot.print(payload);
        flipdot.writeDisplay();
    }
    else if (strcmp(topic, "flipdot/bitmap") == 0)
    {
        if (index == 0)
            flipdot.clear();
        for (uint16_t c = 0; c < len; c++)
        {
            uint8_t x = (c + index) % flipdot.getWidth();
            uint8_t y = (c + index) / flipdot.getWidth();
            if (payload[c] == '1')
                flipdot.drawPixel(x, y, 1);
        }
        if (index + len == total)
            flipdot.writeDisplay();
    }
    else if (strcmp(topic, "flipdot/clear") == 0)
    {
        flipdot.clear();
        flipdot.writeDisplay();
    }
}

void setup()
{

    Serial.begin(115200);

    flipdot.begin();
    flipdot.setFont(&Picopixel);
    flipdot.setTextSize(1);

    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

    WiFi.onEvent(WiFiEvent);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect();

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        })
        .setHostname("Hanover-Flipdot");

    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();
}