#include <stdio.h>
#include "ota.h"

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
/* The interface name value can refer to if_desc in esp_netif_defaults.h */
#if CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF_ETH
static const char *bind_interface_name = EXAMPLE_NETIF_DESC_ETH;
#elif CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF_STA
static const char *bind_interface_name = EXAMPLE_NETIF_DESC_STA;
#endif
#endif

static const char *TAG = "OTA";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void ota_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting OTA task");
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
    esp_netif_t *netif = get_example_netif_from_desc(bind_interface_name);
    if (netif == NULL) {
        ESP_LOGE(TAG, "Can't find netif from interface description");
        abort();
    }
    struct ifreq ifr;
    esp_netif_get_netif_impl_name(netif, ifr.ifr_name);
    ESP_LOGI(TAG, "Bind interface name is %s", ifr.ifr_name);
#endif
    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL,
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach,
#else
        .cert_pem = (char *)server_cert_pem_start,
#endif /* CONFIG_EXAMPLE_USE_CERT_BUNDLE */
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
        .if_name = &ifr,
#endif
    };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s %s", label, hash_print);
}

void get_sha256_of_partitions(void)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}
