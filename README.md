
PandaOS : An x86 based Hobby OS
========

I am doing this for studying the OS internals.

Use the code as you like, with Original BSD license.

Any contribution is welcome :)

Dependencies:

```console
sudo apt-get install build-essential gcc nasm
```
Install [Cross GCC](https://wiki.osdev.org/GCC_Cross-Compiler).

Compile steps:

```console
step1: clone the code

step2: cd Panda;
      ./do.sh
      (will run bochs or qemu, you may need to do some modification for config Qemu/Bochs. 
      details refer to do.sh)
```

## Snapshot

![snapshot](https://github.com/chenyukang/Panda/blob/master/imgs/snapshot.png?raw=true)

## Credits

- Many of the code is from [fleurix](https://github.com/flaneur2020/fleurix).
- Other excellent resources are [osdev](https://wiki.osdev.org/Expanded_Main_Page).
