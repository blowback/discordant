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

\ --- aliases / vocabulary ---
: note-on    ( synth note vel -- )      amy-note-on ;
: note-off   ( synth note -- )          amy-note-off ;
: instrument ( synth patch voices -- )  amy-instrument ;
: reverb     ( lvl live damp -- )       amy-reverb ;
: tone       ( osc wave freq vel -- )   amy-tone ;

\ --- wave constants ---
0 constant sine    1 constant pulse   2 constant saw-down
3 constant saw-up  4 constant triangle 5 constant noise

\ --- musical conveniences ---
: juno  ( synth -- )  6 4 instrument ;
: c4 60 ;  : d4 62 ;  : e4 64 ;  : f4 65 ;
: g4 67 ;  : a4 69 ;  : b4 71 ;

: cmaj  ( synth -- )
   dup c4 100 note-on
   dup e4 100 note-on
       g4 100 note-on ;
