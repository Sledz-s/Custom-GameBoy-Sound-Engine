#ifndef SOUND_H
#define SOUND_H

#include <gb/gb.h>

#include "wave.h"

//-------------------------------------
// notes

#define NOTE_C    0x00
#define NOTE_CIS  0x10
#define NOTE_D    0x20
#define NOTE_DIS  0x30
#define NOTE_E    0x40
#define NOTE_F    0x50
#define NOTE_FIS  0x60
#define NOTE_G    0x70
#define NOTE_GIS  0x80
#define NOTE_A    0x90
#define NOTE_AIS  0xA0
#define NOTE_B    0xB0

#define NOTE(note, len)     ((note & 0xF) | (len & 0x0F))

#define INSTR_LEN(len)      ((len & 0b00111111) << 2)

#define ENV_MODE(mode)      (mode << 3)
#define ENV_VOL(vol)        (vol << 4)
#define ENV_SWEEP(sweep)    (sweep & 0x07)

#define DUTY12    0x00
#define DUTY25    0x01
#define DUTY50    0x02
#define DUTY75    0x03

//-------------------------------------
// commands

#define OCTAVE     0xE
#define SPECIAL    0xF
#define TEMPO      0xD

// special effects
#define OPEN_LOOP  0xE
#define CH_PAN     0x0

#define MAKE_NOTE(note, len)   (((note) << 4) | ((len) & 0x0F))
#define STOP         0xFF
#define CMD_OCT(n)   (0xE0 | (n))
#define CMD_DUTY(d)  (0xF0 | (d))

//-------------------------------------
//  special

#define GET_LOW_NIBBLE(b)  ((b) & 0x0F)
#define GET_HIGH_NIBBLE(b) ((b) >> 4)

//-------------------------------------

#define CH_RESTART      0x80  // Bit 7: Trigger/Restart
#define CH_LOOP_OFF     0x40  // Bit 6: Counter/Consecutive selection
#define CH_ON           0x80  // Bit 7: Sound On (for NR30/NR52)
#define CH_OFF          0x00

#define ENV_UP          0x08  // Bit 3: 1 = Increase
#define ENV_DOWN        0x00

#define AUDIO_ON        0x80
#define AUDIO_OFF       0x00

#define MUTE            0x00

#define AUDIO_PAN_ALL   0xFF
#define AUDIO_VOL_MAX   0x77

#define NOTE_PAUSE      0x0C
#define NOTE_MAX        0x0D

#define DRUM_BD_ENV     0xA1
#define DRUM_BD_FREQ    0x50
#define DRUM_SD_ENV     0x92
#define DRUM_SD_FREQ    0x80
#define DRUM_CH_ENV     0x51
#define DRUM_CH_FREQ    0x11

//-------------------------------------
// channel structs

struct ch1_reg_t {
    uint8_t sweepShifts;
    uint8_t sweepMode;
    uint8_t sweepTime;

    uint8_t soundLength;
    uint8_t patternDuty;

