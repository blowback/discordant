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

## Adding custom words that call out to C 


1. plumb new words into pfcustom.c 
2. copy it up to the host build, and rebuild the static dictionary (just bindings for your new words, no linkage)
3. copy dicdat.h back to the esp32 build
4. at runtime, LoadCustomFunctionTable() patches the words with their (esp32) implementations.

```
cp components/pforth/pfcustom.c ../pforth/csrc
cd ../pforth/platforms/unix
make clean && make WIDTHOPT=-m32 pfdicdat
cp pfdicdat.h ../../../discordant/components/pforth/csrc
cd ../../../discordant
idf.py -p /dev/ttyACM1 build flash monitor
```

## Applying patches to AMY vendor branch 

Amy doesn't like ESP-IDF v6, some patching is required. Here's the general MO:

- `cd components/amy` edit files from here e.g. `src/i2s.c`
- `git diff src/i2s.c > ../patches/0001-i2s-task-signature.patch`
- `git checkout .` - reset to orig
- patches are applied by `components/amy/CMakeLists.txt` 
