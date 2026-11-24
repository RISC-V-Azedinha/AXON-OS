# Plataformas e drivers

A **HAL** (Hardware Abstraction Layer) é o que permite ao mesmo código de kernel — escalonador, memória, sistema de arquivos, shell — compilar e rodar tanto no QEMU quanto no SoC em FPGA sem nenhuma alteração. Cada módulo de hardware tem um **header comum** em `include/hal/` e **duas implementações**, uma por plataforma, em `src/drivers/qemu/` e `src/drivers/fpga/`.

## Como a seleção acontece

O `makefile` decide qual conjunto de `.c` compilar (veja [Compilação e execução](../primeiros-passos.md#como-o-makefile-decide-o-que-compilar)):

```makefile
DRIVERS_QEMU = $(wildcard src/drivers/qemu/*.c)
DRIVERS_FPGA = $(wildcard src/drivers/fpga/*.c)
```

O alvo FPGA também define `-DPLATFORM_FPGA`, que o código pode usar para condicionais quando necessário. Como os dois diretórios de drivers exportam exatamente as mesmas funções (definidas pelos headers em `include/hal/`), o *linker* nunca vê os dois ao mesmo tempo — apenas um conjunto de símbolos existe por binário.

## Módulos da HAL

| Header | Responsabilidade | Presente em |
|---|---|---|
| `hal_uart.h` | Console serial (TX/RX de caracteres) | QEMU e FPGA |
| `hal_timer.h` | Temporização, ciclos, interrupção periódica | QEMU e FPGA |
| `hal_irq.h` | Controle de interrupções da CPU (`mstatus`, `mie`, `mtvec`) + dispatcher de PLIC | QEMU e FPGA (a parte CSR é comum a ambos, por ser inline) |
| `hal_plic.h` | Controlador de interrupções externas (PLIC) | QEMU e FPGA |
| `hal_gpio.h` | Portas (direção + pino): Pmods, LEDs, chaves e botões; displays de 7 segmentos; LEDs RGB | QEMU (estado virtual) e FPGA |
| `hal_dma.h` | Cópia de memória assistida por hardware | QEMU (stub) e FPGA |
| `hal_vga.h` | Framebuffer de vídeo 320×240 | QEMU (stub) e FPGA |
| `hal_npu.h` | Acelerador de rede neural (NPU) | QEMU (stub) e FPGA |

## Diferenças entre as plataformas

Cada plataforma tem seu próprio `memory_map.h` (`src/drivers/{qemu,fpga}/memory_map.h`), já que os endereços físicos e os offsets de registrador são completamente diferentes entre o modelo `virt` do QEMU e o SoC customizado.

=== "QEMU (`virt`)"

    - Clock base: **10 MHz**
    - UART: **16550A padrão**, com registradores clássicos (`RBR`/`THR`, `IER`, `FCR`, `LCR`, `LSR`)
    - CLINT/PLIC: layout **SiFive padrão**
    - GPIO, DMA, VGA e NPU **não existem de fato no QEMU** — os endereços (`0x40000000`, `0x30000000`, `0x90000000`) são "fictícios" (stubs), e as implementações em `src/drivers/qemu/` para esses periféricos apenas simulam comportamento mínimo ou são no-ops, o suficiente para o kernel não travar.

=== "FPGA (SoC customizado)"

    - Clock base: **100 MHz**
    - UART: registrador customizado próprio (`UART_DATA_REG`/`UART_CTRL_REG`, com bits de status `TX_BUSY`/`RX_VALID`)
    - CLINT/PLIC: offsets próprios, definidos no VHDL do SoC (`bus_interconnect.vhd`, `soc_top.vhd` — fora deste repositório)
    - GPIO, DMA, VGA e NPU são periféricos **reais**, mapeados em memória, implementados em hardware (FPGA) próprio do projeto

## O caso especial: `hal_irq.h`

Diferente dos demais módulos, boa parte de `hal_irq.h` é **comum às duas plataformas** — as funções de controle de interrupção da CPU (`hal_irq_global_enable`, `hal_irq_mask_enable`, `hal_irq_set_handler`, etc.) são `static inline` e operam diretamente sobre CSRs padrão do RISC-V (`mstatus`, `mie`, `mtvec`), que existem identicamente em qualquer implementação `rv32i_zicsr`. Só o **dispatcher de PLIC** (`irq_dispatch.c`, em `src/kernel/`, não em `src/drivers/`) e o driver de baixo nível do PLIC (`hal_plic.c`, este sim por plataforma) lidam com diferenças de hardware.

## A NPU (Neural Processing Unit)

A `hal_npu.h` é o driver mais elaborado da HAL, e o que dá nome ao projeto. Ela expõe uma interface de acelerador de multiplicação de matrizes quantizadas (inteiros de 8 bits), pensada para inferência de redes neurais:

- **Configuração**: `hal_npu_configure(k_dim, quant_params)` define a profundidade de acumulação e os parâmetros de quantização (multiplicador de ponto fixo, shift, zero-point, ativação ReLU opcional).
- **Carregamento de dados**: `hal_npu_load_weights()` / `hal_npu_load_inputs()` escrevem em FIFOs de hardware (pesos e ativações, empacotados como `int8x4` dentro de palavras de 32 bits); `hal_npu_load_bias()` carrega o vetor de bias.
- **Execução**: `hal_npu_start()` dispara uma execução limpa (reset de ponteiros e acumuladores); `hal_npu_start_accumulate()` dispara sem limpar os acumuladores — útil para *tiling*, ou seja, quebrar uma multiplicação de matrizes grande em pedaços menores que a FIFO comporta, acumulando parciais.
- **Leitura**: `hal_npu_read_output()` lê os resultados prontos; `hal_npu_wait_done()` faz *polling* bloqueante no bit `DONE` do registrador de status.
- **DMA opcional**: `hal_npu_set_dma_enabled()` permite alternar entre transferências assistidas por DMA (`hal_dma_memcpy`) ou por loop de CPU, dependendo do que estiver disponível/habilitado na plataforma.

Os registradores MMIO da NPU (`NPU_REG_STATUS`, `NPU_REG_CMD`, `NPU_REG_CONFIG`, FIFOs de pesos/entrada/saída, configuração de quantização e bias) estão documentados na [mapa de memória](#npu-neural-processing-unit).

## Mapa de memória

Endereços físicos extraídos diretamente de `src/drivers/qemu/memory_map.h` e `src/drivers/fpga/memory_map.h`.

### Endereços base

| Periférico | QEMU (`virt`) | FPGA (SoC) |
|---|---|---|
| RAM (kernel) | `0x80000000` (256 KB) | `0x80000800` (254 KB — 2 KB reservados ao bootloader) |
| UART | `0x10000000` | `0x10000000` |
| GPIO | — (não existe) | `0x20000000` |
| VGA | `0x30000000` (stub) | `0x30000000` |
| DMA | `0x40000000` (stub) | — (usa o mesmo layout de `hal_dma.h`, `0x40000000`) |
| CLINT | `0x02000000` | `0x50000000` |
| PLIC | `0x0c000000` | `0x60000000` |
| NPU | `0x90000000` (stub) | `0x90000000` |

!!! note "Clock base"
    QEMU roda a **10 MHz** (`SYSTEM_CLOCK_HZ = 10000000`); a FPGA roda a **100 MHz** (`SYSTEM_CLOCK_HZ = 100000000`). Isso afeta diretamente o cálculo de ciclos em `hal_timer_get_freq()`, usado por `scheduler_sleep()` para converter milissegundos em ciclos de clock.

### UART

=== "QEMU — 16550A padrão"

    | Offset | Registrador | Descrição |
    |---|---|---|
    | `0x00` | `RBR` / `THR` | Receiver Buffer (leitura) / Transmitter Holding (escrita) |
    | `0x01` | `IER` | Interrupt Enable |
    | `0x02` | `FCR` | FIFO Control |
    | `0x03` | `LCR` | Line Control |
    | `0x05` | `LSR` | Line Status |

=== "FPGA — registrador customizado"

    | Offset | Registrador | Descrição |
    |---|---|---|
    | `0x00` | `UART_REG_DATA` | Dado (TX/RX) |
    | `0x04` | `UART_REG_CTRL` | Controle/Status |

    Bits de status/comando:

    | Bit | Nome | Significado |
    |---|---|---|
    | `1 << 0` | `UART_STATUS_TX_BUSY` | Transmissor ocupado |
    | `1 << 1` | `UART_STATUS_RX_VALID` | Há dado recebido disponível |
    | `1 << 0` | `UART_CMD_RX_POP` | Comando: avança a FIFO de recepção |
    | `1 << 2` | `UART_CMD_RX_FLUSH` | Comando: limpa a FIFO de recepção |

### GPIO (só FPGA)

O GPIO segue o modelo de microcontrolador: cada porta ocupa 64 bytes e tem os mesmos registradores.

| Offset | Bloco |
|---|---|
| `0x000`, `0x040`, `0x080`, `0x0C0` | Portas JA, JB, JC, JD (Pmods) |
| `0x100` | Porta LED (LD0..LD15) |
| `0x140` | Porta SW (chaves) |
| `0x180` | Porta BTN (BTNU, BTND, BTNL, BTNR) |
| `0x200` | Displays de 7 segmentos (`CTRL`, `HEX`, `RAW_LO`, `RAW_HI`, `DP`, `EN`) |
| `0x280` | LEDs RGB (`RGB0` = LD16, `RGB1` = LD17, `0x00RRGGBB`) |

| Offset na porta | Registrador | Descrição |
|---|---|---|
| `0x00` | `IN` | Estado dos pinos |
| `0x04` | `OUT` | Valor de saída |
| `0x08` | `DIR` | 1 = saída, 0 = entrada |
| `0x0C` / `0x10` / `0x14` | `OUTSET` / `OUTCLR` / `OUTTGL` | Liga / desliga / inverte os pinos marcados |
| `0x18` / `0x1C` / `0x20` | `IE` / `IES` / `IFG` | Interrupção: habilita, borda (0 = subida) e flags (escrever 1 limpa) |

### CLINT (Core Local Interruptor)

| Registrador | QEMU (offset de `CLINT_BASE = 0x02000000`) | FPGA (offset de `CLINT_BASE_ADDR = 0x50000000`) |
|---|---|---|
| `MSIP` | `0x0000` | `0x00` |
| `MTIMECMP_LO` | `0x4000` | `0x08` |
| `MTIMECMP_HI` | `0x4004` | `0x0C` |
| `MTIME_LO` | `0xBFF8` | `0x10` |
| `MTIME_HI` | `0xBFFC` | `0x14` |

O QEMU usa o layout **SiFive padrão**; a FPGA usa um layout próprio, mais compacto, adaptado ao barramento de 32-bit do SoC.

### PLIC (Platform-Level Interrupt Controller)

Layout idêntico (relativo) entre as duas plataformas — só a base muda (`0x0c000000` no QEMU, `0x60000000` na FPGA):

| Registrador | Offset (relativo à base) |
|---|---|
| `PLIC_PRIORITY(id)` | `0x000000 + id*4` |
| `PLIC_PENDING` | `0x001000` |
| `PLIC_ENABLE` | `0x002000` |
| `PLIC_THRESHOLD` | `0x200000` |
| `PLIC_CLAIM` | `0x200004` |

#### IDs de fonte de interrupção (definidos em `hal_plic.h`)

| ID | Fonte |
|---|---|
| `0` | `PLIC_SOURCE_NONE` |
| `1` | `PLIC_SOURCE_UART` |
| `2` | `PLIC_SOURCE_DMA` |
| `3` | `PLIC_SOURCE_NPU` |
| `4` | `PLIC_SOURCE_GPIO` |

`PLIC_MAX_SOURCES = 32` — limite da tabela de dispatcher em software (`irq_dispatch.c`).

### DMA

| Offset (de `DMA_BASE_ADDR = 0x40000000`) | Registrador | Descrição |
|---|---|---|
| `0x00` | `SRC` | Endereço de origem |
| `0x04` | `DST` | Endereço de destino |
| `0x08` | `CNT` | Quantidade de palavras (32-bit) |
| `0x0C` | `CTRL` | Controle/Status |

Bits de `CTRL`:

| Bit | Nome | Significado |
|---|---|---|
| `1 << 0` (escrita) | `DMA_CTRL_START` | Inicia a transferência |
| `1 << 0` (leitura) | `DMA_CTRL_BUSY` | `1` = ocupado |
| `1 << 1` | `DMA_CTRL_FIXED_DST` | `1` = destino fixo (útil para escrever em uma FIFO, ex. da NPU) |

### VGA

| Constante | Valor |
|---|---|
| `VGA_WIDTH` × `VGA_HEIGHT` | 320 × 240 (upscaled para 640×480 na saída) |
| Formato de cor | RGB332 (8 bits: 3-3-2) |
| VSync (só FPGA) | `VGA_BASE_ADDR + 0x1FFFF`, bit `1 << 0` |

### NPU (Neural Processing Unit)

Todos os offsets são relativos a `NPU_BASE_ADDR = 0x90000000`:

| Offset | Registrador | Acesso | Descrição |
|---|---|---|---|
| `0x00` | `NPU_REG_STATUS` | RO | Flags de status |
| `0x04` | `NPU_REG_CMD` | WO | Comandos (start, clear, reset) |
| `0x08` | `NPU_REG_CONFIG` | RW | Tamanho do run (`K_DIM`) |
| `0x10` | `NPU_REG_WRITE_W` | WO | FIFO de pesos (weights) |
| `0x14` | `NPU_REG_WRITE_A` | WO | FIFO de entradas/ativações |
| `0x18` | `NPU_REG_READ_OUT` | RO | FIFO de saída |
| `0x40` | `NPU_REG_QUANT_CFG` | RW | Shift e zero-point de quantização |
| `0x44` | `NPU_REG_QUANT_MULT` | RW | Multiplicador de ponto fixo (PPU) |
| `0x48` | `NPU_REG_FLAGS` | RW | Flags de controle (ex.: ReLU) |
| `0x80`–`0x8C` | `NPU_REG_BIAS_BASE` | RW | Vetor de bias |

#### Bits de `NPU_REG_STATUS` (`0x00`)

| Bit | Nome | Significado |
|---|---|---|
| `1 << 0` | `NPU_STATUS_BUSY` | NPU processando |
| `1 << 1` | `NPU_STATUS_DONE` | Execução concluída |
| `1 << 3` | `NPU_STATUS_OUT_VLD` | Saída válida disponível |

#### Bits de `NPU_REG_CMD` (`0x04`)

| Bit | Nome | Significado |
|---|---|---|
| `1 << 0` | `NPU_CMD_RST_PTRS` | Reseta todos os ponteiros |
| `1 << 1` | `NPU_CMD_START` | Dispara execução |
| `1 << 2` | `NPU_CMD_ACC_CLEAR` | Limpa acumuladores antes de rodar |
| `1 << 3` | `NPU_CMD_ACC_NO_DRAIN` | `1` = mantém resultado no array (tiling); `0` = salva na FIFO de saída |
| `1 << 4` | `NPU_CMD_RST_W_RD` | Reseta leitura de pesos (reuso) |
| `1 << 5` | `NPU_CMD_RST_I_RD` | Reseta leitura de entradas (reuso) |
| `1 << 6` | `NPU_CMD_RST_WR_W` | Reseta escrita de pesos |
| `1 << 7` | `NPU_CMD_RST_WR_I` | Reseta escrita de entradas |

#### Bits de `NPU_REG_FLAGS` (`0x48`)

| Bit | Nome | Significado |
|---|---|---|
| `1 << 0` | `NPU_FLAG_RELU` | `1` = ativa ReLU na saída |

Para a explicação de como esses registradores são usados pela API C da HAL, veja [A NPU](#a-npu-neural-processing-unit).
