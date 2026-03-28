#pragma bank 0
#include <gb/gb.h>
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
    // init
    turnOn_sound();

    font_init();
    font_set(font_load(font_spect));

    SHOW_BKG;

    set_bkg_tiles(1, 6, 19, 1, name);

    DISPLAY_ON;

    while (TRUE) {
        joy = joypad();
        
        sprintf(buffer, "ID\x1A%u", track_id);
        set_bkg_tiles(1, 7, 6, 1, buffer);

        sound_test();

        prev_joy = joy;
        vsync();
        update_song();
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