#include <string>
#include <format>
#include <BusDriver.h>
#include <ModuleScanner.h>
#include <Modules/PushButtonModule.h>
#include <Modules/TeleruptorModule.h>
#include <Pins.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "secrets.h"

#define LED_GPIO GPIO_NUM_2

BusDriver driver;
Bus bus(driver);
ModuleScanner scanner(bus);

auto inputPin = std::make_shared<InputPin<bool>>([](bool state) { gpio_set_level(LED_GPIO, state ? 0 : 1); }, false);
PushButtonModule pushButtonModule(bus, 0x03, 0x8000);

TeleruptorModule teleruptorModule(bus, 0x05, 0x0008);

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    #pragma GCC diagnostic pop

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    // Wait for connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

esp_err_t hello_handler(httpd_req_t *req)
{
    // auto modules = scanner.DetectModules();
    // std::string response = std::format("Found {} module(s)", modules.size());

    // for (int i = 0; i < modules.size(); i++)
    // {
    //     response += std::format("\nAddr: {} - Type: {}", (int)modules[i]->GetAddress(), (int)modules[i]->GetType());
    // }

    // httpd_resp_send(req, response.c_str(), response.length());

    httpd_resp_send(req, "Hello from Domotech!", HTTPD_RESP_USE_STRLEN);

    // auto response = bus.Exchange(0x05, 0x06);

    // if (response.Success)
    // {
    //     std::string responseStr = std::format("Module at address 0x03 responded with type {} and data {}", response.ModuleType, response.Data);
    //     httpd_resp_send(req, responseStr.c_str(), responseStr.length());
    // }
    // else
    // {
    //     httpd_resp_send(req, "Module not found or communication failed", HTTPD_RESP_USE_STRLEN);
    // }

    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello_uri =
        {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = hello_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &hello_uri);
    }

    return server;
}

void ScanTask(void *arg)
{
    while (true)
    {
        pushButtonModule.Process();
        vTaskDelay(1);
        teleruptorModule.Process();
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    driver.Init();
    nvs_flash_init();       // Required for Wi-Fi
    wifi_init_sta();        // Connect to Wi-Fi

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    inputPin->ConnectTo(teleruptorModule.GetDigitalOutputPins()[2]);

    auto pbPins = pushButtonModule.GetDigitalOutputPins();
    auto telPins = teleruptorModule.GetDigitalInputPins();

    for (uint8_t i = 0; i < pbPins.size(); ++i)
    {
        auto pbPin = pbPins[i].lock();
        auto telPin = telPins[i].lock();

        telPin->ConnectTo(pbPin);
    }

    // xTaskCreate(
    //     LedTask,     // Task function
    //     "Led task",  // Name (for debugging)
    //     4096,        // Stack size in bytes
    //     NULL,        // Task parameter
    //     5,           // Task priority (higher = more important)
    //     NULL         // Optional handle
    // );

    xTaskCreate(
        ScanTask,    // Task function
        "Scan task", // Name (for debugging)
        4096,        // Stack size in bytes
        NULL,        // Task parameter
        5,           // Task priority (higher = more important)
        NULL         // Optional handle
    );

    start_webserver();      // Launch HTTP server
}
