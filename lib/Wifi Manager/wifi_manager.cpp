#include "wifi_manager.h"
#include "data_config.h"
WiFiManager::WiFiManager() : server(80) {}

bool WiFiManager::begin()
{
    // Initialize WiFi module and start access point
    setupAP();
    return true;
}

void WiFiManager::setupAP()
{
    const char *ssid = "ESP32_Config";
    const char *password = "11111111";
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 32), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));


    dnsServer.start(53, "esp32.local", IPAddress(192, 168, 1, 32));

    server.on("/", HTTP_GET, std::bind(&WiFiManager::handleRoot, this, std::placeholders::_1));
    server.on("/save", HTTP_POST, std::bind(&WiFiManager::handleSave, this, std::placeholders::_1));
    server.begin();
}

void WiFiManager::handleRoot(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", html);
}

void WiFiManager::handleSave(AsyncWebServerRequest *request)
{
    if (request->hasParam("ssid", true) && request->hasParam("password", true))
    {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();

        // Save in memory
        DataConfig &dataCF = DataConfig::getInstance();
        dataCF.updateWifiConfig(ssid, password);

        request->send(200, "text/html", "WiFi credentials saved. Restarting..." + ssid + ".." + password);
        // Optionally, restart the ESP here
    }
    else
    {
        request->send(400, "text/html", "Invalid request");
    }
}

void WiFiManager::handleClient()
{
    dnsServer.processNextRequest(); // Xử lý các yêu cầu DNS
}

// Định nghĩa phương thức stop()
void WiFiManager::stop()
{
    // Dừng DNSServer
    dnsServer.stop();

    // Dừng WebServer
    server.end();

    // Tắt Access Point
    WiFi.softAPdisconnect(true);
}
