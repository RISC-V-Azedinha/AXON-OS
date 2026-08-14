# Usando o shell

Assim que o AXON-OS termina o boot, a tarefa `Task Shell` assume a UART e apresenta um prompt interativo:

```
root@axon:~$
```

O shell roda como uma tarefa comum do escalonador (prioridade 2) — ele não é um caso especial do kernel. Ele lê caracteres de um buffer circular alimentado pela interrupção da UART (veja [Interrupções e syscalls](../arquitetura/interrupcoes-syscalls.md)), monta uma linha de comando e despacha para a função correspondente em uma tabela estática de comandos.

## Teclas especiais

| Tecla | Efeito |
|---|---|
| `Enter` | Executa o comando digitado |
| `Backspace` | Apaga o último caractere |
| `Ctrl+L` | Limpa a tela e redesenha a linha atual |

## Sintaxe de comandos

```
<comando> [argumentos]
```

O shell separa o comando dos argumentos no primeiro espaço encontrado — não há suporte a aspas, múltiplos espaços ou *pipes*. Se o comando digitado não existir na tabela, o shell responde com `Unknown command: <nome>`.

Para a lista completa de comandos com sintaxe e exemplos, veja [Comandos do shell](../referencia/comandos-shell.md).

## Categorias de comandos

- **Sistema**: `help`, `clear`, `reboot`, `panic`
- **Processos**: `ps`, `stop`, `resume`
- **Memória**: `heap`, `alloc`, `free`, `defrag`, `peek`, `poke`, `memtest`
- **Arquivos**: `ls`, `touch`, `rm`, `cat`, `write`, `edit`

## O editor `edit`

`edit <arquivo>` abre um mini-editor de texto no estilo *nano*, implementado inteiramente dentro do comando de shell (sem processos separados — é só um laço de leitura de teclado dentro da própria tarefa do shell):

```
edit notas.txt
```

- O conteúdo do arquivo é lido para um buffer de 2 KB em RAM.
- **Digitação normal** insere caracteres no fim do buffer (não há um cursor navegável — é um editor de "acrescentar/apagar no final", como um `tail` editável).
- `Ctrl+W` salva o buffer de volta no arquivo (via a mesma syscall usada pelo comando `write`).
- `Ctrl+X` sai do editor e retorna ao prompt do shell.
- `Backspace` remove o último caractere; se o caractere removido for uma quebra de linha, a tela inteira é redesenhada (o editor não mantém um modelo de cursor 2D, então a forma mais simples e segura de refletir a mudança é redesenhar).

Enquanto o editor está ativo, a variável global `g_editor_mode` fica em `1`, o que pausa a atualização do relógio de uptime no canto da tela (feita pela `Task Monitor`) para não conflitar com o redesenho do editor.

## Exemplo de sessão

```
root@axon:~$ touch notas.txt
File created.
root@axon:~$ edit notas.txt
```
```
 AXON NANO    File: notas.txt
 [Ctrl+W] Save   [Ctrl+X] Quit
------------------------------------------------
```
```
root@axon:~$ cat notas.txt
Minha primeira anotação no AXON-OS!
root@axon:~$ ls

  FILES:
  -------------------
  notas.txt
```

## Acesso concorrente à UART

Como três tarefas escrevem na mesma UART (`Task Shell`, `Task Monitor` e `Task LEDs`), todas usam `safe_puts()`/`sys_puts()` protegidos por um mutex compartilhado (`uart_mutex`), evitando que a saída de uma tarefa se intercale no meio da saída de outra. Veja [Escalonador de tarefas](../arquitetura/escalonador.md#mutexes) para os detalhes do mecanismo de exclusão mútua.
