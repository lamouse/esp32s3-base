#include "wifi.hpp"
#include "cstring"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#define WIFI_TAG "wifi sta"
#define IP_TAG "ip sta"
#define DEFAULT_SCAN_LIST_SIZE CONFIG_WIFI_SCAN_LIST_SIZE
#ifdef CONFIG_WIFI_USE_SCAN_CHANNEL_BITMAP
#define USE_CHANNEL_BITMAP 1
#define CHANNEL_LIST_SIZE 3
static uint8_t channel_list[CHANNEL_LIST_SIZE] = {1, 6, 11};
#endif /*WIFI_USE_SCAN_CHANNEL_BITMAP*/
namespace {
void wifi_event(int32_t event_id) {
  switch (event_id) {
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
    esp_wifi_connect();
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
namespace device {
wifi::wifi(){
  //   ESP_ERROR_CHECK(nvs_flash_init());
  //   ESP_ERROR_CHECK(esp_netif_init());
  //   ESP_ERROR_CHECK(esp_event_loop_create_default());
  //   esp_netif_create_default_wifi_sta();
  //   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //   ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  //   esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
  //   wifi_event_handle,
  //                              nullptr);
  //   esp_event_handler_register(WIFI_EVENT, IP_EVENT_STA_GOT_IP,
  //   wifi_event_handle,
  //                              nullptr);
  //   wifi_config_t wifi_conf{
  //       .sta = {.threshold = {.rssi = 0, .authmode = WIFI_AUTH_WPA2_PSK},
  //               .pmf_cfg = {.capable = true, .required = false}}};
  //   std::memset(wifi_conf.sta.ssid, 0, sizeof(wifi_conf.sta.ssid));
  //   std::memcpy(wifi_conf.sta.ssid, SSID.c_str(), SSID.length());
  //   std::memset(wifi_conf.sta.password, 0, sizeof(wifi_conf.sta.password));
  //   std::memcpy(wifi_conf.sta.password, password.c_str(), password.length());

  //   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  //   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_conf));
  //   ESP_ERROR_CHECK(esp_wifi_start());
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
}
/* Initialize Wi-Fi as sta and set scan method */
std::vector<wifi_ap_record_t> wifi::scan(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));



  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

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
  if(number < ap_list.size()){
    ap_list.erase(ap_list.begin() + number, ap_list.end());
    ap_list.shrink_to_fit();
  }
  ESP_LOGI(WIFI_TAG,
           "Total APs scanned = %u", number);
  for (const auto & ap : ap_list) {
    ESP_LOGI(WIFI_TAG, "SSID \t\t%s", ap.ssid);
    ESP_LOGI(WIFI_TAG, "RSSI \t\t%d", ap.rssi);
    ESP_LOGI(WIFI_TAG, "Channel \t\t%d", ap.primary);
  }
  return ap_list;
}
wifi::~wifi() {}
} // namespace device
