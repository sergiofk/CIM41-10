#include <stdio.h>
#include "mqtt_tb.h"

static const char *TAGmqtt = "mqtt";

static esp_mqtt_client_handle_t mqtt_client = NULL;

// MQTT event handler callback
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAGmqtt, "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAGmqtt, "MQTT_EVENT_DISCONNECTED");
            break;
        default:
            ESP_LOGI(TAGmqtt, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

// MQTT event handler registration
static void mqtt_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(TAGmqtt, "Event dispatched from event loop base=%s, event_id=%d", event_base, (int)event_id);
    mqtt_event_handler_cb(event_data);
}

// Initialize MQTT client for ThingsBoard
void mqtt_init() {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = THINGSBOARD_BROKER_URI,
        .credentials.username = THINGSBOARD_ACCESS_TOKEN,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// Publish value to MQTT (ThingsBoard)
void publish_value(int value, const char* nombre) {
    char message[100];
    snprintf(message, sizeof(message), "{\"%s\": %d}", nombre, value);
    esp_mqtt_client_publish(mqtt_client, "v1/devices/me/telemetry", message, 0, 1, 0);
}