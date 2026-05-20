# discordant

....?

## Adding custom words to the pForth dictionary

```
# host side
cd pforth
# put myforth.fth in fth/, and add `include myforth.fth` to the
# end of system.fth (or to a wrapper you compile instead)
cd platforms/unix
make clean
make WIDTHOPT=-m32 pfdicdat      # regenerates pfdicdat.h WITH your words
cp pfdicdat.h <your project>/components/pforth/csrc/
# then rebuild + flash firmware
```

The -m32 is **IMPORTANT** - dictionary must be 32 bit (for esp32 sizing), not 64 bit (host sizing).


