#ifndef MAIN_C
#define MAIN_C

#include "logic.h"

//(((global_i & 0b1) ^ (global_j & 0b1)) << 1) <-- how to make a checkerboard pattern
//16x15 or 32x30
void main(void) {

    initState();
	
    gameMode |= (0b1 << 3);

	ppu_off(); //screen off
    generateBoard();

    //palettes
	pal_bg(bgPalette);
    pal_spr(sprPalette);

    bank_spr(1);

    for(global_i = 0; global_i < HARD_MAX_X; ++global_i) {

        for(global_j = 0; global_j < HARD_MAX_Y; ++global_j) {
            
            temp2 = 0x8 + (((global_i & 0b1) ^ (global_j & 0b1)) << 1); //tile num, alternates as a checkerboard
            cursorX = global_i;
            cursorY = global_j;
            temp2 += getTileIsMineHard(); //1 = mine, 0 = no mine
            vram_adr(NTADR_A(global_i, global_j + 3));
            vram_put(temp2);
        }
    }

    ppu_on_all();

    set_vram_buffer();

    cursorX = 0;
    cursorY = 0;

    printNumber(cursorX, 1, 1);
    printNumber(cursorY, 4, 1);
    printNumber(numFlags, 7, 1);

    while(TRUE) {

        ppu_wait_nmi();

        update();
    }
}

#endif