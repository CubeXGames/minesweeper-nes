#ifndef LOGIC_H
#define LOGIC_H

#include "lib/neslib.h"
#include "lib/nesdoug.h"
#include "defs.h"

#define BUTTON_PRESSED(button) controller & button
#define BUTTON_DOWN(button) (controller & button) && !(prevController & button)
#define BUTTON_UP(button) (prevController & button) && !(controller & button)

//zeropage variables
#pragma bss-name(push, "ZEROPAGE")

//temporary vars, make sure to state use in each function and to ensure no overlap
uchar temp0;
uchar temp1;
uchar temp2;
uchar temp3;
uchar temp4;
unsigned short tempShort0;
uchar global_i;
uchar global_j;
uchar frameCount;
uchar frameCountOverflow;

//uchar boardWidth;

randomState rngState;

uchar prevController;
uchar controller;

uchar gameMode; //bits 0-2: which screen you're on, 3: easy or hard mode, 4: board is updating

uchar cursorX;
uchar cursorY;

uchar tempTileX, tempTileY;

uchar numFlags;

#pragma bss-name(pop)

unsigned short numMinesSetInByte = 0;

sprite hardSelectionSprite;
uchar boardIsMine[BOARD_MEM_SIZE];
uchar boardIsActivated[BOARD_MEM_SIZE];
uchar boardIsFlag[BOARD_MEM_SIZE];

uchar fillStackX[FILL_STACK_SIZE];
uchar fillStackY[FILL_STACK_SIZE];
uchar fillStackPos;

uchar debugTemp0;

