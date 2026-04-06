/*            __  ________ ________  __________                                                                
 *      _____|  | \_____  \\______ \ \____    /                                                                
 *     /  ___/  |   _(__  < |    |  \  /     /                                                                 
 *     \___ \|  |__/       \|    `   \/     /_                                                                 
 *    /____  >____/______  /_______  /_______ \                                                                
 *         \/            \/        \/        \/                                                                
 *      __________________    _________                         ___ ___________               __               
 *     /  _____/\______   \  /   _____/ ____  __ __  ____    __| _/ \_   _____/ ____    ____ |__| ____   ____  
 *    /   \  ___ |    |  _/  \_____  \ /  _ \|  |  \/    \  / __ |   |    __)_ /    \  / ___\|  |/    \_/ __ \ 
 *    \    \_\  \|    |   \  /        (  <_> )  |  /   |  \/ /_/ |   |        \   |  \/ /_/  >  |   |  \  ___/ 
 *     \______  /|______  / /_______  /\____/|____/|___|  /\____ |  /_______  /___|  /\___  /|__|___|  /\___  >
 *            \/        \/          \/                  \/      \/          \/     \//_____/         \/     \/ 
 *      ver. 1.5
 *      
 *      What's new:
 *        ver: 1.5
 *          Format of instrument for CH1 and CH2 was change to optimize RAM, see README
 *        ver: 1.4
 *          More optimazation and changed panning and looping command, see README
 *        ver: 1.3
 *          Added jump table logic and make noise logic easier to expand 
 *
 *        ver: 1.2
 *          Added special check in update_song to prevent CPU usage     
 *          Added new command 0xF0, panning change, see README for more information          
 *
 *        ver: 1.1
 *          optimize write_wave_to_RAM, init_track
 */
#pragma bank 0

#include <gb/gb.h>

#include "sound.h"
#include "main.h"

#include "wave.h"
#include "track.h"

//-------------------------------------

struct ch1_reg_t ch1_reg;
struct ch2_reg_t ch2_reg;
struct ch3_reg_t ch3_reg;

struct channel_data_t ch_data[4];

//-------------------------------------
//  var

// HRAM
SFR note_hi;
SFR note_lo;
SFR channel;


// RAM
uint8_t sfx_flag = 0;
uint8_t sound_engine_state = 0;
struct channel_data_t *curr_ch_ptr;

//--------------------------------------

const uint8_t freqTableLow[] = {
  0x2C, 0x9C, 0x06, 0x6B, 0xC9, 0x23, 0x77, 0xC6, 0x12, 0x56, 0x9B, 0xDA,
  0x16, 0x4E, 0x83, 0xB5, 0xE5, 0x11, 0x3B, 0x63, 0x89, 0xAC, 0xCE, 0xED,
  0x0A, 0x27, 0x42, 0x5B, 0x72, 0x89, 0x9E, 0xB2, 0xC4, 0xD6, 0xE7, 0xF7,
  0x06, 0x14, 0x21, 0x2D, 0x39, 0x44, 0x4F, 0x59, 0x62, 0x6B, 0x73, 0x7B,
  0x83, 0x8A, 0x90, 0x97, 0x9D, 0xA2, 0xA7, 0xAC, 0xB1, 0xB6, 0xBA, 0xBE,
  0xC1, 0xC4, 0xC8, 0xCB, 0xCE, 0xD1, 0xD4, 0xD6, 0xD9, 0xDB, 0xDD, 0xDF
};

const uint8_t freqTableHigh[] = {
  0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
};

const uint8_t octaveOffsets[] = { 0, 12, 24, 36, 48, 60 };

//--------------------------------------
// jump tables
static const uint8_t * const jt_wave[] = {
    square_wave,
    triangle_wave,
    epiano_wave,
    metallic_wave
};

const instr_func ch_func[] = {
    process_instr_ch1, 
    process_instr_ch2, 
    process_instr_ch3
};

const jumptable channel_updates[] = {
    update_channel1,
    update_channel2,
    update_channel3
};

const fx_func fx_handle[] = {
    fx_loop_open,
    fx_panning
};

//--------------------------------------
//  drums

