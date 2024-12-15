#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#define THINGSBOARD_BROKER_URI "mqtt://demo.thingsboard.io"
#define THINGSBOARD_ACCESS_TOKEN "9SNPWL8serDQAIQXAync"

void mqtt_init(void);   //establece la conexion mqtt
void publish_value(int value, const char* nombre); //publica valor en thingsboard