
OUTPUT_ARCH(mips)

#define START_SEGMENT(segmentName, ramAddr) \
    _##segmentName##SegmentStart = ADDR(.segmentName); \
    _##segmentName##SegmentRomStart = romPos; \
    .segmentName ramAddr : AT(romPos) SUBALIGN(16)

#define START_NOLOAD(segmentName, ramAddr) \
    _##segmentName##SegmentBssStart = ADDR(.segmentName.noload); \
    .segmentName.noload ramAddr (NOLOAD) : SUBALIGN(16)

#define START_NOLOAD_AFTER(segmentName, previousSegment) \
    _##segmentName##SegmentBssStart = ADDR(.segmentName.noload); \
    .segmentName.noload ADDR(.previousSegment) + SIZEOF(.previousSegment) (NOLOAD) : SUBALIGN(16)

#define END_SEGMENT(segmentName) \
    _##segmentName##SegmentEnd = ADDR(.segmentName) + SIZEOF(.segmentName); \
    romPos += SIZEOF(.segmentName); \
    _##segmentName##SegmentSize = SIZEOF(.segmentName); \
    _##segmentName##SegmentRomEnd = romPos

#define END_NOLOAD(segmentName) \
    _##segmentName##SegmentBssEnd = ADDR(.segmentName.noload) + SIZEOF(.segmentName.noload); \
    _##segmentName##SegmentBssSize = SIZEOF(.segmentName.noload)

SECTIONS
{
    romPos = 0;

    START_SEGMENT(header, 0xA4000000)
    {
        KEEP(BUILD_ROOT/src/header/header.o(.*data));
        KEEP(BUILD_ROOT/boot/boot.6102.o);
    }
    END_SEGMENT(header);

    romPos = 0x1000;

    START_SEGMENT(main, 0x80000400)
    {
        KEEP(BUILD_ROOT/boot/entry.o(.text*));
        KEEP(BUILD_ROOT/codesegment.o(.text.init));
        BUILD_ROOT/codesegment.o(.text*);
        
        PR/rspboot.o(.text*);
        PR/gspF3DEX2.fifo.o(.text*);

        . = ALIGN(64);
        BUILD_ROOT/codesegment.o(.data*);

        PR/rspboot.o(.*data*);
        PR/gspF3DEX2.fifo.o(.*data*);

        . = ALIGN(64);
        BUILD_ROOT/codesegment.o(.rodata*);
        . = ALIGN(16);
    }
    END_SEGMENT(main);

    START_NOLOAD_AFTER(main, main)
    {
        . = ALIGN(64);
        BUILD_ROOT/codesegment.o(.bss*);
        BUILD_ROOT/codesegment.o(COMMON*);
        BUILD_ROOT/codesegment.o(.scommon*);
        . = ALIGN(16);
    }
    END_NOLOAD(main);

    g_frameBuffers = 0x80300000;
    g_depthBuffer = 0x80200000;

    START_SEGMENT(cachefuncDMA, 0x0)
    {
        BYTE(0x03);
        BYTE(0xE0);
        BYTE(0x00);
        BYTE(0x08);
        BYTE(0x24);
        BYTE(0x02);
        BYTE(0x00);
        BYTE(0x02);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
        BYTE(0x00);
    }
    END_SEGMENT(cachefuncDMA);

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF Extension.  */
  .debug_macro    0 : { *(.debug_macro) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }

    /DISCARD/ :
    {
        *(*);
    }
}