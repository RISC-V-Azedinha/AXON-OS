# Considerações

Antes de entrar na arquitetura do AXON-OS, vale posicionar o projeto: **ele não compete com RTOS de produção como o FreeRTOS ou o Zephyr, nem foi desenhado para isso.** Esta página explica por quê, e o que o AXON-OS de fato se propõe a demonstrar.

## Por que criar mais um RTOS?

Existem RTOS maduros, testados em produção e com suporte a dezenas de arquiteturas — não faria sentido escrever o AXON-OS se o objetivo fosse "ter um RTOS funcional". O objetivo real era outro: o AXON-OS nasceu como **exemplificação prática do uso de interrupções em uma arquitetura RISC-V privilegiada básica** — em particular, do **CLINT** (Core Local Interruptor, responsável pelo timer que viabiliza a preempção) e do **PLIC** (Platform-Level Interrupt Controller, responsável por interrupções externas como a UART), operando inteiramente em **Modo Máquina (M-mode)**.

Ou seja: o AXON-OS é um **estudo de caso**, não um produto. Cada subsistema (escalonador, heap, RamFS, shell) existe para dar contexto de uso realista ao mecanismo central que o projeto queria explorar — o ciclo completo de `mtvec` → trap → `mcause`/`mepc` → `mret`, e como esse ciclo sustenta tanto a preempção via timer quanto a interface de syscalls via `ecall`. Ver [Interrupções e syscalls](arquitetura/interrupcoes-syscalls.md) para o detalhamento desse mecanismo.

## Comparação com FreeRTOS e Zephyr

A tabela abaixo não é uma disputa de "melhor ou pior" — é uma forma de deixar explícito **o que foi deliberadamente deixado de fora** do AXON-OS, e por quê isso é aceitável dado seu objetivo pedagógico.

| Aspecto | AXON-OS | FreeRTOS | Zephyr |
|---|---|---|---|
| Objetivo | Estudo de caso acadêmico (TCC) sobre interrupções/CLINT e M-mode | Kernel RTOS de produção, leve e portável | RTOS de propósito geral, completo, para IoT/embarcados |
| Portabilidade | 2 alvos fixos (QEMU `virt` e um SoC FPGA próprio) | Dezenas de arquiteturas (ARM Cortex-M/A, RISC-V, Xtensa, x86...) | Dezenas de arquiteturas, com camada de abstração de SoC extensa (HAL + device tree) |
| Modo de privilégio | Só M-mode — kernel e tarefas no mesmo nível | Tipicamente sem MMU/MPU no core, mas com suporte a MPU em ports específicos | Suporta M-mode/S-mode (RISC-V), MPU e até MMU com memória virtual em configurações específicas |
| Isolamento de memória | Nenhum — todas as tarefas compartilham o mesmo espaço de endereçamento | Opcional, via MPU em alguns ports (`FreeRTOS-MPU`) | Sim, via `CONFIG_USERSPACE` (isolamento de processos com MPU/MMU) |
| Escalonador | Round-robin com prioridade fixa, pool estático de 4 tarefas | Prioridades fixas ou tempo-compartilhado, altamente configurável, sem limite artificial de tarefas | Multiplas políticas (cooperativo, preemptivo, `EDF`), SMP em múltiplos núcleos |
| Alocação de memória | Um heap único (`kmalloc`/`kfree` com canary) | 5 esquemas prontos (`heap_1` a `heap_5`), do mais simples ao mais robusto | Slab allocator, heap do sistema, `k_mem_pool`, múltiplas políticas configuráveis |
| Sistema de build | Um `makefile` direto, dois alvos fixos | Integração via CMake/Make, mas o kernel em si é só um conjunto de fontes a incluir | `west` + `CMake` + `Kconfig` + *Device Tree* — sistema de build/configuração completo |
| Sistema de arquivos | RamFS volátil, inspirado no Ext2, sem persistência | Não faz parte do core; add-ons como `FreeRTOS+FAT` existem separadamente | Múltiplos sistemas de arquivo (LittleFS, FAT) integrados ao build |
| Rede | Não tem | Não faz parte do core; `FreeRTOS+TCP` é um add-on | Pilha de rede completa integrada (`Zephyr networking stack`, BLE, 802.15.4, etc.) |
| Interface com o usuário | Shell próprio via UART, com editor de texto embutido | Nenhuma por padrão; `FreeRTOS+CLI` é um add-on | Shell embutido (`Zephyr Shell`), bastante completo |
| Certificação / segurança funcional | Não se aplica (projeto acadêmico) | Existem variantes certificadas (ex.: `FreeRTOS Kernel` com qualificação para uso em produtos regulados) | Projeto sob a Linux Foundation, com processo de segurança e CVEs tratados formalmente |
| Maturidade / comunidade | Um projeto individual, único autor, escopo de TCC | Décadas de uso em produção, adotado pela AWS, enorme base instalada | Grande comunidade multi-vendor (Linux Foundation), dezenas de fabricantes de silício contribuindo |

## O que isso significa na prática

- **Não há MMU nem MPU no AXON-OS** — nem porque a arquitetura RISC-V não suporte isso, mas porque o objetivo do projeto era demonstrar o mecanismo de trap/interrupção em M-mode puro, sem a complexidade adicional de troca de modo de privilégio ou paginação. Zephyr, por comparação, é capaz de rodar com isolamento de processo completo quando o hardware permite.
- **A camada HAL do AXON-OS cobre exatamente dois alvos** (veja [HAL e drivers](arquitetura/hal-drivers.md)) porque o projeto foi construído em torno de um SoC específico, não como uma plataforma de abstração genérica — diferente do FreeRTOS (portas mínimas, mas numerosas) e do Zephyr (abstração completa de SoC via device tree).
- **O escalonador é propositalmente simples**: round-robin com prioridade fixa e um pool estático de 4 tarefas (veja [Escalonador de tarefas](arquitetura/escalonador.md)), o suficiente para demonstrar preempção via CLINT sem a sofisticação de escalonadores configuráveis do FreeRTOS ou as múltiplas políticas do Zephyr.
- **O RamFS e o shell existem para dar um caso de uso tangível** à interface de syscalls (`ecall`) — não para competir com sistemas de arquivo ou shells de produção. A motivação foi ter algo *interativo* para demonstrar o ciclo completo de interrupção/exceção em uso real, não construir infraestrutura de armazenamento.

## Conclusão

O AXON-OS deve ser lido como o que é: um kernel didático que usa um caso de uso completo (escalonamento, memória, arquivos, shell) como **veículo** para explicar, de forma concreta e executável, como interrupções de timer (CLINT), interrupções externas (PLIC) e exceções síncronas (`ecall`) funcionam em uma arquitetura RISC-V privilegiada básica. Para esse objetivo, ele cumpre seu papel — e é exatamente por isso que a simplicidade das tabelas acima é uma escolha de projeto, não uma limitação a ser desculpada.
