#ifndef LOGIC_H
#define LOGIC_H

#include "lib/neslib.h"
#include "lib/nesdoug.h"
#include "defs.h"
#include "titleScreenNametable.h"
#include "selectScreenNametable.h"
#include "gameEndScreenNametable.h"

#define BUTTON_PRESSED(button) controller & button
#define BUTTON_DOWN(button) (controller & button) && !(prevController & button)
#define BUTTON_UP(button) (prevController & button) && !(controller & button)

#define IS_GAME_HARD() (gameMode & (0b1u << 3))
#define IS_GAME_PAUSED() (gameMode & (0b1u << 5))

uchar countMinesAroundTile(void);
void activateTileNoCount(void);

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
unsigned short ushort_i;
uchar frameCount; //for things flashing on screen, is ok to set to 0 after screen transitions
uchar secondsFrameCount;

//uchar boardWidth;

randomState rngState;

uchar prevController;
uchar controller;

//bits 0-2: which screen you're on, 3: easy or hard mode, 4: board is updating, 5: is game paused, 6: was flag ever put down
uchar gameMode;

uchar cursorX;
uchar cursorY;

uchar tempTileX, tempTileY;

uchar numFlags;
unsigned short numTilesLeft;

uchar timePlayedSeconds;
uchar timePlayedMinutes;

uchar boardWidth, boardHeight;
uchar maxMines;
uchar minMines;

uchar isCustomSeed;
unsigned short customSeed;

void (*checkAdjacentTilesFunction)(void);

#pragma bss-name(pop)

union {

    sprite hardSelectionSprite;
    sprite easySelectionSprite;
} selectionSprite;

uchar boardIsMine[BOARD_MEM_SIZE];
uchar boardIsActivated[BOARD_MEM_SIZE];
uchar boardIsFlag[BOARD_MEM_SIZE];

//also used for some miscellaneous things (like tile counting)
uchar fillStackX[FILL_STACK_SIZE];
uchar fillStackY[FILL_STACK_SIZE];
uchar fillStackPos;
uchar tileBitShiftOffset; //5 for hard mode and 4 for easy mode

uchar attributeTableMirror[ATTRIBUTE_TABLE_SIZE];

sprite selectionArrow;
uchar selectionArrowIndex;
uchar selectionArrowNumIndices;

const uchar* selectionArrowPositionsX;
const uchar* selectionArrowPositionsY;
uchar* selectionArrowShouldUseArrow;
uchar isNumberArrowUp;

uchar selectionArrowTargetPosX;
uchar selectionArrowTargetPosY;

uchar numMines;
unsigned short boardSeed;

uchar* boardType;

uchar debugTemp0; //todo remove

const char gameBgPalette[] = {

    0x09, 0x1A, 0x0F, 0x30,
    0x09, 0x08, 0x19, 0x30,
    0x09, 0x08, 0x19, 0x21,
    0x09, 0x0F, 0x0A, 0x30
};

const char titleScreenBgPalette[] = {

    0x1B, 0x0F, 0x0F, 0x28,
    0x1B, 0x11, 0x22, 0x30,
    0x1B, 0x0F, 0x0F, 0x21,
    0x1B, 0x30, 0x0F, 0x30
};

const char gameEndPalette[] = {

    0x09, 0x0F, 0x0F, 0x30,
    0x09, 0x21, 0x05, 0x30,
    0x09, 0x22, 0x28, 0x13,
    0x09, 0x08, 0x19, 0x30
};

