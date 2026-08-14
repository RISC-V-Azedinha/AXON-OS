# Instalação do ambiente

Para compilar e executar o AXON-OS você precisa de três coisas: um **toolchain RISC-V** (compilador cruzado), o **QEMU** (para rodar sem hardware físico) e, opcionalmente, **Python + pyserial** (para gravar o binário na FPGA).

## 1. Toolchain RISC-V (`riscv64-unknown-elf-gcc`)

O `makefile` do projeto usa o compilador `riscv64-unknown-elf-gcc` com as flags:

```
-march=rv32i_zicsr -mabi=ilp32 -mcmodel=medany -ffreestanding -O0 -g -Wall
```

Ou seja, mesmo sendo um toolchain "64-bit" no nome (`riscv64-unknown-elf-*`), ele é usado aqui para gerar código **32-bit** (`rv32i` com a extensão `Zicsr`, necessária para instruções de CSR como `csrw`/`csrr` usadas no trap handler).

=== "Ubuntu/Debian"

    ```bash
    sudo apt update
    sudo apt install gcc-riscv64-unknown-elf
    ```

    !!! note
        Dependendo da distro, o pacote pode se chamar `gcc-riscv64-unknown-elf` ou `binutils-riscv64-unknown-elf`. Verifique com `apt search riscv64-unknown-elf`.

=== "Compilando da fonte"

    Caso o pacote não esteja disponível no seu gerenciador de pacotes, compile o [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) oficial, habilitando o suporte a `elf` (baremetal, sem `linux`):

    ```bash
    git clone --recursive https://github.com/riscv-collab/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    ./configure --prefix=/opt/riscv
    make -j"$(nproc)"
    export PATH="/opt/riscv/bin:$PATH"
    ```

Confirme a instalação:

```bash
riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-objcopy --version
```

## 2. QEMU (`qemu-system-riscv32`)

O AXON-OS roda no modelo de máquina `virt` do QEMU para RISC-V 32-bit — é o alvo padrão de desenvolvimento, sem precisar de hardware.

=== "Ubuntu/Debian"

    ```bash
    sudo apt install qemu-system-misc
    ```

=== "Verificação"

    ```bash
    qemu-system-riscv32 --version
    ```

## 3. Python + pyserial (opcional, só para FPGA)

O script `upload.py`, usado para gravar o binário compilado na FPGA via serial, depende de [`pyserial`](https://pypi.org/project/pyserial/):

```bash
pip install pyserial
```

Esta etapa só é necessária se você for gravar o AXON-OS em uma placa FPGA real — para desenvolvimento via QEMU ela pode ser pulada.

## Próximo passo

Com o ambiente pronto, siga para [Build e execução](build-e-execucao.md).
