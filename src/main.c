#pragma bank 0
#include <gb/gb.h>
#include <gb/sgb.h>

#include <gbdk/console.h>
#include <gbdk/font.h>

#include "main.h"
#include "sound.h"
#include "input.h"

#include <stdio.h>

//-------------------------------------
uint8_t joy = 0;
uint8_t prev_joy = 0;
uint8_t track_id = 0;

//-------------------------------------


const unsigned char name[19] = "SLEDZ\0SOUND\0ENGINE";
char buffer[10];

void sound_test(void);

void main(void) {
    char model_buffer[3];
    // init
    turn_on_sound();

    font_init();
    font_set(font_load(font_spect));

    SHOW_BKG;

    set_bkg_tiles(1, 6, 19, 1, name);
    
    if (sgb_check()){
        model_buffer[0] = 'S';
        model_buffer[1] = 'G';
        model_buffer[2] = 'B';
    } else if (DEVICE_SUPPORTS_COLOR) {
        model_buffer[0] = 'C';
        model_buffer[1] = 'G';
        model_buffer[2] = 'B';
    } else if (_is_GBA){
        model_buffer[0] = 'G';
        model_buffer[1] = 'B';
        model_buffer[2] = 'A';
    } else {
        model_buffer[0] = 'M';
        model_buffer[1] = 'D';
        model_buffer[2] = 'G';
    }

    BGP_REG = DMG_BLACK;
    DISPLAY_ON;

    while (TRUE) {
        joy = joypad();
        
        sprintf(buffer, "ID\x1A%u", track_id);
        set_bkg_tiles(1, 7, 6, 1, buffer);
        
        set_bkg_tiles(1, 0, 9, 1, "GB\0MODEL\0");
        set_bkg_tiles(10, 0, 3, 1, model_buffer);
        sound_test();

        prev_joy = joy;
        vsync();
        BGP_REG = DMG_WHITE;
        update_song();
        BGP_REG = DMG_BLACK;
    }
}

void sound_test(void) {
    if (PRESSED_JOY_LEFT) {
        if (track_id != 0) {
            track_id--;
        }
    } else if (PRESSED_JOY_RIGHT){
        if (track_id != 2) {
            track_id++;
        }
    }

    if (PRESSED_JOY_A) {
        init_track(track_id);
    }
    if (PRESSED_JOY_B) {
        stop_track();
    }

    return;
}