const char sprPalette[] = {

    0x09, 0x38, 0x27, 0x30,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

const uchar easySelectorMetasprite[] = {

    0, 0, 0x01, 0,
    8, 0, 0x01, 0 | OAM_FLIP_H,
    0, 8, 0x01, 0 | OAM_FLIP_V,
    8, 8, 0x01, 0 | OAM_FLIP_H | OAM_FLIP_V,
    128
};

const char pausedText[6] = "PAUSED";
const char leaveText[4] = "EXIT";
const char newGameText[8] = "NEW\x9GAME"; //\x9 is the blank tile
const char backText[12] = "BACK\x9TO\x9GAME";

const char pressStartText[11] = "PRESS\x9START";

const char youWonText[8] = "YOU\x9WON!";
const char gameOverText[10] = "GAME\x9OVER!";

const uchar gamePauseMenuPositionsX[] = {     MENU_X(12), MENU_X(12), MENU_X(26) };
const uchar gamePauseMenuPositionsY[] = {     MENU_Y(1),  MENU_Y(2),  MENU_Y(1) };
const uchar gamePauseMenuShouldUseArrow[] = { TRUE,       TRUE,       TRUE };

const uchar gameSelectPositionsX[] = { MENU_X(2), MENU_X(9) + 1, MENU_X(2),  MENU_X(2),  MENU_X(8) + 1, MENU_X(2) };
const uchar gameSelectPositionsY[] = { MENU_Y(7), MENU_Y(7),     MENU_Y(11), MENU_Y(15), MENU_Y(15),    MENU_Y(17) };
uchar gameSelectShouldUseArrow[] = {   TRUE,      TRUE,          FALSE,      TRUE,       TRUE,          TRUE }; //intentionally in ram so can be updated

void showTitleScreen(void);

#pragma region Random Functions

inline void updateController(void) {

    prevController = controller;
    controller = pad_poll(0);

    //mask out left + right and up + down inputs (i have been officially labeled anti-tas now)
    if(BUTTON_PRESSED(PAD_LEFT) && BUTTON_PRESSED(PAD_RIGHT)) controller &= ~(0b11u);
    if(BUTTON_PRESSED(PAD_UP) && BUTTON_PRESSED(PAD_DOWN)) controller &= ~(0b1100u);
}

//for calling at the start of the frame
inline void updateRNG(void) {

    __asm__("lda %v", rngState);
    __asm__("clc"); //todo maybe remove? depends if carry bit is uncertain at end of frame or not
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

#pragma endregion

#pragma region Tile Memory Updating

//uses tempShort0
uchar getTileBase() {

    tempShort0 = ((cursorY << tileBitShiftOffset) + cursorX); //bit offset
    return (boardType[tempShort0 >> 3] >> (tempShort0 & 0b111u)) & 0b1u;
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
void setTileBase(uchar value) {

    tempShort0 = ((cursorY << tileBitShiftOffset) + cursorX); //bit offset
    temp4 = tempShort0 >> 3;
    temp3 = tempShort0 & 0b111;
    boardType[temp4] = (boardType[temp4] & ~(0b1u << temp3)) | (value << temp3);
}

//uses tempShort0
inline uchar getTileIsMine(void) {

    boardType = boardIsMine;
    return getTileBase();
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsMine(uchar value) {

    boardType = boardIsMine;
    setTileBase(value);
}

//uses tempShort0
inline uchar getTileIsActivated(void) {

    boardType = boardIsActivated;
    return getTileBase();
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsActivated(uchar value) {

    boardType = boardIsActivated;
    setTileBase(value);
}

//uses tempShort0
inline uchar getTileIsFlag(void) {

    boardType = boardIsFlag;
    return getTileBase();
}

//uses tempShort0, temp3, temp4, make sure value is 0 or 1
inline void setTileIsFlag(uchar value) {

    boardType = boardIsFlag;
    setTileBase(value);
}

#pragma endregion

#pragma region Printing

//uses temp0, temp2 = number in place value, also deletes x - x + 3 tiles regardless of whatever they are, relatively slow
void printNumber(uchar number, uchar x, uchar y) {

    uchar offset = 0; //also for if the number's begun in bit 7 (save a stack variable)
    temp2 = 0;

    if(number >= 100) {

        while(number >= 100) {

            number -= 100;
            ++temp2;
        }

        one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x, y));
        ++temp0;
        offset += 0b10000001;
    }

    temp2 = 0;
    if(number >= 10) {

        while(number >= 10) {

            number -= 10;
            ++temp2;
        }

        one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x + (offset & 0b11), y));
        ++temp0;
        offset++;
    } else if(offset & 0b10000000) offset++;

    offset &= ~(0b10000000); //clear the has number begun thingy
    one_vram_buffer(number + NUMBER_TO_TILE, NTADR_A(x + offset, y));
    ++temp0;

    //overwrite any remaining tiles
    ++offset;
    while(offset < 3) {

        one_vram_buffer(0x9, NTADR_A(x + offset, y));
        ++offset;
        ++temp0;
    }
}

//uses temp2, for the seed
void printHexNumber(unsigned short number, uchar x, uchar y) {

    one_vram_buffer((uchar)(number >> 12) + NUMBER_TO_TILE, NTADR_A(x, y));

    ++x;
    one_vram_buffer((uchar)((number >> 8) & ~(0b11110000)) + NUMBER_TO_TILE, NTADR_A(x, y));

    ++x;
    one_vram_buffer((uchar)((number >> 4) & ~(0b111111110000)) + NUMBER_TO_TILE, NTADR_A(x, y));

    ++x;
    one_vram_buffer((uchar)(number & ~(0b1111111111110000)) + NUMBER_TO_TILE, NTADR_A(x, y));
}

//blanks 6 spaces no matter what, uses temp1, temp2, temp3, temp4
void printTime(uchar x, uchar y) {

    if(temp0 > (MAX_SCREEN_UPDATES_PER_FRAME - 12)) return; //ignore if too many pending screen updates
    
    temp1 = FALSE; //if there needs to be an extra blank tile at the end
    temp2 = 0; //place counter
    temp3 = timePlayedMinutes; //copied over from time played so the time played doesn't get modified
    temp4 = 0; //x offset for vram buffer

    if(timePlayedMinutes >= 10) {

        ++temp2;
        temp3 -= 10;
        while(temp3 >= 10) {

            ++temp2;
            temp3 -= 10;
        }

        one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x, y));
        ++temp4;
    } else temp1 = TRUE;

    one_vram_buffer(temp3 + NUMBER_TO_TILE, NTADR_A(x + temp4, y));
    ++temp4;

    one_vram_buffer(COLON, NTADR_A(x + temp4, y));
    ++temp4;

    temp2 = 0;
    temp3 = timePlayedSeconds; //copied over from time played so the time played doesn't get modified

    while(temp3 >= 10) {

        ++temp2;
        temp3 -= 10;
    }

    one_vram_buffer(temp2 + NUMBER_TO_TILE, NTADR_A(x + temp4, y));
    ++temp4;
    one_vram_buffer(temp3 + NUMBER_TO_TILE, NTADR_A(x + temp4, y));

    if(temp1) one_vram_buffer(BLANK_TILE, NTADR_A(x + temp4 + 1, y));
}

void printString(const char* string, uchar stringLength, uchar x, uchar y) {

    multi_vram_buffer_horz(string, stringLength, NTADR_A(x, y));
}

//prints a 2x2 tile (w/ ppu off)
void printTile(uchar tileStart, int address) {

    vram_adr(address);
    vram_put(tileStart);
    vram_put(tileStart + 1);
    vram_adr(address + 0x20);
    tileStart += 0x10;
    vram_put(tileStart);
    vram_put(tileStart + 1);
}

void clearScreen(uchar length, uchar x, uchar y) {

    multi_vram_buffer_horz_fill(BLANK_TILE, length, NTADR_A(x, y));
}

inline void clearScreenPrecalculated(uchar length, int address) {

    multi_vram_buffer_horz_fill(BLANK_TILE, length, address);
}

#pragma endregion

#pragma region Tile Graphics Updating

