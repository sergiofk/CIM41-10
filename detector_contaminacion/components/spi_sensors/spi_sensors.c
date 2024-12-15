#include <stdio.h>
#include "spi_sensors.h"

static const char *TAGspi = "SPI";

// Declaración de variables y estructuras necesarias
static esp_err_t ret;
static spi_device_handle_t spi_light;
static spi_device_handle_t spi_sound;

// Función para inicializar el bus SPI del sensor de luz (VSPI)
void spi_init_light_sensor(void)
{
    spi_bus_config_t buscfg_light = {
        .miso_io_num = PIN_NUM_MISO_LIGHT,
        .sclk_io_num = PIN_NUM_CLK_LIGHT,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 512 * 8
    };

    spi_device_interface_config_t devcfg_light = {
        .clock_speed_hz = 4 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS_LIGHT,
        .queue_size = 7
    };

    ret = spi_bus_initialize(ESP_HOST_LIGHT, &buscfg_light, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(ESP_HOST_LIGHT, &devcfg_light, &spi_light);
    ESP_ERROR_CHECK(ret);
}

// Función para inicializar el bus SPI del sensor de sonido (HSPI)
void spi_init_sound_sensor(void)
{
    spi_bus_config_t buscfg_sound = {
        .miso_io_num = PIN_NUM_MISO_SOUND,
        .sclk_io_num = PIN_NUM_CLK_SOUND,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 512 * 8
    };

    spi_device_interface_config_t devcfg_sound = {
        .clock_speed_hz = 4 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS_SOUND,
        .queue_size = 7
    };

    ret = spi_bus_initialize(ESP_HOST_SOUND, &buscfg_sound, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(ESP_HOST_SOUND, &devcfg_sound, &spi_sound);
    ESP_ERROR_CHECK(ret);
}


// Función para leer datos del sensor de luz
uint16_t spi_read_light_sensor(void)
{
    uint8_t tx_data[2] = {0x80, 0x00};  // Comando para leer datos
    spi_transaction_t trans_desc = {
        .flags = SPI_TRANS_USE_RXDATA,
        .tx_buffer = tx_data,
        .rx_buffer = NULL,
        .rxlength = 16,
        .length = 16
    };

    gpio_set_level(PIN_NUM_CS_LIGHT, 0);  // Activar CS del sensor de luz
    ret = spi_device_polling_transmit(spi_light, &trans_desc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAGspi, "Error en la transmisión SPI del sensor de luz");
    }

    uint16_t received_data = (trans_desc.rx_data[0] << 8) | trans_desc.rx_data[1]; // Extraer los 16 bits
    uint16_t light_level = (received_data >> 3) & 0xFF;  // Extraer 8 bits de datos (bits 4-11)

    //gpio_set_level(PIN_NUM_CS_LIGHT, 1);  // Desactivar CS del sensor de luz

    return light_level;
}

// Función para leer datos del sensor de sonido
uint16_t spi_read_sound_sensor(void)
{
    uint8_t tx_data[2] = {0x00, 0x00};  // Comando para leer datos (dummy data)
    spi_transaction_t trans_desc = {
        .flags = SPI_TRANS_USE_RXDATA,
        .tx_buffer = tx_data,
        .rx_buffer = NULL,
        .rxlength = 16,
        .length = 16
    };

    gpio_set_level(PIN_NUM_CS_SOUND, 0);  // Activar CS del sensor de sonido
    ret = spi_device_polling_transmit(spi_sound, &trans_desc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAGspi, "Error en la transmisión SPI del sensor de sonido");
    }

    uint16_t received_data = (trans_desc.rx_data[0] << 8) | trans_desc.rx_data[1]; // Extraer los 16 bits
    uint16_t mic_data = received_data & 0x0FFF;  // Extraer los 12 bits de datos (bits 4-15)
    mic_data = abs(mic_data-2047); // Acondionar los valores de sonido posicionando con simetría respecto al eje x con y=0 y obteniendo su valor absoluto para obtener el volumen
    //gpio_set_level(PIN_NUM_CS_SOUND, 1);  // Desactivar CS del sensor de sonido

    return mic_data; 
}
