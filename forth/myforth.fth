HEX
: gpio-out    60004004 ;
: gpio-w1ts   60004008 ;
: gpio-w1tc   6000400C ;
: pin-mask    1 SWAP LSHIFT ;
: pin-set     pin-mask gpio-w1ts ! ;
: pin-clr     pin-mask gpio-w1tc ! ;
DECIMAL

\ runs automatically at every boot, before the REPL:
: AUTO.INIT
   HEX                  \ start sessions in hex — handy for register work
;
