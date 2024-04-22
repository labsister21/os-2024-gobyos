ENTRY(_start)
OUTPUT_FORMAT("binary")

SECTIONS {
    . = 0x00000000;    /* Assuming OS will load this program at virtual address 0x00000000 */
    
    .text ALIGN(4):
    {
        crt0.o(.text)  /* Put user-entry at front of executable */
        *(.text)
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .string ALIGN(4):
    {
        string.o(.text)
        string.o(.data)
        string.o(.rodata*)
    }

    .stdmem.c ALIGN(4):
    {
        stdmem.o(.text)
        stdmem.o(.data)
        stdmem.o(.rodata*)
    }

    _linker_user_program_end = .;
    ASSERT ((_linker_user_program_end <= 1 * 1024 * 1024), "Error: User program linking result is more than 1 MiB")
}