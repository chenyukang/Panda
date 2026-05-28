
PandaOS : An x86 based Hobby OS
========

I am doing this for studying the OS internals.

Use the code as you like, with Original BSD license.

Any contribution is welcome :)

Dependencies:

```console
sudo apt-get install build-essential gcc-multilib binutils nasm qemu-system-x86
```
You can also use an i386 ELF cross GCC if you already have one installed.

On macOS, install the build tools with Homebrew:

```console
brew install llvm lld binutils nasm qemu
```

Build and run:

```console
make all
make qemu
```

The build uses QEMU as the only supported emulator.

For CI-style validation:

```console
make smoke
make os-test
```

## Snapshot

![snapshot](https://github.com/chenyukang/Panda/blob/master/imgs/snapshot.png?raw=true)

## Credits

- Many of the code is from [fleurix](https://github.com/flaneur2020/fleurix).
- Other excellent resources are [osdev](https://wiki.osdev.org/Expanded_Main_Page).
