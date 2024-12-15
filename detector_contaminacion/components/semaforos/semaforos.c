#include <stdio.h>
#include "semaforos.h"

const char *TAGsem = "SEMAFOROS";

esp_err_t init_semaforo(gpio_num_t led_v, gpio_num_t led_a, gpio_num_t led_r){

    gpio_reset_pin(led_v);
    gpio_reset_pin(led_a);
    gpio_reset_pin(led_r);

    gpio_set_direction(led_v, GPIO_MODE_OUTPUT);
    gpio_set_direction(led_a, GPIO_MODE_OUTPUT);
    gpio_set_direction(led_r, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAGsem, "init_semaforo");
    
    return ESP_OK;
}

esp_err_t init_sem_general(){

    gpio_reset_pin(PIN_ALRM_5);
    gpio_reset_pin(PIN_ALRM_4);
    gpio_reset_pin(PIN_ALRM_3);
    gpio_reset_pin(PIN_ALRM_2);
    gpio_reset_pin(PIN_ALRM_1);

    gpio_set_direction(PIN_ALRM_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_ALRM_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_ALRM_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_ALRM_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_ALRM_1, GPIO_MODE_OUTPUT);    

    ESP_LOGI(TAGsem, "init_semaforo_general");
    
    return ESP_OK;
}

esp_err_t enciende_led(gpio_num_t pin){
    
    gpio_set_level(pin,1);

    return ESP_OK;
}

esp_err_t apaga_led(gpio_num_t pin){
    
    gpio_set_level(pin,0);

    return ESP_OK;
}

void init_semaforos(void){
    init_semaforo(PIN_LED_V_S, PIN_LED_A_S, PIN_LED_R_S);
    init_semaforo(PIN_LED_V_L, PIN_LED_A_L, PIN_LED_R_L);
    init_sem_general();
}

int controla_semaforo_sound(int val){
    if (val < VA_S){
        enciende_led(PIN_LED_V_S);
        apaga_led(PIN_LED_A_S);
        apaga_led(PIN_LED_R_S);
        ESP_LOGI(TAGsem, "Enciende verde sonido");
        return 1;
    }
    else if (val < AR_S){
        apaga_led(PIN_LED_V_S);
        enciende_led(PIN_LED_A_S);
        apaga_led(PIN_LED_R_S);
        ESP_LOGI(TAGsem, "Enciende amarillo sonido");
        return 2;
    }
    else if (val > AR_S){
        apaga_led(PIN_LED_V_S);
        apaga_led(PIN_LED_A_S);
        enciende_led(PIN_LED_R_S);
        ESP_LOGI(TAGsem, "Enciende rojo sonido");
        return 3;
    }
    else {
        apaga_led(PIN_LED_V_S);
        apaga_led(PIN_LED_A_S);
        apaga_led(PIN_LED_R_S);
        ESP_LOGI(TAGsem, "Apaga semaforo sonido");
        ESP_LOGE(TAGsem, "Valor incorrecto para semáforo sonido");
        return -1;
    }
}

int controla_semaforo_light(uint16_t val){
    if (val < VA_L){
        enciende_led(PIN_LED_V_L);
        apaga_led(PIN_LED_A_L);
        apaga_led(PIN_LED_R_L);
        ESP_LOGI(TAGsem, "Enciende verde luz");
        return 1;
    }
    else if (val < AR_L){
        apaga_led(PIN_LED_V_L);
        enciende_led(PIN_LED_A_L);
        apaga_led(PIN_LED_R_L);
        ESP_LOGI(TAGsem, "Enciende amarillo luz");
        return 2;
    }
    else if (val > AR_L){
        apaga_led(PIN_LED_V_L);
        apaga_led(PIN_LED_A_L);
        enciende_led(PIN_LED_R_L);
        ESP_LOGI(TAGsem, "Enciende rojo luz");
        return 3;
    }
    else {
        apaga_led(PIN_LED_V_L);
        apaga_led(PIN_LED_A_L);
        apaga_led(PIN_LED_R_L);
        ESP_LOGI(TAGsem, "Apaga semaforo luz");
        ESP_LOGE(TAGsem, "Valor incorrecto para semáforo luz");
        return -1;
    }
}

void controla_sem_general(int a, int b){
    if(a==1 && b==1){
        apaga_led(PIN_ALRM_5);
        apaga_led(PIN_ALRM_4);
        apaga_led(PIN_ALRM_3);
        apaga_led(PIN_ALRM_2);
        apaga_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General bien");
    }
    else if((a==1 && b==2)|| (a==2 && b==1)){
        apaga_led(PIN_ALRM_5);
        apaga_led(PIN_ALRM_4);
        apaga_led(PIN_ALRM_3);
        apaga_led(PIN_ALRM_2);
        enciende_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General 1");
    }
    else if((a==1 && b==3)|| (a==3 && b==1)){
        apaga_led(PIN_ALRM_5);
        apaga_led(PIN_ALRM_4);
        apaga_led(PIN_ALRM_3);
        enciende_led(PIN_ALRM_2);
        enciende_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General 2");
    }
    else if(a==2 && b==2){
        apaga_led(PIN_ALRM_5);
        apaga_led(PIN_ALRM_4);
        enciende_led(PIN_ALRM_3);
        enciende_led(PIN_ALRM_2);
        enciende_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General 3");
    }
    else if((a==2 && b==3)|| (a==3 && b==2)){
        apaga_led(PIN_ALRM_5);
        enciende_led(PIN_ALRM_4);
        enciende_led(PIN_ALRM_3);
        enciende_led(PIN_ALRM_2);
        enciende_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General 4");
    }
    else if((a==3 && b==3)){
        enciende_led(PIN_ALRM_5);
        enciende_led(PIN_ALRM_4);
        enciende_led(PIN_ALRM_3);
        enciende_led(PIN_ALRM_2);
        enciende_led(PIN_ALRM_1);
        ESP_LOGI(TAGsem, "General 5");
    }
    else ESP_LOGE(TAGsem, "Valor incorrecto para semáforo general");;


}
