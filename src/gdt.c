#include "header/cpu/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }, // null descriptor
        {
            0xFFFF, 0x0000, 0x00, 0x0A, 0x01, 0x00, 0x01, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x01
            }, // kernel code segment
        {
            0xFFFF, 0x0000, 0x00, 0x02, 0x01, 0x00, 0x01, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x01
            }  // kernel data segment
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table.table) - 1,
    .address = &global_descriptor_table
};
