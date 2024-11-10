#ifndef MAIN_C
#define MAIN_C

#include "logic.h"

//(((global_i & 0b1) ^ (global_j & 0b1)) << 1) <-- how to make a checkerboard pattern
//16x15 or 32x30
void main(void) {

    initState();
	
    gameMode |= (0b1 << 3);
    generateBoard();

	ppu_off(); //screen off

    //palettes
	pal_bg(bgPalette);
    pal_spr(sprPalette);

    bank_spr(1);

    set_vram_buffer();

    ppu_on_all();

    temp0 = 0;
    temp1 = 0;
    temp3 = 0;
    global_j = 0;
    global_i = 0;

    {

        here:

        ppu_wait_nmi();

        temp0 = 0;
        for(; global_j < HARD_MAX_Y; ++global_j) {

            for(; global_i < HARD_MAX_X; ++global_i) {
                
                temp2 = 0x8 + (((global_i & 0b1) ^ (global_j & 0b1)) << 1); //offset
                temp2 += getTileIsMineHard(global_i, global_j); //1 = mine, 0 = no mine
                one_vram_buffer(temp2, NTADR_A(global_i, global_j + 3));
                ++temp0;
                if(temp0 > 25) goto here; //only 25 updates per frame
            }

            global_i = 0;
        }
    }

    ppu_wait_nmi();

    while(TRUE) {

        ppu_wait_nmi();

        update();
    }
}

#endif