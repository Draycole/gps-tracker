#include <stdio.h>
#include <string.h>
#include <stdlib.h>      
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/time.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define LINE_BUF_SIZE 100
char line[LINE_BUF_SIZE];
int line_index = 0;

// latest parsed GGA data (no volatile used)
float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;
int fix_quality = 0;              // 0 = no fix, 1 = GPS, 2 = DGPS
char utc_time[11] = "000000.00";  // hhmmss.ss

void parse_gga(char *sentence);

int main() {
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    sleep_ms(2000);
    printf("GPS reader started. (Printing every 2 seconds)\n");

    absolute_time_t next_update = make_timeout_time_ms(2000);

    while (true) {
        // 1. receive and assemble lines from GPS
        while (uart_is_readable(UART_ID)) {
            char ch = uart_getc(UART_ID);;

            if (ch == '\n') {
                line[line_index] = '\0';
                if (strncmp(line, "$GPGGA", 6) == 0) {
                    parse_gga(line);
                }
                line_index = 0;
            } else if (ch != '\r') {
                if (line_index < LINE_BUF_SIZE - 1) {
                    line[line_index++] = ch;
                }
            }
        }

        // 2. every 2 seconds, print a readable, converted summary
        if (time_reached(next_update)) {
            if (fix_quality == 0) {
                printf("No fix yet. (quality=%d)\n", fix_quality);
            } else {
                printf("GPS Fix: Time=%s Lat=%.4f Lon=%.4f Alt=%.1fm Quality=%d\n",
                       utc_time, latitude, longitude, altitude, fix_quality);
            }
            next_update = make_timeout_time_ms(2000);
        }

        sleep_ms(10);
    }

    return 0;
}

void parse_gga(char *sentence) {
    // $GPGGA,time,lat,N,lon,E,quality,numSV,HDOP,alt,M,geoidSep,M,age,refID*cs
    char *token;
    int field = 0;

    token = strtok(sentence, ",");
    while (token != NULL) {
        switch (field) {
            case 1: // UTC time
                strncpy(utc_time, token, 10);
                utc_time[10] = '\0';
                break;
            case 2: // latitude
                latitude = atof(token);
                break;
            case 3: // N/S (ignored)
                break;
            case 4: // longitude
                longitude = atof(token);
                break;
            case 5: // E/W (ignored)
                break;
            case 6: // fix quality
                fix_quality = atoi(token);
                break;
            case 9: // altitude
                altitude = atof(token);
                break;
        }
        token = strtok(NULL, ",");
        field++;
    }
}