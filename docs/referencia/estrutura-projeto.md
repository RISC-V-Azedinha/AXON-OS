# Estrutura do projeto

```text
AXON-OS/
├── makefile                  # Build para QEMU e FPGA (veja Guias → Build e execução)
├── upload.py                 # Gravador/monitor serial para a FPGA
│
├── include/                  # Todos os headers (.h) do projeto
│   ├── apps/                 # Headers das tarefas e comandos de shell
│   │   ├── apps.h
│   │   ├── apps_tasks.h
│   │   ├── commands.h
│   │   └── shell_utils.h
│   ├── hal/                  # Interfaces da HAL (comuns às duas plataformas)
│   │   ├── hal_dma.h
│   │   ├── hal_gpio.h
│   │   ├── hal_irq.h
│   │   ├── hal_npu.h
│   │   ├── hal_plic.h
│   │   ├── hal_timer.h
│   │   ├── hal_uart.h
│   │   └── hal_vga.h
│   ├── kernel/                # Headers do núcleo do kernel
│   │   ├── fs.h
│   │   ├── logger.h
│   │   ├── mm.h
│   │   ├── mutex.h
│   │   └── task.h
│   ├── sys/
│   │   └── syscall.h          # Números de syscall + wrappers inline para tarefas
│   └── util/
│       ├── circular_buffer.h  # Buffer circular (usado pelo RX da UART)
│       └── math_ops.h
│
├── src/
│   ├── apps/                  # Tarefas de usuário (rodam sob o escalonador)
│   │   ├── task_leds.c
│   │   ├── task_monitor.c
│   │   └── task_shell.c
│   ├── bin/                   # Implementação dos comandos de shell
│   │   ├── clear.c
│   │   ├── gpio_cmds.c        # gpio, seg, rgb
│   │   ├── help.c
│   │   ├── mem_cmds.c         # heap, peek, poke, free, defrag
│   │   ├── memtest.c
│   │   ├── panic.c
│   │   ├── ps.c
│   │   ├── reboot.c
│   │   └── ts_cmds.c          # stop, resume, alloc
│   ├── bsp/                   # Board Support Package: boot e linker scripts
│   │   ├── start.s            # Ponto de entrada (_start)
│   │   ├── link.ld            # Linker script — QEMU
│   │   └── link_fpga.ld       # Linker script — FPGA
│   ├── drivers/
│   │   ├── qemu/               # Implementação da HAL para QEMU virt
│   │   │   ├── hal_dma.c, hal_gpio.c, hal_npu.c, hal_plic.c,
│   │   │   ├── hal_timer.c, hal_uart.c
│   │   │   └── memory_map.h    # Endereços MMIO específicos do QEMU
│   │   └── fpga/                # Implementação da HAL para o SoC em FPGA
│   │       ├── hal_dma.c, hal_gpio.c, hal_npu.c, hal_plic.c,
│   │       ├── hal_timer.c, hal_uart.c, hal_vga.c
│   │       └── memory_map.h    # Endereços MMIO específicos da FPGA
│   └── kernel/                 # Núcleo do sistema operacional
│       ├── main.c              # kernel_main() + trap_handler() (veja Boot / Interrupções)
│       ├── trap.s              # Troca de contexto em Assembly
│       ├── scheduler.c         # Escalonador de tarefas
│       ├── mm.c                # Alocador de heap (kmalloc/kfree)
│       ├── fs.c                # Sistema de arquivos (RamFS)
│       ├── irq_dispatch.c      # Dispatcher de interrupções externas (PLIC)
│       ├── apps.c               # Inicialização de mutexes/buffers das apps
│       ├── file_cmds.c          # Comandos de shell relacionados a arquivos (ls, cat, edit...)
│       ├── logger.c             # Funções de log coloridas (log_info, log_ok, ...)
│       └── math_ops.c
│
└── build/                      # Gerado pelo make — não versionado (.gitignore)
    └── obj/{qemu,fpga}/...      # Objetos espelhados por plataforma
```

## Convenção de nomes

- **`hal_*`**: funções da camada de abstração de hardware (uma implementação por plataforma).
- **`sys_*`**: wrappers `static inline` em `syscall.h`, chamados pelo espaço de "usuário" (tarefas) para disparar uma `ecall`.
- **`k*`** (`kmalloc`, `kfree`, `kheap_dump`...): funções internas do kernel que implementam a lógica por trás de uma syscall.
- **`cmd_*`**: implementação de um comando do shell, registrado na tabela `shell_commands[]` em `task_shell.c`.
- **`task_*`**: função de entrada de uma tarefa (o "main" de cada tarefa, passado para `task_create`).