    uint8_t envNbSweep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch2_reg_t {
    uint8_t soundLength;
    uint8_t patternDuty;

    uint8_t envNbSweep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch3_reg_t {
    uint8_t on_Off;

    uint8_t soundLength;

    uint8_t selOutputLevel;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch4_reg_t {

    uint8_t soundLength;

    uint8_t envNbStep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t polyCounterDiv;
    uint8_t polyCounterStep;
    uint8_t polyCounterFreq;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct channel_data_t {
    uint8_t* ptr;           // 2 bytes
    uint8_t curr_octv;      //unused for CH4

    uint8_t tick_counter;
    uint8_t ticks_per_row;
    uint8_t loop_iterator;
    uint8_t padding[2];
};

//-------------------------------------

extern struct ch1_reg_t ch1_reg;
extern struct ch2_reg_t ch2_reg;
extern struct ch3_reg_t ch3_reg;
extern struct ch4_reg_t ch4_reg;

//-------------------------------------
//  channel 1 macros

#define UPDATE_NR11	(NR11_REG = ch1_reg.soundLength | (ch1_reg.patternDuty << 6))
#define UPDATE_NR12	(NR12_REG = ch1_reg.envNbSweep | (ch1_reg.envMode << 3) | (ch1_reg.envInitialValue << 4))

inline void update_channel1(uint8_t freqLow, uint8_t freqHigh) {
    UPDATE_NR11;
    NR13_REG = freqLow;
    UPDATE_NR12;
    NR14_REG = ch1_reg.restart | freqHigh;
}

//-------------------------------------
//  channel 2 macros


#define UPDATE_NR21	(NR21_REG = ch2_reg.soundLength | (ch2_reg.patternDuty << 6))
#define UPDATE_NR22	(NR22_REG = ch2_reg.envNbSweep | (ch2_reg.envMode << 3) | (ch2_reg.envInitialValue << 4))

inline void update_channel2(uint8_t freqLow, uint8_t freqHigh) {
    UPDATE_NR21;
    NR23_REG = freqLow;
    UPDATE_NR22;
    NR24_REG = ch2_reg.restart | freqHigh;
}

//-------------------------------------
//  channel 3 macros

#define UPDATE_NR30 (NR30_REG = ch3_reg.on_Off << 7)
#define UPDATE_NR31	(NR31_REG = ch3_reg.soundLength)
#define UPDATE_NR32	(NR32_REG = ch3_reg.selOutputLevel)

#define TURN_OFF_CH3    NR30_REG = 0
#define TURN_ON_CH3     NR30_REG = 0x80

#define WAVE_COUNT     0x03

inline void update_channel3(uint8_t freqLow, uint8_t freqHigh) {
    TURN_ON_CH3;
    UPDATE_NR31;
    UPDATE_NR32;
    NR33_REG = freqLow;
    NR34_REG = ch3_reg.restart | freqHigh;
}


//-------------------------------------
//  channel 4 macros

#define UPDATE_NR41 (NR41_REG = ch4_reg.soundLength)
#define UPDATE_NR42 (NR42_REG = ch4_reg.envNbStep | (ch4_reg.envMode << 3) | (ch4_reg.envInitialValue << 4))
#define UPDATE_NR43 (NR43_REG = ch4_reg.polyCounterDiv | (ch4_reg.polyCounterStep << 3) | (ch4_reg.polyCounterFreq << 4))
#define UPDATE_NR44 (NR44_REG = (ch4_reg.counter_ConsSel << 6) | (ch4_reg.restart << 7))

#define CH4_RESTART NR44_REG = 0x80

//-------------------------------------

void turnOn_sound(void);
void turnOFF_sound(void);

void play_sfx(void);
void play_note(void);
void play_event(void);

void simple_sfx_handler(void);

void stop_channel(void);
void stop_track(void);

void init_track(uint8_t track_id);
void update_song(void);

#endif

#ifndef SOUND_H
#define SOUND_H

#include <gb/gb.h>

#include "wave.h"

//-------------------------------------
// notes

#define NOTE_C    0x00
#define NOTE_CIS  0x10
#define NOTE_D    0x20
#define NOTE_DIS  0x30
#define NOTE_E    0x40
#define NOTE_F    0x50
#define NOTE_FIS  0x60
#define NOTE_G    0x70
#define NOTE_GIS  0x80
#define NOTE_A    0x90
#define NOTE_AIS  0xA0
#define NOTE_B    0xB0

#define NOTE(note, len)     ((note & 0xF) | (len & 0x0F))

#define INSTR_LEN(len)      ((len & 0b00111111) << 2)

#define ENV_MODE(mode)      (mode << 3)
#define ENV_VOL(vol)        (vol << 4)
#define ENV_SWEEP(sweep)    (sweep & 0x07)

#define DUTY12    0x00
#define DUTY25    0x01
#define DUTY50    0x02
#define DUTY75    0x03

//-------------------------------------
// commands

#define OCTAVE     0xE
#define SPECIAL    0xF
#define TEMPO      0xD

#define OPEN_LOOP  0xE

#define MAKE_NOTE(note, len)   (((note) << 4) | ((len) & 0x0F))
#define STOP         0xFF
#define CMD_OCT(n)   (0xE0 | (n))
#define CMD_DUTY(d)  (0xF0 | (d))

//-------------------------------------
//  special

#define GET_LOW_NIBBLE(b)  ((b) & 0x0F)
#define GET_HIGH_NIBBLE(b) ((b) >> 4)

//-------------------------------------

#define CH_RESTART      0x80  // Bit 7: Trigger/Restart
#define CH_LOOP_OFF     0x40  // Bit 6: Counter/Consecutive selection
#define CH_ON           0x80  // Bit 7: Sound On (for NR30/NR52)
#define CH_OFF          0x00

#define ENV_UP          0x08  // Bit 3: 1 = Increase
#define ENV_DOWN        0x00

#define AUDIO_ON        0x80
#define AUDIO_OFF       0x00

#define MUTE            0x00

#define AUDIO_PAN_ALL   0xFF
#define AUDIO_VOL_MAX   0x77

#define NOTE_PAUSE      0x0C
#define NOTE_MAX        0x0D

#define DRUM_BD_ENV     0xA1
#define DRUM_BD_FREQ    0x50
#define DRUM_SD_ENV     0x92
#define DRUM_SD_FREQ    0x80
#define DRUM_CH_ENV     0x51
#define DRUM_CH_FREQ    0x11

//-------------------------------------
// channel structs

struct ch1_reg_t {
    uint8_t sweepShifts;
    uint8_t sweepMode;
    uint8_t sweepTime;

    uint8_t soundLength;
    uint8_t patternDuty;

    uint8_t envNbSweep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch2_reg_t {
    uint8_t soundLength;
    uint8_t patternDuty;

    uint8_t envNbSweep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch3_reg_t {
    uint8_t on_Off;

    uint8_t soundLength;

    uint8_t selOutputLevel;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch4_reg_t {

    uint8_t soundLength;

    uint8_t envNbStep;
    uint8_t envMode;
    uint8_t envInitialValue;

    uint8_t polyCounterDiv;
    uint8_t polyCounterStep;
    uint8_t polyCounterFreq;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct channel_data_t {
    uint8_t* ptr;
    uint8_t curr_octv;      //unused for CH4

    uint8_t tick_counter;
    uint8_t ticks_per_row;
    uint8_t loop_iterator;
};

//-------------------------------------

extern struct ch1_reg_t ch1_reg;
extern struct ch2_reg_t ch2_reg;
extern struct ch3_reg_t ch3_reg;
extern struct ch4_reg_t ch4_reg;

//-------------------------------------
//  channel 1 macros

#define UPDATE_NR11	(NR11_REG = ch1_reg.soundLength | (ch1_reg.patternDuty << 6))
#define UPDATE_NR12	(NR12_REG = ch1_reg.envNbSweep | (ch1_reg.envMode << 3) | (ch1_reg.envInitialValue << 4))

inline void update_channel1(uint8_t freqLow, uint8_t freqHigh) {
    UPDATE_NR11;
    NR13_REG = freqLow;
    UPDATE_NR12;
    NR14_REG = ch1_reg.restart | freqHigh;
}

//-------------------------------------
//  channel 2 macros


#define UPDATE_NR21	(NR21_REG = ch2_reg.soundLength | (ch2_reg.patternDuty << 6))
#define UPDATE_NR22	(NR22_REG = ch2_reg.envNbSweep | (ch2_reg.envMode << 3) | (ch2_reg.envInitialValue << 4))

inline void update_channel2(uint8_t freqLow, uint8_t freqHigh) {
    UPDATE_NR21;
    NR23_REG = freqLow;
    UPDATE_NR22;
    NR24_REG = ch2_reg.restart | freqHigh;
}

//-------------------------------------
//  channel 3 macros

#define UPDATE_NR30 (NR30_REG = ch3_reg.on_Off << 7)
#define UPDATE_NR31	(NR31_REG = ch3_reg.soundLength)
#define UPDATE_NR32	(NR32_REG = ch3_reg.selOutputLevel)

#define TURN_OFF_CH3    NR30_REG = 0
#define TURN_ON_CH3     NR30_REG = 0x80

#define WAVE_COUNT     0x03

inline void update_channel3(uint8_t freqLow, uint8_t freqHigh) {
    TURN_ON_CH3;
    UPDATE_NR31;
    UPDATE_NR32;
    NR33_REG = freqLow;
    NR34_REG = ch3_reg.restart | freqHigh;
}


//-------------------------------------
//  channel 4 macros

#define UPDATE_NR41 (NR41_REG = ch4_reg.soundLength)
#define UPDATE_NR42 (NR42_REG = ch4_reg.envNbStep | (ch4_reg.envMode << 3) | (ch4_reg.envInitialValue << 4))
#define UPDATE_NR43 (NR43_REG = ch4_reg.polyCounterDiv | (ch4_reg.polyCounterStep << 3) | (ch4_reg.polyCounterFreq << 4))
#define UPDATE_NR44 (NR44_REG = (ch4_reg.counter_ConsSel << 6) | (ch4_reg.restart << 7))

#define CH4_RESTART NR44_REG = 0x80

//-------------------------------------

void turnOn_sound(void);
void turnOFF_sound(void);

void play_sfx(void);
void play_note(uint8_t channel);
void play_event(uint8_t channel);

void stop_channel(uint8_t channel);

void stop_track(void);

void init_track(uint8_t track_id);
void start_song(const uint8_t *track1, const uint8_t *track2);
void update_song(void);

#endif

