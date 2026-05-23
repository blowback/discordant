
set shell := ["bash", "-uc"]

FORTH := "forth/myforth.fth"
CFORTH := "components/pforth/pfcustom.c"
DICT := "pfdicdat.h"

FORTH_ROOT := "../pforth"
FORTH_BUILD := FORTH_ROOT + "/platforms/unix"

words:
    cp {{FORTH}} {{FORTH_ROOT}}/fth/
    cp {{CFORTH}} {{FORTH_ROOT}}/csrc/
    cd {{FORTH_BUILD}} && make clean && make WIDTHOPT=-m32 pfdicdat
    cp {{FORTH_BUILD}}/{{DICT}} components/pforth/csrc/ 


