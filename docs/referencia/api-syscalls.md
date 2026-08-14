# Tabela de syscalls

Todas as syscalls são definidas em `include/sys/syscall.h`. O mecanismo geral (ABI, como o número e os argumentos viajam até o kernel) está descrito em [Interrupções e syscalls](../arquitetura/interrupcoes-syscalls.md#excecoes-ecall-sincrona-cause_code-11).

## Convenção de chamada

- O número da syscall vai no registrador `a7`.
- Argumentos (até 3, no AXON-OS) vão em `a0`, `a1`, `a2`.
- O valor de retorno (quando houver) volta em `a0`.
- A instrução `ecall` dispara a exceção síncrona.

Cada syscall tem um wrapper `static inline` correspondente em `syscall.h`, que esconde o Assembly bruto — é essa função que as tarefas devem chamar.

## Lista completa

| # | Constante | Wrapper C | Assinatura de retorno/args | Descrição |
|---|---|---|---|---|
| 1 | `SYS_YIELD` | `sys_yield()` | `void` | Cede a CPU voluntariamente para a próxima tarefa pronta |
| 2 | `SYS_WRITE` | `sys_write_char(char c)` / `sys_puts(const char* s)` | `void` | Escreve um caractere (ou string) na UART |
| 3 | `SYS_SLEEP` | `sys_sleep(uint32_t ms)` | `void` | Bloqueia a tarefa atual por `ms` milissegundos |
| 4 | `SYS_LOCK` | `int sys_mutex_lock(mutex_t *m)` | `1` = obteve a trava, `0` = já estava travado | Tenta travar um mutex |
| 5 | `SYS_UNLOCK` | `sys_mutex_unlock(mutex_t *m)` | `void` | Destrava um mutex (só o dono pode) |
| 6 | `SYS_GET_TASKS` | `int sys_get_tasks(task_info_t *buffer, int max_count)` | Nº de tarefas copiadas | Copia informações de todas as tarefas para `buffer` |
| 7 | `SYS_PEEK` | `uint32_t sys_peek(uint32_t addr)` | Valor lido | Lê 32 bits de um endereço físico arbitrário |
| 8 | `SYS_POKE` | `sys_poke(uint32_t addr, uint32_t val)` | `void` | Escreve 32 bits em um endereço físico arbitrário |
| 9 | `SYS_HEAP_INFO` | `sys_heap_info()` | `void` | Imprime o mapa completo do heap na UART (`kheap_dump`) |
| 10 | `SYS_MALLOC` | `void* sys_malloc(uint32_t size)` | Ponteiro alocado ou `NULL` | Aloca memória no heap do kernel |
| 11 | `SYS_FREE` | `uint8_t sys_free(void* ptr)` | `0` = sucesso, não-zero = erro | Libera memória alocada com `sys_malloc` |
| 12 | `SYS_DEFRAG` | `sys_defrag()` | `void` | Funde blocos livres adjacentes no heap |
| 13 | `SYS_SUSPEND` | `int sys_suspend(uint32_t pid)` | `0` = sucesso, `-1` = erro | Suspende uma tarefa pelo PID |
| 14 | `SYS_RESUME` | `int sys_resume(uint32_t pid)` | `0` = sucesso, `-1` = erro | Retoma uma tarefa suspensa pelo PID |
| 15 | `SYS_FS_CREATE` | `sys_fs_create(const char *name)` | `0` = sucesso, `<0` = erro | Cria um arquivo vazio no RamFS |
| 16 | `SYS_FS_WRITE` | `sys_fs_write(const char *name, const uint8_t *data, uint32_t len)` | Bytes escritos ou `<0` | Sobrescreve o conteúdo de um arquivo |
| 17 | `SYS_FS_READ` | `sys_fs_read(const char *name, uint8_t *buffer, uint32_t max_len)` | Bytes lidos ou `<0` | Lê o conteúdo de um arquivo |
| 18 | `SYS_FS_LIST` | `sys_fs_list(char *buffer, uint32_t max_len)` | `0` | Lista os arquivos do diretório raiz |
| 19 | `SYS_FS_DELETE` | `sys_fs_delete(const char *name)` | `0` = sucesso, `<0` = erro | Remove um arquivo |
| 20 | `SYS_FS_FORMAT` | (sem wrapper dedicado) | `0` | Reformata o RamFS |

!!! note "Syscalls sem wrapper `static inline` próprio"
    `SYS_FS_CREATE`, `SYS_FS_WRITE`, `SYS_FS_READ`, `SYS_FS_LIST` e `SYS_FS_DELETE` não têm wrapper em `syscall.h` — eles são implementados diretamente em `src/kernel/file_cmds.c` (`sys_fs_create`, `sys_fs_write`, etc.), próximos aos comandos de shell que os usam, seguindo o mesmo padrão de `asm volatile` com `ecall`.

## `task_info_t`

Struct usada por `sys_get_tasks()` para copiar o estado de todas as tarefas de uma vez:

```c
typedef struct {
    uint32_t id;         // PID (= tid)
    char     name[16];
    uint32_t state;       // 0=Ready, 1=Running, 2=Blocked, 3=Suspended
    uint32_t priority;
    uint32_t sp;           // Stack Pointer atual
    uint64_t wake_time;    // Ciclo de mtime para acordar (se Blocked)
} task_info_t;
```

É o que alimenta o comando de shell [`ps`](comandos-shell.md#ps).

## Exemplo: implementando uma nova syscall

Para adicionar uma syscall nova, três lugares precisam ser tocados:

1. **`include/sys/syscall.h`**: adicionar `#define SYS_MINHA_SYSCALL <próximo número livre>` e, opcionalmente, um wrapper `static inline` no padrão dos existentes.
2. **`src/kernel/main.c`** (`trap_handler`): adicionar um `case SYS_MINHA_SYSCALL:` no `switch`, lendo argumentos de `ctx[9]`/`ctx[10]`/`ctx[11]` (`a0`/`a1`/`a2`) e escrevendo o retorno em `ctx[9]` (`a0`) se aplicável.
3. A **função real** que a syscall invoca deve morar no kernel (`src/kernel/*.c`), nunca em `src/apps/` ou `src/bin/` — o handler roda em contexto de trap, com acesso direto às estruturas internas do kernel.
