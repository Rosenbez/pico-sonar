#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "boards/adafruit_feather_rp2040.h"
#include "US_100.hpp"
#include "stepper.hpp"
#include "tft_driver.hpp"
#include "sonar_display.hpp"



int main()
{
    stdio_init_all();

    auto uart = uart0;
    int uart_tx_pin = PICO_DEFAULT_UART_TX_PIN;
    int uart_rx_pin = PICO_DEFAULT_UART_RX_PIN;

    uart_init(uart, 9600);
    gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);

    auto us_100 = US100(uart0, 1);

    sleep_ms(5000); 
    puts("Hello, world!");
    Stepper motor = Stepper(5, 6, 10, 9);
    int steps_per_rotation = 129;
    float motor_deg_per_step = 2.8;
    float degrees = 0;

    float deg_step_multiplier = 1.062; // tune for drive/pulley system
    float deg_per_step = motor_deg_per_step * deg_step_multiplier;

    auto tft = TFTDriver();
    tft.init();
    tft.fill_screen(60, 60, 60);

    uint8_t red_color[3] = {0, 0, 60};
    puts("writing px");

    tft.write_pixel(red_color, 159 - 2, 119 - 2, 5);
    puts("starting us100");

    auto sonar_disp = SonarDisplay(tft, tft.width, tft.height);
    puts("running plot test");

    int test_dist = 1500;
    for (int angle = 0; angle < 360; angle += 44) {
        printf("plotting: %d mm, %d deg \n", test_dist, angle);
        sonar_disp.plot_reading(test_dist, angle);
    }
    sleep_ms(5*1000);

    tft.fill_screen(60, 60, 60);
    tft.write_pixel(red_color, 159 - 2, 119 - 2, 5);

    while (1) {
        puts("ping sensor");
        us_100.ping();
        sonar_disp.clear_3_within(degrees);
        sleep_ms(15);

        uint16_t distance_read = us_100.read_distance();
        printf("found distance %d mm, degrees: %f \n", distance_read, degrees);

        if (distance_read < 3000) {
            // change above to handle distances correctly
            sonar_disp.plot_reading(distance_read, degrees);
        }

        puts("move motor..");

        motor.full_step(10);
        degrees += deg_per_step;
        if (degrees > 360) degrees = 0;


    }


    return 0;
}