//can do whatever function is needed by using checkAdjacentTilesFunction, make sure to set it or else bad things will (might) happen
void checkAdjacentTiles(void) {

    if(cursorX == 0) {

        if(cursorY == 0) {
            
            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorY;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
        } else if(cursorY == (boardHeight - 1)) {

            ++cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
        } else {

            ++cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
        }
    } else if(cursorX == (boardWidth - 1)) {

        if(cursorY == 0) {

            --cursorX;
            checkAdjacentTilesFunction();
            ++cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
        } else if(cursorY == (boardHeight - 1)) {

            --cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
        } else {

            ++cursorY;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
        }
    } else {

        if(cursorY == 0) {

            --cursorX;
            checkAdjacentTilesFunction();
            ++cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
        } else if(cursorY == (boardHeight - 1)) {

            --cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorY;
            checkAdjacentTilesFunction();
        } else {

            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorY;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
            --cursorX;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            --cursorY;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
            ++cursorX;
            checkAdjacentTilesFunction();
        }
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

void _countMines(void) {

    temp2 += getTileIsMine();
}

void _checkFloodFillPos(void) {

    if(!getTileIsMine() && !getTileIsActivated()) {

        pushCursorXY();
        temp2 = countMinesAroundTile();
        popCursorXY();

        if(temp2 == 0) pushCursorXY();
        else activateTileNoCount();

        checkAdjacentTilesFunction = _checkFloodFillPos; //checkAdjacentTilesFunction got changed by countMinesAroundTile so change it back
    }
}

void _checkPos2(void) {

    if(getTileIsFlag()) ++temp2;
    else pushCursorXY();
}

//uses temp0, coords are cursorX + cursorY
void changeAttributeTable(uchar value) {

    ++fillStackPos;
    fillStackX[fillStackPos] = ((cursorY >> 1) << 3) + (cursorX >> 1) + 8;
    fillStackY[fillStackPos] = (((cursorX & 0b1) << 1) + ((cursorY & 0b1) << 2)); //bit offset
    attributeTableMirror[fillStackX[fillStackPos]] &= (~(0b11 << fillStackY[fillStackPos])); //clear the bits there...
    attributeTableMirror[fillStackX[fillStackPos]] |= (value << fillStackY[fillStackPos]); //...so they can be set
    one_vram_buffer(attributeTableMirror[fillStackX[fillStackPos]], 0x23C0 + fillStackX[fillStackPos]);
    ++temp0;
    --fillStackPos;
}

//uses temp2, cursorX, cursorY, checkAdjacentTileFunction, doesn't check if space is already a mine
uchar countMinesAroundTile(void) {

    temp2 = 0;

    checkAdjacentTilesFunction = _countMines;
    checkAdjacentTiles();

    return temp2;
}

void activateTileBase(void) {

    if(temp0 >= MAX_SCREEN_UPDATES_PER_FRAME) {

        ppu_wait_nmi();
        temp0 = 0;
    }

    if(IS_GAME_HARD()) {

        one_vram_buffer(temp2 + NUMBER_TO_NUMBER_TILE, NTADR_A(cursorX, cursorY + 4));
        ++temp0;
    } else {

        if(temp2 == 8) temp2 = 0x10; //quick hack to get 8-tiles to work properly (they're rare though, so doesn't really matter)

        one_vram_buffer(0x80 + (temp2 << 1), NTADR_A(cursorX << 1, (cursorY << 1) + 4));
        one_vram_buffer(0x81 + (temp2 << 1), NTADR_A((cursorX << 1) + 1, (cursorY << 1) + 4));
        one_vram_buffer(0x90 + (temp2 << 1), NTADR_A(cursorX << 1, (cursorY << 1) + 5));
        one_vram_buffer(0x91 + (temp2 << 1), NTADR_A((cursorX << 1) + 1, (cursorY << 1) + 5));

        //make the tile blue
        changeAttributeTable(0b11);
        /*++fillStackPos;
        fillStackX[fillStackPos] = ((cursorY >> 1) << 3) + (cursorX >> 1) + 8;
        attributeTableMirror[fillStackX[fillStackPos]] |= (0b11 << (((cursorX & 0b1) << 1) + ((cursorY & 0b1) << 2)));
        one_vram_buffer(attributeTableMirror[fillStackX[fillStackPos]], 0x23C0 + fillStackX[fillStackPos]);
        --fillStackPos;*/

        temp0 += 5;

        if(temp2 == 0x10) temp2 = 8; //restore temp2 if it got clobbered from the 8-tile shenanigans
    }

    --numTilesLeft;
}

//returns the number of mines around it in temp2, uses temp2, checks temp0, waits a frame if too many updates
void activateTile(void) {

    if(!getTileIsActivated()) {

        setTileIsActivated(TRUE);
        countMinesAroundTile();

        activateTileBase();
    }
}

//checks temp0, waits a frame if too many updates
void activateTileNoCount(void) {

    if(!getTileIsActivated() && !getTileIsFlag()) {

        setTileIsActivated(TRUE);
        activateTileBase();
    }
}

//separated into a different function to avoid the update function getting too big, uses temp0, could potentially take >1 frame
inline void floodFillZeros(void) {

    unsigned char currentStackPos = fillStackPos;

    ++fillStackPos;
    fillStackX[fillStackPos] = tempTileX;
    fillStackY[fillStackPos] = tempTileY;

    temp0 = 0;

    while(fillStackPos != currentStackPos) {

        cursorX = fillStackX[fillStackPos];
        cursorY = fillStackY[fillStackPos];
        
        --fillStackPos;

        //resets the stack, gives it another chance to complete the fill (usually gets like 90% of the way there)
        if(fillStackPos >= (FILL_STACK_SIZE - 0x4)) {
            
            fillStackPos = currentStackPos + 1;
            fillStackX[fillStackPos] = cursorX;
            fillStackY[fillStackPos] = cursorY;
        }

        if(getTileIsActivated()) continue;

        if(!getTileIsFlag()) {

            setTileIsActivated(TRUE);
            
            if(temp0 >= MAX_SCREEN_UPDATES_PER_FRAME) {

                ppu_wait_nmi();
                temp0 = 0;
            }

            //anything here will always be a 0 tile, which allows for some shortcuts
            if(IS_GAME_HARD()) {

                one_vram_buffer(NUMBER_TO_NUMBER_TILE, NTADR_A(cursorX, cursorY + 4));
                ++temp0;
            } else {

                one_vram_buffer(0x80, NTADR_A(cursorX << 1, (cursorY << 1) + 4));
                one_vram_buffer(0x81, NTADR_A((cursorX << 1) + 1, (cursorY << 1) + 4));
                one_vram_buffer(0x90, NTADR_A(cursorX << 1, (cursorY << 1) + 5));
                one_vram_buffer(0x91, NTADR_A((cursorX << 1) + 1, (cursorY << 1) + 5));

                changeAttributeTable(0b11);

                /*++fillStackPos;
                fillStackX[fillStackPos] = ((cursorY >> 1) << 3) + (cursorX >> 1) + 8;
                attributeTableMirror[fillStackX[fillStackPos]] |= (0b11 << (((cursorX & 0b1) << 1) + ((cursorY & 0b1) << 2)));
                one_vram_buffer(attributeTableMirror[fillStackX[fillStackPos]], 0x23C0 + fillStackX[fillStackPos]);
                --fillStackPos;*/

                temp0 += 5;
            }

            --numTilesLeft;
        }

        checkAdjacentTilesFunction = _checkFloodFillPos;
        checkAdjacentTiles();
    }
}

#pragma endregion

#pragma region Game Update

//uses temp0, temp1, temp2, temp3, temp4, temp5, tempShort0
inline void gameUpdateDifficultyIndependent(void) {

    //update cursor x and y, wrap around the screen as needed

    if(debugTemp0) {

        --debugTemp0;
        if(!debugTemp0) one_vram_buffer(0x09, PALETTE_MEMORY_BEGIN + 0x0); //make the screen normal again
    }

    //BEGIN using temp3 = some temp stuff
    //used assembly for the bpl instruction, the compiler might not generate it if I compared the number to 255 or something like that

    if(BUTTON_DOWN(PAD_RIGHT)) {

        __asm__("ldx %v", cursorX);
        __asm__("inx");
        __asm__("cpx %v", boardWidth);
        __asm__("bne @padRightSkip");
        __asm__("ldx #$00");
        __asm__("@padRightSkip:");
        __asm__("stx %v", cursorX);
    }

    if(BUTTON_DOWN(PAD_LEFT)) {

        __asm__("ldx %v", cursorX);
        __asm__("dex");
        __asm__("bpl @padLeftSkip");
        __asm__("ldx %v", boardWidth);
        __asm__("dex");
        __asm__("@padLeftSkip:");
        __asm__("stx %v", cursorX);
    }

    if(BUTTON_DOWN(PAD_DOWN)) {

        __asm__("ldx %v", cursorY);
        __asm__("inx");
        __asm__("cpx %v", boardHeight);
        __asm__("bne @padDownSkip");
        __asm__("ldx #00");
        __asm__("@padDownSkip:");
        __asm__("stx %v", cursorY);
    }

    if(BUTTON_DOWN(PAD_UP)) {

        __asm__("ldx %v", cursorY);
        __asm__("dex");
        __asm__("bpl @padUpSkip");
        __asm__("ldx %v", boardHeight);
        __asm__("dex");
        __asm__("@padUpSkip:");
        __asm__("stx %v", cursorY);
    }

    //BEGIN using temp2, printNumber vars
    //END using temp3

    //END using temp1

    //alternate the color palette of the selector every 32 frames
    if((frameCount & 0b11111) == 0) {

        if((frameCount & 0b111111) == 0) one_vram_buffer(WHITE, PALETTE_MEMORY_BEGIN + 0x11);
        else one_vram_buffer(0x38, PALETTE_MEMORY_BEGIN + 0x11);
    }

    //BEGIN using temp1, temp3 for temp coordinates

    if(BUTTON_DOWN(PAD_B) && !getTileIsActivated()) {

        if(getTileIsFlag()) {

            ++numFlags;
            setTileIsFlag(FALSE);

            if(IS_GAME_HARD()) {

                one_vram_buffer(0x9 + (((cursorX & 0b1) ^ (cursorY & 0b1)) << 1), NTADR_A(cursorX, cursorY + 4));
                ++temp0;
            } else {

                temp1 = cursorX << 1;
                temp3 = (cursorY << 1) + 4;
                one_vram_buffer(0xA2, NTADR_A(temp1, temp3));
                one_vram_buffer(0xA3, NTADR_A(temp1 + 1, temp3));

                ++temp3;
                one_vram_buffer(0xB2, NTADR_A(temp1, temp3));
                one_vram_buffer(0xB3, NTADR_A(temp1 + 1, temp3));

                changeAttributeTable(0b01);

                /*++fillStackPos;
                fillStackX[fillStackPos] = ((cursorY >> 1) << 3) + (cursorX >> 1) + 8;
                fillStackY[fillStackPos] = (((cursorX & 0b1) << 1) + ((cursorY & 0b1) << 2)); //bit offset
                attributeTableMirror[fillStackX[fillStackPos]] &= (~(0b11 << fillStackY[fillStackPos]));
                attributeTableMirror[fillStackX[fillStackPos]] |= (0b10 << fillStackY[fillStackPos]);
                one_vram_buffer(attributeTableMirror[fillStackX[fillStackPos]], 0x23C0 + fillStackX[fillStackPos]);
                --fillStackPos;*/

                temp0 += 5;
            }
        } else if(numFlags > 0) {

            --numFlags;
            setTileIsFlag(TRUE);
            gameMode |= (0b1 << 6); //set that a flag was put down

            if(IS_GAME_HARD()) {

                one_vram_buffer(0xA + (((cursorX & 0b1) ^ (cursorY & 0b1)) << 1), NTADR_A(cursorX, cursorY + 4));
                ++temp0;
            } else {

                temp1 = cursorX << 1;
                temp3 = (cursorY << 1) + 4;
                one_vram_buffer(0xA4, NTADR_A(temp1, temp3));
                one_vram_buffer(0xA5, NTADR_A(temp1 + 1, temp3));

                ++temp3;
                one_vram_buffer(0xB4, NTADR_A(temp1, temp3));
                one_vram_buffer(0xB5, NTADR_A(temp1 + 1, temp3));

                changeAttributeTable(0b10);

                /*++fillStackPos;
                fillStackX[fillStackPos] = ((cursorY >> 1) << 3) + (cursorX >> 1) + 8;
                fillStackY[fillStackPos] = (((cursorX & 0b1) << 1) + ((cursorY & 0b1) << 2)); //bit offset
                attributeTableMirror[fillStackX[fillStackPos]] &= (~(0b11 << fillStackY[fillStackPos]));
                attributeTableMirror[fillStackX[fillStackPos]] |= (0b01 << fillStackY[fillStackPos]);
                one_vram_buffer(attributeTableMirror[fillStackX[fillStackPos]], 0x23C0 + fillStackX[fillStackPos]);
                --fillStackPos;*/

                temp0 += 5;
            }
        }

        printNumber(numFlags, 2, 3);
    }

    //END using temp1, temp2, temp3
    //BEGIN using temp0: number of flood-fill sprite updates this frame
    //BEGIN using temp1: saved mined around tile while temp2 is used for other things
    //BEGIN using temp2: mines around tile
    
    //cursor x and y are about to be clobbered so preserve them here
    tempTileX = cursorX;
    tempTileY = cursorY;

    //activating tiles
    if(!getTileIsFlag() && !getTileIsActivated()) {

        if(BUTTON_DOWN(PAD_A)) {

            if(getTileIsMine()) {

                //first of all, oof
                one_vram_buffer(0x06, PALETTE_MEMORY_BEGIN + 0x0); //second of all, make the screen red (temp, todo remove)
                debugTemp0 = 5;
            } else {

                pushCursorXY();
                countMinesAroundTile();
                popCursorXY();

                if(temp2 == 0) floodFillZeros(); //enjoy
                else activateTileNoCount();
            }
        }

        //unclobber cursor x and y
        cursorX = tempTileX;
        cursorY = tempTileY;
    }

    //when select is pressed, activate all surrounding tiles if the number of flags around a tile is equivalent to the tile number
    if(BUTTON_DOWN(PAD_SELECT) && getTileIsActivated()) {

        pushCursorXY();
        temp2 = countMinesAroundTile();
        popCursorXY();

        //save num mines around tile
        temp1 = temp2;

        ++fillStackPos;
        fillStackX[fillStackPos] = 0xFF; //end of list of non-mine tiles

        //get num flags around tile
        temp2 = 0;
        checkAdjacentTilesFunction = _checkPos2;
        checkAdjacentTiles();

        if(temp2 == temp1) {

            while(fillStackX[fillStackPos] != 0xFF) {

                popCursorXY();

                if(getTileIsMine()) {

                    //oof
                    one_vram_buffer(0x06, PALETTE_MEMORY_BEGIN + 0x0); //second of all, make the screen red (temp, todo remove, make screen be destroyed)
                    debugTemp0 = 5;
                } else {

                    pushCursorXY();
                    countMinesAroundTile();
                    popCursorXY();

                    ++fillStackPos;
                    fillStackX[fillStackPos] = tempTileX;
                    fillStackY[fillStackPos] = tempTileY;

                    tempTileX = cursorX;
                    tempTileY = cursorY;

                    if(temp2 == 0) floodFillZeros();
                    else activateTileNoCount();

                    tempTileX = fillStackX[fillStackPos];
                    tempTileY = fillStackY[fillStackPos];
                    --fillStackPos;
                }
            }

            --fillStackPos;
        }

        //unclobber cursor x and y
        cursorX = tempTileX;
        cursorY = tempTileY;
    }

    //END using temp0, temp1, temp2, temp3, temp4, tempShort0 (whew)

    if(numTilesLeft == 0) {

        //horray! good job

        oam_clear();
        ppu_wait_nmi();

        color_emphasis(COL_EMP_GREEN);
        pal_bg_bright(7);
        delay(NUM_FLASH_FRAMES);

        color_emphasis(COL_EMP_NORMAL);
        pal_bright(4);
        delay(NUM_WAIT_FRAMES);

        pal_fade_to(4, 0);

        ppu_off();
        vram_adr(NAMETABLE_A);
        vram_fill(BLANK_TILE, 32 * 30);
        vram_adr(NAMETABLE_A);
        vram_unrle(gameEndScreenNametable);
        pal_bg(gameEndPalette);

        //add game features
        if(!IS_GAME_HARD()) {

            printTile(0x82, NTADR_A(18, 12));

            vram_adr(0x23DC); //get the right palette
            vram_put(0b01001100);
        }

        if(!(gameMode & (0b1 << 6))) {

            printTile(0xA6, NTADR_A(18, 14));
        }

        if(gameMode & (0b1 << 4)) {

            printTile(0xA8, NTADR_A(18, 16));
        }

        pal_bright(0);
        ppu_on_all();

        one_vram_buffer(0b00001100, 0x23F4);
        printString(youWonText, sizeof(youWonText), 12, 10);
        printTime(11, 14);
        printHexNumber(boardSeed, 11, 18);
        one_vram_buffer(OPENING_BRACKET, NTADR_A(16, 18));

        temp0 = 0;
        printNumber(numMines, 17, 18);
        
        if(numMines < 10) temp1 = 1;
        else if(numMines < 100) temp1 = 2;
        else temp1 = 3;

        one_vram_buffer(CLOSING_BRACKET, NTADR_A(17 + temp1, 18));

        delay(60);

        pal_fade_to(0, 4);

        gameMode &= ~(0b111); //set to game end screen
        gameMode |= 0b011;
 
        temp0 = TRUE;

        return;
    }

    //pause the game if the start button is pressed
    if(BUTTON_DOWN(PAD_START)) {

        //there's gonna be lots of updates here so make sure there's enough space for them
        if(temp0 > 0) {
            
            ppu_wait_nmi();
            temp0 = 0;
        }

        gameMode |= (0b1 << 5); //make the gamemode paused

        //set up the correct selection arrow variables
        selectionArrowIndex = 0;
        selectionArrowNumIndices = sizeof(gamePauseMenuPositionsX);
        selectionArrowPositionsX = gamePauseMenuPositionsX;
        selectionArrowPositionsY = gamePauseMenuPositionsY;
        selectionArrowShouldUseArrow = gamePauseMenuShouldUseArrow;

        printString(pausedText, sizeof(pausedText), 12, 1);
        printString(backText, sizeof(backText), 13, 2);
        printString(newGameText, sizeof(newGameText), 13, 3);
        printString(leaveText, sizeof(leaveText), 27, 2);

        selectionArrow.xPos = gamePauseMenuPositionsX[0];
        selectionArrow.yPos = gamePauseMenuPositionsY[0];
    }

    //BEGIN using temp1, temp2, temp3, temp4 = temps for the write time function

    ++secondsFrameCount;
    if(secondsFrameCount == 60) {

        secondsFrameCount = 0;
        ++timePlayedSeconds;
        if(timePlayedSeconds == 60) {

            ++timePlayedMinutes;
            if(timePlayedMinutes == 60) { //cap the timer at 59:59
                
                timePlayedMinutes = 59;
                timePlayedSeconds = 59;
            } else timePlayedSeconds = 0;
        }

        printTime(7, 3);
    }

    //END all above (i don't want to write them out again)

    temp0 = FALSE;
}

inline void gameUpdateHard(void) {

    if(temp0) {

        temp0 = FALSE;
        return;
    }

    selectionSprite.hardSelectionSprite.xPos = cursorX << 3; //x8 to align w/ tiles
    selectionSprite.hardSelectionSprite.yPos = (cursorY << 3) + 31; //1 less to be 1 higher (properly aligned)

    oam_clear();
    oam_spr(selectionSprite.hardSelectionSprite.xPos, selectionSprite.hardSelectionSprite.yPos,
    selectionSprite.hardSelectionSprite.tile, selectionSprite.hardSelectionSprite.attributes);
}

inline void gameUpdateEasy(void) {

    if(temp0) {

        temp0 = FALSE;
        return;
    }

    selectionSprite.easySelectionSprite.xPos = cursorX << 4;
    selectionSprite.easySelectionSprite.yPos = (cursorY << 4) + 31; //1 less to be 1 higher (properly aligned)

    oam_clear();
    oam_meta_spr(selectionSprite.easySelectionSprite.xPos, selectionSprite.easySelectionSprite.yPos, easySelectorMetasprite);
}

#pragma endregion

#pragma region Board Generation

//uses temp0, temp1, temp2, temp3, tempShort0, kinda slow, todo make faster
void generateBoard(void) {

    //temp0 = number of mines left to place
    //tempShort0 = number of spaces left
    //temp1 = board size y
    //temp2 = array size to go through
    //temp3 = boardIsMine[global_i]

    //gameMode |= 0b1 << 4; //set board is updating

    if(IS_GAME_HARD()) {

        temp0 = numMines; //200 mines by default
        numFlags = numMines;
        numTilesLeft = (HARD_MAX_X * HARD_MAX_Y) - numMines;
        temp1 = HARD_MAX_Y;
        temp2 = BOARD_MEM_SIZE;
        tempShort0 = HARD_MAX_X * HARD_MAX_Y;
    } else { //otherwise game is in easy mode

        temp0 = numMines; //60 mines by default
        numFlags = numMines;
        numTilesLeft = (EASY_MAX_X * EASY_MAX_Y) - numMines;
        temp1 = EASY_MAX_Y;
        temp2 = EASY_BOARD_USED_MEM_SIZE;
        tempShort0 = EASY_MAX_X * EASY_MAX_Y;
    }

    gameMode &= ~(0b1 << 6); //clear if flag was ever put down

    if(isCustomSeed) rngState.longState = customSeed;

    //save the board seed for later
    boardSeed = rngState.longState;

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
            }

            --tempShort0;
        }

        boardIsMine[global_i] = temp3;
        boardIsActivated[global_i] = 0;
        boardIsFlag[global_i] = 0;
    }

    gameMode &= ~(0b1 << 4); //clear board is updating
}

