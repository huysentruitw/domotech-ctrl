#include <string>
#include <format>
#include <BusDriver.h>
#include <ModuleScanner.h>
#include <Pin.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "secrets.h"

BusDriver driver;
BusProtocol bus(driver);
ModuleScanner scanner(bus);

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
    auto modules = scanner.DetectModules();
    std::string response = std::format("Found {} module(s)", modules.size());

    for (int i = 0; i < modules.size(); i++)
    {
        response += std::format("\nAddr: {} - Type: {}", (int)modules[i]->GetAddress(), (int)modules[i]->GetType());
    }

    httpd_resp_send(req, response.c_str(), response.length());

    /*bus.Exchange(0x03, 0x0006, 1);
    httpd_resp_send(req, "OK", 2);*/

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

#define LED_GPIO GPIO_NUM_2

void led_toggle_task(void *arg)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_GPIO, 1); // Turn LED on
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(LED_GPIO, 0); // Turn LED off
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Optional: cleanup (never reached unless the task is explicitly deleted)
    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    driver.Init();
    nvs_flash_init();       // Required for Wi-Fi
    wifi_init_sta();        // Connect to Wi-Fi

    xTaskCreate(
        led_toggle_task,    // Task function
        "LED toggle",       // Name (for debugging)
        4096,               // Stack size in bytes
        NULL,               // Task parameter
        5,                  // Task priority (higher = more important)
        NULL                // Optional handle
    );

    start_webserver();      // Launch HTTP server
}
