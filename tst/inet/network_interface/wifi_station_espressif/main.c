/**
 * @file main.c
 *
 * @section License
 * Copyright (C) 2014-2016, Erik Moqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#include "simba.h"

#if !defined(SSID)
#    pragma message "WiFi connection variable SSID is not set. Using default value MySSID"
#    define SSID Qvist
#endif

#if !defined(PASSWORD)
#    pragma message "WiFi connection variable PASSWORD is not set. Using default value MyPassword"
#    define PASSWORD Recmyng8
#endif

#if !defined(ESP8266_IP)
#    pragma message "WiFi connection variable ESP8266_IP is not set. Using default value 192.168.1.100"
#    define ESP8266_IP 192.168.1.103
#endif

/* Ports. */
#define UDP_PORT         30303
#define TCP_PORT         40404
#define TCP_PORT_SIZES   40405

static struct network_interface_wifi_station_espressif_t wifi;
static uint8_t buffer[5000];

static int test_init(struct harness_t *harness_p)
{
    struct inet_ip_addr_t addr;
    char buf[20];

    network_interface_module_init();
    socket_module_init();

    std_printf(FSTR("Connecting to SSID=%s\r\n"), STRINGIFY(SSID));

    /* Initialize WiFi in station mode with given SSID and
       password. */
    network_interface_wifi_station_espressif_module_init();
    network_interface_wifi_station_espressif_init(&wifi,
                                                  (uint8_t *)STRINGIFY(SSID),
                                                  (uint8_t *)STRINGIFY(PASSWORD));

    network_interface_add(&wifi.network_interface);

    /* Start WiFi and connect to the Access Point with given SSID and
       password. */
    network_interface_start(&wifi.network_interface);

    /* Wait for a connection to the WiFi access point. */
    while (network_interface_is_up(&wifi.network_interface) == 0) {
        thrd_sleep(1);
    }

    network_interface_get_ip_address(&wifi.network_interface,
                                     &addr);

    std_printf(FSTR("Connected to Access Point (AP). Got IP %s.\r\n"),
               inet_ntoa(&addr, buf));

    BTASSERT(network_interface_get_by_name("esp-wlan") == &wifi.network_interface);
    BTASSERT(network_interface_get_by_name("none") == NULL);

    return (0);
}

static int test_udp(struct harness_t *harness_p)
{
    struct socket_t sock;
    struct inet_addr_t addr;
    char buf[16];
    char addrbuf[20];
    size_t size;
    struct chan_list_t list;
    int workspace[16];

    BTASSERT(chan_list_init(&list, workspace, sizeof(workspace)) == 0);
    
    std_printf(FSTR("UDP test\r\n"));

    std_printf(FSTR("opening socket\r\n"));
    BTASSERT(socket_open_udp(&sock) == 0);

    BTASSERT(chan_list_add(&list, &sock) == 0);
    
    std_printf(FSTR("binding to %d\r\n"), UDP_PORT);
    inet_aton(STRINGIFY(ESP8266_IP), &addr.ip);
    addr.port = UDP_PORT;
    BTASSERT(socket_bind(&sock, &addr) == 0);

    std_printf(FSTR("recvfrom\r\n"));

    size = socket_recvfrom(&sock, buf, sizeof(buf), 0, &addr);
    BTASSERT(size == 9);
    buf[size] = '\0';
    std_printf(FSTR("received '%s' from %s:%d\r\n"),
               buf,
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);

    std_printf(FSTR("sending '%s' to %s:%d\r\n"),
               buf,
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);
    BTASSERT(socket_sendto(&sock, buf, size, 0, &addr) == size);

    BTASSERT(chan_list_poll(&list, NULL) == &sock);
    
    size = socket_recvfrom(&sock, buf, sizeof(buf), 0, &addr);
    BTASSERT(size == 9);
    buf[size] = '\0';
    std_printf(FSTR("received '%s' from %s:%d\r\n"),
               buf,
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);

    std_printf(FSTR("sending '%s' to %s:%d\r\n"),
               buf,
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);
    BTASSERT(socket_sendto(&sock, buf, size, 0, &addr) == size);
    
    std_printf(FSTR("closing socket\r\n"));
    BTASSERT(socket_close(&sock) == 0);

    return (0);
}

