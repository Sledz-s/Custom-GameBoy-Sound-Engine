# Custom-GameBoy-Sound-Engine

# ====================================

    sl3dz sound engie for GB version 1.0

        free to use sound engine
			written in C with
				GBDK 2020
	
		from amateur for everyone
		
# ====================================

    # HOW TO USE

    ## 1. Adding a New Track

        Sound engine can handle up to 256 different tracks
        to add your own track you can do it in two ways

        you can create an array of data          (not recommended)
        or you can create an asm file with the data  (recommended)

        to import a table from asm you have to extern table for asm, for example:
		
		### Step 1: Define Data in Assembly
		Create an `.s` file and export your label using `.globl`.
		
        in asm:
        ```asm
		.area _CODE
		.globl _my_track_ch1

		_my_track_ch1::
			.db 0xD0, 0x01, 0xE3  ; Set Tempo/Instrument, Octave 3
			.db 0x03, 0x73, 0xFF  ; Notes, End Track
		```
		
		### Step 2: Link in C
        Declare the data as an extern array in your header file and add it to the track_pointers 2D array
		
		```
		extern uint8_t my_track_ch1[];
		// track_pointers[track_id][channel_index]
		const uint8_t* track_pointers[][4] = { {my_track_ch1, ch2_data, ch3_data, ch4_data} };
		```
		> **Note:** track_pointers is declare in `sound.c` and instead of NULL use `empty_track` as a placeholder

    ## 2. Technical Specifications
        So you you have created arrays for your track.
		Your next step should be to fill these arrays with data, so 
        to compose, you have to know a few importat things:

        ## 1. note format

        0xXY
        X - command
        y - parameter
		
		Hex Code  |  Command  |  Description
		0x0-0xB   |    note   |  Plays a note (C, C#, D... B) with a specific length.
		0xCX	  |   pause   |  Silences the channel for X ticks. 
		0xDX	  |   tempo   |  Sets the instrument/tempo parameters for the channel
		0xEX      |   octave  |  Sets the current octave (0-7)	> **Note:** octave only accept values from 2 to 7, other values are ignored
		0xFE      |    loop   |  0xFE [ADDR_LOW] [ADDR_HIGH] [COUNT] - Repeats a section
		0xFF      |    stop   |  Ends playback on the channel
		

    ## 3. Creating instruments
        To create instrument you have to use 0xDY command
        bytes following the tempo command are use for instrument

        ### Pulse Channels (CH1 & CH2)
        
            Byte 1 (LLLL LLDD): L = Length, D = Duty Cycle.
            Byte 2 (VVVV MSSS): V = Initial Volume, M = Mode (0: Down, 1: Up), S = Envelope Steps
		
		> **Note:** if you set S to 0, your instrument will have a constant volume
		
        ### Wave Channel (CH3)
		
        Requires 1 byte after the command (0YY0 00XX):
		
		0 - unused
		
		YY: Volume level (0x00: Mute, 0x20: 100%, 0x40: 50%, 0x60: 25%).
		XX: Wave ID (0: Square, 1: Triangle, 2: E-Piano, 3: Metallic).


        if you wish to have more waves, simply change `WAVE_COUNT` in `sound.h`
        and add new cases in switch in `void process_intrument(uint8_t channel)` in `sound.c`
        then add your new waves to `wave.s` and extern array in `wave.h`
		
		### Noise Channel (Ch4)

        channel 4 works on preset instruments

        X: Preset ID
        Y: Note Lenght 
		
		> **Note:** octave is ignored and tempo doesn't move pointer
	## 4. Special commands
		Engine supports 2 special commands

		0xF0 -> Loop open
		0xF1 -> Change panning

		## 0xF0 Loop syntax:
			0xF0, address_lo, address_hi, how many times

		I recomnd using ASM for making tracks with loops
		example from track loop_test:
		
			_loop_test::    ;simply loop test
    			.db 0xD3, 0xF8, 0x93, 0xE4
			loop_start:
			    .db 0x01, 0x11, 0x21, 0x31
			    .db 0x41, 0x51, 0x61, 0x71
			    .db LOOP_OPEN	-> 0xF0
			    .dw loop_start 	-> address where to jump
			    .db 0x02		-> how many times
			    .db STOP
		if you want infinite loop simply use 0xFF for how many times

		## 0xF1 Change panning syntax:
			0xF1, bit mask
		    8      7      6      5      4      3      2      1
		| CH4L | CH4R | CH3L | CH3R | CH2L | CH2R | CH1L | CH1R |
		
# ====================================
