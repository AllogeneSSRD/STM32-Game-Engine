#include "Game_3.h"
#include "Game_3_Menu.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include "Joystick.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control


//Runtime map, it should be revised during the game as player eats dots
static uint8_t game_map[MAP_ROWS][MAP_COLS];

//Pac-man Movement parameters
static uint32_t pacman_last_move_tick = 0;

//Ghost Movement parameters
static uint32_t ghost_move_delay_ms = 0;
static uint32_t ghost_last_move_tick = 0;

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
void Mapmatching(DifficultyState diffi)
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

void Draw_Map(void)
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

//Calling coordinate structure
static tile_pos pacman;
static tile_pos ghosts[GHOSTS];

//Initialize player at center of screen
void Initialize_Player_Center(void)
{
    pacman.x = MAP_COLS / 2 - 1;   // 9
    pacman.y = MAP_ROWS / 2 + 1;  // 10
}

//Draw the player
void Draw_Player(void)
{
    //tile to pixel
    int px = MAP_ORIGIN_X + pacman.x * TILE + TILE / 2;
    int py = MAP_ORIGIN_Y + pacman.y * TILE + TILE / 2;

    LCD_Draw_Circle(px, py, 4, 5, 1);//Larger than dots
}

//Calling pacman movement direction structure
static PacmanDirection pacman_dir = DIR_NONE; //Initialize to none, staying still firstly

//Ghost movement direction, setting initial direction
static uint8_t ghost_dir[GHOSTS] = {1, 2, 3, 4};

//Random number generater for generating random direction
static uint8_t simple_rand(void)
{
    static uint32_t n = 1;
    n = n * 1664525 + 1013904223; // LCG: Linear Congruential Generator for random numbers
    return (uint8_t)((n >> 24) % 4) + 1; //Use highest 8 digits due to better randomness, becomes 1-4 finally
}

//Movement to Joystick direction
void Movement_to_Joystick(Joystick_t *joystick_data)
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
void Move_Pacman_One_Tile(void)
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
            return; // DIR_NONE，staying still this time
    }

    //Clamp position to map boundaries (prevent player from leaving the display)
    if (next_x < 0 || next_x >= MAP_COLS ||
        next_y < 0 || next_y >= MAP_ROWS)
        return; //Jump out of the function, staying still this time

    //Clamp position to wall boundaries (prevent player from going through the wall)
    if (game_map[next_y][next_x] != TILE_WALL)
    {
        pacman.x = next_x;
        pacman.y = next_y;
    }
}

//Ghost Placement coordinate
static const tile_pos ghost_spawn_map1[GHOSTS] = {
    {7, 7}, {12, 7}, {6, 12}, {13, 12}
};

static const tile_pos ghost_spawn_map2[GHOSTS] = {
    {7, 7}, {12, 7}, {6, 13}, {13, 13}
};

static const tile_pos ghost_spawn_map3[GHOSTS] = {
    {8, 7}, {11, 7}, {7, 13}, {12, 13}
};

//Initialize Ghost spawn
void Initialize_Ghosts(DifficultyState diffi)
{
    const tile_pos *spawn_points;

    //According the difficulty, choosing the spawn position and speed
    switch (diffi)
    {
        case DIFFICULTY_EASY:
            spawn_points = ghost_spawn_map1;//The address of first element
            ghost_move_delay_ms = 450;
            break;
        case DIFFICULTY_NORMAL:
            spawn_points = ghost_spawn_map2;
            ghost_move_delay_ms = 300;
            break;
        case DIFFICULTY_HARD:
            spawn_points = ghost_spawn_map3;
            ghost_move_delay_ms = 180;
            break;
        default: break;
    }

    for (int i = 0; i < GHOSTS; i++)//Load each coordinate to ghost by reference
    {
        ghosts[i].x = spawn_points[i].x;
        ghosts[i].y = spawn_points[i].y;
        ghost_dir[i] = i + 1; //Reset position, corresponding to elements in it
    }
}

//Draw Ghost
void Draw_Ghosts(void)
{
    // Ghost 1
    int px0 = MAP_ORIGIN_X + ghosts[0].x * TILE + TILE / 2;
    int py0 = MAP_ORIGIN_Y + ghosts[0].y * TILE + TILE / 2;
    LCD_Draw_Circle(px0, py0, 4, 2, 1);

    // Ghost 2
    int px1 = MAP_ORIGIN_X + ghosts[1].x * TILE + TILE / 2;
    int py1 = MAP_ORIGIN_Y + ghosts[1].y * TILE + TILE / 2;
    LCD_Draw_Circle(px1, py1, 4, 6, 1);

    // Ghost 3
    int px2 = MAP_ORIGIN_X + ghosts[2].x * TILE + TILE / 2;
    int py2 = MAP_ORIGIN_Y + ghosts[2].y * TILE + TILE / 2;
    LCD_Draw_Circle(px2, py2, 4, 4, 1);

    // Ghost 4
    int px3 = MAP_ORIGIN_X + ghosts[3].x * TILE + TILE / 2;
    int py3 = MAP_ORIGIN_Y + ghosts[3].y * TILE + TILE / 2;
    LCD_Draw_Circle(px3, py3, 4, 3, 1);
}

