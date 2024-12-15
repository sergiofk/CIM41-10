#include <math.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt_tb.h"
#include "semaforos.h"
#include "spi_sensors.h"
#include "wifi_conect.h"

#define TASK_FREQUENCY 2000 // Frecuencia de la tarea en milisegundos (2 segundos)
#define MED_SOUND 500       // Mediciones de sonido durante TASK_FREQUENCY para obtener la máxima

void init_wifi(void);                                       // Para inicializar Wi-Fi y gestionar la configuración de conexión
int read_publish_sem_light(void);                          // Para leer, imprimir y publicar el nivel de luz
int read_publish_sem_sound(void);                          // Para leer, procesar y publicar el nivel de sonido
void delay_for_sync(int start_time, int task_frequency);    // Función para sincronizar el ciclo con un retraso configurado

void app_main(void) {
    
    init_wifi();
    spi_init_light_sensor();    // Initialize light sensor
    spi_init_sound_sensor();    // Initialize sound sensor
    init_semaforos();   

    if (conectado) {
        mqtt_init();    // Initialize MQTT client
        
        while (1) {
            // Tiempo de inicio para la sincronización de tareas
            int start_time = xTaskGetTickCount();  // Obtener el tiempo actual (en ticks de FreeRTOS)

            // Llamar a las funciones para leer, imprimir, publicar y activar semáforos con los valores de luz y sonido
            int a = read_publish_sem_light();
            int b = read_publish_sem_sound();
            controla_sem_general(a, b);

            // Esperar hasta que pase el tiempo deseado (configurado por TASK_FREQUENCY)
            delay_for_sync(start_time, TASK_FREQUENCY);
        }
    }
}

// Función para inicializar Wi-Fi y gestionar la configuración de conexión
void init_wifi(void){
    delete_txt();
    nvs_flash_init();               // Initialize NVS flash for Wi-Fi credentials storage
    init_txt();
	//sta_init("SBC","SBCwifi$");
	if(txt_exists()) {
		if(txt_empty()) {
			printf("Credentials esta vacio. Inicializando modo AP.\n");
			init_ap();
		}
		else {
			printf("Credentials no esta vacio. Inicializando modo WIFI e intentando conectar.\n");
			read_txt();
			sta_init(WIFI_SSID, WIFI_PASS);
		}
	}
	else {
		create_txt();
		esp_restart();
	}
}

// Función para leer, imprimir, publicar y activar semáforo del sensor de luz
int read_publish_sem_light(void) {
    uint16_t light_level = spi_read_light_sensor(); // Leer el sensor de luz
    printf("Nivel de luz: %d\n", light_level);      // Mostrar el valor de luz
    publish_value(light_level, "light_level");      // Enviar el valor de luz a ThingsBoard
    return controla_semaforo_light(light_level);           // Activa semáforo correspondiente en función del valor
}

// Función para leer, procesar, imprimir, publicar y activar semáforo del sensor de sonido
int read_publish_sem_sound(void) {
    int cont = 0;             // Contador de mediciones
    int sum_squares = 0;      // Suma de los cuadrados de las mediciones
    uint16_t mic_data = 0;    // Variable para almacenar la lectura del sensor de sonido

    while (cont < MED_SOUND) {
        mic_data = spi_read_sound_sensor();  // Leer el sensor de sonido
        sum_squares += mic_data * mic_data;  // Sumar el cuadrado de la medición
        cont++;  // Incrementar el contador de mediciones
        vTaskDelay((TASK_FREQUENCY / MED_SOUND) / portTICK_PERIOD_MS);  // Realiza el número de MED_SOUND mediciones durante el tiempo de TASK_FREQUENCY
    }
    // Después de MED_SOUND mediciones de sonido, calcular el valor RMS
    int rms_value = (int)sqrt((double)sum_squares / MED_SOUND);  // Calcular el RMS y convertir a int

    printf("Nivel de sonido RMS: %d\n", rms_value); // Mostrar el valor de sonido
    publish_value(rms_value, "mic_data");           // Enviar el valor de sonido a ThingsBoard
    return controla_semaforo_sound(rms_value);             // Activa semáforo correspondiente en función del valor RMS
}

// Función para sincronizar el ciclo con un retraso configurado
void delay_for_sync(int start_time, int task_frequency) {
    int elapsed_time = xTaskGetTickCount() - start_time;
    if (elapsed_time < task_frequency / portTICK_PERIOD_MS) {
        vTaskDelay((task_frequency / portTICK_PERIOD_MS) - elapsed_time);  // Asegurar que el ciclo dure el tiempo configurado
    }
}
