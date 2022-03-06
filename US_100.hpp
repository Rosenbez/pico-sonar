#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"


#include "boards/adafruit_feather_rp2040.h"


class US100 {
public:
    US100(uart_inst_t *uart, bool debug_print) : _uart(uart), debug(debug_print) {}

    void ping() {
        //printf("send distance read cmd %d \n", distance_command);
        uart_write_blocking(_uart, &dist_cmd, 1);
    }

    uint16_t read_distance() {
        uint8_t read_buff;
        uint16_t distance_reading;

        //puts("reading 1st uart byte");
        uart_read_blocking(_uart, &read_buff, 1);

        //printf("read byte into buffer: %d \n", buff);
        distance_reading = read_buff << 8;

        //printf("found distance %d \n", distance_read);
        //puts("reading 2nd uart byte");
        uart_read_blocking(_uart, &read_buff, 1);

        //puts("byte read");
        distance_reading += read_buff;

        return distance_reading;
    }

    uart_inst_t* _uart;
    bool debug = 0;
    uint8_t dist_cmd = 0x55;
    uint8_t temp_cmd = 0x50;
};