//Ghost Movement
void Move_Ghosts_One_Tile(void)
{
    for (int i = 0; i < GHOSTS; i++)
    {
        int16_t next_x = ghosts[i].x;
        int16_t next_y = ghosts[i].y;

        switch (ghost_dir[i])
        {
            case DIR_UP:    next_y--; break;
            case DIR_DOWN:  next_y++; break;
            case DIR_LEFT:  next_x--; break;
            case DIR_RIGHT: next_x++; break;
            default:
                ghost_dir[i] = simple_rand(); //Invalid direction, randomizing again
                continue;                     // Jump to next ghost, this time ghost won't move
        }

        //Clamp position to map boundaries (prevent ghosts from leaving the display)
        if (next_x < 0 || next_x >= MAP_COLS ||
            next_y < 0 || next_y >= MAP_ROWS)
        {
            ghost_dir[i] = simple_rand(); 
            continue;
        }

        //Clamp position to wall boundaries (prevent ghosts from going through the wall)
        if (game_map[next_y][next_x] != TILE_WALL)
        {
            ghosts[i].x = next_x;
            ghosts[i].y = next_y;
        }
        else
        {
            ghost_dir[i] = simple_rand(); 
        }
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
    
    //Back to main menu
    if (selected_diffi == BACK) {
        return exit_state;
    }


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
        default: break;
    }

    unsigned long remaining_lives = 3; //Player starts with 3 lives
    unsigned long score = 0; //Player's score starts at 0

    //Game starts
    while (remaining_lives > 0) 
    {
        //Reset player and ghost position
        Initialize_Player_Center();
        Initialize_Ghosts(selected_diffi);
        HAL_Delay(300);

        // Reset movement timers, preventing moving immediatetly
        pacman_last_move_tick = HAL_GetTick(); 
        ghost_last_move_tick = HAL_GetTick();

        //Each life loop
        while(1)
        {         
            //Step 1. Read Joystick
            Joystick_Read(&joystick_cfg, &joystick_data);
            Movement_to_Joystick(&joystick_data);

            //Step 2. Rate-limited movement
            //Pacman: push the stick for above 200ms to move one tile
            uint32_t now = HAL_GetTick();
            if ((now - pacman_last_move_tick) >= PACMAN_MOVE_DELAY_MS &&
                pacman_dir != DIR_NONE)
            {
                Move_Pacman_One_Tile();
                pacman_last_move_tick = now;
            }

            //Ghost: move one tile automatically based on difficulty speed
            if ((now - ghost_last_move_tick) >= ghost_move_delay_ms)
            {
                Move_Ghosts_One_Tile();
                ghost_last_move_tick = now;
            }

            //Step 3. Draw the map and player
            //Draw Map
            Draw_Map();
            //Draw the player at the center
            Draw_Player();
            //Draw ghosts at the corner
            Draw_Ghosts();

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
            uint8_t is_life_lost = 0;
            for (uint8_t i = 0; i < GHOSTS; i++)
            {
                if (ghosts[i].x == pacman.x && ghosts[i].y == pacman.y)
                {
                    //Caught audio
                    buzzer_note(&buzzer_cfg, NOTE_D4, 60);
                    HAL_Delay(250);
                    buzzer_off(&buzzer_cfg);

                    // LED flashing
                    PWM_SetFreq(&pwm_cfg, 1000); //Reset frequency
                    for (int j = 0; j < 10; j++)
                    {
                        PWM_SetDuty(&pwm_cfg, 5);  //Turn on
                        HAL_Delay(120);

                        PWM_SetDuty(&pwm_cfg, 100);   //Turn off
                        HAL_Delay(120);
                    }

                    remaining_lives--;
                    is_life_lost = 1;
                    break;
                }
            }

            if (is_life_lost == 1) break; //Jump out of loop of this life

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
                LCD_printString("YOU WIN!", 40, 120, 1, 3);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);

                LCD_Fill_Buffer(0);
                LCD_printString("BACK TO MENU...", 40, 120, 1, 2);
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
    LCD_printString("GAME OVER", 40, 120, 1, 3);
    LCD_Refresh(&cfg0);
    HAL_Delay(1000);

    LCD_Fill_Buffer(0);
    LCD_printString("BACK TO MENU...", 40, 120, 1, 2);
    LCD_Refresh(&cfg0);

    HAL_Delay(4000);

    return exit_state;  // Tell main where to go next
}