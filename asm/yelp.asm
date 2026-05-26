; ============================================================
; yelp.asm
;
; Send a framed DISCORDANT wire message to I2C device 0x2A 
;
; Frame format on the wire (after START + addr/W):
; START 0x54(=0x2a<<1|W=0)  0x02  N  <N bytes of payload> STOP
;
; Returns Carry SET on success, CLEAR on any NACK.
; Interrupts should be disabled before calling.
; ============================================================

                    OUTPUT  yelp.com 
                    ORG     0x0100

                    INCLUDE "bios.inc"


I2C_TARGET          EQU    0x2a                    ; DISCORDANT's I2C address

                    DI 
                    CALL    amy_reset
                    LD      HL, message
                    CALL    C, amy_send
                    LD      HL, message2
                    CALL    C, amy_send
                    EI 

                    RET 



amy_reset           LD      HL, reset
                    CALL    amy_send
                    RET



                    ; send string pointed to by HL - show status
amy_send
                    CALL    i2c_send
                    PUSH    AF
                    LD      DE, ok_msg
                    JR      C, print_result
                    LD      DE, err_msg
print_result        LD      C, BDOS_PRINTSTRING 
                    CALL    BDOS
                    POP     AF
                    RET



                    ; send string pointed to by HL
i2c_send            PUSH    HL
                    LD      H, I2C_TARGET
                    LD      L, 0x02                 ; first payload byte sent right after addr
                    CALL    MBB_I2C_WR_ADDRESS      ; START + addr(W) + 0x02
                    JP      NC, _send_done

                    POP     HL
                    LD      A, (HL)                 ; A = N
                    INC     HL
                    LD      E, A                    ; E = remaining-byte counter
                                                    ; (i2c_write uses A,B,C,D — E and HL are safe)
                    CALL    MBB_I2C_WRITE           ; send N
                    JP      NC, _send_done

_send_loop          LD      A, (HL)
                    INC     HL
                    CALL    MBB_I2C_WRITE
                    JP      NC, _send_done
                    DEC     E
                    JR      NZ, _send_loop

                    CALL    MBB_I2C_STOP
                    SCF
                    RET

_send_done          CALL    MBB_I2C_STOP
                    AND     A                       ; CF = 0 on failure path
                    RET

reset               DP      "S16384Z"
message             DP     "v11w0f3,0,0,0.5A0,1,0,0.2Zv12w1l1a1,0,0,1f440,0,0,0,0.2,0.3A0,1,2000,0B0,1,0.5,0L11Z"
message2            DP     "v11l0H660,0,0Zv12l0H960,0,1Z"

ok_msg              DB      "I2C send OK\r\n$"
err_msg             DB      "I2C send FAILED\r\n$"

                    END
