#pragma once
#include <esp_http_server.h>

#define WEBSERVER_TAG "HTTP_SERVER"

/* Task Handler */
extern TaskHandle_t hHttpServer;

/* HTTP Task */
void vHttpServer(void* pvParameters);

/* Initialization Function */
httpd_handle_t start_http_server();

/* Deinitializer Functions */
esp_err_t stop_webserver(httpd_handle_t webserver);

/*Global Variables*/

/* URI Handlers */
static esp_err_t portal_get_handler(httpd_req_t* req);
static esp_err_t portal_styles_get_handler(httpd_req_t* req);
static esp_err_t provision_post_handler(httpd_req_t* req);

/* Handlers */
static const httpd_uri_t portal = {
    .uri       = "/portal",
    .method    = HTTP_GET,
    .handler   = portal_get_handler,
    .user_ctx  = NULL,
};

static const httpd_uri_t portal_styles = {
    .uri = "/styles.css",
    .method = HTTP_GET,
    .handler = portal_styles_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t provision = {
  .uri = "/provision",
  .method =  HTTP_POST,
  .handler = provision_post_handler,
  .user_ctx = NULL,
};
