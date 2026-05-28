# Modern macOS/QEMU Upgrade Notes

This note records the issues found while making Panda OS build and run on a modern macOS + Homebrew QEMU setup. The original project was written for an old Linux/i386-style toolchain and an older QEMU, so several failures were caused by environment drift plus a few latent kernel bugs.

## Build Script And Toolchain

Symptom:

- The original run script could not work on macOS.
- It referenced old absolute paths such as `/usr/local/gcc-4.5.2-for-linux32`.
- Modern macOS usually has Homebrew LLVM/Clang, `ld.lld`, Homebrew binutils, and Homebrew QEMU instead.

Root cause:

- The build assumed an old i386 Linux cross toolchain.
- Modern `ld.lld` does not emit the old `a.out-i386-linux` format used by the user programs.
- Clang may generate instructions, such as SSE, that are not safe before this OS has configured the relevant CPU state.

Fix:

- Make `do.sh` discover tools instead of using hardcoded paths.
- On macOS, use Homebrew LLVM Clang with `-target i386-unknown-elf`, `ld.lld`, Homebrew `objcopy`, and `qemu-system-i386`.
- Add `-fcommon -mno-sse -mno-sse2 -mno-mmx` to avoid modern compiler defaults that break this kernel.
- Link user programs as ELF with Panda's executable header emitted by the linker script, then use `objcopy -O binary` to produce the final user binaries.

## QEMU Boot Failure

Symptom:

- QEMU reported:

```text
Boot failed: not a bootable disk
```

Root cause:

- Old QEMU accepted looser floppy/hard-disk arguments such as `-fda a.img -hdb hd.img`.
- Modern QEMU is stricter about image format probing.
- The generated floppy image also needed to be padded to a standard 1.44 MB floppy size.

Fix:

- Start QEMU with explicit raw image configuration:

```bash
qemu-system-i386 \
  -boot a \
  -drive file=a.img,format=raw,if=floppy \
  -drive file=hd.img,format=raw,index=1,media=disk \
  -rtc base=localtime \
  -m 128
```

- Pad `a.img` to 1.44 MB after concatenating boot, setup, and kernel binaries.

## Boot Hangs Or Blinking Cursor

Symptom:

- QEMU printed early boot messages such as:

```text
Booting from Floppy...
Loading Panda OS
Setuping Panda OS
```

- Then the VM appeared to hang or blink forever.

Root cause:

- `boot/setup.s` tried to read the kernel with a large single BIOS disk read and then looped using `AL`, whose value is not reliable after `int 13h`.
- The setup code also copied `systemsize` dwords instead of `systemsize / 4` dwords.
- After the loader issue was fixed, Clang-generated SSE instructions could still cause early crashes unless SSE was disabled in compiler flags.

Fix:

- Read the kernel sector-by-sector with an explicit CHS loop.
- Reset the disk and retry on BIOS read errors.
- Use `rep movsd` with a dword count of `systemsize / 4`.
- Compile kernel and user code with SSE/MMX disabled.

## File Creation Looked Broken

Symptom:

- `touch now.scm` appeared to run, but `ls` still did not show the new file.
- Some filesystem updates were visible only in memory or not durable inside the disk image.

Root cause:

- Several filesystem paths changed in-memory buffers but did not write the dirty buffer back to disk.
- `writei()` calculated the writable bytes in a block with `BSIZE - off / BSIZE` instead of `BSIZE - off % BSIZE`.
- Kernel and user `strcpy()` did not write the trailing `\0`, which made path/string handling fragile.

Fix:

- Add `buf_write()` after modifying allocation bitmaps, inodes, indirect blocks, and file data blocks.
- Fix `writei()` to use the block offset modulo.
- Fix kernel/user `strcpy()` and kernel `strcat()` to terminate strings.

## External Commands Produced No Output

Symptom:

- Builtin shell commands worked:

```text
[PandaOS]$ pwd
current path: /home
```

- External commands looked like no-ops:

```text
[PandaOS]$ ls
[PandaOS]$ touch now.scm
[PandaOS]$ scm
```

- Even a nonexistent command did not print `no such command`.

Root cause:

- The shell forks before running external commands.
- The child process did not reliably see `fork()` return `0`, so it skipped the `exec()` path.
- The real bug was in copy-on-write page table cloning:
  - `copy_pgd()` incremented the refcount for `fpte->pt_base` instead of `fpte[k].pt_base`.
  - The parent page table's write permissions were cleared, but the parent's TLB was not flushed.
- As a result, parent and child could keep writing the same user stack page. The parent overwrote the child-visible stack value for `fork()`, so the child behaved like the parent and never entered `exec()`.

Fix:

- Use `fpte[k].pt_base` when incrementing copied page refcounts.
- Flush the parent page directory after clearing write permissions in `copy_pgd()`.
- Add a terminating `NULL` entry to the user `argv` array built by `exec()`.

## Exit And Wait Behavior

Symptom:

- Failed or completed child commands could behave inconsistently after returning from user code.

Root cause:

- `do_exit()` marked the task as `ZOMBIE`, woke the parent, and then returned even though an exiting task must never resume user execution.

Fix:

- After wakeup, call `sched()` and then loop forever in `do_exit()`.

## Verification

A clean run after the fixes should show external commands working:

```text
[PandaOS]$ ls
.. . README.md cat cat.elf cowsay cowsay.elf date date.elf init init.elf ls ...
[PandaOS]$ touch now.scm
content:hello
[PandaOS]$ ls
.. . README.md cat cat.elf ... vi vi.elf now.scm
[PandaOS]$ scm
no such command: scm
```

Use:

```bash
./do.sh -qemu
```

If an old QEMU window is already running, close it first. It will keep using the old in-memory VM state and will not pick up newly generated `a.img` and `hd.img`.
