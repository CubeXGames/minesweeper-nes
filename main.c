#ifndef MAIN_C
#define MAIN_C

#include "logic.h"

void main(void) {

    initState();
    showTitleScreen();

    while(TRUE) {

        ppu_wait_nmi();

        update();
    }
}

#endif