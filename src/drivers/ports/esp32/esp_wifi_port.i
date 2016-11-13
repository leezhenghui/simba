/**
 * @section License
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014-2016, Erik Moqvist
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the Simba project.
 */

#include "simba.h"

#undef BIT
#undef O_RDONLY
#undef O_WRONLY
#undef O_RDWR
#undef O_APPEND
#undef O_CREAT
#undef O_TRUNC
#undef O_EXCL
#undef O_SYNC

#include "esp_wifi.h"    
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

extern int esp_wifi_station_port_set_connect_status(enum esp_wifi_station_status_t status);
extern int esp_wifi_softap_port_station_connected(void);
extern int esp_wifi_softap_port_station_disconnected(void);

/**
 * WiFi event callback.
 */
static esp_err_t event_handler(void *ctx_p, system_event_t *event_p)
{
    switch (event_p->event_id) {

    case SYSTEM_EVENT_WIFI_READY:
        break;

    case SYSTEM_EVENT_SCAN_DONE:
        break;

    case SYSTEM_EVENT_STA_START:
        break;

    case SYSTEM_EVENT_STA_STOP:
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        esp_wifi_station_port_set_connect_status(esp_wifi_station_status_idle_t);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_station_port_set_connect_status(esp_wifi_station_status_idle_t);
        break;

    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        esp_wifi_station_port_set_connect_status(esp_wifi_station_status_got_ip_t);
        break;

    case SYSTEM_EVENT_AP_START:
        break;

    case SYSTEM_EVENT_AP_STOP:
        break;

    case SYSTEM_EVENT_AP_STACONNECTED:
        esp_wifi_softap_port_station_connected();
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED:
        esp_wifi_softap_port_station_disconnected();
        break;

    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        break;

    default:
        break;
    }
    
    return (ESP_OK);
}

static int esp_wifi_port_module_init(void)
{
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

    tcpip_adapter_init();
    esp_event_loop_init(event_handler, NULL);
    esp_wifi_init(&config);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_start();

    return (0);
}

static int esp_wifi_port_set_op_mode(enum esp_wifi_op_mode_t mode)
{
    return (esp_wifi_set_mode(mode));
}

static enum esp_wifi_op_mode_t esp_wifi_port_get_op_mode()
{
    wifi_mode_t mode;
    
    if (esp_wifi_get_mode(&mode) != ESP_OK) {
        return (-1);
    }

    return (mode);
}

static int esp_wifi_port_set_phy_mode(enum esp_wifi_phy_mode_t mode)
{
    return (-1);
}

static enum esp_wifi_phy_mode_t esp_wifi_port_get_phy_mode()
{
    return (-1);
}
