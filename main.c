#ifndef MAIN_C
#define MAIN_C

#include "logic.h"

//(((global_i & 0b1) ^ (global_j & 0b1)) << 1) <-- how to make a checkerboard pattern
//16x15 or 32x30
void main(void) {

    ppu_off();

    initState();
    showTitleScreen();
    
    set_vram_buffer();
	
    /*setToEasyMode();

    newGame();

    printNumber(cursorX, 1, 1);
    printNumber(cursorY, 4, 1);
    printNumber(numFlags, 7, 1);*/

    while(TRUE) {

        ppu_wait_nmi();

        update();
    }
}

#endif