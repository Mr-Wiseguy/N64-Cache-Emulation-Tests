.section .data 
.byte  0x80, 0x37, 0x12, 0x40 # PI BSD Domain 1 register
.word  0x0000000F             # Clock Rate
.word  EntryPoint             # Entry point
.word  0x0000144C             # OS 2.0L
.word  0xFFFFFFFF             # Checksum
.word  0xFFFFFFFF             # Checksum
.word  0x00000000             # 
.word  0x00000000             # 
.ascii "CACHE TESTING       " # ROM name: 20 bytes
.word  0x00000000             # 
.word  0x0000004E             # N
.ascii "ED"                   # Cartridge ID
.ascii "\x00"                 # Country
.byte  0x12                   # Revision