static int test_tcp(struct harness_t *harness_p)
{
    struct socket_t listener;
    struct socket_t client;
    struct inet_addr_t addr;
    char buf[16];
    char addrbuf[20];
    size_t size;
    struct chan_list_t list;
    int workspace[16];

    BTASSERT(chan_list_init(&list, workspace, sizeof(workspace)) == 0);

    std_printf(FSTR("TCP test\r\n"));

    std_printf(FSTR("opening listener socket\r\n"));
    BTASSERT(socket_open_tcp(&listener) == 0);

    std_printf(FSTR("binding to %d\r\n"), TCP_PORT);
    inet_aton(STRINGIFY(ESP8266_IP), &addr.ip);
    addr.port = TCP_PORT;
    BTASSERT(socket_bind(&listener, &addr) == 0);

    BTASSERT(socket_listen(&listener, 5) == 0);

    std_printf(FSTR("listening on %d\r\n"), TCP_PORT);

    BTASSERT(socket_accept(&listener, &client, &addr) == 0);
    std_printf(FSTR("accepted client %s:%d\r\n"),
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);

    BTASSERT(chan_list_add(&list, &client) == 0);

    size = socket_read(&client, buf, 5);
    BTASSERT(size == 5);
    size += socket_read(&client, &buf[5], 4);
    BTASSERT(size == 9);
    buf[size] = '\0';
    std_printf(FSTR("read %d bytes: '%s'\r\n"), size, buf);

    std_printf(FSTR("writing '%s'\r\n"), buf);
    BTASSERT(socket_write(&client, buf, size) == size);

    BTASSERT(chan_list_poll(&list, NULL) == &client);

    size = socket_read(&client, buf, 5);
    BTASSERT(size == 5);
    size += socket_read(&client, &buf[5], 4);
    BTASSERT(size == 9);
    buf[size] = '\0';
    std_printf(FSTR("read %d bytes: '%s'\r\n"), size, buf);

    std_printf(FSTR("writing '%s'\r\n"), buf);
    BTASSERT(socket_write(&client, buf, size) == size);
    
    std_printf(FSTR("closing client socket\r\n"));
    BTASSERT(socket_close(&client) == 0);
    
    std_printf(FSTR("closing listener socket\r\n"));
    BTASSERT(socket_close(&listener) == 0);

    return (0);
}

static int test_tcp_sizes(struct harness_t *harness_p)
{
    struct socket_t listener;
    struct socket_t client;
    struct inet_addr_t addr;
    char addrbuf[20];
    size_t size;
    struct chan_list_t list;
    int workspace[16];

    std_printf(FSTR("TCP test\r\n"));

    BTASSERT(chan_list_init(&list, workspace, sizeof(workspace)) == 0);

    std_printf(FSTR("opening listener socket\r\n"));
    BTASSERT(socket_open_tcp(&listener) == 0);

    BTASSERT(chan_list_add(&list, &listener) == 0);

    std_printf(FSTR("binding to %d\r\n"), TCP_PORT_SIZES);
    inet_aton(STRINGIFY(ESP8266_IP), &addr.ip);
    addr.port = TCP_PORT_SIZES;
    BTASSERT(socket_bind(&listener, &addr) == 0);

    BTASSERT(socket_listen(&listener, 5) == 0);

    std_printf(FSTR("listening on %d\r\n"), TCP_PORT_SIZES);

    BTASSERT(chan_list_poll(&list, NULL) == &listener);

    BTASSERT(socket_accept(&listener, &client, &addr) == 0);
    std_printf(FSTR("accepted client %s:%d\r\n"),
               inet_ntoa(&addr.ip, addrbuf),
               addr.port);

    /* Range of packet sizes. */
    for (size = 1; size < sizeof(buffer); size += 128) {
        BTASSERT(socket_read(&client, buffer, size) == size);
        BTASSERT(socket_write(&client, buffer, size) == size);
    }
    
    std_printf(FSTR("closing client socket\r\n"));
    BTASSERT(socket_close(&client) == 0);

    std_printf(FSTR("closing listener socket\r\n"));
    BTASSERT(socket_close(&listener) == 0);
    
    return (0);
}

int main()
{
    struct harness_t harness;
    struct harness_testcase_t harness_testcases[] = {
        { test_init, "test_init" },
        { test_udp, "test_udp" },
        { test_tcp, "test_tcp" },
        { test_tcp_sizes, "test_tcp_sizes" },
        { NULL, NULL }
    };

    sys_start();
    inet_module_init();

    harness_init(&harness);
    harness_run(&harness, harness_testcases);

    return (0);
}
