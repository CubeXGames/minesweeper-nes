#ifndef DEFS_H
#define DEFS_H

typedef unsigned char uchar;
//typedef unsigned short ushort;

typedef struct {

    uchar yPos;
    uchar tile;
    uchar attributes;
    uchar xPos;
} sprite;

typedef union {

    unsigned short longState;
    struct {

        unsigned char byte1;
        unsigned char byte2;
    };
} randomState;

#define BLACK 0x0f
#define WHITE 0x30

#define TRUE 1
#define FALSE 0
#define DARK_GREEN_TILE 8

#define HARD_MAX_X 32
#define HARD_MAX_Y 23
#define HARD_NUM_MINES 150
#define HARD_MAX_MINES 250
#define HARD_MIN_MINES 100

#define EASY_MAX_X 16
#define EASY_MAX_Y 11
#define EASY_NUM_MINES 40
#define EASY_MAX_MINES 110
#define EASY_MIN_MINES 25

//32 by 23 memory size (to make memory aligned better) for hard mode, 1 bit per tile, = 32 * 23 / 8 = 92
#define BOARD_MEM_SIZE 92
#define EASY_BOARD_USED_MEM_SIZE 26

#define NUMBER_TO_TILE 0x30
#define NUMBER_TO_NUMBER_TILE 0x00
#define BLANK_TILE 0x9

#define FILL_STACK_SIZE 0x40

//usually doesn't shake the screen
#define MAX_SCREEN_UPDATES_PER_FRAME 0x20

#define ATTRIBUTE_TABLE_SIZE 0x40

#define PALETTE_MEMORY_BEGIN 0x3f00

#define OPENING_BRACKET 0x5D
#define CLOSING_BRACKET 0x5E
#define UNDERSCORE 0x5F
#define COLON 0x2D

/*//custom rle stuff to be at certain coordinates

#define RLE_SINGLE_TILE 0b00
#define RLE_MULTI_TILE 0b01
#define RLE_NEW_LINE 010
#define RLE_ATT_TABLE 0b11*/

#define NUM_FLASH_FRAMES 3
#define NUM_WAIT_FRAMES 75

#define MENU_X(x) x * 8
#define MENU_Y(y) y * 8 + 6

#define GAME_START_MUSIC 0
#define GAME_NORMAL_MUSIC 1
#define GAME_END_MUSIC 2
//#define TILES_UPDATING_MUSIC 3

#define SFX_SELECT 0
#define SFX_TILE_SELECT 1

//commence visual studio code appeasement
#ifndef __CC65__

uchar __A__;
#define __asm__()

#endif
//conclude visual studio code appeasement

#endif