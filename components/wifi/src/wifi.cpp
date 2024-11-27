#include "wifi.hpp"
#include "cstring"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <mutex>
#define WIFI_TAG "wifi sta"
#define IP_TAG "ip sta"
#define DEFAULT_SCAN_LIST_SIZE CONFIG_WIFI_SCAN_LIST_SIZE
#ifdef CONFIG_AP_MODE
#define AP_SSID CONFIG_AP_MODE_SSID
#define AP_PASSWORD CONFIG_AP_MODE_PASSWORD
#define AP_CHANNEL CONFIG_AP_MODE_CHANNEL
#define AP_MAX_CONNECTIONS CONFIG_AP_MODE_MAX_CONNECTIONS
#endif
#ifdef CONFIG_WIFI_USE_SCAN_CHANNEL_BITMAP
#define USE_CHANNEL_BITMAP 1
#define CHANNEL_LIST_SIZE 3
static uint8_t channel_list[CHANNEL_LIST_SIZE] = {1, 6, 11};
#endif /*WIFI_USE_SCAN_CHANNEL_BITMAP*/

namespace device {
  namespace {
std::once_flag init_flag;
wifi_connect_callback_t wifi_connect_cb = nullptr;
void wifi_event(int32_t event_id) {

  switch (event_id) {
  case WIFI_EVENT_STA_START:
    break;
  case WIFI_EVENT_STA_CONNECTED:
    ESP_LOGI(WIFI_TAG, "esp32 connected to ap");
      if(wifi_connect_cb){
        wifi_connect_cb(event_id);
      }
    break;
  case WIFI_EVENT_STA_DISCONNECTED:
    if(wifi_connect_cb){
      wifi_connect_cb(event_id);
    }
    ESP_LOGI(WIFI_TAG, "esp32 connected to ap failed");
    break;
  default:
    break;
  }
}

void ip_event(int32_t event_id) {
  switch (event_id) {
  case IP_EVENT_STA_GOT_IP:
    ESP_LOGI(IP_TAG, "esp32 get ip");
    break;
  case WIFI_EVENT_STA_CONNECTED:
    ESP_LOGI(WIFI_TAG, "esp32 connected to ap");
    break;
  case WIFI_EVENT_STA_DISCONNECTED:
    ESP_LOGI(WIFI_TAG, "esp32 reconnected to ap");

    break;
  default:
    break;
  }
}

#ifdef USE_CHANNEL_BITMAP
static void array_2_channel_bitmap(const uint8_t channel_list[],
                                   const uint8_t channel_list_size,
                                   wifi_scan_config_t *scan_config) {

  for (uint8_t i = 0; i < channel_list_size; i++) {
    uint8_t channel = channel_list[i];
    scan_config->channel_bitmap.ghz_2_channels |= (1 << channel);
  }
}
#endif /*USE_CHANNEL_BITMAP*/
void wifi_event_handle(void *event_handler_arg, esp_event_base_t event_base,
                       int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    wifi_event(event_id);
  } else if (event_base == IP_EVENT) {
    ip_event(event_id);
  }
}
} // namespace
wifi::wifi() {
  std::call_once(init_flag, []() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handle,
                               nullptr);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                               wifi_event_handle, nullptr);

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
#ifdef CONFIG_AP_MODE
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

#ifdef CONFIG_AP_MODE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
#else
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
#endif

#ifdef CONFIG_AP_MODE
    assert(ap_netif);
    // 配置AP
    wifi_config_t ap_config = {
        .ap =
            {
                .ssid = AP_SSID,
                .password = AP_PASSWORD,
                .ssid_len = strlen(AP_SSID),
                .channel = AP_CHANNEL,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .max_connection = AP_MAX_CONNECTIONS,
            },
    };
    if (strlen("password") == 0) {
      ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
#endif

    ESP_ERROR_CHECK(esp_wifi_start());
  });
}
/* Initialize Wi-Fi as sta and set scan method */
std::vector<wifi_ap_record_t> wifi::scan(void) {

#ifdef USE_CHANNEL_BITMAP
  wifi_scan_config_t *scan_config =
      (wifi_scan_config_t *)calloc(1, sizeof(wifi_scan_config_t));
  if (!scan_config) {
    ESP_LOGE(TAG, "Memory Allocation for scan config failed!");
    return;
  }
  array_2_channel_bitmap(channel_list, CHANNEL_LIST_SIZE, scan_config);
  esp_wifi_scan_start(scan_config, true);
  free(scan_config);

#else
  esp_wifi_scan_start(NULL, true);
#endif /*USE_CHANNEL_BITMAP*/
  uint16_t number = DEFAULT_SCAN_LIST_SIZE;
  std::vector<wifi_ap_record_t> ap_list;

  ESP_LOGI(WIFI_TAG, "Max AP number ap_info can hold = %u", number);
  ap_list.resize(number);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_list.data()));
  if (number < ap_list.size()) {
    ap_list.erase(ap_list.begin() + number, ap_list.end());
    ap_list.shrink_to_fit();
  }
  ESP_LOGI(WIFI_TAG, "Total APs scanned = %u", number);
  for (const auto &ap : ap_list) {
    ESP_LOGI(WIFI_TAG, "SSID \t\t%s", ap.ssid);
    ESP_LOGI(WIFI_TAG, "RSSI \t\t%d", ap.rssi);
    ESP_LOGI(WIFI_TAG, "Channel \t\t%d", ap.primary);
  }
  return ap_list;
}
void wifi::connect(std::string SSID, std::string password, wifi_connect_callback_t cb,
                   wifi_auth_mode_t auth_mode) {
  wifi_connect_cb = cb;
  //   wifi_config_t wifi_conf{
  //       .sta = {.threshold = {.rssi = 0, .authmode = WIFI_AUTH_WPA2_PSK},
  //               .pmf_cfg = {.capable = true, .required = false}}};

  wifi_config_t sta_config = {
      .sta =
          {
              .threshold = {.rssi = 0, .authmode = auth_mode},
              .pmf_cfg = {.capable = true, .required = false},
              .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
          },
  };
  std::memset(sta_config.sta.ssid, 0, sizeof(sta_config.sta.ssid));
  std::memcpy(sta_config.sta.ssid, SSID.c_str(), SSID.length());
  std::memset(sta_config.sta.password, 0, sizeof(sta_config.sta.password));
  std::memcpy(sta_config.sta.password, password.c_str(), password.length());
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_connect());
}
wifi::~wifi() {}
} // namespace device
