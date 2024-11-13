#ifndef DEFS_H
#define DEFS_H

typedef unsigned char uchar;

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
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

#define TRUE 1
#define FALSE 0
#define DARK_GREEN_TILE 8

#define HARD_MAX_X 32
#define HARD_MAX_Y 26
#define HARD_NUM_MINES 250

#define EASY_MAX_X 16
#define EASY_MAX_Y 13
#define EASY_NUM_MINES 70

//32 by 26 max, 1 bit per tile, = 33 * 27 / 8 = 104
#define BOARD_MEM_SIZE 104
#define EASY_BOARD_USED_MEM_SIZE 26

#define NUMBER_TO_TILE 0x30
#define NUMBER_TO_NUMBER_TILE 0x10

#define FILL_STACK_SIZE 0x28
#define MAX_FLOOD_FILL_UPDATES 30

#define PALETTE_MEMORY_BEGIN 0x3f00

#endif