#ifndef GAME_3_H
#define GAME_3_H

#include <stdint.h>
#include "Menu.h"
#include "Game_3_Menu.h"
#include "Joystick.h"

/**
 * @brief Game 3 - Student can implement their own game here
 *
 * Placeholder for Student 3's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 *
 * The menu system calls this function when Game 3 is selected.
 * The function runs its own loop and returns when the game exits.
 *
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */
#define MAP_ORIGIN_X 20
#define MAP_ORIGIN_Y 60

//Map size = 20 x 18 = 360 tiles
#define TILE        10          //8 pixels per map tile for LCD display
#define MAP_COLS    20         //Number of columns in the map grid
#define MAP_ROWS    18         //Number of rows in the map grid
#define GHOSTS  4          //Number of ghosts on screen

// Tile types stored in the map array
#define TILE_EMPTY  0   //Free space, player can pass
#define TILE_WALL   1   //Wall, player cannot pass
#define TILE_DOT    2   //Dots

//Count dots amount
// Easy / Map 1
#define TOTAL_DOTS_MAP1 130

// Normal / Map 2
#define TOTAL_DOTS_MAP2 152

// Hard / Map 3
#define TOTAL_DOTS_MAP3 154

//Pacman movement delay
#define PACMAN_MOVE_DELAY_MS 200 // Milliseconds between movement updates

#define BTN3_HOLD_MS 2000

// Initialize coordinate
typedef struct {
    int16_t x;
    int16_t y;
} tile_pos;

//Pacman movement direction
typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} PacmanDirection;

/** Sprites 
 * 8x8 pixels, palette indices: 6=YELLOW  0=BLACK  2=RED  1=WHITE  255=transparent
 * Draw with: LCD_Draw_Sprite(x, y, SPR_SIZE, SPR_SIZE, pacman_right);
 */

#define SPR_SIZE 8

//Two directions of pacman
static const uint8_t pacman_right[SPR_SIZE * SPR_SIZE] = {
    255,   6,   6,   6,   6,   6, 255, 255,
      6,   6,   6,   0,   6,   6,   6, 255,
      6,   6,   6,   6,   6,   6, 255, 255,
      6,   6,   6,   6,   6, 255, 255, 255,
      6,   6,   6,   6,   6, 255, 255, 255,
      6,   6,   6,   6,   6,   6, 255, 255,
      6,   6,   6,   0,   6,   6,   6, 255,
    255,   6,   6,   6,   6,   6, 255, 255,
};

static const uint8_t pacman_left[SPR_SIZE * SPR_SIZE] = {
    255, 255,   6,   6,   6,   6, 255, 255,
    255,   6,   6,   6,   0,   6,   6, 255,
    255, 255,   6,   6,   6,   6,   6,   6,
    255, 255, 255,   6,   6,   6,   6,   6,
    255, 255, 255,   6,   6,   6,   6,   6,
    255, 255,   6,   6,   6,   6,   6,   6,
    255,   6,   6,   6,   0,   6,   6, 255,
    255, 255,   6,   6,   6,   6, 255, 255,
};

//Four colors of ghosts
static const uint8_t ghost_red[SPR_SIZE * SPR_SIZE] = {
    255, 255,   2,   2,   2,   2, 255, 255,
    255,   2,   2,   2,   2,   2,   2, 255,
      2,   2,   1,   1,   2,   1,   1,   2,
      2,   2,   1,   0,   2,   1,   0,   2,
      2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,
      2, 255,   2, 255,   2, 255,   2, 255,
};

static const uint8_t ghost_green[SPR_SIZE * SPR_SIZE] = {
    255, 255,   3,   3,   3,   3, 255, 255,
    255,   3,   3,   3,   3,   3,   3, 255,
      3,   3,   1,   1,   3,   1,   1,   3,
      3,   3,   1,   0,   3,   1,   0,   3,
      3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,
      3, 255,   3, 255,   3, 255,   3, 255,
};

static const uint8_t ghost_blue[SPR_SIZE * SPR_SIZE] = {
    255, 255,   4,   4,   4,   4, 255, 255,
    255,   4,   4,   4,   4,   4,   4, 255,
      4,   4,   1,   1,   4,   1,   1,   4,
      4,   4,   1,   0,   4,   1,   0,   4,
      4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,
      4, 255,   4, 255,   4, 255,   4, 255,
};

static const uint8_t ghost_orange[SPR_SIZE * SPR_SIZE] = {
    255, 255,   5,   5,   5,   5, 255, 255,
    255,   5,   5,   5,   5,   5,   5, 255,
      5,   5,   1,   1,   5,   1,   1,   5,
      5,   5,   1,   0,   5,   1,   0,   5,
      5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,
      5, 255,   5, 255,   5, 255,   5, 255,
};

//Function declaration
void Mapmatching(DifficultyState diffi);
void Game3_Draw_Map(void);
void Initialize_Player_Center(void);
void Draw_Player(void);
void Movement_to_Joystick(Joystick_t *joystick_data);
void Move_Pacman_One_Tile(void);
void Initialize_Ghosts(DifficultyState diffi);
void Draw_Ghosts(void);
void Move_Ghosts_One_Tile(void);

MenuState Game3_Run(void);

#endif // GAME_3_H