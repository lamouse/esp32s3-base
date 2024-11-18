#include "wifi.hpp"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "cstring"
#define WIFI_TAG "wifi sta"
#define IP_TAG "ip sta"
namespace
{
    void wifi_event(int32_t event_id){
            switch (event_id)
            {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(WIFI_TAG, "esp32 connected to ap");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(WIFI_TAG, "esp32 reconnected to ap");
                esp_wifi_connect();
                break;
            default:
                break;
            }
    }

        void ip_event(int32_t event_id){
            switch (event_id)
            {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI(IP_TAG, "esp32 get ip");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(WIFI_TAG, "esp32 connected to ap");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(WIFI_TAG, "esp32 reconnected to ap");
                esp_wifi_connect();
                break;
            default:
                break;
            }
    }

    void wifi_event_handle(void *event_handler_arg,
                           esp_event_base_t event_base,
                           int32_t event_id,
                           void *event_data)
    {
        if(event_base == WIFI_EVENT){
            wifi_event(event_id);
        }else if(event_base == IP_EVENT){
            ip_event(event_id);
        }
    }
}
namespace device
{
    wifi::wifi(std::string ssid, std::string password):SSID(ssid),password(password)
    {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handle, nullptr);
        esp_event_handler_register(WIFI_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handle, nullptr);
        wifi_config_t wifi_conf
        {
            .sta = {
                .threshold = {.rssi = 0, .authmode = WIFI_AUTH_WPA2_PSK},
                .pmf_cfg = {.capable = true, .required = false}
                }
        };
        std::memset(wifi_conf.sta.ssid, 0 , sizeof(wifi_conf.sta.ssid));
        std::memcpy(wifi_conf.sta.ssid, SSID.c_str(), SSID.length());
        std::memset(wifi_conf.sta.password, 0 , sizeof(wifi_conf.sta.password));
        std::memcpy(wifi_conf.sta.password, password.c_str(), password.length());


        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_conf));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    wifi::~wifi()
    {
    }
}
