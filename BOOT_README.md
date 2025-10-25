Boot sector tutorial — first step

This folder contains a minimal boot sector `boot.asm` that prints a message using BIOS int 0x10.

Files created:
- `boot.asm` - 16-bit real-mode boot sector (512 bytes, ends with 0x55AA signature).
- `Makefile` - targets: `all` (assemble), `run` (launch in QEMU), `clean`.

How to build and run (on Linux with nasm and qemu installed):

```bash
# assemble to a 512-byte raw boot image
nasm -f bin -o boot.bin boot.asm

# run with qemu
qemu-system-x86_64 -drive format=raw,file=boot.bin
```

What `boot.asm` does (contract):
- Inputs: loaded by BIOS at address 0x7C00; no command-line args.
- Outputs: writes characters to the screen via BIOS int 0x10.
- Success criteria: BIOS transfers control, message is printed, CPU halts.
- Error modes: if assembled incorrectly (wrong size or missing signature), BIOS won't boot from the image.

Key implementation notes:
- `org 0x7C00` sets the code origin to where BIOS loads the sector.
- We set up a tiny stack at 0x0000:0x7C00.
- Text output uses BIOS teletype function (int 0x10, ah=0x0E).
- The sector is padded to 510 bytes and ends with the 2-byte boot signature 0x55AA.

Next steps suggestions:
1. Verify by assembling and running in QEMU.
2. Replace prints with a simple disk-reading routine to load a 2nd stage.
3. Learn about GDT/IDT and switch to protected mode or long mode.
4. Implement a tiny kernel entry in 32-bit protected mode.

If you'd like, I can assemble and run this now in a terminal and paste the QEMU output, or extend this bootloader to load a second-stage kernel. Which would you prefer?