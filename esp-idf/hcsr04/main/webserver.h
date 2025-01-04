#pragma once

#include <freertos/idf_additions.h>
#include <esp_http_server.h>

#define WEBSERVER_TAG "WEBSERVER"

extern TaskHandle_t HttpServer_h;

/* Post Request Handler for submitting a Network*/
static esp_err_t submitNetwork_h(httpd_req_t *req);

/* Get request Handler for the ESP Configuration Portal */
static esp_err_t portalESP_h(httpd_req_t *req);

/* Get request Handler for the ESP Configuration Portal Styles */
static esp_err_t portalESP_styles_h(httpd_req_t *req);

/*Blocking function that starts http server*/
httpd_handle_t start_http_server();

/* Task for Starting HTTP Server as FreeRTOS Task */
void HttpServer_Task(void *pvParameters);

/* Handlers */
static const httpd_uri_t Portal_uri;
static const httpd_uri_t Portal_styles_uri;
static const httpd_uri_t submitNetwork_uri;
