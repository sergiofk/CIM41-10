#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <esp_spiffs.h>
#include <esp_http_client.h>
#include <esp_http_server.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "ota.h"

#define AP_SSID      "SBCCIM41M10B"
#define AP_PASS      "01234567"
//#define WIFI_SSID      "SBC"
//#define WIFI_PASS      "SBCwifi$"
#define maxIntentos    10

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//Para almacenar credenciales wifi
extern char WIFI_SSID[32];
extern char WIFI_PASS[32];

extern int conectado; // Variable para verificar que está conectado a una red

void sta_init(const char* ssid, const char* pass); // Initialize Wi-Fi as Station (STA)

void init_ap();									// Inicializa access point
void start_server();							// Inicia servidor
esp_err_t root_handler(httpd_req_t *req);		// Controla html
esp_err_t submit_handler(httpd_req_t *req);		// Controla inputs cliente

void init_txt(void);								// Inicializa spiffs
void create_txt(void);								// Crea credenciales.txt
bool txt_exists(void);								// Verifica si existe credenciales.txt
bool txt_empty(void);								// Verifica si hay algo escrito en credenciales.txt
void read_txt(void);								// Lee una linea de credenciales.txt y asigna a WIFI_SSID y WIFI_PASS
void write_txt(char *ssid, char *password);		// Escribe ssid y password en credenciales.txt
void writeDollar(char *str);					// Escribe '$' en vez de '%24'
void delete_txt(void);								// Borra credenciales.txt