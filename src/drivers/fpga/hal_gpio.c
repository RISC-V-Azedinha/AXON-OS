#include "../../../include/hal/hal_gpio.h"
#include "memory_map.h"

// ======================================================================================
// Inicialização
// ======================================================================================

void hal_gpio_init(void) {
    for (int p = GPIO_JA; p <= GPIO_JD; p++) GPIO_REG(p, GPIO_REG_DIR) = 0;   // Pmods como entrada
    GPIO_REG(GPIO_LED, GPIO_REG_OUT) = 0;
    SEG7_REG_EN = 0;
    RGB_REG(0) = 0;
    RGB_REG(1) = 0;
}

// ======================================================================================
// Pino a pino (OUTSET/OUTCLR/OUTTGL: sem ler-modificar-escrever, seguro entre tarefas)
// ======================================================================================

void hal_gpio_set_dir(gpio_port_t port, uint32_t pin, gpio_dir_t dir) {
    if (dir == GPIO_OUTPUT) GPIO_REG(port, GPIO_REG_DIR) |=  (1u << pin);
    else                    GPIO_REG(port, GPIO_REG_DIR) &= ~(1u << pin);
}

void hal_gpio_write(gpio_port_t port, uint32_t pin, uint32_t value) {
    if (value) GPIO_REG(port, GPIO_REG_OUTSET) = 1u << pin;
    else       GPIO_REG(port, GPIO_REG_OUTCLR) = 1u << pin;
}

void hal_gpio_toggle(gpio_port_t port, uint32_t pin) {
    GPIO_REG(port, GPIO_REG_OUTTGL) = 1u << pin;
}

uint32_t hal_gpio_read(gpio_port_t port, uint32_t pin) {
    return (GPIO_REG(port, GPIO_REG_IN) >> pin) & 1u;
}

// ======================================================================================
// Porta inteira
// ======================================================================================

void     hal_gpio_port_set_dir(gpio_port_t port, uint32_t mask) { GPIO_REG(port, GPIO_REG_DIR) = mask; }
uint32_t hal_gpio_port_get_dir(gpio_port_t port)                { return GPIO_REG(port, GPIO_REG_DIR); }
void     hal_gpio_port_write(gpio_port_t port, uint32_t value)  { GPIO_REG(port, GPIO_REG_OUT) = value; }
uint32_t hal_gpio_port_read(gpio_port_t port)                   { return GPIO_REG(port, GPIO_REG_IN); }

void     hal_leds_write(uint16_t value) { GPIO_REG(GPIO_LED, GPIO_REG_OUT) = value; }
uint16_t hal_leds_read(void)            { return (uint16_t)GPIO_REG(GPIO_LED, GPIO_REG_OUT); }
uint16_t hal_switches_read(void)        { return (uint16_t)GPIO_REG(GPIO_SW, GPIO_REG_IN); }
uint8_t  hal_buttons_read(void)         { return (uint8_t)GPIO_REG(GPIO_BTN, GPIO_REG_IN); }

// ======================================================================================
// Displays de 7 segmentos e LEDs RGB
// ======================================================================================

void hal_seg7_write_hex(uint32_t value) {
    SEG7_REG_CTRL = 0;
    SEG7_REG_HEX  = value;
}

void hal_seg7_write_dec(uint32_t value) {
    uint32_t bcd = 0, digits = 0;
    do {
        bcd |= (value % 10) << (4 * digits);
        value /= 10;
        digits++;
    } while (value != 0 && digits < 8);
    SEG7_REG_CTRL = 0;
    SEG7_REG_HEX  = bcd;
    SEG7_REG_EN   = (1u << digits) - 1;
}

void hal_seg7_enable(uint8_t digits_mask) {
    SEG7_REG_EN = digits_mask;
}

void hal_rgb_set(uint32_t led, uint8_t r, uint8_t g, uint8_t b) {
    RGB_REG(led) = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
