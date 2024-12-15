idf_component_register(SRCS "ota.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_event" "app_update" "esp_http_client" "esp_https_ota" "protocol_examples_common" "esp_netif" "mbedtls" "nvs_flash" "esp_wifi")
