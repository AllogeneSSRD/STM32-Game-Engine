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

//Function declaration
void Mapmatching(DifficultyState diffi);
void Draw_Map(void);
void Initialize_Player_Center(void);
void Draw_Player(void);
void Movement_to_Joystick(Joystick_t *joystick_data);
void Move_Pacman_One_Tile(void);
void Initialize_Ghosts(DifficultyState diffi);
void Draw_Ghosts(void);
void Move_Ghosts_One_Tile(void);

MenuState Game3_Run(void);


#endif // GAME_3_H
