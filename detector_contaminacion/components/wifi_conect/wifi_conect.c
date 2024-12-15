#include <stdio.h>
#include "wifi_conect.h"

static const char *TAGwifi = "WIFI";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

char WIFI_SSID[32] = "SBC";
char WIFI_PASS[32] = "SBCwifi$";

int conectado = 0;

// Wi-Fi event handler
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < maxIntentos) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAGwifi, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAGwifi, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        conectado = 1;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAGwifi, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Initialize Wi-Fi as Station (STA)
void sta_init(const char* ssid, const char* pass) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAGwifi, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAGwifi, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
        //OTA
        get_sha256_of_partitions();
    	esp_wifi_set_ps(WIFI_PS_NONE);
    	xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAGwifi, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
        ESP_LOGI(TAGwifi, "Volviendo a modo AP");
        init_ap();
    } else {
        ESP_LOGE(TAGwifi, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

// HTML
char *web = "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<style>"
            "body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; font-family: Arial, sans-serif; }"
            "form { text-align: center; }"
            "h2 { font-size: 40px; margin-bottom: 20px; }"
            "label { font-size: 28px; }"
            "input[type='text'] { width: 100%; padding: 20px; margin: 10px 0; border: 2px solid #ccc; border-radius: 4px; font-size: 24px; }"  // Aumentar tamaño de los cuadros de texto
            "input[type='submit'] { padding: 20px 40px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 26px; }"  // Aumentar tamaño del botón
            "input[type='submit']:hover { background-color: #45a049; }"
            "</style>"
            "</head>"
            "<body>"
            "<form action=\"/submit\" method=\"post\">"
            "<h2>Introduzca SSID y PASSWORD:</h2>"
            "<label for=\"ssid\">SSID:</label><br>"
            "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>"
            "<label for=\"password\">PASSWORD:</label><br>"
            "<input type=\"text\" id=\"password\" name=\"password\"><br>"
            "<input type=\"submit\" value=\"Enviar\">"
            "</form>"
            "</body>"
            "</html>";

void init_ap() {		//	Configuración esp32 para que sea un access point, inicializa la web
    esp_netif_init();										//	Inicializamos interfaz de red
    esp_event_loop_create_default();						//	Bucle de eventos por defecto
    esp_netif_create_default_wifi_ap();						//	Interfaz de red para el modo AP
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();	//	Config prefeterminada del wifi
    esp_wifi_init(&cfg);								//	Inicializa el wifi con la config asignada
    
    wifi_config_t wifi_config = {							//	Config del AP
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    esp_wifi_set_mode(WIFI_MODE_AP);								//	Modo AP
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);	//	Aplicamos config AP
    esp_wifi_start();													//	Inicia el wifi
    start_server();														//	Inicia la web
}

// Inicia servidor
void start_server() {		// 	Configuración servidor http
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();				//	Config predeterminada
    config.server_port = 80;									//	Establecemos el puerto 80 para la web
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {//	Iniciamos servidor
    	//ruta para root_handler
        httpd_register_uri_handler(server, &(httpd_uri_t){ .uri = "/", .method = HTTP_GET, .handler = root_handler, .user_ctx = NULL });
        //ruta para submit_handler
        httpd_register_uri_handler(server, &(httpd_uri_t){ .uri = "/submit", .method = HTTP_POST, .handler = submit_handler, .user_ctx = NULL });
    }
}

// Controla html 
esp_err_t root_handler(httpd_req_t *req) { //esp_err_t es un tipo de dato que representa error o exito
    const char *html = web;
    httpd_resp_send(req, html, strlen(html));		//	Enviamos el HTML al cliente
    return ESP_OK;			//	Devolvemos exito
}

// Controla inputs cliente
esp_err_t submit_handler(httpd_req_t *req) {		//	Guardar SSID y PW y mostrarla en terminal
	char buf[200];
	int received = httpd_req_recv(req, buf, sizeof(buf) - 1);
	if (received <= 0) return ESP_FAIL;
	buf[received] = '\0';							//	Se añade \0 para que sea una cadena de texto valida
	char *ssid, *password;
	//	Busca cadena ssid= en los datos recibidos, lo mismo con password=
    ssid = strstr(buf, "ssid=");
    password = strstr(buf, "password=");
    if (ssid && password) {
        ssid += strlen("ssid=");			//	El puntero de ssid se situa a continuacion de ssid=
        password += strlen("password=");	//	El puntero de password se situa a continuacion de password=
        char *end = strchr(ssid, '&');		//	Busca el & que separa los datos ingresados en la web 
        if (end) *end = '\0';				//	Si lo encuentra, mete un \0 para cortar ahi y quedarse solo con el ssid

        for (char *p = ssid; *p; p++) if (*p == '+') *p = ' ';		// Cambiar '+' a espacio
        
        end = strchr(password , '&');
        if (end) *end = '\0';
         
        for (char *p = password; *p; p++) if (*p == '+') *p = ' ';
         
        printf("SSID recibido: %s\n", ssid);
        printf("Password recibido: %s\n", password);
        write_txt(ssid, password);
		read_txt();
		esp_restart();
                
    }

    httpd_resp_set_status(req, "303 See Other");		//	Codigo de respuesta (redireccion)
    httpd_resp_set_hdr(req, "Location", "/");		//	Dirigimos al cliente a la pagina principal (/)
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void init_txt(){
	esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);
}

void create_txt(){
	printf("Creando archivo\n");
    FILE *f = fopen("/spiffs/credentials.txt", "w");
    if (f == NULL)
    {
        printf("Fallo a la hora de abrir el archivo para escribir\n");
        return;
    }
    fclose(f);
}

bool txt_exists(){
	bool ok = true;
    FILE *f = fopen("/spiffs/credentials.txt", "r");
    if (f == NULL){
        ok = false;
        printf("No existe credenciales.txt\n");
    }
    else printf("Existe credenciales.txt\n");
    fclose(f);
    return ok;
}

bool txt_empty(){
    FILE *f = fopen("/spiffs/credentials.txt", "r");
    if (f == NULL)
    {
        printf("No existe credenciales.txt");
    }
	fseek(f, 0, SEEK_END);
  	long size = ftell(f);
   	fclose(f);  
    return size == 0;
}

void read_txt(){
	FILE *file = fopen("/spiffs/credentials.txt", "r");
    char buffer[256]; 
    char SSID[64];
    char PASS[64];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "%63s %63s", SSID, PASS) == 2) {
			writeDollar(SSID);
			writeDollar(PASS);
			
			printf("SSID: %s, PASSWORD: %s\n", SSID, PASS);
			strcpy(WIFI_SSID, SSID);
			strcpy(WIFI_PASS, PASS);
        }
    }
    fclose(file);
}

void write_txt(char *ssid, char *password){
	FILE *f = fopen("/spiffs/credentials.txt", "a");
	printf("Escribiendo\n");
    fprintf(f, "%s %s\n", ssid, password);
    fclose(f);
    printf("Escritura finalizada\n");
}

void writeDollar(char *str) {
    char *pos;
    while ((pos = strstr(str, "%24")) != NULL) {
        *pos = '$';                        // Sustituimos % por $
        memmove(pos + 1, pos + 3, strlen(pos + 3) + 1);  // Mueve el resto de la cadena
    }
}

void delete_txt(){
	remove("/spiffs/credentials.txt");
}