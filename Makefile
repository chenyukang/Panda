UNAME_S := $(shell uname -s)

BOOT := boot
KERNEL := core
INC := inc
TOOL := tool
OBJDIR := objs
USEROBJDIR := $(OBJDIR)/usr
USERLIBOBJDIR := $(USEROBJDIR)/lib

BREW := $(shell command -v brew 2>/dev/null)
LLVM_PREFIX := $(shell if [ -n "$(BREW)" ]; then "$(BREW)" --prefix llvm 2>/dev/null; fi)
LLD_PREFIX := $(shell if [ -n "$(BREW)" ]; then "$(BREW)" --prefix lld 2>/dev/null; fi)
BINUTILS_PREFIX := $(shell if [ -n "$(BREW)" ]; then "$(BREW)" --prefix binutils 2>/dev/null; fi)

NASM ?= nasm
HOSTCC ?= cc

ifeq ($(UNAME_S),Darwin)
DARWIN_CC := $(shell command -v i686-elf-gcc 2>/dev/null || command -v i386-elf-gcc 2>/dev/null || command -v i586-pc-linux-gcc 2>/dev/null)
ifeq ($(DARWIN_CC),)
ifneq ($(LLVM_PREFIX),)
DARWIN_CC := $(LLVM_PREFIX)/bin/clang -target i386-unknown-elf
else
DARWIN_CC := clang -target i386-unknown-elf
endif
endif
TARGET_CC ?= $(DARWIN_CC)
ifneq ($(wildcard $(LLVM_PREFIX)/bin/ld.lld),)
TARGET_LD ?= $(LLVM_PREFIX)/bin/ld.lld
else ifneq ($(LLD_PREFIX),)
TARGET_LD ?= $(LLD_PREFIX)/bin/ld.lld
else
TARGET_LD ?= ld.lld
endif
ifneq ($(BINUTILS_PREFIX),)
OBJCOPY ?= $(BINUTILS_PREFIX)/bin/objcopy
else
OBJCOPY ?= objcopy
endif
else
LINUX_CC := $(shell command -v i686-elf-gcc 2>/dev/null || command -v i386-elf-gcc 2>/dev/null || command -v i586-pc-linux-gcc 2>/dev/null || printf gcc)
LINUX_LD := $(shell command -v i686-elf-ld 2>/dev/null || command -v i386-elf-ld 2>/dev/null || printf ld)
LINUX_OBJCOPY := $(shell command -v i686-elf-objcopy 2>/dev/null || command -v i386-elf-objcopy 2>/dev/null || printf objcopy)
TARGET_CC ?= $(LINUX_CC)
TARGET_LD ?= $(LINUX_LD)
OBJCOPY ?= $(LINUX_OBJCOPY)
endif

QEMU ?= $(shell command -v qemu-system-i386 2>/dev/null || command -v qemu-system-x86_64 2>/dev/null || printf qemu-system-i386)
QEMU_ARGS := -boot a -drive file=a.img,format=raw,if=floppy -drive file=hd.img,format=raw,index=1,media=disk -rtc base=localtime -m 128
QEMU_COCOA_DISPLAY_ARGS ?= -display cocoa,zoom-to-fit=on
QEMU_CURSES_DISPLAY_ARGS ?= -display curses
QEMU_DISPLAY_ARGS ?= $(QEMU_COCOA_DISPLAY_ARGS)
QEMU_EXTRA_ARGS ?=
SMOKE_SECONDS ?= 5

TARGET_CC_CMD := $(firstword $(TARGET_CC))
TARGET_LD_CMD := $(firstword $(TARGET_LD))
OBJCOPY_CMD := $(firstword $(OBJCOPY))
QEMU_CMD := $(firstword $(QEMU))

