/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_EXAMPLE";

#define PUBLISH_PERIOD 2000
uint32_t MQTT_CONNECTED = 0;
esp_mqtt_client_handle_t mqtt_client = NULL;
// FFM

bool machine_identified = false;
#define MAX_N_MACHINES 30
#define MAX_N_CHARS 100

// Define un tipo de estructura llamado machine_t que contiene: 
// 1 cadena de caracteres para el nombre de la máquina
// 1 cadena de caracteres para el correo del destintario de la notificación
// 1 cadena de caracteres para el estado actual de la máquina
// 1 cadena de caracteres para la asignación de trabajo
// 1 cadena de caracteres para la asignación de trabajador
// 1 cadena de caracteres para el progreso del trabajo
// 1 cadena de caracteres para la notificación
typedef struct
{
    char machine_name[MAX_N_CHARS];
    char IP_address[MAX_N_CHARS];
    char current_status[MAX_N_CHARS];
    char work_assignment[MAX_N_CHARS];
    char worker_assignment[MAX_N_CHARS];
    char progress[MAX_N_CHARS];
    char notification[MAX_N_CHARS];
} machine_t;

machine_t student_machine = {
    .machine_name = "por descubrir",
    .IP_address = "por descubrir",
    .current_status = "por descubrir",
    .work_assignment = "por descubrir",
    .worker_assignment = "itahisa.hernandezf@alumnos.upm.es",
    .progress = "por descubrir",
    .notification = "por descubrir"
};

// Declare a struct array machines_generating_notifications of type machine_t
machine_t machines_generating_notifications[MAX_N_MACHINES];
int num_machines_generating_notifications = 0;

// Funcion que extrae el nombre de la máquina de la cadena de caracteres de un topico
// Por ejemplo, si topic es "LSE/machines/machine12/worker_assignment" extraemos machine12
char *extract_machine_name(char *topic_string)
{
    char *pch;
    pch = strtok(topic_string, "/");
    pch = strtok(NULL, "/");
    pch = strtok(NULL, "/");
    return pch;
}

char *extract_info(char *topic_string)
{
    char *pch;
    pch = strtok(topic_string, "/");
    pch = strtok(NULL, "/");
    pch = strtok(NULL, "/");
    pch = strtok(NULL, "/");

    return pch;
}

// Funcion que dado un nombre de maquina devuelve el indice en el array de maquinas machine_t en caso de que esta este inlcuida en el mismo y -1 si no lo está
int find_machine_index(char *machine_name, int num_machines_generating_notifications)
{
    int i = 0;
    int index = -1;

    if (num_machines_generating_notifications > 0) {
        for (i = 0; i < num_machines_generating_notifications; i++)
        {
            if (strcmp(machines_generating_notifications[i].machine_name, machine_name) == 0)
            {
                //printf("\r\nFOUND in posicion %d %s\r\n", i, machine_name);
                index = i;
                break;
            }
        }    
    }
    return index;
}

// Funcion que dado un indice de maquina en el array de maquinas machine_t imprime por consola el contenido de la misma
void print_machine(machine_t machine)
{
    printf("\r\nMACHINE REPORT\r\n");
    printf("--------------\r\n");
    printf("machine_name: %s\r\n", machine.machine_name);
    printf("IP_address: %s\r\n", machine.IP_address);
    printf("current_status: %s\r\n", machine.current_status);
    printf("work_assignment: %s\r\n", machine.work_assignment);
    
    // Compruebo si worker_assignment es NULL para pintar "UNASSIGNED" en lugar de NULL
    if (strlen(machine.worker_assignment)<=0)
    {
        printf("worker_assignment: UNASSIGNED\r\n");
    }
    else
    {
        printf("worker_assignment: %s\r\n", machine.worker_assignment);
    }
    
    printf("progress: %s\r\n", machine.progress);
    printf("notification: %s\r\n", machine.notification);
}

