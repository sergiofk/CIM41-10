#include <stdlib.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

#define PIN_LED_R_S   GPIO_NUM_13
#define PIN_LED_A_S   GPIO_NUM_27
#define PIN_LED_V_S   GPIO_NUM_26

#define PIN_LED_R_L   GPIO_NUM_25
#define PIN_LED_A_L   GPIO_NUM_33
#define PIN_LED_V_L   GPIO_NUM_32

#define PIN_ALRM_5   GPIO_NUM_23
#define PIN_ALRM_4   GPIO_NUM_22
#define PIN_ALRM_3   GPIO_NUM_21
#define PIN_ALRM_2   GPIO_NUM_4
#define PIN_ALRM_1   GPIO_NUM_2


// Defines para los umbrales de activación de los semáforos
#define VA_L 85     // Cambio de verde a amarillo para luz
#define AR_L 170    // Cambio de amarillo a rojo para luz
#define VA_S 683    // Cambio de verde a amarillo para sonido
#define AR_S 1366   // Cambio de amarillo a rojo para sonido

void init_semaforos(void); // inicializa semáforos luz y sonido

// Controladores de semáforos en función de los defines
int controla_semaforo_sound(int val);  
int controla_semaforo_light(uint16_t val);
void controla_sem_general(int a, int b);