NASMFLAGS := -f elf
COMMON_CFLAGS := -Wall -m32 -O -nostdinc -fno-builtin -fno-stack-protector -fcommon -mno-sse -mno-sse2 -mno-mmx -Wno-implicit-function-declaration -finline-functions -I./$(INC)/
KERNEL_CFLAGS := $(COMMON_CFLAGS)
USER_CFLAGS := -I./$(INC) -m32 -fno-builtin -Wno-implicit-function-declaration -fno-stack-protector -fcommon -mno-sse -mno-sse2 -mno-mmx -nostdinc
USER_LIB_CFLAGS := $(USER_CFLAGS) -DUSR

BOOT_SRCS := $(wildcard $(BOOT)/*.s)
BOOT_OBJS := $(patsubst $(BOOT)/%.s,$(OBJDIR)/%.O,$(BOOT_SRCS))

KERNEL_ASM_SRCS := $(wildcard $(KERNEL)/*.s)
KERNEL_ASM_OBJS := $(patsubst $(KERNEL)/%.s,$(OBJDIR)/%.o,$(KERNEL_ASM_SRCS))
KERNEL_C_SRCS := $(wildcard $(KERNEL)/*.c)
KERNEL_C_OBJS := $(patsubst $(KERNEL)/%.c,$(OBJDIR)/%.o,$(KERNEL_C_SRCS))
KERNEL_OBJS := $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS)

USER_SRCS := $(wildcard usr/*.c)
USER_NAMES := $(basename $(notdir $(USER_SRCS)))
USER_OBJS := $(addprefix $(USEROBJDIR)/,$(addsuffix .o,$(USER_NAMES)))
USER_ELFS := $(addprefix $(USEROBJDIR)/,$(addsuffix .elf,$(USER_NAMES)))
USER_BINS := $(addprefix $(USEROBJDIR)/,$(USER_NAMES))

USER_LIB_C_SRCS := $(wildcard usr/lib/*.c)
USER_LIB_OBJS := $(patsubst usr/lib/%.c,$(USERLIBOBJDIR)/%.o,$(USER_LIB_C_SRCS))
USER_ENTRY := $(USERLIBOBJDIR)/entry.O
USER_LIB_ALL := $(USER_ENTRY) $(USER_LIB_OBJS)

FS_PAYLOADS := $(USER_BINS) $(USEROBJDIR)/README.md $(USEROBJDIR)/prog.scm

.PHONY: all images compile qemu qemu-cocoa qemu-curses run smoke clean line help tools check-build-tools check-qemu
.NOTPARALLEL:

all: images

images: a.img hd.img

compile: a.img

tools:
	@echo "HOSTCC=$(HOSTCC)"
	@echo "NASM=$(NASM)"
	@echo "TARGET_CC=$(TARGET_CC)"
	@echo "TARGET_LD=$(TARGET_LD)"
	@echo "OBJCOPY=$(OBJCOPY)"
	@echo "QEMU=$(QEMU)"

check-build-tools:
	@set -e; \
	missing=0; \
	check() { \
		if ! command -v "$$1" >/dev/null 2>&1; then \
			echo "error: missing $$2: $$1"; \
			missing=1; \
		fi; \
	}; \
	check "$(HOSTCC)" "host C compiler"; \
	check "$(NASM)" "NASM assembler"; \
	check "$(TARGET_CC_CMD)" "i386 target C compiler"; \
	check "$(TARGET_LD_CMD)" "i386 linker"; \
	check "$(OBJCOPY_CMD)" "objcopy"; \
	if [ "$$missing" -ne 0 ]; then \
		echo ""; \
		echo "Install hints:"; \
		echo "  macOS:        brew install llvm lld binutils nasm qemu"; \
		echo "  Ubuntu/Debian: sudo apt-get update && sudo apt-get install -y build-essential gcc-multilib binutils nasm qemu-system-x86"; \
		echo ""; \
		echo "You can also override tools, for example:"; \
		echo "  make TARGET_CC='clang -target i386-unknown-elf' TARGET_LD=ld.lld OBJCOPY=objcopy"; \
		exit 1; \
	fi

check-qemu:
	@set -e; \
	if ! command -v "$(QEMU_CMD)" >/dev/null 2>&1; then \
		echo "error: missing QEMU executable: $(QEMU_CMD)"; \
		echo ""; \
		echo "Install hints:"; \
		echo "  macOS:        brew install qemu"; \
		echo "  Ubuntu/Debian: sudo apt-get update && sudo apt-get install -y qemu-system-x86"; \
		echo ""; \
		echo "You can override it with: make QEMU=/path/to/qemu-system-i386 qemu"; \
		exit 1; \
	fi

$(OBJDIR) $(USEROBJDIR) $(USERLIBOBJDIR):
	mkdir -p $@

$(OBJDIR)/%.O: $(BOOT)/%.s | $(OBJDIR) check-build-tools
	$(NASM) $(NASMFLAGS) $< -o $@

$(OBJDIR)/%.o: $(KERNEL)/%.s | $(OBJDIR) check-build-tools
	$(NASM) $(NASMFLAGS) $< -o $@

$(OBJDIR)/%.o: $(KERNEL)/%.c | $(OBJDIR) check-build-tools
	$(TARGET_CC) $(KERNEL_CFLAGS) -c $< -o $@

$(USEROBJDIR)/%.o: usr/%.c | $(USEROBJDIR) check-build-tools
	$(TARGET_CC) $(USER_CFLAGS) -c $< -o $@

$(USERLIBOBJDIR)/%.o: usr/lib/%.c | $(USERLIBOBJDIR) check-build-tools
	$(TARGET_CC) $(USER_LIB_CFLAGS) -c $< -o $@

$(USER_ENTRY): usr/lib/entry.s | $(USERLIBOBJDIR) check-build-tools
	$(NASM) $(NASMFLAGS) $< -o $@

$(USEROBJDIR)/%.elf: $(USEROBJDIR)/%.o $(USER_LIB_ALL) $(TOOL)/user.ld | $(USEROBJDIR) check-build-tools
	$(TARGET_LD) $(USER_ENTRY) $(USER_LIB_OBJS) $< -m elf_i386 -e _start -o $@ -T $(TOOL)/user.ld

$(USEROBJDIR)/%: $(USEROBJDIR)/%.elf | check-build-tools
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/boot.elf: $(OBJDIR)/boot.O $(TOOL)/boot.ld | check-build-tools
	$(TARGET_LD) -m elf_i386 $< -o $@ -T $(TOOL)/boot.ld

$(OBJDIR)/boot.bin: $(OBJDIR)/boot.elf | check-build-tools
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/setup.elf: $(OBJDIR)/setup.O $(TOOL)/setup.ld | check-build-tools
	$(TARGET_LD) -m elf_i386 $< -o $@ -T $(TOOL)/setup.ld

$(OBJDIR)/setup.bin: $(OBJDIR)/setup.elf | check-build-tools
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/kernel.elf: $(OBJDIR)/head.O $(KERNEL_OBJS) $(TOOL)/kernel.ld | check-build-tools
	$(TARGET_LD) -m elf_i386 -T $(TOOL)/kernel.ld -o $@ $(OBJDIR)/head.O $(KERNEL_OBJS)

$(OBJDIR)/kernel.bin: $(OBJDIR)/kernel.elf | check-build-tools
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $< $@

a.img: $(OBJDIR)/boot.bin $(OBJDIR)/setup.bin $(OBJDIR)/kernel.bin
	cat $^ > $@
	@min_img_size=$$((1440 * 1024)); \
	img_size=$$(wc -c < $@); \
	if [ "$$img_size" -lt "$$min_img_size" ]; then \
		dd if=/dev/zero bs=1 count=0 seek=$$min_img_size of=$@ 2>/dev/null; \
	fi
	@ls -lh $(OBJDIR)/kernel.bin

$(OBJDIR)/mkfs.exe: $(TOOL)/mkfs.c $(INC)/fs.h $(INC)/buf.h $(INC)/blk.h $(INC)/stat.h $(INC)/dirent.h | $(OBJDIR) check-build-tools
	$(HOSTCC) $< -o $@

$(USEROBJDIR)/README.md: README.md | $(USEROBJDIR)
	cp $< $@

$(USEROBJDIR)/prog.scm: usr/prog.scm | $(USEROBJDIR)
	cp $< $@

$(USEROBJDIR)/hd.img: $(OBJDIR)/mkfs.exe $(FS_PAYLOADS)
	cd $(USEROBJDIR) && ../mkfs.exe hd.img $(notdir $(FS_PAYLOADS))

hd.img: $(USEROBJDIR)/hd.img
	cp $< $@

qemu run: images check-qemu
	$(QEMU) $(QEMU_ARGS) $(QEMU_DISPLAY_ARGS) $(QEMU_EXTRA_ARGS)

qemu-cocoa: QEMU_DISPLAY_ARGS := $(QEMU_COCOA_DISPLAY_ARGS)
qemu-cocoa: qemu

qemu-curses: QEMU_DISPLAY_ARGS := $(QEMU_CURSES_DISPLAY_ARGS)
qemu-curses: qemu

smoke: images check-qemu
	@set -e; \
	echo "Starting QEMU smoke test for $(SMOKE_SECONDS)s..."; \
	tmp_a=$$(mktemp "$${TMPDIR:-/tmp}/panda-a.XXXXXX.img"); \
	tmp_h=$$(mktemp "$${TMPDIR:-/tmp}/panda-hd.XXXXXX.img"); \
	cp a.img "$$tmp_a"; \
	cp hd.img "$$tmp_h"; \
	log="$(OBJDIR)/qemu-smoke.log"; \
	rm -f "$$log"; \
	trap 'rm -f "$$tmp_a" "$$tmp_h"' EXIT INT TERM; \
	$(QEMU) -boot a -drive file="$$tmp_a",format=raw,if=floppy -drive file="$$tmp_h",format=raw,index=1,media=disk -rtc base=localtime -m 128 -display none -monitor none -serial none -no-reboot $(QEMU_EXTRA_ARGS) >"$$log" 2>&1 & \
	pid=$$!; \
	sleep "$(SMOKE_SECONDS)"; \
	if kill -0 "$$pid" >/dev/null 2>&1; then \
		kill "$$pid" >/dev/null 2>&1 || true; \
		wait "$$pid" >/dev/null 2>&1 || true; \
		echo "QEMU smoke test passed"; \
	else \
		wait "$$pid" || true; \
		cat "$$log"; \
		echo "error: QEMU exited before smoke test completed"; \
		exit 1; \
	fi

clean:
	rm -rf a.img hd.img bochsout.txt $(OBJDIR) $(TOOL)/mkfs.exe

line:
	find ./ -name '*.[chs]' -or -name '*.asm' | xargs cat | wc -l

help:
	@echo "Targets:"
	@echo "  make all      Build a.img and hd.img"
	@echo "  make qemu     Build and run with QEMU Cocoa display"
	@echo "  make qemu-cocoa"
	@echo "                Build and run with QEMU Cocoa display"
	@echo "  make qemu-curses"
	@echo "                Build and run with QEMU curses display"
	@echo "  make smoke    Build and run a short headless QEMU smoke test"
	@echo "  make clean    Remove generated files"
	@echo "  make tools    Print resolved tool paths"
	@echo ""
	@echo "Common overrides:"
	@echo "  TARGET_CC='clang -target i386-unknown-elf'"
	@echo "  TARGET_LD=ld.lld"
	@echo "  OBJCOPY=objcopy"
	@echo "  QEMU=qemu-system-i386"