const drum_kit_t my_drums[] = {
    {DRUM_BD_ENV, DRUM_BD_FREQ}, // 0x0: BD
    {DRUM_SD_ENV, DRUM_SD_FREQ}, // 0x1: SD
    {DRUM_CH_ENV, DRUM_CH_FREQ}  // 0x2: CH
};

//--------------------------------------

const uint8_t* track_pointers[][4] = {
    {BWV847_ch1, BWV847_ch2, BWV847_ch3, empty_track},
    {empty_track, empty_track, empty_track, drum_loop},
    {CV3_prelude_ch1, CV3_prelude_ch2, CV3_prelude_ch3, empty_track}
};

//-------------------------------------

void write_wave_to_RAM(const uint8_t *wavetable) {
    TURN_OFF_CH3;
    
    uint8_t *wave_ram = WAVERAM_START; 

    // unrolled loop 
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;

    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram++ = *wavetable++;
    *wave_ram++ = *wavetable++; *wave_ram = *wavetable;
}

//--------------------------------------
//Process Byte

void play_note(void) {
    if (note_hi > NOTE_MAX) return;
    curr_ch_ptr->tick_counter = (curr_ch_ptr->ticks_per_row * note_lo) - 1;

    if (note_hi == NOTE_PAUSE) return stop_channel();
    
    if (channel == 3) {
        NR42_REG = my_drums[note_hi].env; 
        NR43_REG = my_drums[note_hi].freq;
        NR44_REG = 0x80; 
        return;
    }

    uint8_t index = note_hi + octaveOffsets[curr_ch_ptr->curr_octv - 2];

    note_lo = freqTableLow[index];
    note_hi = freqTableHigh[index];

    channel_updates[channel]();
}

void play_event(void) {
    uint8_t event;
    uint8_t *temp_p = curr_ch_ptr->ptr;
    while (1) {
        event = *temp_p++;

        if (event == STOP) {
            curr_ch_ptr->ptr = temp_p;
            return;
        }

        note_hi = GET_HIGH_NIBBLE(event);
        note_lo = GET_LOW_NIBBLE(event);

        if (note_hi < TEMPO) {
            note_lo++;
            curr_ch_ptr->ptr = temp_p;
            return play_note();
        }

        if (note_hi == SPECIAL) {
            temp_p = fx_handle[note_lo & 0x01](temp_p);
            continue;

        } else if (note_hi == OCTAVE) {
            curr_ch_ptr->curr_octv = (note_lo & 0x07);
            continue;

        } else if (note_hi == TEMPO) {   
            curr_ch_ptr->ticks_per_row = note_lo;
            if (channel == 3) continue;
            temp_p = ch_func[channel](temp_p);
        }
    }
}

//--------------------------------------

void update_channel1(void) {
    if (sfx_flag & SFX_CH1) return;
    UPDATE_NR11;
    NR13_REG = note_lo;
    UPDATE_NR12;
    NR14_REG = ch1_reg.restart | note_hi;
}

void update_channel2(void) {
    if (sfx_flag & SFX_CH2) return;
    UPDATE_NR21;
    NR23_REG = note_lo;
    UPDATE_NR22;
    NR24_REG = ch2_reg.restart | note_hi;
}

void update_channel3(void) {
    if (sfx_flag & SFX_CH3) return;
    TURN_ON_CH3;
    UPDATE_NR31;
    UPDATE_NR32;
    NR33_REG = note_lo;
    NR34_REG = ch3_reg.restart | note_hi;
}

//--------------------------------------

uint8_t *process_instr_ch1(uint8_t *temp_p) {
    struct ch1_reg_t *ch_reg = &ch1_reg;
    ch_reg->duty_and_lenght = *temp_p++;   
    ch_reg->envelope = *temp_p++;

    return temp_p;
}

uint8_t * process_instr_ch2(uint8_t *temp_p) {
    struct ch2_reg_t *ch_reg = &ch2_reg;
    ch_reg->duty_and_lenght = *temp_p++;   
    ch_reg->envelope = *temp_p++;

    return temp_p;
}

