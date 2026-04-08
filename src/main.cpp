#include <HomeAssistantBridge.h>
#include <LittleFsStorage.h>
#include <Manager.h>
#include <Wifi.h>

#include "KeyVault.h"
#include "SetupWebServer.h"
#include "SystemClock.h"
#include "WebServer.h"

#include "esp_log.h"
#include "nvs_flash.h"

#define VERSION "1.0"
#define POSIX_TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3" // Belgium

LittleFsStorage storage;
Wifi wifi;
HomeAssistantBridge homeAssistantBridge;
Manager manager(storage, homeAssistantBridge);
SystemClock systemClock(POSIX_TIMEZONE);

void ProcessTask(void *arg)
{
    while (true)
    {
        manager.ProcessNext();
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());  // Required for Wi-Fi
    wifi.Init();

    Secrets secrets = {};
    if (!KeyVault::LoadSecrets(secrets) || !wifi.SwitchToStationMode(secrets.WifiSsid, secrets.WifiPass, pdMS_TO_TICKS(10000)))
    {
        ScanLed led;
        ESP_LOGW("MAIN", "STA failed, starting AP config mode");
        wifi.SwitchToAccessPointMode("DOMOTECH_CTRL", "domotech");

        SetupWebServer::Start();

        while (true)
        {
            led.Toggle();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI("MAIN", "Started!");

    systemClock.Init();
    homeAssistantBridge.Init(secrets.HaMqttUri, secrets.HaMqttUser, secrets.HaMqttPass);
    manager.Start();

    xTaskCreate(
        ProcessTask,    // Task function
        "PROCESS",      // Name (for debugging)
        4096,           // Stack size in bytes
        NULL,           // Task parameter
        5,              // Task priority (higher = more important)
        NULL            // Optional handle
    );

    WebServer::Start(
        VERSION,
        storage,
        manager,
        systemClock,
        wifi);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