//uses global_i, global_j, temp1, temp2, do when the ppu is off
void displayBoard(void) {

    pal_bg(gameBgPalette); //set stuff up for the actual game
    pal_spr(sprPalette);
    bank_spr(1);

    vram_adr(NTADR_A(0, 0)); //clear the screen
    vram_fill(BLANK_TILE, 32 * 30);

    printTile(0x60, NTADR_A(0, 2)); //draw the flag and the clock
    printTile(0x62, NTADR_A(5, 2));

    if(IS_GAME_HARD()) {

        for(global_i = 0; global_i < boardWidth; ++global_i) {

            for(global_j = 0; global_j < boardHeight; ++global_j) {
                
                vram_adr(NTADR_A(global_i, global_j + 4));
                vram_put(0x9 + (((global_i & 0b1) ^ (global_j & 0b1)) << 1)); //alternates as a checkerboard
            }
        }

        temp2 = 0b00000000; //palette should be 00
    } else {

        for(global_i = 0; global_i < boardWidth; ++global_i) {

            for(global_j = 0; global_j < boardHeight; ++global_j) {
                
                //uncomment all below to show mines when board is rendered
                /*cursorX = global_i;
                cursorY = global_j;
                temp4 = getTileIsMine() << 1;*/

                printTile(0xA2, NTADR_A(global_i << 1, (global_j << 1) + 4));
                /*vram_adr(NTADR_A(global_i << 1, (global_j << 1) + 4));
                vram_put(0xA2 + temp4);
                vram_put(0xA3 + temp4);
                vram_adr(NTADR_A(global_i << 1, (global_j << 1) + 5));
                vram_put(0xB2 + temp4);
                vram_put(0xB3 + temp4);*/
            }
        }

        temp2 = 0b01010101; //palette should be 01 by default
    }

    //make the correct palettes with magic™
    vram_adr(0x23C0);
    for(global_i = 0; global_i < 8; ++global_i) {

        for(global_j = 0; global_j < 8; ++global_j) {

            vram_put(temp2);
            attributeTableMirror[(global_j << 3) + global_i] = temp2;
        }
    }

    vram_adr(0x23C0); //make the flag in the top left blue, and the clock black
    vram_put(0b00100010);
    vram_put(0b00000000);
    vram_put(0b00000000);

    ppu_on_all();
    ppu_wait_nmi();

    //pre-activate some tiles at the start to make it less tedious

    if(numMines == 0) goto finished; //AAAUGH HE USED GOTO ARREST HIM

    for(cursorX = 0; cursorX < boardWidth; ++cursorX) {
        
        for(cursorY = 0; cursorY < boardHeight; ++cursorY) {

            if(getTileIsMine()) continue;

            pushCursorXY();
            countMinesAroundTile();
            popCursorXY();

            if(temp2 == 0) {

                tempTileX = cursorX;
                tempTileY = cursorY;

                floodFillZeros();
                goto finished;
            }
        }
    }

    if(IS_GAME_HARD()) {

        if((HARD_MAX_X * HARD_MAX_Y) - numMines <= 1) goto finished;
    } else if((EASY_MAX_X * EASY_MAX_Y) - numMines <= 1) goto finished;

    for(cursorX = 0; cursorX < boardWidth; ++cursorX) {
        
        for(cursorY = 0; cursorY < boardHeight; ++cursorY) {

            if(!getTileIsMine()) {

                activateTile();
                goto finished;
            }
        }
    }

    finished:

    cursorX = 0;
    cursorY = 0;

    if(temp0 >= (MAX_SCREEN_UPDATES_PER_FRAME - 18)) {

        ppu_wait_nmi();
        temp0 = 0;
    }

    printNumber(numFlags, 2, 3);
    printTime(7, 3);
}