const char bgPalette[] = {

    0x09, 0x1A, 0x0F, 0x30,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

const char sprPalette[] = {

    0x09, 0x30, 0x52, 0x30,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

inline void updateController(void) {

    prevController = controller;
    controller = pad_poll(0);

    //mask out l+r or u+d inputs (i have been officially labeled anti-tas now)
    if((controller & PAD_LEFT) && (controller & PAD_RIGHT)) controller &= ~(0b11u);
    if((controller & PAD_UP) && (controller & PAD_DOWN)) controller &= ~(0b1100u);
}

//for calling at the start of the frame
inline void updateRNG(void) {

    __asm__("lda %v", rngState);
    __asm__("clc");
    __asm__("adc %v", controller);
    __asm__("sta %v", rngState);
    
    rngState.longState ^= rngState.longState << 7;
	rngState.longState ^= rngState.longState >> 9;
	rngState.longState ^= rngState.longState << 8;
}

//for sub-frame calls (no fancy schmancy rng manip for yü mr tas)
void updateRNGNoController(void) {

    rngState.longState ^= rngState.longState << 7;
	rngState.longState ^= rngState.longState >> 9;
	rngState.longState ^= rngState.longState << 8;
}

#pragma region Tile Updaters

//uses tempShort0
uchar getTileBaseHard(uchar* boardType) {

    tempShort0 = ((cursorY << 5) + cursorX); //bit offset
    return (boardType[tempShort0 >> 3] >> (tempShort0 & 0b111u)) & 0b1u;
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
void setTileBaseHard(uchar* boardType, uchar value) {

    tempShort0 = ((cursorY << 5) + cursorX); //bit offset
    temp4 = tempShort0 >> 3;
    temp3 = tempShort0 & 0b111;
    boardType[temp4] = (boardType[temp4] & ~(0b1u << temp3)) | (value << temp3);
}

//uses tempShort0
uchar getTileBaseEasy(uchar* boardType) {

    tempShort0 = ((cursorY << 4) + cursorX); //bit offset
    return (boardType[tempShort0 >> 3] >> (tempShort0 & 0b111u)) & 0b1u;
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
void setTileBaseEasy(uchar* boardType, uchar value) {

    tempShort0 = ((cursorY << 4) + cursorX); //bit offset
    temp4 = tempShort0 >> 3;
    temp3 = tempShort0 & 0b111;
    boardType[temp4] = (boardType[temp4] & ~(0b1u << temp3)) | (value << temp3);
}

//uses tempShort0
inline uchar getTileIsMineHard(void) {

    return getTileBaseHard(boardIsMine);
}

//uses tempShort0
inline uchar getTileIsMineEasy(void) {

    return getTileBaseEasy(boardIsMine);
}

//uses tempShort0
inline uchar getTileIsActivatedHard(void) {

    return getTileBaseHard(boardIsActivated);
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsActivatedHard(uchar value) {

    setTileBaseHard(boardIsActivated, value);
}

//uses tempShort0
inline uchar getTileIsActivatedEasy(void) {

    return getTileBaseEasy(boardIsActivated);
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsActivatedEasy(uchar value) {

    setTileBaseEasy(boardIsActivated, value);
}

//uses tempShort0
inline uchar getTileIsFlagHard(void) {

    return getTileBaseHard(boardIsFlag);
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsFlagHard(uchar value) {

    setTileBaseHard(boardIsFlag, value);
}

//uses tempShort0
inline uchar getTileIsFlagEasy(void) {

    return getTileBaseEasy(boardIsFlag);
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsFlagEasy(uchar value) {

    setTileBaseEasy(boardIsFlag, value);
}

#pragma endregion

//todo fix number length bug
void printNumberDebug(unsigned short number, uchar x, uchar y) {

    uchar output = 0;
    uchar numberOffset = 0;
    uchar firstNumber = FALSE;

    //if(number >= 10000) {

        while(number >= 10000) {

            number -= 10000;
            ++output;
        }

        one_vram_buffer(output + NUMBER_TO_TILE, NTADR_A(x, y));
        firstNumber = TRUE;
        ++numberOffset;
        output = 0;
    //}
    
    //if(number >= 1000) {

        while(number >= 1000) {

            number -= 1000;
            ++output;
        }

        one_vram_buffer(output + NUMBER_TO_TILE, NTADR_A(x + 1, y));
        firstNumber = TRUE;
        ++numberOffset;
        output = 0;
    //} else if(firstNumber) ++numberOffset;

    //if(number >= 100) {

        while(number >= 100) {

            number -= 100;
            ++output;
        }

        one_vram_buffer(output + NUMBER_TO_TILE, NTADR_A(x + 2, y));
        firstNumber = TRUE;
        ++numberOffset;
        output = 0;
    //} else if(firstNumber) ++numberOffset;

    //if(number >= 10) {

        while(number >= 10) {

            number -= 10;
            ++output;
        }

        one_vram_buffer(output + NUMBER_TO_TILE, NTADR_A(x + 3, y));
        firstNumber = TRUE;
        ++numberOffset;
        output = 0;
    //} else if(firstNumber) ++numberOffset;
    
    one_vram_buffer(number + NUMBER_TO_TILE, NTADR_A(x + 4, y));
}

//relatively slow, clobbers 3 tiles regardless of whatever they are, uses temp2 = number in place value
void printNumber(uchar number, uchar x, uchar y) {

    uchar offset = 0; //also for if the number's begun in bit 7 (save a stack variable)
    temp2 = 0;

    if(number >= 100) {

        while(number >= 100) {

            number -= 100;
            ++temp2;
        }

        one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x, y));
        offset += 0b10000001;
    }

    temp2 = 0;
    if(number >= 10) {

        while(number >= 10) {

            number -= 10;
            ++temp2;
        }

        one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x + (offset & 0b11), y));
        offset++;
    } else if(offset & 0b10000000) offset++;

    offset &= ~(0b10000000); //clear the has number begun thingy
    one_vram_buffer(number + NUMBER_TO_TILE, NTADR_A(x + offset, y));

    ++offset;
    while(offset < 3) {

        one_vram_buffer(0, NTADR_A(x + offset, y));
        ++offset;
    }
}

//uses temp2, cursorX, cursorY, doesn't check if space is already a mine, todo make more efficient? dunno what else i could do
uchar countMinesAroundTileHard(void) {

    temp2 = 0;
    if(cursorX == 0) {

        if(cursorY == 0) {
            
            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorY;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
        } else if(cursorY == (HARD_MAX_Y - 1)) {

            ++cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
        } else {

            ++cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
        }
    } else if(cursorX == (HARD_MAX_X - 1)) {

        if(cursorY == 0) {

            --cursorX;
            temp2 += getTileIsMineHard();
            ++cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
        } else if(cursorY == (HARD_MAX_Y - 1)) {

            --cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
        } else {

            ++cursorY;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
        }
    } else {

        if(cursorY == 0) {

            --cursorX;
            temp2 += getTileIsMineHard();
            ++cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
        } else if(cursorY == (HARD_MAX_Y - 1)) {

            --cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorY;
            temp2 += getTileIsMineHard();
        } else {

            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorY;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
            --cursorX;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            --cursorY;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
            ++cursorX;
            temp2 += getTileIsMineHard();
        }
    }

    return temp2;
}

//kinda slow, todo make faster?
void generateBoard(void) {

    //temp0 = number of mines
    //tempShort0 = number of spaces left
    //boardWidth = board size x
    //temp1 = board size y
    //temp2 = array size to go through
    //temp3 = boardIsMine[global_i]

    gameMode |= 0b1 << 4; //set board is updating

    if((gameMode >> 3) & 0b1) { //is the game in hard mode?

        temp0 = HARD_NUM_MINES; //250 mines
        numFlags = HARD_NUM_MINES;
        //boardWidth = HARD_MAX_X;
        temp1 = HARD_MAX_Y;
        temp2 = BOARD_MEM_SIZE;
        tempShort0 = HARD_MAX_X * HARD_MAX_Y;
    } else {

        temp0 = EASY_NUM_MINES; //85 mines
        numFlags = EASY_NUM_MINES;
        //boardWidth = EASY_MAX_X;
        temp1 = EASY_MAX_Y;
        temp2 = EASY_BOARD_USED_MEM_SIZE;
        tempShort0 = EASY_MAX_X * EASY_MAX_Y;
    }

    for(global_i = 0; global_i < temp2; global_i++) {
        
        temp3 = 0; //clear the board
        for(global_j = 0; global_j < 8; global_j++) { //8 bits per byte

            updateRNGNoController();

            //Chance of it being a mine: num mines left / num spaces left
            //Mine put down if (rng % num spaces left) < num mines left

            temp3 = temp3 << 1;
            if((rngState.longState % tempShort0) < temp0) { //if randomness says to, add a mine

               --temp0; //remove 1 flag from the flags counter
               temp3 |= 0b1; //add a mine to the lsb of the byte
               ++numMinesSetInByte;
            }

            --tempShort0;
        }

        boardIsMine[global_i] = temp3;
        boardIsActivated[global_i] = 0;
        boardIsFlag[global_i] = 0;
    }

    gameMode &= ~(0b1 << 4); //clear board is updating
}

//returns the number of mines around it in temp2, uses temp2
void activateTile(void) {

    setTileIsActivatedHard(TRUE);

    countMinesAroundTileHard();
    one_vram_buffer(temp2 + NUMBER_TO_NUMBER_TILE, NTADR_A(tempTileX, tempTileY + 3));
}

//checks temp0, waits a frame if too many updates
void activateTileNoCount(void) {

    setTileIsActivatedHard(TRUE);
    one_vram_buffer(temp2 + NUMBER_TO_NUMBER_TILE, NTADR_A(cursorX, cursorY + 3));

    ++temp0;
    if(temp0 >= MAX_FLOOD_FILL_UPDATES) {

        ppu_wait_nmi();
        temp0 = 0;
    }
}

void pushCursorXY(void) {

    ++fillStackPos;
    fillStackX[fillStackPos] = cursorX;
    fillStackY[fillStackPos] = cursorY;
}

void popCursorXY(void) {

    cursorX = fillStackX[fillStackPos];
    cursorY = fillStackY[fillStackPos];
    --fillStackPos;
}

//separated into a different function to avoid the update function getting too big, uses temp0, could potentially take >1 frame
inline void floodFillZerosHard(void) {

    fillStackPos = 1;
    fillStackX[1] = tempTileX;
    fillStackY[1] = tempTileY;

    temp0 = 0;
    while(fillStackPos > 0) {

        cursorX = fillStackX[fillStackPos];
        cursorY = fillStackY[fillStackPos];

        if(getTileIsActivatedHard()) goto continyue;

        setTileIsActivatedHard(TRUE);
        one_vram_buffer(NUMBER_TO_NUMBER_TILE, NTADR_A(cursorX, cursorY + 3));

        ++temp0;
        if(temp0 >= MAX_FLOOD_FILL_UPDATES) {

            ppu_wait_nmi();
            temp0 = 0;
        }

        if(cursorX == 0) {

            if(cursorY == 0) {
                
                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {

                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                    
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                    
                pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else if(cursorY == (HARD_MAX_Y - 1)) {

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
                
                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else {

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
                
                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }
                
                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            }
        } else if(cursorX == (HARD_MAX_X - 1)) {

            if(cursorY == 0) {

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else if(cursorY == (HARD_MAX_Y - 1)) {

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else {

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            }
        } else {

            if(cursorY == 0) {

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else if(cursorY == (HARD_MAX_Y - 1)) {

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
                
                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                    
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
            } else {

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }
                
                ++cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                --cursorY;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 == 0) {

                        ++fillStackPos;
                        fillStackX[fillStackPos] = cursorX;
                        fillStackY[fillStackPos] = cursorY;
                    } else activateTileNoCount();
                }

                ++cursorX;
                if(!getTileIsMineHard() && !getTileIsActivatedHard()) {
                        
                    pushCursorXY();                     temp2 = countMinesAroundTileHard();                     popCursorXY();
                    if(temp2 != 0) activateTileNoCount();
                }
            }
        }
    
        continyue:

        --fillStackPos;
    }

    temp0 = 0;
}

//uses temp0
void hardUpdate(void) {

    //update cursor x and y, wrap around the screen as needed

    if(debugTemp0) {

        --debugTemp0;
        if(!debugTemp0) one_vram_buffer(0x09, PALETTE_MEMORY_BEGIN + 0x0); //make the screen normal again
    }

    //BEGIN using temp1 = is button pressed
    //used assembly for the bpl instruction, the compiler might not generate it if I compared the number to 255 or something like that

    __asm__("lda #$00");
    __asm__("sta %v", temp1);

    if(BUTTON_DOWN(PAD_RIGHT)) {

        __asm__("ldx %v", cursorX);
        __asm__("inx");
        __asm__("cpx #%b", HARD_MAX_X);
        __asm__("bne @padRightSkip");
        __asm__("ldx #$00");
        __asm__("@padRightSkip:");
        __asm__("stx %v", cursorX);
        __asm__("lda #%b", TRUE);
        __asm__("sta %v", temp1);
    }

    if(BUTTON_DOWN(PAD_LEFT)) {

        __asm__("ldx %v", cursorX);
        __asm__("dex");
        __asm__("bpl @padLeftSkip");
        __asm__("ldx #%b", HARD_MAX_X - 1);
        __asm__("@padLeftSkip:");
        __asm__("stx %v", cursorX);
        __asm__("lda #%b", TRUE);
        __asm__("sta %v", temp1);
    }

    if(BUTTON_DOWN(PAD_DOWN)) {

        __asm__("ldx %v", cursorY);
        __asm__("inx");
        __asm__("cpx #%b", HARD_MAX_Y);
        __asm__("bne @padDownSkip");
        __asm__("ldx #00");
        __asm__("@padDownSkip:");
        __asm__("stx %v", cursorY);
        __asm__("lda #%b", TRUE);
        __asm__("sta %v", temp1);
    }

    if(BUTTON_DOWN(PAD_UP)) {

        __asm__("ldx %v", cursorY);
        __asm__("dex");
        __asm__("bpl @padUpSkip");
        __asm__("ldx #%b", HARD_MAX_Y - 1);
        __asm__("@padUpSkip:");
        __asm__("stx %v", cursorY);
        __asm__("lda #%b", TRUE);
        __asm__("sta %v", temp1);
    }

    //BEGIN using temp2, printNumber vars

    if(temp1) {

        printNumber(cursorX, 1, 1);
        printNumber(cursorY, 4, 1);
    }

    //END using temp1

    hardSelectionSprite.xPos = cursorX << 3; //x8 to align w/ tiles
    hardSelectionSprite.yPos = (cursorY << 3) + 23; //1 less to be 1 higher (properly aligned)

    //alternate the color palette of the selector every 32 frames
    if((frameCount & 0b11111) == 0) {

        if((frameCount & 0b111111) == 0) one_vram_buffer(WHITE, PALETTE_MEMORY_BEGIN + 0x11);
        else one_vram_buffer(0x38, PALETTE_MEMORY_BEGIN + 0x11);
    }

    if(BUTTON_DOWN(PAD_B) && !getTileIsActivatedHard()) {

        if(getTileIsFlagHard()) {

            ++numFlags;
            setTileIsFlagHard(FALSE);
            one_vram_buffer(0x8 + (((cursorX & 0b1) ^ (cursorY & 0b1)) << 1), NTADR_A(cursorX, cursorY + 3));
        } else if(numFlags > 0) {

            --numFlags;
            setTileIsFlagHard(TRUE);
            one_vram_buffer(0x9 + (((cursorX & 0b1) ^ (cursorY & 0b1)) << 1), NTADR_A(cursorX, cursorY + 3));
        }

        printNumber(numFlags, 7, 1);
    }

    //END using temp2
    //BEGIN using temp2: mines around tile
    //BEGIN using temp0: number of flood-fill sprite updates this frame
    
    //cursor x and y are about to be clobbered so preserve them here
    tempTileX = cursorX;
    tempTileY = cursorY;

    //activating mines
    if(BUTTON_DOWN(PAD_A) && !getTileIsFlagHard() && !getTileIsActivatedHard()) {

        if(getTileIsMineHard()) {

            //oof
            one_vram_buffer(0x06, PALETTE_MEMORY_BEGIN + 0x0); //second of all, make the screen red (temp, todo remove)
            debugTemp0 = 5;
        } else {

            countMinesAroundTileHard();
            if(temp2 == 0) floodFillZerosHard(); //enjoy
            else {

                cursorX = tempTileX;
                cursorY = tempTileY;
                activateTileNoCount();
            }
        }

        //unclobber cursor x and y
        cursorX = tempTileX;
        cursorY = tempTileY;
    }

    oam_clear();
    oam_spr(hardSelectionSprite.xPos, hardSelectionSprite.yPos, hardSelectionSprite.tile, hardSelectionSprite.attributes);
}

inline void initState(void) {

    prevController = 0;
    controller = 0;

    rngState.byte1 = 0xC2;
    rngState.byte2 = 0xE3;

    frameCount = 0;

    hardSelectionSprite.tile = 0x00;
    hardSelectionSprite.attributes = 0b00000000;
}

void update(void) {

    updateController();
    updateRNG();

    hardUpdate();

    __asm__("inc %v", frameCount);
    __asm__("bne @frameCountNoOverflow"); //aka zero flag is set aka overflow happened
    frameCountOverflow = TRUE;
    return;

    __asm__("@frameCountNoOverflow:");
    frameCountOverflow = FALSE;
}

#endif