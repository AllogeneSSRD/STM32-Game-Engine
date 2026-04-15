#include "Game_3.h"
#include "Game_3_Menu.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include "Joystick.h"
#include <stdio.h>
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control


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

//Count dots amount
// Easy / Map 1
#define TOTAL_DOTS_MAP1 128

// Normal / Map 2
#define TOTAL_DOTS_MAP2 146

// Hard / Map 3
#define TOTAL_DOTS_MAP3 136


//Runtime map, it should be revised during the game as player eats dots
static uint8_t game_map[MAP_ROWS][MAP_COLS];

// Movement parameters
#define PACMAN_MOVE_DELAY_MS 200 // Milliseconds between movement updates
static uint32_t last_move_tick = 0;


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
    {1,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,1},
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
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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
    {1,0,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,0,1},
    {1,1,1,2,1,1,2,0,0,0,0,0,0,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,0,0,0,0,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

//Matching maps of different levels to runtime map
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
    //Clear the screen
    LCD_Fill_Buffer(0);

    for (int row = 0; row < MAP_ROWS; row++) 
    {
        for (int col = 0; col < MAP_COLS; col++) 
        {
            int px = MAP_ORIGIN_X + col * TILE;
            int py = MAP_ORIGIN_Y + row * TILE;

            if (game_map[row][col] == TILE_WALL) 
            {
                //Draw the walls
                LCD_Draw_Rect(px, py, TILE, TILE, 1, 1);
            }
            else if (game_map[row][col] == TILE_DOT) 
            {
                //Draw thr dots, ensuring centered
                LCD_Draw_Circle(px + TILE / 2, py + TILE / 2, 2, 1, 1);
            }
        }
    }
}

// Initialize player coordinate
typedef struct {
    int16_t x;
    int16_t y;
} Player_tile;
static Player_tile pacman;

//Initialize player at center of screen
static void Initialize_Player_Center(void)
{
    pacman.x = MAP_COLS / 2 - 1;   // 9
    pacman.y = MAP_ROWS / 2 + 1;  // 10
}

//Draw the player
static void Draw_Player(void)
{
    //tile to pixel
    int px = MAP_ORIGIN_X + pacman.x * TILE + TILE / 2;
    int py = MAP_ORIGIN_Y + pacman.y * TILE + TILE / 2;

    LCD_Draw_Circle(px, py, 4, 5, 1);//Larger than dots
}

//Define movement direction
typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} PacmanDirection;
static PacmanDirection pacman_dir = DIR_NONE; //Initialize to none, staying still firstly

//Movement to Joystick direction
static void Movement_to_Joystick(Joystick_t *joystick_data)
{
    switch (joystick_data->direction)
    {
        case N: pacman_dir = DIR_UP; break;
        case NE: pacman_dir = DIR_UP; break;
        case NW: pacman_dir = DIR_UP; break;
        case S: pacman_dir = DIR_DOWN; break;
        case SE: pacman_dir = DIR_DOWN; break;
        case SW: pacman_dir = DIR_DOWN; break;
        case E: pacman_dir = DIR_RIGHT; break;
        case W: pacman_dir = DIR_LEFT; break;
        default: pacman_dir = DIR_NONE; break;
    }
}

//Pacman Movement
static void Move_Pacman_One_Tile(void)
{
    int16_t next_x = pacman.x;
    int16_t next_y = pacman.y;

    switch (pacman_dir)
    {
        case DIR_UP:    next_y--; break;
        case DIR_DOWN:  next_y++; break;
        case DIR_LEFT:  next_x--; break;
        case DIR_RIGHT: next_x++; break;
        default:
            return; // DIR_NONE，staying still
    }

    //Clamp position to map boundaries (prevent player from leaving the display)
    if (next_x < 0 || next_x >= MAP_COLS ||
        next_y < 0 || next_y >= MAP_ROWS)
        return;

    //Clamp position to wall boundaries (prevent player from going through the wall)
    if (game_map[next_y][next_x] != TILE_WALL)
    {
        pacman.x = next_x;
        pacman.y = next_y;
    }
}