#pragma endregion

#pragma region Miscellaneous

inline void setToEasyMode(void) {

    gameMode &= ~(0b1 << 3);
    tileBitShiftOffset = 4;

    boardWidth = EASY_MAX_X;
    boardHeight = EASY_MAX_Y;

    numMines = EASY_NUM_MINES;
    maxMines = (gameMode & (0b1 << 4)) ? 255 : EASY_MAX_MINES;
}

inline void setToHardMode(void) {

    gameMode |= (0b1 << 3);
    tileBitShiftOffset = 5;

    boardWidth = HARD_MAX_X;
    boardHeight = HARD_MAX_Y;
    numMines = HARD_NUM_MINES;
    maxMines = (gameMode & (0b1 << 4)) ? 255 : HARD_MAX_MINES;
}

//call while ppu is off
inline void initState(void) {

    prevController = 0;
    controller = 0;

    gameMode = 0b00000000; //on title screen by default, no other modifiers

    rngState.byte1 = 0xB1;
    rngState.byte2 = 0x68;

    frameCount = 0;

    selectionSprite.hardSelectionSprite.tile = 0x00;
    selectionSprite.hardSelectionSprite.attributes = 0b00000000;

    selectionArrow.tile = 0x02;
    selectionArrow.attributes = 0b000000000;
    
    setToEasyMode();
    isCustomSeed = FALSE;
    customSeed = 0;
    isNumberArrowUp = TRUE;

    pal_spr(sprPalette);
    bank_spr(1);
}

