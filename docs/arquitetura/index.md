# Arquitetura

Esta seção descreve como o AXON-OS é construído internamente, dos primeiros ciclos de boot até a troca de contexto entre tarefas.

- [Visão geral](visao-geral.md) — a "planta baixa" do sistema: camadas, fluxo de execução e por que certas decisões de projeto foram tomadas.
- [Boot](boot.md) — o que acontece do reset da CPU até o primeiro `schedule()`.
- [Escalonador de tarefas](escalonador.md) — o TCB, o algoritmo de escalonamento e a troca de contexto em Assembly.
- [Gerenciamento de memória](memoria.md) — o alocador de heap `kmalloc`/`kfree`.
- [Sistema de arquivos](sistema-arquivos.md) — o RamFS inspirado no Ext2.
- [Interrupções e syscalls](interrupcoes-syscalls.md) — o trap handler central, o PLIC e a tabela de syscalls.
- [HAL e drivers](hal-drivers.md) — como o mesmo kernel roda em QEMU e em FPGA.

!!! tip "Leitura sugerida"
    Se você é novo no projeto, leia [Visão geral](visao-geral.md) e [Boot](boot.md) primeiro — as demais páginas assumem que você já entende o fluxo geral de execução.
