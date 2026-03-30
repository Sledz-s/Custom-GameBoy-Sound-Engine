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
 *      ver. 1.1
 *      What's new:
 *        optimize write_wave_to_RAM, init_track
 */

#pragma bank 0

#include <gb/gb.h>
#include <stdio.h>
#include "sound.h"
#include "main.h"
#include "wave.h"
#include "track.h"

//-------------------------------------

struct ch1_reg_t ch1_reg;
struct ch2_reg_t ch2_reg;
struct ch3_reg_t ch3_reg;
struct ch4_reg_t ch4_reg;

struct channel_data_t ch_data[4];

//-------------------------------------
//  var

uint8_t note_hi;
uint8_t note_lo;
uint8_t channel;

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



//  ch1,    ,ch2    ,ch3    ,ch4

const uint8_t* track_pointers[][4] = {
    {loop_test, empty_track, empty_track, empty_track},
    {empty_track, empty_track, empty_track, drum_loop},
    {CV3_prelude_ch1, CV3_prelude_ch2, CV3_prelude_ch3, empty_track}
};

//-------------------------------------

void write_wave_to_RAM(const uint8_t *wavetable) {
    TURN_OFF_CH3;

    //pointer to the start of wave ram
    uint8_t *wave_ram = (uint8_t *)0xFF30; 

    // for (uint8_t i = 16; i != 0; i--) AUD3WAVE[i] = wavetable[i];
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

void turnOn_sound(void) {
    NR52_REG = AUDIO_ON;
    NR51_REG = AUDIO_PAN_ALL;
    NR50_REG = AUDIO_VOL_MAX;

    ch3_reg.restart = CH_RESTART;
    ch2_reg.restart = CH_RESTART;
    ch1_reg.restart = CH_RESTART;
    
    NR32_REG = 0x00;
}

void turnOFF_sound(void) {
    NR52_REG = AUDIO_OFF;
}

//--------------------------------------
//Process Byte

void play_note(void) {
    if (note_hi > NOTE_MAX) return;

    (ch_data + channel)->tick_counter = ((ch_data + channel)->ticks_per_row * note_lo) - 1;

    if (note_hi == NOTE_PAUSE) {            // Pause
        if (channel == 2) TURN_OFF_CH3;     // turn off ch3 during pause
        else stop_channel();
        return;
    }

    if (channel == 3) {
        switch (note_hi) {
            case 0x0: // BD - Kick Drum
                NR42_REG = DRUM_BD_ENV; 
                NR43_REG = DRUM_BD_FREQ;
                break;
            case 0x1: // SD - Snare
                NR42_REG = DRUM_SD_ENV; 
                NR43_REG = DRUM_SD_FREQ;
                break;
            case 0x2: // CH - Closed Hi-Hat
                NR42_REG = DRUM_CH_ENV; 
                NR43_REG = DRUM_CH_FREQ;
                break;
        }
        CH4_RESTART;
        return;
    }

    uint8_t index = note_hi + octaveOffsets[(ch_data + channel)->curr_octv - 2];

    uint8_t low = freqTableLow[index];
    uint8_t high = freqTableHigh[index];

    if (channel == 0){
        update_channel1(low, high);
    }
    if (channel == 1){
        update_channel2(low, high);
    }
    if (channel == 2){
        update_channel3(low, high);
    }
}

void process_intrument(void) {
    if (channel == 3) return;
    uint8_t event = *(ch_data + channel)->ptr++;

    if (channel == 0 || channel == 1) {

        note_hi = event & 0x03;                 //  LLLL LLDD
        note_lo = event >> 2;                   //  L - Lenght, D - Duty

        if (channel == 0) {
            ch1_reg.patternDuty = note_hi;
            ch1_reg.soundLength = note_lo;
        }

        if (channel == 1) {
            ch2_reg.patternDuty = note_hi;
            ch2_reg.soundLength = note_lo;
        }

        event = *(ch_data + channel)->ptr++;
    
        uint8_t envM = (event >> 3) & 0x01;     // VVVV ---- (Initial Volume, 0-15)
        uint8_t envV = event >> 4;              // ---- M--- (0 = down, 1 = up)
        uint8_t envS = event & 0x07;            // ---- -SSS (Number of envelope steps, 0-7)

        if (channel == 0) {
            ch1_reg.envInitialValue = envV;
            ch1_reg.envMode = envM;
            ch1_reg.envNbSweep = envS;
        }
        if (channel == 1) {
            ch2_reg.envInitialValue = envV;
            ch2_reg.envMode = envM;
            ch2_reg.envNbSweep = envS;
        }
        return;
    }

    if (channel == 2) {
        ch3_reg.selOutputLevel = event;
        const uint8_t *wave_pointer = NULL;

        switch(event & WAVE_COUNT) {
            case 0x00: wave_pointer = square_wave;   break;
            case 0x01: wave_pointer = triangle_wave; break;
            case 0x02: wave_pointer = epiano_wave;   break;
            case 0x03: wave_pointer = metallic_wave; break;
        }
        write_wave_to_RAM(wave_pointer);
    }
}

void play_event(void) {
    while (1) {
        uint8_t event = *(ch_data + channel)->ptr++;
        if (event == STOP) return;

        note_hi = GET_HIGH_NIBBLE(event);
        note_lo = GET_LOW_NIBBLE(event);

        if (note_hi < TEMPO) {
            note_lo++;
            play_note();
            return;
        }

        if (note_hi == SPECIAL) {
            simple_sfx_handler();

        } else if (note_hi == OCTAVE) {
            
            if (note_lo < 2 || note_lo > 7) continue;

            (ch_data + channel)->curr_octv = note_lo;
            continue;

        } else if (note_hi == TEMPO) {   
            (ch_data + channel)->ticks_per_row = note_lo;
            process_intrument();
        }
    }
}

void simple_sfx_handler(void) {
    struct channel_data_t *curr_ch = &ch_data[channel];

    switch (note_lo) {
    case CH_PAN:

    note_lo = *curr_ch->ptr++;
    NR51_REG = note_lo;

    return;
    case OPEN_LOOP: // Looping Logic
        note_lo = *curr_ch->ptr++;
        note_hi = *curr_ch->ptr++;
        
        uint8_t repeats = *curr_ch->ptr++; 

        if (repeats == 0xFF) {
            uint16_t dest = (uint16_t)note_lo | ((uint16_t)note_hi << 8);
            curr_ch->ptr = (uint8_t *)dest;
            return;
        }

        if (curr_ch->loop_iterator < repeats - 1) {
            curr_ch->loop_iterator++;
            uint16_t dest = (uint16_t)note_lo | ((uint16_t)note_hi << 8);
            curr_ch->ptr = (uint8_t *)dest;
        } else {
            curr_ch->loop_iterator = 0;
        }
        return;
    }
}

//--------------------------------------

void init_track(uint8_t track_id) {
    //make sure that every channel is silent
    NR12_REG = MUTE; 
    NR22_REG = MUTE; 
    TURN_OFF_CH3;
    NR42_REG = MUTE; 
    
    NR14_REG = CH_OFF;
    NR24_REG = CH_OFF;
    NR44_REG = CH_OFF;

    if (track_id > 2) track_id = 0; //sound test check
    //--------------------------------------
    //set data
    uint8_t *raw_p = (uint8_t *)ch_data;    // set pointer to the first element of ch_data array
    const uint8_t **curr_ptr_track = track_pointers[track_id]; //set pointers to track

    for (uint8_t i = 4; i != 0; i--) {
        uint16_t track_addr = (uint16_t)*curr_ptr_track++;

        *raw_p++ = (uint8_t)(track_addr & 0xFF); // Low byte
        *raw_p++ = (uint8_t)(track_addr >> 8);   // High byte

        *raw_p++ = 3;    // Set curr_octv
        *raw_p++ = 0;    // Set tick_counter
        *raw_p++ = 7;    // Set ticks_per_row
        *raw_p++ = 0;    // Set loop_iterator
        raw_p += 2;      // Skip padding to get to next struct
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
    struct channel_data_t *curr_ch = &ch_data[channel];

    if (curr_ch->tick_counter > 0) {
        curr_ch->tick_counter--;

        // Special commands handler 
        // will add soon

        return;
    }

    if (*curr_ch->ptr == STOP) {
        stop_channel();
        return;
    }

    curr_ch->tick_counter--;

    play_event();
}

void update_song(void) {
    channel = 0;
    channel_update();   //ch 1 update
    channel++;
    channel_update();   //ch 2 update
    channel++;
    channel_update();   //ch 3 update
    channel++;
    channel_update();   //ch 4 update
}

void stop_track(void) {
    NR12_REG = MUTE; 
    NR14_REG = CH_RESTART; 

    NR22_REG = MUTE; 
    NR24_REG = CH_RESTART; 

    TURN_OFF_CH3;

    NR42_REG = MUTE; 
    NR44_REG = CH_RESTART; 

    for (uint8_t i = 4; i != 0; i--) ch_data[i].ptr = NULL;
}