inline void newGame(void) {

    ppu_off();

    clear_vram_buffer(); //clear any pending vram updates...
    oam_clear(); //...and any remaining sprites
    gameMode &= ~(0b1 << 5); //set game not paused (for if pressed from the pause menu)
    gameMode = (gameMode & ~(0b111)) | 0b001; //set the gamemode to playing a game (gamemode 1)

    cursorX = 0;
    cursorY = 0;
    fillStackPos = 0;
    frameCount = 0;
    selectionArrowIndex = 0;
    
    secondsFrameCount = 0;
    timePlayedSeconds = 0;
    timePlayedMinutes = 0;

    generateBoard();
    displayBoard();
}

//uses temp4, temp3
void updateSelectionArrow(void) {

    temp4 = selectionArrowShouldUseArrow[selectionArrowIndex]; //the arrow shouldn't be able to be moved left to right when in this mode

    if(!temp3) {

        if(BUTTON_DOWN(PAD_DOWN) || (BUTTON_DOWN(PAD_RIGHT) && temp4)) {

            ++selectionArrowIndex;
            if(selectionArrowIndex == selectionArrowNumIndices) selectionArrowIndex = 0;
        } else if(BUTTON_DOWN(PAD_UP) || (BUTTON_DOWN(PAD_LEFT) && temp4)) {

            __asm__("ldx %v", selectionArrowIndex);
            __asm__("dex");
            __asm__("bpl @dontUnderflow");
            __asm__("lda %v", selectionArrowNumIndices);
            __asm__("tax");
            __asm__("dex");
            __asm__("@dontUnderflow:");
            __asm__("stx %v", selectionArrowIndex);
        }
    }

    temp3 = FALSE;

    oam_clear();

    if(selectionArrowShouldUseArrow[selectionArrowIndex]) {

        selectionArrowTargetPosX = selectionArrowPositionsX[selectionArrowIndex];
        selectionArrowTargetPosY = selectionArrowPositionsY[selectionArrowIndex];

        //update y pos

        temp4 = selectionArrowTargetPosY - selectionArrow.yPos;
        ++fillStackPos;
        fillStackX[fillStackPos] = temp4; //save it

        temp4 >>= 1;

        if(temp4 & (0b1 << 6)) { //if negative, bit 6 instead of 7 because temp4 was just bitshifted by 1 (div by 2)

            --temp4; //make temp4 negative (but treat it as a 7 bit number, kinda weird but whatever)
            temp4 ^= 0b01111111;
            selectionArrow.yPos -= temp4;

            //this approach will usually lead to being 1 off with no way to get there, so make it get there if it's 1 away
            if(fillStackX[fillStackPos] == 1) --selectionArrow.yPos;
        } else {

            selectionArrow.yPos += temp4;
            
            if(fillStackX[fillStackPos] == 0b11111111) ++selectionArrow.yPos;
        }

        temp4 = selectionArrowTargetPosX - selectionArrow.xPos;
        fillStackX[fillStackPos] = temp4; //save it

        temp4 >>= 1; //divide by 2 to make a smooth effect

        if(temp4 & (0b1 << 6)) { //checks if temp4 is negative, bit 6 instead of 7 because temp4 was just bitshifted by 1 (div by 2)

            --temp4; //make temp4 negative, 2's complement style (but treat it as a 7 bit number per above)
            temp4 ^= 0b01111111;
            selectionArrow.xPos -= temp4;

            //this approach will usually lead to being 1 off with no way to get there, so make it get there if it's 1 away
            if(fillStackX[fillStackPos] == 1) --selectionArrow.xPos;
        } else {

            selectionArrow.xPos += temp4;
            
            if(fillStackX[fillStackPos] == 0b11111111) ++selectionArrow.xPos;
        }

        --fillStackPos;
        oam_spr(selectionArrow.xPos, selectionArrow.yPos, selectionArrow.tile, selectionArrow.attributes);
    }
}

