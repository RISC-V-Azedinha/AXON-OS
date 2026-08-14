# Comandos do shell

Referência de todos os comandos registrados em `shell_commands[]` (`src/apps/task_shell.c`). Para uma introdução ao shell em si, veja [Usando o shell](../guias/shell.md).

## Sistema

### `help`

Lista todos os comandos disponíveis, com uma breve descrição de cada um.

```
help
```

### `clear`

Limpa a tela e reimprime o cabeçalho do AXON-OS.

```
clear
```

### `reboot`

Reinicia o sistema imediatamente (aguarda 500 ms e salta de volta para `_start()`).

```
reboot
```

### `panic`

Dispara deliberadamente uma exceção de **instrução ilegal** (`.word 0xFFFFFFFF`), para exercitar o caminho de tratamento de crash do kernel (veja [Interrupções e syscalls → Exceções não tratadas](../arquitetura/interrupcoes-syscalls.md#excecoes-nao-tratadas-crash)). O sistema reinicia automaticamente 3 segundos depois.

```
panic
```

## Processos

### `ps`

Lista todas as tarefas do sistema: PID, nome, prioridade, estado, *stack pointer* atual e (se bloqueada) o ciclo de despertar.

```
ps
```

```
  PID   NAME            PRIO   STATE         SP          WAKE_TIME
  --------------------------------------------------------------------
  0     Task LEDs       1      READY         0x8003FC80  -
  1     Task Monitor    1      RUNNING       0x8003F880  -
  2     Task Shell      2      READY         0x8003F480  -
  3     Idle            0      READY         0x8003F080  -
```

### `stop <pid>`

Suspende a tarefa com o PID informado (`SYS_SUSPEND`). A tarefa `Idle` (prioridade 0) não pode ser suspensa.

```
stop 0
```

### `resume <pid>`

Retoma uma tarefa previamente suspensa (`SYS_RESUME`).

```
resume 0
```

## Memória

### `heap`

Imprime o mapa completo do heap: endereço de cada bloco, tamanho, status (`FREE`/`USED`) e resultado da checagem de integridade do canary.

```
heap
```

### `alloc <bytes>`

Aloca um bloco de `bytes` bytes no heap do kernel via `sys_malloc` e imprime o endereço retornado — pensado para ser usado em conjunto com `poke`, como uma forma "segura" de uma tarefa reservar memória própria antes de escrever nela.

```
alloc 128
```

### `free <addr>`

Libera o bloco alocado no endereço hexadecimal informado.

```
free 0x8003a010
```

### `defrag`

Solicita a fusão (*coalescing*) de blocos livres adjacentes no heap.

```
defrag
```

### `peek <addr>`

Lê 32 bits do endereço físico informado (hexadecimal, com ou sem prefixo `0x`).

```
peek 0x10000000
```

### `poke <addr> <val>`

Escreve 32 bits (`val`) no endereço físico informado (`addr`).

```
poke 0x20000000 0xFF
```

!!! warning "Sem verificação de limites"
    `peek`/`poke` acessam memória física diretamente, sem nenhuma validação — é possível corromper o próprio kernel escrevendo em um endereço errado. Use com cuidado.

### `memtest`

Teste simples de alocação: aloca 128 bytes com `kmalloc`, imprime o endereço e libera em seguida.

```
memtest
```

## Sistema de arquivos

### `ls`

Lista os arquivos existentes no RamFS.

```
ls
```

### `touch <arquivo>`

Cria um arquivo vazio.

```
touch notas.txt
```

### `rm <arquivo>`

Remove um arquivo.

```
rm notas.txt
```

### `cat <arquivo>`

Imprime o conteúdo de um arquivo na tela.

```
cat notas.txt
```

### `write <arquivo> <dados>`

Escreve texto simples em um arquivo, sobrescrevendo o conteúdo anterior. Tudo depois do primeiro espaço é tratado como o conteúdo a ser gravado (sem suporte a aspas).

```
write notas.txt Ola Mundo
```

### `edit <arquivo>`

Abre o [mini-editor de texto embutido](../guias/shell.md#o-editor-edit) (`Ctrl+W` salva, `Ctrl+X` sai).

```
edit notas.txt
```

## Resumo

| Categoria | Comandos |
|---|---|
| Sistema | `help`, `clear`, `reboot`, `panic` |
| Processos | `ps`, `stop`, `resume` |
| Memória | `heap`, `alloc`, `free`, `defrag`, `peek`, `poke`, `memtest` |
| Arquivos | `ls`, `touch`, `rm`, `cat`, `write`, `edit` |
