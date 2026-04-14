#include "Game_3.h"
#include "Game_3_Menu.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control


#define MAP_ORIGIN_X 20
#define MAP_ORIGIN_Y 60


//Map size = 20 x 18 = 360 tiles
#define TILE        10          //8 pixels per map tile for LCD display
#define MAP_COLS    20         // number of columns in the map grid
#define MAP_ROWS    18         // number of rows in the map grid
#define MAX_GHOSTS  4          // maximum number of ghosts on screen

// Tile types stored in the map array
#define TILE_EMPTY  0   //Free space, player can pass
#define TILE_WALL   1   //Wall, player cannot pass
#define TILE_DOT    2   //Dots

//Runtime map, it should be revised during the game as player eats dots
static uint8_t game_map[MAP_ROWS][MAP_COLS];

//MAP DEFINITIONS  (1 = wall, 2 = dot, 0 = open)
//Three different maps — one per difficulty level

//Map 1
static const uint8_t map1[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1},
    {1,1,1,1,2,1,1,0,0,0,0,0,0,1,1,2,1,1,1,1},
    {1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1},
    {1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1},
    {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0},
    {1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1},
    {1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

//Map 2
static const uint8_t map2[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,1,1,2,2,1,1,1,2,1,1,1,2,1},
    {1,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,1},
    {1,2,2,2,1,2,1,2,2,1,1,2,2,1,2,1,2,2,2,1},
    {1,1,1,2,1,2,1,2,1,1,1,1,2,1,2,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,0,0,0,0,0,0,1,2,1,1,1,2,1},
    {1,2,2,2,2,2,0,0,0,0,0,0,0,0,2,2,2,2,2,1},
    {1,2,1,1,1,2,0,0,0,0,0,0,0,0,2,1,1,1,2,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,1,1,2,0,0,0,0,0,0,0,0,2,1,1,1,2,1},
    {1,2,2,2,2,2,0,0,0,0,0,0,0,0,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,0,0,0,0,0,0,1,2,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

//Map 3
static const uint8_t map3[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,1},
    {1,2,1,1,1,2,1,1,1,2,2,1,1,1,2,1,1,1,2,1},
    {1,2,1,2,2,2,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
    {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
    {1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,0,0,0,0,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,1},
    {1,1,1,2,1,1,2,0,0,0,0,0,0,2,1,1,2,1,1,1},
    {0,0,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,0,0},
    {1,1,1,2,1,1,2,0,0,0,0,0,0,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,0,0,0,0,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static void Mapmatching(DifficultyState diffi)
{
    if (diffi == DIFFICULTY_EASY) {
        memcpy(game_map, map1, sizeof(game_map));//Copy map1 to game_map for runtime use
    }
    else if (diffi == DIFFICULTY_NORMAL) {
        memcpy(game_map, map2, sizeof(game_map));//Copy map2 to game_map for runtime use
    }
    else { // DIFFICULTY_HARD
        memcpy(game_map, map3, sizeof(game_map));//Copy map3 to game_map for runtime use
    }
}

static void Draw_Map(void)
{
    // 先清屏
    LCD_Fill_Buffer(0);

    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {

            // 当前格子的像素左上角
            int px = MAP_ORIGIN_X + col * TILE;
            int py = MAP_ORIGIN_Y + row * TILE;

            if (game_map[row][col] == TILE_WALL) {
                // 画墙：实心矩形
                LCD_Draw_Rect(px, py, TILE, TILE, 1, 1);
            }
            else if (game_map[row][col] == TILE_DOT) {
                // 画豆子：小圆点（居中）
                LCD_Draw_Circle(
                    px + TILE / 2,
                    py + TILE / 2,
                    2,          // 半径
                    1,          // 颜色
                    1
                );
            }
            // TILE_EMPTY：什么都不画
        }
    }

    LCD_Refresh(&cfg0);
}


MenuState Game3_Run(void) 
{
    MenuState exit_state = MENU_STATE_HOME;
    
    //Ready for calling these structures
    DifficultySystem diffi;
    DifficultyState selected_diffi;
    
    //Initialize difficulty menu
    Difficulty_Init(&diffi);

    //Run difficulty menu
    selected_diffi = Difficulty_Run(&diffi);

    //Match the selected difficulty to the corresponding map
    Mapmatching(selected_diffi);
    
    unsigned long remaining_lives = 3; //Player starts with 3 lives
    unsigned long score = 0; //Player's score starts at 0

    // ✅ 只画地图
    Draw_Map();
    char lives_text[32];
    char score_text[32];
    sprintf(lives_text, "Lives: %lu", (unsigned long)remaining_lives);
    sprintf(score_text, "Score: %lu", (unsigned long)score);
    LCD_printString(lives_text, 10, 10, 2, 2);
    LCD_printString(score_text, 130, 10, 10, 2);
    LCD_Refresh(&cfg0);
    // 停住，方便你观察
    while (1);

    return exit_state;  // Tell main where to go next
}