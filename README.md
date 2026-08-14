# AXON-OS

**AXON RTOS** é um sistema operacional de tempo real (RTOS) educacional, escrito do zero em C e Assembly para a arquitetura **RISC-V de 32 bits** (`rv32i_zicsr`). O projeto nasceu como parte de um Trabalho de Conclusão de Curso (TCC) e serve, sobretudo, como **estudo de caso prático sobre interrupções** — em particular o **CLINT** (timer) e o **PLIC** (interrupções externas) — e sobre uma **arquitetura privilegiada básica** em Modo Máquina (M-mode) puro.

Para o contexto completo do projeto — o que ele se propõe (e não se propõe) a ser, incluindo uma comparação com RTOS de produção como FreeRTOS e Zephyr — veja [Considerações](docs/consideracoes.md) na documentação.

## Características

- Escalonador preemptivo por prioridades com round-robin;
- Gerenciador de memória dinâmica (heap) com proteção contra corrupção (*canary*);
- Sistema de arquivos em RAM inspirado no Ext2 (RamFS);
- Interface de syscalls via `ecall`, no estilo RISC-V/SBI;
- Shell interativo via UART, com editor de texto embutido;
- Camada HAL que permite compilar o mesmo kernel tanto para o **QEMU** (`virt`) quanto para um **SoC customizado em FPGA**, este equipado com um acelerador de rede neural (NPU) próprio.

## Começando rápido

Pré-requisitos: toolchain `riscv64-unknown-elf-gcc` e `qemu-system-riscv32`.

```bash
# Compilar para o QEMU
make

# Rodar
make run
```

Para compilar/gravar na FPGA e para instruções detalhadas de instalação, veja o guia [Build e execução](docs/guias/build-e-execucao.md) e [Upload para a FPGA](docs/guias/upload-fpga.md).

## Documentação

A documentação completa do projeto — arquitetura interna, guias de uso e referência de syscalls/comandos — está em [`docs/`](docs/) e é servida com [MkDocs](https://www.mkdocs.org/) + [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/).

Para visualizar localmente:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```

Depois abra `http://127.0.0.1:8000`.

### Sumário

- [Considerações](docs/consideracoes.md) — o que o projeto é (e não é), e comparação com FreeRTOS/Zephyr
- **Guias**: [Instalação](docs/guias/instalacao.md) · [Build e execução](docs/guias/build-e-execucao.md) · [Upload para a FPGA](docs/guias/upload-fpga.md) · [Usando o shell](docs/guias/shell.md)
- **Arquitetura**: [Visão geral](docs/arquitetura/visao-geral.md) · [Boot](docs/arquitetura/boot.md) · [Escalonador](docs/arquitetura/escalonador.md) · [Memória](docs/arquitetura/memoria.md) · [Sistema de arquivos](docs/arquitetura/sistema-arquivos.md) · [Interrupções e syscalls](docs/arquitetura/interrupcoes-syscalls.md) · [HAL e drivers](docs/arquitetura/hal-drivers.md)
- **Referência**: [Estrutura do projeto](docs/referencia/estrutura-projeto.md) · [Mapa de memória](docs/referencia/mapa-memoria.md) · [Tabela de syscalls](docs/referencia/api-syscalls.md) · [Comandos do shell](docs/referencia/comandos-shell.md)

## Estrutura do repositório

```text
AXON-OS/
├── docs/            # Documentação (MkDocs)
├── include/         # Headers do kernel, HAL, apps e utilitários
├── src/
│   ├── apps/        # Tarefas de usuário (shell, monitor, LEDs)
│   ├── bin/         # Implementação dos comandos do shell
│   ├── bsp/         # Boot (start.s) e linker scripts
│   ├── drivers/      # HAL: implementações para QEMU e FPGA
│   └── kernel/       # Núcleo: escalonador, memória, RamFS, trap handler
├── makefile         # Build para QEMU e FPGA
└── upload.py        # Gravador/monitor serial para a FPGA
```

Veja [Estrutura do projeto](docs/referencia/estrutura-projeto.md) para o detalhamento completo.

