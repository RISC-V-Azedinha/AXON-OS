# AXON-OS

**AXON RTOS** é um sistema operacional de tempo real (RTOS) educacional, escrito do zero em C e Assembly, para a arquitetura **RISC-V de 32 bits** (`rv32i_zicsr`). O projeto nasceu como parte de um Trabalho de Conclusão de Curso (TCC) e implementa, sobre hardware bare-metal, os pilares básicos de um sistema operacional:

- Um **escalonador preemptivo** por prioridades com round-robin;
- Um **gerenciador de memória dinâmica** (heap) com proteção contra corrupção;
- Um **sistema de arquivos** em RAM inspirado no Ext2;
- Uma **interface de syscalls** via `ecall`, no estilo RISC-V/SBI;
- Um **shell interativo** com um editor de texto embutido;
- Uma **camada HAL** (Hardware Abstraction Layer) que permite rodar o mesmo kernel tanto no emulador **QEMU** quanto em um **SoC customizado em FPGA**, este último equipado com um acelerador de rede neural (**NPU**) próprio.

!!! info "Para quem é esta documentação"
    Esta documentação cobre a arquitetura interna do kernel, como compilar e executar o AXON-OS, como usar o shell e como estender o sistema com novos drivers, comandos ou syscalls.

## Características principais

<div class="grid cards" markdown>

- :material-cpu-64-bit: __RISC-V bare-metal__

    Kernel em Modo Máquina (M-mode) puro, sem depender de SBI ou OpenSBI. Trap handler único para interrupções e exceções, com troca de contexto em Assembly.

- :material-swap-horizontal: __Escalonador preemptivo__

    Round-robin com prioridades, preempção por tick de timer (100 ms) e suporte a `sleep`, `suspend`/`resume` e mutexes cooperativos.

- :material-memory: __Heap com canary__

    Alocador `kmalloc`/`kfree` de lista encadeada com *splitting*, *coalescing* (desfragmentação) e detecção de corrupção via valores sentinela (*canary*).

- :material-file-tree: __RamFS (mini-Ext2)__

    Sistema de arquivos volátil em RAM com superbloco, bitmaps de inodes/blocos e diretório raiz — inspirado na estrutura on-disk do Ext2.

- :material-console: __Shell + editor__

    Terminal interativo via UART com histórico de comandos, redimensionamento de tela ANSI e um mini-editor de texto (`edit`) no estilo *nano*.

- :material-chip: __Duas plataformas, um kernel__

    O mesmo código-fonte compila para o `virt` do QEMU (desenvolvimento rápido) e para um SoC próprio em FPGA com UART, GPIO, DMA, VGA e NPU customizados.

</div>

## Por onde começar

- Se você quer entender **o que o projeto se propõe (e não se propõe) a ser**, leia [Considerações](consideracoes.md) antes de tudo.
- Se você quer **compilar e rodar** o AXON-OS agora, vá para [Instalação do ambiente](guias/instalacao.md) e depois [Build e execução](guias/build-e-execucao.md).
- Se você quer **entender como o kernel funciona por dentro**, comece pela [Visão geral da arquitetura](arquitetura/visao-geral.md).
- Se você está procurando uma **syscall ou comando de shell específico**, vá direto para a [Referência](referencia/index.md).

## Estado do projeto

É um projeto acadêmico em desenvolvimento ativo — algumas simplificações são intencionais (ex.: alocação estática de tarefas, ausência de modo usuário/MMU) e estão documentadas ao longo do texto onde relevante.