/*

//uses the tile in temp1
void setRleTile(void) {

    one_vram_buffer(temp1, NTADR_A(global_i, global_j));
    ++temp0;
    if(temp0 >= MAX_SCREEN_UPDATES_PER_FRAME) {

        ppu_wait_nmi();
        temp0 = 0;
    }
}

//uses temp1, temp2, temp3, ushort_i, global_i, global_j, expects that x and y are divisible by 2
void unrleWithCoords(uchar x, uchar y, uchar* data) {

    global_i = x;
    global_j = y;
    temp2 = FALSE;
    temp3 = 0;

    while(TRUE) {

        switch(temp1 & 0b11) {

            case RLE_SINGLE_TILE:

            temp1 >>= 2;
            setRleTile();
            break;

            case RLE_MULTI_TILE:

            temp1 >>= 2; //extract length
            temp3 = temp1;
            ++ushort_i;
            temp1 = data[ushort_i]; //extract tile type

            while(temp3 > 0) {

                setRleTile();
                --temp3;
            }

            break;

            case RLE_NEW_LINE:

            ++global_j;
            break;

            case RLE_ATT_TABLE:

            goto doAttributeData;
        }

        ++global_i;
        ++ushort_i;
    }

    doAttributeData:

    global_i = x >> 1;
    global_j = y >> 1;

    while(TRUE) {

        temp1 = data[ushort_i];
        if(temp2) { //attribute data

            switch(temp1 & 0b11) {

                case RLE_SINGLE_TILE:

                ++ushort_i;
                
                one_vram_buffer(data[ushort_i], 0x23C0 + ((global_j << 3) + global_i));
                
                ++temp0;
                if(temp0 >= MAX_SCREEN_UPDATES_PER_FRAME) {

                    ppu_wait_nmi();
                    temp0 = 0;
                }

                break;

                case RLE_MULTI_TILE:

                temp1 >>= 2;
                temp3 = temp1;

                ++ushort_i;
                temp1 = data[ushort_i];

                while(temp3 > 0) {

                    one_vram_buffer(temp1, 0x23C0 + ((global_j << 3) + global_i));
                    --temp3;
                    
                    ++temp0;
                    if(temp0 >= MAX_SCREEN_UPDATES_PER_FRAME) {

                        ppu_wait_nmi();
                        temp0 = 0;
                    }
                }

                break;

                case RLE_NEW_LINE:

                ++global_j;
                break;

                case RLE_ATT_TABLE:

                return; //finished (phew)
            }
        }

        ++global_i;
        ++ushort_i;
    }
}*/

#pragma endregion

#pragma region Paused Game

inline void gameUpdatePaused(void) {

    updateSelectionArrow();

    if(BUTTON_DOWN(PAD_START)) {

        switch(selectionArrowIndex) {

            case 0: //back to game

            clearScreen(sizeof(pausedText), 12, 1);
            clearScreen(sizeof(backText), 13, 2);
            clearScreen(sizeof(newGameText), 13, 3);
            clearScreen(sizeof(leaveText), 27, 2);
            gameMode &= ~(0b1 << 5);
            break;

            case 1: //new game

            isCustomSeed = FALSE;
            oam_clear();
            newGame();

            break;

            case 2: //back to title screen

            gameMode &= ~(0b111); //set the gamemode to title screen (mode 0)
            oam_clear();
            showTitleScreen();

            break;
        }
    }
}

#pragma endregion

#pragma region Title and Game Select Screens

void showTitleScreen(void) {

    ppu_off();
    vram_adr(NAMETABLE_A);
    vram_fill(BLANK_TILE, 32 * 30);
    vram_adr(NAMETABLE_A);
    vram_unrle(titleScreenNametable);
    pal_bg(titleScreenBgPalette);
    ppu_on_all();
}

void updateTitleScreen(void) {

    //flash the press start text every 32 frames
    if((frameCount & 0b11111) == 0) {

        if((frameCount & 0b111111) == 0) printString(pressStartText, sizeof(pressStartText), 10, 17);
        else clearScreen(sizeof(pressStartText), 10, 17);
    }

    if(BUTTON_DOWN(PAD_START)) {

        loadSelectScreen();
    }
}

void showSelectScreen(void) {

    ppu_off();
    vram_adr(NAMETABLE_A);
    vram_fill(BLANK_TILE, 32 * 30);
    vram_adr(NAMETABLE_A);
    vram_unrle(selectScreenNametable);
    pal_bg(titleScreenBgPalette);
    ppu_on_all();

    printNumber(numMines, 6, 12);
}

void loadSelectScreen(void) {

    gameMode &= ~(0b111);
    gameMode |= 0b010; //set the gamemode to choose type of game

    selectionArrowIndex = 0;
    selectionArrowNumIndices = sizeof(gameSelectPositionsX);
    selectionArrowPositionsX = gameSelectPositionsX;
    selectionArrowPositionsY = gameSelectPositionsY;
    selectionArrowShouldUseArrow = gameSelectShouldUseArrow;

    selectionArrow.xPos = gameSelectPositionsX[0];
    selectionArrow.yPos = gameSelectPositionsY[0];

    setToEasyMode();
    showSelectScreen();
}

