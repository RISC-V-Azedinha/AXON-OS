#include "../../../include/hal/hal_gpio.h"

// ======================================================================================
// GPIO virtual (QEMU): guarda o estado em memória para os comandos e tarefas funcionarem
// ======================================================================================

static uint32_t v_out[GPIO_PORT_COUNT];
static uint32_t v_dir[GPIO_PORT_COUNT];

void hal_gpio_init(void) {
    for (int p = 0; p < GPIO_PORT_COUNT; p++) { v_out[p] = 0; v_dir[p] = 0; }
}

void hal_gpio_set_dir(gpio_port_t port, uint32_t pin, gpio_dir_t dir) {
    if (dir == GPIO_OUTPUT) v_dir[port] |= (1u << pin); else v_dir[port] &= ~(1u << pin);
}
void hal_gpio_write(gpio_port_t port, uint32_t pin, uint32_t value) {
    if (value) v_out[port] |= (1u << pin); else v_out[port] &= ~(1u << pin);
}
void hal_gpio_toggle(gpio_port_t port, uint32_t pin)         { v_out[port] ^= (1u << pin); }
uint32_t hal_gpio_read(gpio_port_t port, uint32_t pin)       { return (hal_gpio_port_read(port) >> pin) & 1u; }

void     hal_gpio_port_set_dir(gpio_port_t port, uint32_t mask) { v_dir[port] = mask; }
uint32_t hal_gpio_port_get_dir(gpio_port_t port)                { return port == GPIO_LED ? 0xFFFF : v_dir[port]; }
void     hal_gpio_port_write(gpio_port_t port, uint32_t value)  { v_out[port] = value; }
uint32_t hal_gpio_port_read(gpio_port_t port) {
    // Sem pinos físicos: as saídas são lidas de volta; chaves e botões ficam em 0
    return (port == GPIO_SW || port == GPIO_BTN) ? 0 : (v_out[port] & hal_gpio_port_get_dir(port));
}

void     hal_leds_write(uint16_t value) { v_out[GPIO_LED] = value; }
uint16_t hal_leds_read(void)            { return (uint16_t)v_out[GPIO_LED]; }
uint16_t hal_switches_read(void)        { return 0; }
uint8_t  hal_buttons_read(void)         { return 0; }

void hal_seg7_write_hex(uint32_t value)                         { (void)value; }
void hal_seg7_write_dec(uint32_t value)                         { (void)value; }
void hal_seg7_enable(uint8_t digits_mask)                       { (void)digits_mask; }
void hal_rgb_set(uint32_t led, uint8_t r, uint8_t g, uint8_t b) { (void)led; (void)r; (void)g; (void)b; }
