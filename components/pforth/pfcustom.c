/* @(#) pfcustom.c 98/01/26 1.3 */

#ifndef PF_USER_CUSTOM

/***************************************************************
** Call Custom Functions for pForth
**
** Create a file similar to this and compile it into pForth
** by setting -DPF_USER_CUSTOM="mycustom.c"
**
** Using this, you could, for example, call X11 from Forth.
** See "pf_cglue.c" for more information.
**
** Author: Phil Burk
** Copyright 1994 3DO, Phil Burk, Larry Polansky, David Rosenboom
**
** Permission to use, copy, modify, and/or distribute this
** software for any purpose with or without fee is hereby granted.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
** THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
** CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
** FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
** OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**
***************************************************************/

#include "pf_all.h"

#ifdef ESP_PLATFORM
/* defined by the IDF toolchain */

#include "driver/gpio.h"
#include "esp_timer.h"

#include "amy.h"

static cell_t CW_GpioSet(cell_t pin, cell_t level) {
    gpio_set_level((gpio_num_t)pin, (uint32_t)level);
    return 0;
}

static cell_t CW_Micros(void) { 
    return (cell_t)esp_timer_get_time(); 
}

// ( synth note vel127 -- )
void CW_AmyNoteOn(cell_t synth, cell_t note, cell_t vel) {
    amy_event e = amy_default_event();
    e.synth     = synth;
    e.midi_note = (float)note;
    e.velocity  = vel / 127.0f;
    amy_add_event(&e);
}

// ( synth note -- )
void CW_AmyNoteOff(cell_t synth, cell_t note) {
    amy_event e = amy_default_event();
    e.synth     = synth;
    e.midi_note = (float)note;
    e.velocity  = 0.0f;
    amy_add_event(&e);
}

// ( synth patch voices -- )
void CW_AmyInstrument(cell_t synth, cell_t patch, cell_t voices) {
    amy_event e = amy_default_event();
    e.synth        = synth;
    e.patch_number = patch;
    e.num_voices   = voices;
    amy_add_event(&e);
}

// ( level1000 liveness1000 damping1000 -- )
void CW_AmyReverb(cell_t level, cell_t liveness, cell_t damping) {
    amy_event e = amy_default_event();
    e.reverb_level    = level    / 1000.0f;
    e.reverb_liveness = liveness / 1000.0f;
    e.reverb_damping  = damping  / 1000.0f;
    amy_add_event(&e);
}

// ( osc wave freqhz vel127 -- )
void CW_AmyTone(cell_t osc, cell_t wave, cell_t freq, cell_t vel) {
    if (wave < SINE || wave > NOISE) return;   // basic waves only
    amy_event e = amy_default_event();
    e.osc           = osc;
    e.wave          = wave;
    e.freq_coefs[0] = (float)freq;
    e.velocity      = vel / 127.0f;
    amy_add_event(&e);
}

#else
/* host dictionary build — no IDF */
static cell_t CW_GpioSet(cell_t pin, cell_t level) {
    (void)pin;
    (void)level;
    return 0;
}

static cell_t CW_Micros(void) {
    return 0;
}

void CW_AmyNoteOn(cell_t synth, cell_t note, cell_t vel) {
    (void)synth;
    (void)note;
    (void)vel;
}

void CW_AmyNoteOff(cell_t synth, cell_t note) {
    (void)synth;
    (void)note;
}

void CW_AmyInstrument(cell_t synth, cell_t patch, cell_t voices) {
    (void)synth;
    (void)patch;
    (void)voices;
}

void CW_AmyReverb(cell_t level, cell_t liveness, cell_t damping) {
    (void)level;
    (void)liveness;
    (void)damping;
}

void CW_AmyTone(cell_t osc, cell_t wave, cell_t freq, cell_t vel) {
    (void)osc;
    (void)wave;
    (void)freq;
    (void)vel;
}

#endif



#ifdef PF_NO_GLOBAL_INIT
/******************
** If your loader does not support global initialization, then you
** must define PF_NO_GLOBAL_INIT and provide a function to fill
** the table. Some embedded system loaders require this!
** Do not change the name of LoadCustomFunctionTable()!
** It is called by the pForth kernel.
*/
#define NUM_CUSTOM_FUNCTIONS (7)
CFunc0 CustomFunctionTable[NUM_CUSTOM_FUNCTIONS];

Err LoadCustomFunctionTable(void) {
    CustomFunctionTable[0] = (CFunc0)CW_GpioSet;
    CustomFunctionTable[1] = (CFunc0)CW_Micros;
    CustomFunctionTable[2] = (CFunc0)CW_AmyNoteOn;
    CustomFunctionTable[3] = (CFunc0)CW_AmyNoteOff;
    CustomFunctionTable[4] = (CFunc0)CW_AmyInstrument;
    CustomFunctionTable[5] = (CFunc0)CW_AmyReverb;
    CustomFunctionTable[6] = (CFunc0)CW_AmyTone;
    return 0;
}

#else
/******************
** If your loader supports global initialization (most do.) then just
** create the table like this.
*/
CFunc0 CustomFunctionTable[] = {(CFunc0)CTest0, (CFunc0)CTest1};
#endif


#if (!defined(PF_NO_INIT)) && (!defined(PF_NO_SHELL))
Err CompileCustomFunctions(void) {
    Err err;
    int i = 0;
    err = CreateGlueToC("GPIO-SET",       i++, C_RETURNS_VOID,  2); if (err < 0) return err;
    err = CreateGlueToC("MICROS",         i++, C_RETURNS_VALUE, 0); if (err < 0) return err;
    err = CreateGlueToC("AMY-NOTE-ON",    i++, C_RETURNS_VOID,  3); if (err < 0) return err;
    err = CreateGlueToC("AMY-NOTE-OFF",   i++, C_RETURNS_VOID,  2); if (err < 0) return err;
    err = CreateGlueToC("AMY-INSTRUMENT", i++, C_RETURNS_VOID,  3); if (err < 0) return err;
    err = CreateGlueToC("AMY-REVERB",     i++, C_RETURNS_VOID,  3); if (err < 0) return err;
    err = CreateGlueToC("AMY-TONE",       i++, C_RETURNS_VOID,  4); if (err < 0) return err;

    return 0;
}
#else
Err CompileCustomFunctions(void) { return 0; }
#endif


#endif /* PF_USER_CUSTOM */