static void MessageFunction(void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    int msg_id;
    int machine_index = -1;
    char machine_name[MAX_N_CHARS];
    char machine_topic[MAX_N_CHARS];
    char machine_info[MAX_N_CHARS];


    // El siguiente codigo comprueba si la cadena de caracteres target_worker 
    // está incluída o forma parte de la cadena de caracteres data recibida via event
    char topic[MAX_N_CHARS] = "";
    char topic_2[MAX_N_CHARS] = "";
    char data_info[MAX_N_CHARS] = "";



    char *p_machine_name;
    char *p_info_machine;

    int topic_len = 0;
    int data_len = 0;
    
    printf("--------------------------------------------\r\n");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    printf("--------------------------------------------\r\n");
    topic_len = event->topic_len;
    data_len = event->data_len;

    printf("topic_len = %d\r\n", topic_len);
    printf("data_len = %d\r\n", data_len);
    // Extraemos el nombre de la máquina de la cadena de caracteres topic
    // Por ejemplo, si topic es "LSE/machines/machine12/worker_assignment" extraemos machine12
    sprintf(topic, "%.*s", topic_len, event->topic);
    sprintf(data_info, "%.*s", data_len, event->data);

    strcpy(topic_2, topic);   

    p_machine_name = extract_machine_name(topic);

    strcpy(machine_name, p_machine_name);
    printf("machine_name = %s\r\n", machine_name);
    //printf("topic = %s\r\n", topic);

    ESP_LOGI(TAG, "machine_name = %.*s\r\n", strlen(machine_name), machine_name);

    // TODO: ver enunciado
 
    strcpy(student_machine.machine_name , machine_name);

    p_info_machine = extract_info( topic_2);
    strcpy(machine_info, p_info_machine);


    if (!strcmp(machine_info,"IP_address")){

        strcpy(student_machine.IP_address , data_info);   
    }
    else if(!strcmp(machine_info,"current_status")){
        strcpy(student_machine.current_status , data_info);

    }else if (!strcmp(machine_info,"work_assignment")){
        strcpy(student_machine.work_assignment , data_info);
    }else if (!strcmp(machine_info,"progress")){
        strcpy(student_machine.progress , data_info);
    }else if (!strcmp(machine_info,"worker_assignment")){
        strcpy(student_machine.worker_assignment , data_info);
    }

    print_machine(student_machine);

	//Ejecicio 2.- Publicar el estado running
    msg_id = esp_mqtt_client_publish( mqtt_client , "LSE/machines/machine18/current_status", "running", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);



    // TODO: ver enunciado
}
// FFM


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    int ip_machine;
    int current_status;
    int worker_assign;
    char *root_topic = "LSE/miriam.ortega.bustos@alumnos.upm.es/workers/mv.guarachi@alumnos.upm.es/worker_performance";


    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        MQTT_CONNECTED=1; // FFM
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        
        // TO DO: Subscribe to the topics
		// Ejecicio 1 .- Obtener la maquina del alumno
        msg_id = esp_mqtt_client_subscribe(client, "LSE/machines/machine18/#", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

		//Ejercicio 1 .- Obtener valores de IP, estado actual y trabajo asignado 
/*
        ip_machine = esp_mqtt_client_subscribe(client, "LSE/machines/machine18/IP_address", 0);
        ESP_LOGI(TAG, "sent subscribe successful, ip_machine=%d", ip_machine);

        current_status = esp_mqtt_client_subscribe(client, "LSE/machines/machine18/current_status", 0);
        ESP_LOGI(TAG, "sent subscribe successful, current_status=%d", current_status);

        worker_assign = esp_mqtt_client_subscribe(client, "LSE/machines/machine18/work_assignment", 0);
        ESP_LOGI(TAG, "sent subscribe successful, worker_assign=%d", worker_assign);
*/



        // TO DO: Subscribe to the topics
        break;
    case MQTT_EVENT_DISCONNECTED:
        MQTT_CONNECTED=0; // FFM
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        MessageFunction(event_data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .broker.address.port = 1885,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    mqtt_client = client; // FFM
    
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void Publisher_Task(void *params)
{
    int msg_id = 0;
    ESP_LOGI(TAG,"Publisher_Task");
    char *root_topic = "LSE/itahisa.hernandezf@alumnos.upm.es/workers";
    char topic[2*MAX_N_CHARS] = "";
    char payload[MAX_N_CHARS] = "";
    //char log_string[3*MAX_N_CHARS] = "";
    
    while (true)
    {
        if(MQTT_CONNECTED)
        {
            for(int i=0; i < num_machines_generating_notifications; i++)
            {
                sprintf(topic, "%s/%s/worker_performance", root_topic, machines_generating_notifications[i].worker_assignment);
                strcpy(payload, machines_generating_notifications[i].notification);

                if(strlen(machines_generating_notifications[i].worker_assignment)>0) {
                    printf("\r\n[Publisher_Task]\r\n[TOPIC][%s]\r\n[PAYLOAD][%s]\r\n", topic, payload);
                    
                    // TO DO

 
                    // TO DO
                }
                else
                    printf("Work generating notification in %s but still UNASSIGNED!!!\r\n", machines_generating_notifications[i].machine_name);
            }
        }
        else
            ESP_LOGI(TAG,"[Publisher_Task][MQTT NOT CONNECTED]");

        vTaskDelay(PUBLISH_PERIOD / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);

    mqtt_app_start();
}

