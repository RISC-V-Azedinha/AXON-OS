#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

// ======================================================================================
// GPIO: portas no estilo de microcontrolador (direção + pino), displays e LEDs RGB
// ======================================================================================
//
// Pmods: o pino i da porta é o pino i do conector na ordem 1, 2, 3, 4, 7, 8, 9, 10.

typedef enum {
    GPIO_JA  = 0,               // Pmod JA (8 pinos, entrada ou saída)
    GPIO_JB  = 1,               // Pmod JB
    GPIO_JC  = 2,               // Pmod JC
    GPIO_JD  = 3,               // Pmod JD
    GPIO_LED = 4,               // LD0..LD15 (só saída)
    GPIO_SW  = 5,               // SW0..SW15 (só entrada)
    GPIO_BTN = 6                // BTNU, BTND, BTNL, BTNR (só entrada)
} gpio_port_t;

#define GPIO_PORT_COUNT 7

typedef enum { GPIO_INPUT = 0, GPIO_OUTPUT = 1 } gpio_dir_t;

/**
 * @brief Inicializa o GPIO: apaga LEDs, displays e LEDs RGB e deixa os Pmods como entrada.
 */
void hal_gpio_init(void);

// Pino a pino
void     hal_gpio_set_dir(gpio_port_t port, uint32_t pin, gpio_dir_t dir);
void     hal_gpio_write(gpio_port_t port, uint32_t pin, uint32_t value);
void     hal_gpio_toggle(gpio_port_t port, uint32_t pin);
uint32_t hal_gpio_read(gpio_port_t port, uint32_t pin);

// Porta inteira
void     hal_gpio_port_set_dir(gpio_port_t port, uint32_t mask);
uint32_t hal_gpio_port_get_dir(gpio_port_t port);
void     hal_gpio_port_write(gpio_port_t port, uint32_t value);
uint32_t hal_gpio_port_read(gpio_port_t port);

// Atalhos
void     hal_leds_write(uint16_t value);
uint16_t hal_leds_read(void);
uint16_t hal_switches_read(void);
uint8_t  hal_buttons_read(void);

// Displays de 7 segmentos (dígito 0 = direita)
void hal_seg7_write_hex(uint32_t value);
void hal_seg7_write_dec(uint32_t value);
void hal_seg7_enable(uint8_t digits_mask);

// LEDs RGB (0 = LD16, 1 = LD17), brilho 0..255 por cor
void hal_rgb_set(uint32_t led, uint8_t r, uint8_t g, uint8_t b);

#endif // HAL_GPIO_H
