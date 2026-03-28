;-------------------------------------

.area _CODE

;-------------------------------------
; GLOBALS

.globl _loop_test, _drum_loop

;-------------------------------------
; track data

_loop_test::    ;simply loop test
    .db 0xD3, 0xF8, 0x93, 0xE4
loop_start:
    .db 0x01, 0x11, 0x21, 0x31
    .db 0x41, 0x51, 0x61, 0x71
    .db 0xFE
    .dw loop_start 
    .db 0x02
    .db 0xFF

;-------------------------------------

_drum_loop::    ;only ch4
    .db 0xD4, 0x00, 0x00, 0x1F
    .db 0x2F, 0x0F
    .db 0xFE
    .dw _drum_loop
    .db 0xFF        