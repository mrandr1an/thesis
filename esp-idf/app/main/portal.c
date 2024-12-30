#include "portal.h"
#include "esp_err.h"
#include "http_parser.h"
#include "nvs.h"
#include <esp_http_server.h>
#include <freertos/idf_additions.h>
#include <esp_log.h>
#include <stdint.h>

TaskHandle_t hHttpServer = NULL;

static esp_err_t provision_post_handler(httpd_req_t *req)
{
 char buf[128]; // Buffer to hold incoming data
 int received_len = httpd_req_recv(req, buf, sizeof(buf) - 1); // Receive data

  if (received_len <= 0) {
        if (received_len == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req); // Respond with timeout error
        }
        return ESP_FAIL;
    }
  buf[received_len] = '\0'; // Null-terminate the received data

     // Log the received data
    ESP_LOGI(WEBSERVER_TAG, "Received data: %s", buf);

    char* pairs[2];
    pairs[0] = strtok(buf, "&");
    pairs[1] = strtok(NULL,"&");

    ESP_LOGI(WEBSERVER_TAG,"SSID:%s",pairs[0]);
    ESP_LOGI(WEBSERVER_TAG,"PWD:%s",pairs[1]);

    char* ssid = strchr(pairs[0],'=') + 1;
    char* pwd = strchr(pairs[1],'=') + 1;
    
    ESP_LOGI(WEBSERVER_TAG,"SSID:%s",ssid);
    ESP_LOGI(WEBSERVER_TAG,"PWD:%s",pwd);

    nvs_handle_t storage_h;

    esp_err_t ret = nvs_open("nvs",NVS_READWRITE,&storage_h);

    if (ret != ESP_OK)
    {
      ESP_LOGI(WEBSERVER_TAG,"Could not open NVS: %s.",esp_err_to_name(ret));
    }
      
    ret = nvs_set_str(storage_h,"ssid",ssid);
    
    if (ret != ESP_OK)
    {
      ESP_LOGI(WEBSERVER_TAG,"Could not set ssid to nvs: %s.",esp_err_to_name(ret));
    }

    ret = nvs_set_str(storage_h,"pwd",pwd);

    if (ret != ESP_OK)
    {
      ESP_LOGI(WEBSERVER_TAG,"Could not set pwd to nvs: %s.",esp_err_to_name(ret));
    }

    const char *response = "Credentials received and stored in flash, connecting to WiFi";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    nvs_close(storage_h);

    return ESP_OK; 
}

static esp_err_t portal_get_handler(httpd_req_t* req)
{
   const char* filepath = "/storage/index.html"; // Path to your file in SPIFFS

    // Open the file for reading
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open file");
        return ESP_FAIL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate a buffer to read the file
    char* buffer = malloc(filesize + 1);
    if (buffer == NULL) {
        fclose(file);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to allocate memory");
        return ESP_FAIL;
    }

    // Read the file content
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // Null-terminate the buffer
    fclose(file);

    // Set Content-Type header
    httpd_resp_set_type(req, "text/html"); // Update to "text/css" for styles.css

    // Send the file content as response
    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    // Clean up
    free(buffer);
    return ESP_OK;
}


static esp_err_t portal_styles_get_handler(httpd_req_t* req)
{
   const char* filepath = "/storage/styles.css"; // Path to your file in SPIFFS

    // Open the file for reading
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open file");
        return ESP_FAIL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate a buffer to read the file
    char* buffer = malloc(filesize + 1);
    if (buffer == NULL) {
        fclose(file);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to allocate memory");
        return ESP_FAIL;
    }

    // Read the file content
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // Null-terminate the buffer
    fclose(file);

    // Set Content-Type header
    httpd_resp_set_type(req, "text/css"); // Update to "text/css" for styles.css

    // Send the file content as response
    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    // Clean up
    free(buffer);
    return ESP_OK;
}

httpd_handle_t start_http_server()
{
  httpd_handle_t server = NULL;

  //Default port is 80
  httpd_config_t webserver_config = HTTPD_DEFAULT_CONFIG();
  ESP_LOGI(WEBSERVER_TAG, "Starting server on port: %d", webserver_config.server_port);

  if(httpd_start(&server,&webserver_config) == ESP_OK) {   
    ESP_LOGI(WEBSERVER_TAG, "Registering URI handlers");

    esp_err_t ret = httpd_register_uri_handler(server, &portal);

    ret = httpd_register_uri_handler(server, &portal_styles);

    if(ret != ESP_OK) {
      ESP_LOGI(WEBSERVER_TAG, "Could not register portal uri: %s.", esp_err_to_name(ret));
    }

    ret = httpd_register_uri_handler(server,&provision);

    if(ret != ESP_OK) {
      ESP_LOGI(WEBSERVER_TAG, "Could not register provision uri: %s.", esp_err_to_name(ret));
    }
  }
  return server;
}

void vHttpServer(void *pvParameters)
{
  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
  httpd_handle_t server = start_http_server();
  while(server)
  {
	vTaskDelay(500/portTICK_PERIOD_MS);
  }
}