MenuState Game3_Run(void)
{
    MenuState exit_state = MENU_STATE_HOME;

    Joystick_t joystick_data;
    
    //Ready for calling these structures
    DifficultySystem diffi;
    DifficultyState selected_diffi;
    
    //Initialize difficulty menu
    Difficulty_Init(&diffi);

    //Run difficulty menu
    selected_diffi = Difficulty_Run(&diffi);

    //Match the selected difficulty to the corresponding map
    Mapmatching(selected_diffi);
    
    //Allocating dots regarding maps
    uint16_t total_dots;

    switch (selected_diffi)
    {
        case DIFFICULTY_EASY:
            total_dots = TOTAL_DOTS_MAP1;
            break;

        case DIFFICULTY_NORMAL:
            total_dots = TOTAL_DOTS_MAP2;
            break;

        case DIFFICULTY_HARD:
            total_dots = TOTAL_DOTS_MAP3;
            break;
    }

    unsigned long remaining_lives = 3; //Player starts with 3 lives
    unsigned long score = 0; //Player's score starts at 0

    //Game starts
    while (remaining_lives > 0) 
    {
        //Reset player position
        Initialize_Player_Center();
        HAL_Delay(300);

        //Each life loop
        while(1)
        {         
            //Step 1. Read Joystick
            Joystick_Read(&joystick_cfg, &joystick_data);
            Movement_to_Joystick(&joystick_data);

            //Step 2. Rate-limited movement
            uint32_t now = HAL_GetTick();
            if ((now - last_move_tick) >= PACMAN_MOVE_DELAY_MS &&
                pacman_dir != DIR_NONE)
            {
                Move_Pacman_One_Tile();
                last_move_tick = now;
            }

            //Step 3. Draw the map and player
            //Draw Map
            Draw_Map();
            //Draw the player at the center
            Draw_Player();

            //Step 4. Output lives and score
            char lives_text[32];
            char score_text[32];
            sprintf(lives_text, "Lives: %lu", (unsigned long)remaining_lives);
            sprintf(score_text, "Score: %lu", (unsigned long)score);
            LCD_printString(lives_text, 10, 10, 2, 2);
            LCD_printString(score_text, 130, 10, 10, 2);
            LCD_Refresh(&cfg0);

            HAL_Delay(50);

            //Step 5. Collision Detection
            if (game_map[pacman.y][pacman.x] == TILE_DOT) 
            {
                // Reset dots to free spaces
                game_map[pacman.y][pacman.x] = TILE_EMPTY;

                //Increment a score counter
                score++;

                //Play audio for getting score
                buzzer_note(&buzzer_cfg, NOTE_C6, 40);
                HAL_Delay(50);
                buzzer_off(&buzzer_cfg);
            }

            //Step 6. Caught by ghosts

            //Step 7. Collect all dots           
            if (score == total_dots)
            {
                //Play victory sounds and lights
                buzzer_note(&buzzer_cfg, NOTE_C7, 40);
                HAL_Delay(1000);
                buzzer_off(&buzzer_cfg);
                PWM_SetDuty(&pwm_cfg, 100);

                //Output victory message
                LCD_Fill_Buffer(0);
                LCD_printString("YOU WIN!", 70, 110, 1, 3);
                LCD_Refresh(&cfg0);

                HAL_Delay(4000);

                //Back to main menu
                return exit_state;
            }

        }
    }

    //Game over
    // Play game over sounds and lights
    buzzer_note(&buzzer_cfg, NOTE_A4, 60);
    HAL_Delay(1000);
    buzzer_off(&buzzer_cfg);
    PWM_SetDuty(&pwm_cfg, 0);               

    // Output game over message
    LCD_Fill_Buffer(0);
    LCD_printString("GAME OVER", 60, 110, 1, 3);
    LCD_Refresh(&cfg0);

    HAL_Delay(4000);

    return exit_state;  // Tell main where to go next
}