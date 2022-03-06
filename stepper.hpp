#include <stdio.h>
#include "pico/stdlib.h"


class Stepper { 

public:
    Stepper(int pin_a1, int pin_a2, int pin_b1, int pin_b2) : a1(pin_a1), a2(pin_a2), b1(pin_b1), b2(pin_b2) {

        set_motor_pins();
    }

    void set_gpio_out(int gpio_pin) {
        gpio_init(gpio_pin);
        gpio_set_dir(gpio_pin, true);
        gpio_put(gpio_pin, 0);
    }
    
    
    void set_motor_pins() {
        set_gpio_out(a1);
        set_gpio_out(a2);
        set_gpio_out(b1);
        set_gpio_out(b2);
    }
    void step_1() {
        gpio_put(a1, 1);
        gpio_put(a2, 0);
        gpio_put(b1, 0);
        gpio_put(b2, 0);
    }

    void step_2() {
        gpio_put(a1, 0);
        gpio_put(a2, 0);
        gpio_put(b1, 1);
        gpio_put(b2, 0);
    }
    void step_3() {
        gpio_put(a1, 0);
        gpio_put(a2, 1);
        gpio_put(b1, 0);
        gpio_put(b2, 0);
    }
    void step_4() {
        gpio_put(a1, 0);
        gpio_put(a2, 0);
        gpio_put(b1, 0);
        gpio_put(b2, 1);
    }

    void full_step(int step_delay) {
        step_1();
        sleep_ms(step_delay);
        step_2();
        sleep_ms(step_delay);
        step_3();
        sleep_ms(step_delay);
        step_4();
        sleep_ms(step_delay);
    }

private:
    int a1 = 0;
    int a2 = 0;
    int b1 = 0; 
    int b2 = 0;
    int last_step = 1;
};
