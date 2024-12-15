idf_component_register(SRCS "wifi_conect.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_wifi" "esp_http_client" "esp_http_server" "spiffs" "ota")
