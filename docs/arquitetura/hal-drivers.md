# HAL e drivers

A **HAL** (Hardware Abstraction Layer) é o que permite ao mesmo código de kernel — escalonador, memória, sistema de arquivos, shell — compilar e rodar tanto no QEMU quanto no SoC em FPGA sem nenhuma alteração. Cada módulo de hardware tem um **header comum** em `include/hal/` e **duas implementações**, uma por plataforma, em `src/drivers/qemu/` e `src/drivers/fpga/`.

## Como a seleção acontece

O `makefile` decide qual conjunto de `.c` compilar (veja [Build e execução](../guias/build-e-execucao.md#como-o-makefile-decide-o-que-compilar)):

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

## Mapas de memória (MMIO): QEMU vs. FPGA

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

Veja a [tabela completa de endereços](../referencia/mapa-memoria.md) lado a lado.

## O caso especial: `hal_irq.h`

Diferente dos demais módulos, boa parte de `hal_irq.h` é **comum às duas plataformas** — as funções de controle de interrupção da CPU (`hal_irq_global_enable`, `hal_irq_mask_enable`, `hal_irq_set_handler`, etc.) são `static inline` e operam diretamente sobre CSRs padrão do RISC-V (`mstatus`, `mie`, `mtvec`), que existem identicamente em qualquer implementação `rv32i_zicsr`. Só o **dispatcher de PLIC** (`irq_dispatch.c`, em `src/kernel/`, não em `src/drivers/`) e o driver de baixo nível do PLIC (`hal_plic.c`, este sim por plataforma) lidam com diferenças de hardware.

## A NPU (Neural Processing Unit)

A `hal_npu.h` é o driver mais elaborado da HAL, e o que dá nome ao projeto (veja [Início](../index.md#por-que-axon)). Ela expõe uma interface de acelerador de multiplicação de matrizes quantizadas (inteiros de 8 bits), pensada para inferência de redes neurais:

- **Configuração**: `hal_npu_configure(k_dim, quant_params)` define a profundidade de acumulação e os parâmetros de quantização (multiplicador de ponto fixo, shift, zero-point, ativação ReLU opcional).
- **Carregamento de dados**: `hal_npu_load_weights()` / `hal_npu_load_inputs()` escrevem em FIFOs de hardware (pesos e ativações, empacotados como `int8x4` dentro de palavras de 32 bits); `hal_npu_load_bias()` carrega o vetor de bias.
- **Execução**: `hal_npu_start()` dispara uma execução limpa (reset de ponteiros e acumuladores); `hal_npu_start_accumulate()` dispara sem limpar os acumuladores — útil para *tiling*, ou seja, quebrar uma multiplicação de matrizes grande em pedaços menores que a FIFO comporta, acumulando parciais.
- **Leitura**: `hal_npu_read_output()` lê os resultados prontos; `hal_npu_wait_done()` faz *polling* bloqueante no bit `DONE` do registrador de status.
- **DMA opcional**: `hal_npu_set_dma_enabled()` permite alternar entre transferências assistidas por DMA (`hal_dma_memcpy`) ou por loop de CPU, dependendo do que estiver disponível/habilitado na plataforma.

Os registradores MMIO da NPU (`NPU_REG_STATUS`, `NPU_REG_CMD`, `NPU_REG_CONFIG`, FIFOs de pesos/entrada/saída, configuração de quantização e bias) estão documentados na [tabela de mapa de memória](../referencia/mapa-memoria.md#npu-neural-processing-unit).
