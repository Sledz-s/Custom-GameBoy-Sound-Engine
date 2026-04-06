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

//-------------------------------------
// commands

#define OCTAVE     0xE
#define SPECIAL    0xF
#define TEMPO      0xD

// special effects
#define OPEN_LOOP  0x0
#define CH_PAN     0x1

#define STOP         0xFF
#define CMD_OCT(n)   (0xE0 | (n))
#define NOTE(note, len)     ((note & 0xF) | (len & 0x0F))
#define INSTR_LEN(len)      ((len & 0b00111111))
#define ENV_MODE(mode)      (mode << 3)
#define ENV_VOL(vol)        (vol << 4)
#define ENV_SWEEP(sweep)    (sweep & 0x07)

#define DUTY12    0x00
#define DUTY25    0x01
#define DUTY50    0x02
#define DUTY75    0x03
//-------------------------------------
//  special

#define GET_LOW_NIBBLE(b)  ((b) & 0x0F)
#define GET_HIGH_NIBBLE(b) ((b) >> 4)

//-------------------------------------
//  sound effect bit mask

#define SFX_CH1 0x01
#define SFX_CH2 0x02
#define SFX_CH3 0x04
#define SFX_CH4 0x08

//-------------------------------------
//  registers and memory

#define WAVERAM_START   (uint8_t *)0xFF30

//-------------------------------------
// default data

#define DEFAULT_OCTV            3
#define DEFAULT_TICKS_PER_ROW   7
#define CLEAR                   0
#define HEAR_ALL_CH             0xFF

//-------------------------------------

#define CH_RESTART      0x80  
#define CH_LOOP_OFF     0x40  
#define CH_ON           0x80  
#define CH_OFF          0x00

#define ENV_UP          0x08  
#define ENV_DOWN        0x00

#define AUDIO_ON        0x80
#define AUDIO_OFF       0x00

#define MUTE            0x00

#define AUDIO_PAN_ALL   0xFF
#define AUDIO_VOL_MAX   0x77

#define NOTE_PAUSE      0x0C
#define NOTE_MAX        0x0D

//-------------------------------------

typedef void (*jumptable)(void);    // jump table type for functions

typedef uint8_t* (*instr_func)(uint8_t*); 
typedef uint8_t* (*fx_func)(uint8_t*);

//-------------------------------------
// channel structs

struct ch1_reg_t {
    uint8_t sweep;
    
    uint8_t duty_and_lenght;

    uint8_t envelope;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch2_reg_t {
    uint8_t duty_and_lenght;

    uint8_t envelope;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct ch3_reg_t {
    uint8_t soundLength;

    uint8_t selOutputLevel;

    uint8_t counter_ConsSel;
    uint8_t restart;
};

struct channel_data_t {
    uint8_t* ptr;           
    uint8_t curr_octv;      //unused for CH4

    uint8_t tick_counter;
    uint8_t ticks_per_row;
    uint8_t loop_iterator;

    uint8_t effect_flags;
    uint8_t effect_param;
};

typedef const struct {
    uint8_t env;  // NR42
    uint8_t freq; // NR43
} drum_kit_t;

//-------------------------------------

extern struct ch1_reg_t ch1_reg;
extern struct ch2_reg_t ch2_reg;
extern struct ch3_reg_t ch3_reg;

//-------------------------------------
//  channel 1 macros

#define UPDATE_NR11	(NR11_REG = ch1_reg.duty_and_lenght)
#define UPDATE_NR12	(NR12_REG = ch1_reg.envelope)

//-------------------------------------
//  channel 2 macros


#define UPDATE_NR21	(NR21_REG = ch2_reg.duty_and_lenght)
#define UPDATE_NR22	(NR22_REG = ch2_reg.envelope)

//-------------------------------------
//  channel 3 macros

#define UPDATE_NR31	(NR31_REG = ch3_reg.soundLength)
#define UPDATE_NR32	(NR32_REG = ch3_reg.selOutputLevel)

#define TURN_OFF_CH3    NR30_REG = 0
#define TURN_ON_CH3     NR30_REG = 0x80

#define WAVE_COUNT     0x03

//-------------------------------------
//  channel 4 macros

#define CH4_RESTART NR44_REG = 0x80

#define DRUM_BD_ENV     0xA1
#define DRUM_BD_FREQ    0x50
#define DRUM_SD_ENV     0x92
#define DRUM_SD_FREQ    0x80
#define DRUM_CH_ENV     0x51
#define DRUM_CH_FREQ    0x11

//-------------------------------------

void turn_on_sound(void);
void turn_off_sound(void);

//---------- core funcs
void play_note(void);
void play_event(void);

//---------- update channel funcs
void update_channel1(void);
void update_channel2(void);
void update_channel3(void);

//---------- instrument funcs
uint8_t *process_instr_ch3(uint8_t *temp_p);
uint8_t *process_instr_ch2(uint8_t *temp_p);
uint8_t *process_instr_ch1(uint8_t *temp_p);

//---------- fx func
uint8_t *fx_loop_open(uint8_t *temp_p);
uint8_t *fx_panning(uint8_t *temp_p);

//---------- sfx func
void play_sfx(uint8_t sfx_id);

void stop_channel(void);
void stop_track(void);

void init_track(uint8_t track_id);
void update_song(void);

void channel_update(void);

#endif

