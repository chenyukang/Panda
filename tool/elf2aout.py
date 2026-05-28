#!/usr/bin/env python3
import struct
import sys

NMAGIC = 0x6400CC
SHT_NOBITS = 8
SHF_ALLOC = 0x2


def die(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


def read_elf32(path):
    data = open(path, "rb").read()
    if data[:4] != b"\x7fELF":
        die("%s is not an ELF file" % path)
    if data[4] != 1 or data[5] != 1:
        die("%s is not a little-endian ELF32 file" % path)

    header = struct.unpack_from("<16sHHIIIIIHHHHHH", data, 0)
    entry = header[4]
    shoff = header[6]
    shentsize = header[11]
    shnum = header[12]
    shstrndx = header[13]

    sections = []
    for i in range(shnum):
        off = shoff + i * shentsize
        sh = struct.unpack_from("<IIIIIIIIII", data, off)
        sections.append(
            {
                "name_off": sh[0],
                "type": sh[1],
                "flags": sh[2],
                "addr": sh[3],
                "offset": sh[4],
                "size": sh[5],
            }
        )

    shstr = sections[shstrndx]
    names = data[shstr["offset"] : shstr["offset"] + shstr["size"]]
    for sec in sections:
        start = sec["name_off"]
        end = names.find(b"\0", start)
        sec["name"] = names[start:end].decode("ascii")

    return data, entry, sections


def section_bytes(elf, sections, start, end):
    out = bytearray(end - start)
    for sec in sections:
        if (sec["flags"] & SHF_ALLOC) == 0 or sec["type"] == SHT_NOBITS:
            continue
        sec_start = sec["addr"]
        sec_end = sec_start + sec["size"]
        if sec_end <= start or sec_start >= end:
            continue
        copy_start = max(sec_start, start)
        copy_end = min(sec_end, end)
        src = sec["offset"] + (copy_start - sec_start)
        dst = copy_start - start
        out[dst : dst + (copy_end - copy_start)] = elf[src : src + (copy_end - copy_start)]
    return bytes(out)


def main():
    if len(sys.argv) != 3:
        die("usage: elf2aout.py input.elf output")

    elf, entry, sections = read_elf32(sys.argv[1])
    alloc = [s for s in sections if s["flags"] & SHF_ALLOC]
    text = next((s for s in alloc if s["name"] == ".text"), None)
    data = next((s for s in alloc if s["name"] == ".data"), None)
    bss = next((s for s in alloc if s["name"] == ".bss"), None)
    if text is None:
        die("missing .text section")

    text_base = text["addr"]
    data_base = data["addr"] if data is not None else (bss["addr"] if bss is not None else text_base + text["size"])
    bss_base = bss["addr"] if bss is not None else (data_base + (data["size"] if data is not None else 0))
    text_size = data_base - text_base
    data_size = bss_base - data_base
    bss_size = bss["size"] if bss is not None else 0

    header = struct.pack("<IIIIIIII", NMAGIC, text_size, data_size, bss_size, 0, entry, 0, 0)
    body = section_bytes(elf, alloc, text_base, data_base)
    body += section_bytes(elf, alloc, data_base, bss_base)
    open(sys.argv[2], "wb").write(header + body)


if __name__ == "__main__":
    main()