uint8_t * process_instr_ch3(uint8_t *temp_p) {
    struct ch3_reg_t *ch_reg = &ch3_reg;

    uint8_t event = *temp_p++;

    ch_reg->selOutputLevel = event;
    const uint8_t *wave_pointer = NULL;

    wave_pointer = jt_wave[event & WAVE_COUNT];
    write_wave_to_RAM(wave_pointer);
    return temp_p;
}

//--------------------------------------

uint8_t *fx_loop_open(uint8_t *temp_p) {
    note_lo = *temp_p++;
    note_hi = *temp_p++;
        
    uint8_t repeats = *temp_p++; 

    if (repeats == 0xFF) {
        uint16_t dest = (uint16_t)note_lo | ((uint16_t)note_hi << 8);
        temp_p = (uint8_t *)dest;
        return temp_p;
    }

    if (curr_ch_ptr->loop_iterator < repeats - 1) {
        curr_ch_ptr->loop_iterator++;
        uint16_t dest = (uint16_t)note_lo | ((uint16_t)note_hi << 8);
        temp_p = (uint8_t *)dest;
    } else {
        curr_ch_ptr->loop_iterator = 0;
    }
        
    return temp_p;
}

uint8_t *fx_panning(uint8_t *temp_p) {
    NR51_REG = *temp_p++;
    return temp_p;
}

//--------------------------------------

void init_track(uint8_t track_id) {
    sound_engine_state = 1;

    // Reset Hardware: Mute and stop all channels
    NR12_REG = MUTE; 
    NR22_REG = MUTE; 
    TURN_OFF_CH3;
    NR42_REG = MUTE; 
    
    NR14_REG = CH_OFF;
    NR24_REG = CH_OFF;
    NR44_REG = CH_OFF;

    if (track_id > 2) track_id = 0; 

    uint8_t **selected_track = track_pointers[track_id];
    curr_ch_ptr = &ch_data[0];
    uint8_t i = 4;

    NR51_REG = HEAR_ALL_CH;

    do {
        curr_ch_ptr->ptr = *selected_track;

        curr_ch_ptr->curr_octv = DEFAULT_OCTV;
        curr_ch_ptr->tick_counter = i;
        curr_ch_ptr->ticks_per_row = DEFAULT_TICKS_PER_ROW;
        curr_ch_ptr->loop_iterator = CLEAR;
        curr_ch_ptr->effect_flags = CLEAR;
        curr_ch_ptr++;
        selected_track++;

    } while (--i);
}

void update_song(void) {
    if (!sound_engine_state) return;
    
    curr_ch_ptr = &ch_data[0];
    for (channel = 0;channel < 4; channel++) {
        channel_update();
        curr_ch_ptr++;
    }
}


void stop_channel(void) {
    switch (channel) {
        case 0: NR12_REG = 0x00; NR14_REG = CH_RESTART; break; 
        case 1: NR22_REG = 0x00; NR24_REG = CH_RESTART; break; 
        case 2: TURN_OFF_CH3; break;                 
        case 3: NR42_REG = 0x00; NR44_REG = CH_RESTART; break;
    }
}

void channel_update(void) {
    if (curr_ch_ptr->tick_counter > 0) {
        curr_ch_ptr->tick_counter--;
        return;
    }

    if (*curr_ch_ptr->ptr == STOP) {
        stop_channel();
        return;
    }
    play_event();
}

//--------------------------------------

void turn_on_sound(void) {
    NR52_REG = AUDIO_ON;
    NR51_REG = AUDIO_PAN_ALL;
    NR50_REG = AUDIO_VOL_MAX;

    ch3_reg.restart = CH_RESTART;
    ch2_reg.restart = CH_RESTART;
    ch1_reg.restart = CH_RESTART;
    
    NR32_REG = 0x00;
}

void turn_off_sound(void) {
    NR52_REG = AUDIO_OFF;
}

void stop_track(void) {
    sound_engine_state = 0;
    NR12_REG = MUTE; 
    NR14_REG = CH_RESTART; 

    NR22_REG = MUTE; 
    NR24_REG = CH_RESTART; 

    TURN_OFF_CH3;

    NR42_REG = MUTE; 
    NR44_REG = CH_RESTART; 

    for (uint8_t i = 4; i != 0 ; i--) ch_data[i].ptr = NULL;
}