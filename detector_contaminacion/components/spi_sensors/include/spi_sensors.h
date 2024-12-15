#include <stdlib.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

// Definición de pines y configuraciones para cada sensor
// Sensor de Luz (VSPI)
#define PIN_NUM_CS_LIGHT    GPIO_NUM_5
#define PIN_NUM_MISO_LIGHT  GPIO_NUM_19
#define PIN_NUM_CLK_LIGHT   GPIO_NUM_18
#define ESP_HOST_LIGHT      VSPI_HOST

// Sensor de Sonido (HSPI)
#define PIN_NUM_CS_SOUND    GPIO_NUM_15
#define PIN_NUM_MISO_SOUND  GPIO_NUM_12
#define PIN_NUM_CLK_SOUND   GPIO_NUM_14
#define ESP_HOST_SOUND      HSPI_HOST

// Funciones para la inicialización y lectura
void spi_init_light_sensor(void);
void spi_init_sound_sensor(void);
uint16_t spi_read_light_sensor(void);
uint16_t spi_read_sound_sensor(void);