//uses temp0, temp1, temp2
void updateGameSelection(void) {

    printNumber(numMines, 6, 12);
    if(isCustomSeed) printHexNumber(customSeed, 14, 16);
    else clearScreenPrecalculated(4, NTADR_A(14, 16));

    clearScreenPrecalculated(4, NTADR_A(14, 17));

    if(selectionArrowIndex == 2) {

        gameSelectShouldUseArrow[3] = TRUE;
        
        if(BUTTON_DOWN(PAD_RIGHT)) {
            
            if(isNumberArrowUp) isNumberArrowUp = FALSE;
            else {

                ++selectionArrowIndex;
                temp3 = TRUE;
                goto skipNA;
            }
        } else if(BUTTON_DOWN(PAD_LEFT)) {

            if(!isNumberArrowUp) isNumberArrowUp = TRUE;
            else {

                --selectionArrowIndex;
                temp3 = TRUE;
                goto skipNA;
            }
        }

        temp0 = BUTTON_DOWN(PAD_A);
        temp1 = BUTTON_DOWN(PAD_B);

        if(isNumberArrowUp) {

            one_vram_buffer(0x1E, NTADR_A(3, 12));
            one_vram_buffer(0x0F, NTADR_A(4, 12));

            if(temp0 && numMines <= (minMines - 1)) ++numMines;
            if(temp1 && numMines <= (minMines - 10)) numMines += 10;
        } else {

            one_vram_buffer(0x0E, NTADR_A(3, 12));
            one_vram_buffer(0x1F, NTADR_A(4, 12));

            if(temp0 && numMines >= minMines) --numMines;
            if(temp1 && numMines >= minMines + 10) numMines -= 10;
        }
    } else if(gameSelectShouldUseArrow[3] == FALSE) {
        
        //make the arrows white
        one_vram_buffer(0x0E, NTADR_A(3, 12));
        one_vram_buffer(0x0F, NTADR_A(4, 12));

        //temp1 = pos of cursor

        if(BUTTON_DOWN(PAD_RIGHT)) {

            isNumberArrowUp = TRUE;
            ++temp1;
            if(temp1 == 4) {

                gameSelectShouldUseArrow[3] = TRUE;
                ++selectionArrowIndex;
                temp3 = TRUE;
                goto skipNA;
            }
        } else if(BUTTON_DOWN(PAD_LEFT)) {

            isNumberArrowUp = FALSE;
            --temp1;
            if(temp1 == 255) {

                gameSelectShouldUseArrow[3] = TRUE;
                --selectionArrowIndex;
                temp3 = TRUE;
                goto skipNA;
            }
        }
        
        one_vram_buffer(UNDERSCORE, NTADR_A(14 + temp1, 17));
        
        if(BUTTON_DOWN(PAD_A)) {

            tempShort0 = (customSeed >> ((3 - temp1) << 2));
            ++tempShort0;
            tempShort0 &= 0b1111;
            customSeed &= ~(0b1111 << ((3 - temp1) << 2));
            customSeed |= (tempShort0 << ((3 - temp1) << 2));
        } else if(BUTTON_DOWN(PAD_B)) {

            tempShort0 = (customSeed >> ((3 - temp1) << 2));
            --tempShort0;
            tempShort0 &= 0b1111;
            customSeed &= ~(0b1111 << ((3 - temp1) << 2));
            customSeed |= (tempShort0 << ((3 - temp1) << 2));
        }

        if(customSeed == 0xCB58 && BUTTON_DOWN(PAD_SELECT)) {
            
            gameMode |= (0b1 << 4);
            one_vram_buffer(0x28, PALETTE_MEMORY_BEGIN);
            delay(5);
            one_vram_buffer(0x1B, PALETTE_MEMORY_BEGIN);

            minMines = 0;
            maxMines = 255;
        }
    } else {

        //make the arrows white
        one_vram_buffer(0x0E, NTADR_A(3, 12));
        one_vram_buffer(0x0F, NTADR_A(4, 12));

        if(BUTTON_DOWN(PAD_RIGHT) || BUTTON_DOWN(PAD_DOWN)) isNumberArrowUp = TRUE;
        else if(BUTTON_DOWN(PAD_LEFT) || BUTTON_DOWN(PAD_UP)) isNumberArrowUp = FALSE;

        gameSelectShouldUseArrow[3] = TRUE;

        if(BUTTON_DOWN(PAD_A) || BUTTON_DOWN(PAD_START)) {

            switch(selectionArrowIndex) {

                case 0: //set to easy mode

                setToEasyMode();
                one_vram_buffer(OPENING_BRACKET, NTADR_A(3, 8));
                one_vram_buffer(CLOSING_BRACKET, NTADR_A(8, 8));

                one_vram_buffer(BLANK_TILE, NTADR_A(10, 8));
                one_vram_buffer(BLANK_TILE, NTADR_A(15, 8));

                break;

                case 1: //set to hard mode

                setToHardMode();
                one_vram_buffer(OPENING_BRACKET, NTADR_A(10, 8));
                one_vram_buffer(CLOSING_BRACKET, NTADR_A(15, 8));

                one_vram_buffer(BLANK_TILE, NTADR_A(3, 8));
                one_vram_buffer(BLANK_TILE, NTADR_A(8, 8));

                break;

                case 3: //set custom seed

                gameSelectShouldUseArrow[3] = FALSE;
                temp1 = 0;
                isCustomSeed = TRUE;

                one_vram_buffer(OPENING_BRACKET, NTADR_A(3, 16));
                one_vram_buffer(CLOSING_BRACKET, NTADR_A(7, 16));

                one_vram_buffer(BLANK_TILE, NTADR_A(9, 16));
                one_vram_buffer(BLANK_TILE, NTADR_A(12, 16));

                break;

                case 4: //set no custom seed

                isCustomSeed = FALSE;

                one_vram_buffer(OPENING_BRACKET, NTADR_A(9, 16));
                one_vram_buffer(CLOSING_BRACKET, NTADR_A(12, 16));

                one_vram_buffer(BLANK_TILE, NTADR_A(3, 16));
                one_vram_buffer(BLANK_TILE, NTADR_A(7, 16));

                break;

                case 5: //start game

                startGame:

                if(customSeed == 0 && !(gameMode & (0b1 << 4))) isCustomSeed = FALSE;
                frameCount = 0;
                newGame();

                return;

                default:

                break;
            }
        }
    }

    skipNA:

    if(BUTTON_DOWN(PAD_SELECT)) goto startGame;

    updateSelectionArrow(); //done last because otherwise it messed with the number arrow
}

#pragma endregion

void update(void) {

    updateController();
    updateRNG();

    switch(gameMode & 0b111) {

    case 0b001: //playing the game

    if(IS_GAME_PAUSED()) {

        gameUpdatePaused();
    } else {

        gameUpdateDifficultyIndependent();

        if(IS_GAME_HARD()) gameUpdateHard();
        else gameUpdateEasy();
    }

    break;

    case 0b010: //game selection

    updateGameSelection();

    break;

    case 0b011: //game won

    break;

    case 0b000: //0 or anything else (safest to default back to the title screen, I guess?)
    default:

        updateTitleScreen();
    }

    ++frameCount;
}

#endif