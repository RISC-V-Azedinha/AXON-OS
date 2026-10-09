#include "apps/commands.h"
#include "apps/shell_utils.h"
#include "hal/hal_gpio.h"

// ======================================================================================
// COMANDOS: GPIO, SEG, RGB
// ======================================================================================
//
//   gpio                    lista as portas (direção, saída e entrada)
//   gpio ja                 mostra a porta JA
//   gpio ja 3 out           pino 3 da JA como saída (in: entrada)
//   gpio ja 3 1             escreve 1 no pino 3 da JA (0: zero, t: inverte)
//   gpio sw 2               lê o pino 2 das chaves
//   seg 1234 | seg x1F | seg off
//   rgb 0 255 0 64

static const char *port_names[GPIO_PORT_COUNT] = { "ja", "jb", "jc", "jd", "led", "sw", "btn" };

static const char *next_word(const char *p, char *out, int max) {
    int i = 0;
    while (*p == ' ') p++;
    while (*p && *p != ' ' && i < max - 1) out[i++] = *p++;
    out[i] = 0;
    return p;
}

static uint32_t parse_num(const char *s, int base) {
    uint32_t v = 0;
    while (*s) {
        uint32_t d;
        if (*s >= '0' && *s <= '9') d = *s - '0';
        else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
        else break;
        if (d >= (uint32_t)base) break;
        v = v * base + d;
        s++;
    }
    return v;
}

static void show_port(int p) {
    char buf[12];
    safe_puts("  "); safe_puts(port_names[p]); safe_puts("\t");
    safe_puts(" dir=");  val_to_hex(hal_gpio_port_get_dir((gpio_port_t)p), buf); safe_puts(buf);
    safe_puts(" in=");   val_to_hex(hal_gpio_port_read((gpio_port_t)p), buf);    safe_puts(buf);
    safe_puts("\n");
}

void cmd_gpio(const char *args) {
    char w_port[8], w_pin[8], w_op[8];
    const char *p = next_word(args ? args : "", w_port, sizeof w_port);

    if (!w_port[0]) {
        safe_puts(SH_BOLD "\n  PORT  DIR (1 = out)  IN\n" SH_RESET);
        for (int i = 0; i < GPIO_PORT_COUNT; i++) show_port(i);
        return;
    }

    int port = -1;
    for (int i = 0; i < GPIO_PORT_COUNT; i++) if (sys_strcmp(w_port, port_names[i]) == 0) port = i;
    if (port < 0) { safe_puts("Usage: gpio [ja|jb|jc|jd|led|sw|btn] [<pin> in|out|0|1|t]\n"); return; }

    p = next_word(p, w_pin, sizeof w_pin);
    next_word(p, w_op, sizeof w_op);
    if (!w_pin[0]) { show_port(port); return; }

    uint32_t pin = parse_num(w_pin, 10);
    if (pin > 15) { safe_puts("Invalid pin.\n"); return; }

    if      (!w_op[0])                         { safe_puts(hal_gpio_read((gpio_port_t)port, pin) ? "1\n" : "0\n"); return; }
    else if (sys_strcmp(w_op, "out") == 0)     hal_gpio_set_dir((gpio_port_t)port, pin, GPIO_OUTPUT);
    else if (sys_strcmp(w_op, "in") == 0)      hal_gpio_set_dir((gpio_port_t)port, pin, GPIO_INPUT);
    else if (sys_strcmp(w_op, "t") == 0)       hal_gpio_toggle((gpio_port_t)port, pin);
    else                                       hal_gpio_write((gpio_port_t)port, pin, parse_num(w_op, 10));
    show_port(port);
}

void cmd_seg(const char *args) {
    char w[12];
    next_word(args ? args : "", w, sizeof w);
    if (!w[0])                          safe_puts("Usage: seg <dec> | seg x<hex> | seg off\n");
    else if (sys_strcmp(w, "off") == 0) hal_seg7_enable(0);
    else if (w[0] == 'x')               { hal_seg7_write_hex(parse_num(w + 1, 16)); hal_seg7_enable(0xFF); }
    else                                hal_seg7_write_dec(parse_num(w, 10));
}

void cmd_rgb(const char *args) {
    char w[4][6];
    const char *p = args ? args : "";
    for (int i = 0; i < 4; i++) p = next_word(p, w[i], sizeof w[i]);
    if (!w[3][0]) { safe_puts("Usage: rgb <0|1> <r> <g> <b>  (0..255)\n"); return; }
    hal_rgb_set(parse_num(w[0], 10) & 1, parse_num(w[1], 10), parse_num(w[2], 10), parse_num(w[3], 10));
}
