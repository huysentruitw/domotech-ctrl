#include "WebServer.h"

#include <FilterFactory.h>
#include <IniReader.h>

#include "KeyVault.h"

#include "esp_http_server.h"

#include <string>

struct WebServerContext
{
    std::string_view version;
    IStorage* storage;
    Manager* manager;
    SystemClock* systemClock;
    Wifi* wifi;
};

esp_err_t index_handler(httpd_req_t *req)
{
    const auto* context = (WebServerContext*)req->user_ctx;

    std::string response = "Domotech CTRL\n\n";

    response.append("Version: ");
    response.append(context->version);
    response.append("\n");

    response.append("Free memory: ");
    response.append(std::to_string(esp_get_free_heap_size()));
    response.append("bytes\n");

    int8_t rssi = context->wifi->GetRssi();
    if (rssi > INT8_MIN)
    {
        response.append("Wifi RSSI: ");
        response.append(std::to_string(rssi));
        response.append("dBm, quality: ");
        const auto quality = Wifi::RssiToQuality(rssi);
        response.append(std::to_string(quality));
        response.append("%\n");
    }

    response.append("Current time: ");
    context->systemClock->WriteCurrentTime(response);

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t known_filters_handler(httpd_req_t *req)
{
    const auto ini = FilterFactory::GetKnownFiltersIni();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, ini.c_str());
    return ESP_OK;
}

esp_err_t configuration_rescan_handler(httpd_req_t *req)
{
    const auto* context = (WebServerContext*)req->user_ctx;
    const auto result = context->manager->RescanModules();
    std::string response = "Found " + std::to_string(result.NumberOfDetectedModules) + " module(s)";
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t configuration_create_filter_handler(httpd_req_t *req)
{
    const auto* context = (WebServerContext*)req->user_ctx;

    char body[256];
    int received = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0)
    {
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }

    body[received] = '\0'; // Terminate

    bool inFilterSection;
    std::string id;
    std::string type;
    std::string connections;
    size_t numberOfFiltersCreated = 0;

    IniReader reader;
    reader.OnSection([&](std::string_view section)
    {
        inFilterSection = (section == "Filter");
        id.clear();
        type.clear();
        connections.clear();
    });
    
    reader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (!inFilterSection)
            return;

        if (key == "Id") id = value;
        if (key == "Type") type = value;
        if (key == "Connections") connections = value;

        if (!id.empty() && !type.empty() && !connections.empty())
        {
            if (context->manager->CreateFilter(id, type, connections).Status == CreateFilterStatus::NoError)
                numberOfFiltersCreated++;

            type.clear();
            connections.clear();
        }
    });
    reader.Feed(body, received);
    reader.Finalize();

    std::string response = "Created " + std::to_string(numberOfFiltersCreated) + " filter(s)";
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t list_files_handler(httpd_req_t *req)
{
    const auto* context = (WebServerContext*)req->user_ctx;

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "test/plain");

    bool ok = context->storage->EnumerateFiles(
        [&](std::string_view fileName)
        {
            if (httpd_resp_send_chunk(req, fileName.data(), fileName.size()) != ESP_OK)
                return false;

            if (httpd_resp_send_chunk(req, "\n", 1) != ESP_OK)
                return false;

            return true;
        });

    httpd_resp_send_chunk(req, nullptr, 0);

    return ok ? ESP_OK : ESP_FAIL;
}

esp_err_t get_file_handler(httpd_req_t *req)
{
    const auto* context = (WebServerContext*)req->user_ctx;
    const char* fileName = req->uri + strlen("/files/");

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/octet-stream");

    bool ok = context->storage->ReadFileInChunks(
        fileName,
        [&](const char* chunk, size_t chunkSize)
        {
            return httpd_resp_send_chunk(req, chunk, chunkSize) == ESP_OK;
        });

    httpd_resp_send_chunk(req, nullptr, 0);

    return ok ? ESP_OK : ESP_FAIL;
}

esp_err_t reset_handler(httpd_req_t *req)
{
    KeyVault::Clear();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Reset complete! Please reboot the device.");
    return ESP_OK;
}

void WebServer::Start(std::string_view version, IStorage& storage, Manager& manager, SystemClock& systemClock, Wifi& wifi) noexcept
{
    static WebServerContext context
    {
        .version = version,
        .storage = &storage,
        .manager = &manager,
        .systemClock = &systemClock,
        .wifi = &wifi,
    };

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    config.stack_size = 8192;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello_uri =
        {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &hello_uri);

        httpd_uri_t known_filters_uri =
        {
            .uri       = "/known-filters",
            .method    = HTTP_GET,
            .handler   = known_filters_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &known_filters_uri);

        httpd_uri_t configuration_rescan_uri =
        {
            .uri       = "/configuration/rescan",
            .method    = HTTP_POST,
            .handler   = configuration_rescan_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &configuration_rescan_uri);

        httpd_uri_t configuration_create_filter_uri =
        {
            .uri       = "/configuration/filter",
            .method    = HTTP_POST,
            .handler   = configuration_create_filter_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &configuration_create_filter_uri);

        httpd_uri_t list_files_uri =
        {
            .uri       = "/files",
            .method    = HTTP_GET,
            .handler   = list_files_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &list_files_uri);

        httpd_uri_t get_file_uri =
        {
            .uri       = "/files/*",
            .method    = HTTP_GET,
            .handler   = get_file_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &get_file_uri);

        httpd_uri_t reset_uri =
        {
            .uri       = "/reset",
            .method    = HTTP_POST,
            .handler   = reset_handler,
            .user_ctx  = &context,
        };
        httpd_register_uri_handler(server, &reset_uri);
    }
}
