#include "Game_2.h"
#include "Game2_menu.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include "rng.h"


extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control
extern RNG_HandleTypeDef hrng;// Random number generator for enemy car placement


// Road boundary
#define ROAD_LEFT   20
#define ROAD_RIGHT  220
#define ROAD_TOP    40
#define ROAD_BOTTOM 240

//Player starts with 3 lives
unsigned long remaining_lives = 3; 
//Player's score starts at 0
long score = 0; 

// Road animation offset
static int road_offset = 0; 

// Frame rate for this game (in milliseconds) - runs slower than Game 1
#define GAME2_FRAME_TIME_MS 50  // ~20 FPS (different from Game 1!)

#define MAX_ENEMY 4  //Limit the number of cars on the screen at once
#define MAX_LANES 5     //Maximum number of lanes for the game
#define CAR_SPEED 3  //Speed of the cars 
#define PLAYER_RADIUS 6
#define ENEMY_RADIUS  6
#define ENEMY_SPACING 68   //Minimum distance between enemy vehicles
#define REWARD_SCORE_GAIN   5
#define REWARD_SCORE_LOSS   3
#define WIN_SCORE 20

// number of lanes
static int num_lanes = 3;
// The X-coordinate of each lane
static int lane_x[MAX_LANES];

static int player_x;
static int player_y;


typedef struct
{
    int x;
    int y;
    int speed;
} EnemyCar;

static EnemyCar enemies[MAX_ENEMY];
static int enemy_count = 1;

typedef struct
{
    int x;
    int y;
} Reward;

static Reward reward; 

void Map_SetLanes(int lanes)
{
    //Save the current number of lanes
    num_lanes = lanes;

    //Calculate road width
    int road_width = ROAD_RIGHT - ROAD_LEFT;

    //Width of each lane
    int lane_width = road_width / num_lanes;

    //Calculate the center X coordinate of each lane
    for(int i = 0; i < num_lanes; i++)
    {
        lane_x[i] = ROAD_LEFT + lane_width/2 + i * lane_width;
    }
}

static uint8_t Random_Car(void)
{
    uint32_t rnd;
    HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
    return (uint8_t)(rnd % num_lanes);
}


//Draw lanes
void Draw_Game2_Map(void)
{
    int road_width;
    int lane_width;
    //Draw the left and right boundary lines of the road
    LCD_Draw_Line(ROAD_LEFT,  ROAD_TOP, ROAD_LEFT,  ROAD_BOTTOM, 1);
    LCD_Draw_Line(ROAD_RIGHT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM, 1);

    road_width = ROAD_RIGHT - ROAD_LEFT;
    lane_width = road_width / num_lanes;

    //Draw lane dividing lines
    for(int i = 1; i < num_lanes; i++)
    {
        int x = ROAD_LEFT + i * lane_width;
        for(int y = ROAD_TOP- road_offset; y < ROAD_BOTTOM; y += 20)
        {
            LCD_Draw_Line(x, y, x, y+10, 1);
        }
    }
}

//Define movement direction
typedef enum
{
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} CarDirection;

static CarDirection car_dir = DIR_NONE;//Initialize to none

static void Movement_to_Joystick(Joystick_t *joystick_data)
{
    car_dir = DIR_NONE;

    switch (joystick_data->direction)
    {
        case N:car_dir = DIR_UP;break;
        case NE:car_dir = DIR_UP;break;
        case NW:car_dir = DIR_UP;break;
        case S:car_dir = DIR_DOWN;break;
        case SE:car_dir = DIR_DOWN;break;
        case SW:car_dir = DIR_DOWN;break;
        case E:car_dir = DIR_RIGHT;break;
        case W:car_dir = DIR_LEFT;break;
        default:car_dir = DIR_NONE;break;
    }

        switch(car_dir)
    {
        case DIR_UP:
            player_y -= CAR_SPEED;
            break;

        case DIR_DOWN:
            player_y += CAR_SPEED;
            break;

        case DIR_LEFT:
            player_x -= CAR_SPEED;
            break;

        case DIR_RIGHT:
            player_x += CAR_SPEED;
            break;

        default:
            break;
    }

    if(player_x < ROAD_LEFT+5)
    {
        player_x = ROAD_LEFT+5;
    }

    if(player_x > ROAD_RIGHT-10)
    {
        player_x = ROAD_RIGHT-10;
    }

    if(player_y < ROAD_TOP+5)
    {
        player_y = ROAD_TOP+5;
    }

    if(player_y > ROAD_BOTTOM-10)
    {
        player_y = ROAD_BOTTOM-10;
    }
}

static uint8_t Circles_Overlap(int x1, int y1, int r1, int x2, int y2, int r2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int distance = dx * dx + dy * dy;
    int r_sum = r1 + r2;

    return (distance <= (r_sum * r_sum));
}


static void Draw_Player(void)
{
    LCD_printString("A", player_x, player_y, 1, 2);
}



static void Draw_Reward(void)
{
    LCD_printString("*", reward.x, reward.y, 1, 2);
}


static void Initialize_Player_Center(void)
{
    player_x = (ROAD_LEFT + ROAD_RIGHT) / 2;
    player_y = ROAD_BOTTOM - 20;
}

//To avoid duplicate enemy vehicles
static int EnemyFrontCar(EnemyCar *car)
{
    int i;

    for (i = 0; i < enemy_count; i++)
    {
        if (&enemies[i] == car)
            continue; 

        if (enemies[i].x == car->x)
        {
            if (enemies[i].y > car->y)
            {
                if (enemies[i].y - car->y < ENEMY_SPACING)
                    return 1;
            }
            else
            {
                if (car->y - enemies[i].y < ENEMY_SPACING)
                    return 1;
            }
        }
    }
    return 0;

}

static void Mapmatching(MapState map)
{
    if (map == MENU_GAME2_MAP1)
    {
        Map_SetLanes(3);
    }
    else if (map == MENU_GAME2_MAP2)
    {
        Map_SetLanes(4);
    }
    else
    {
        Map_SetLanes(5);
    }
}

static void Enemy_Init(void)
{
    for (int i = 0; i < enemy_count; i++)
    {
        do
        {
            enemies[i].x = lane_x[Random_Car()];
            enemies[i].y = ROAD_TOP - i * 40;
        }while (EnemyFrontCar(&enemies[i]));
 
        enemies[i].speed = CAR_SPEED;     
    }
}

static void Reward_Init(void)
{
    reward.x = lane_x[Random_Car()];
    reward.y = ROAD_TOP;
}


static void Reward_Update(void)
{
    reward.y += CAR_SPEED;

    if (reward.y > ROAD_BOTTOM)
    {
        score -= REWARD_SCORE_LOSS;
        if (score < 0) 
        {
            score = 0;
        }
        Reward_Init(); 
    }
}


MenuState Game2_Run(void) {
    // Initialize game state
    int win = 0;
    remaining_lives = 3;
    score = 0;

    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1200, 30);  // 1.2kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu

    Joystick_t joystick_data;

    //Map selection menu
    static MapSystem map;
    MapState selected_map;

    Map_Init(&map);
    selected_map = Map_Run(&map);

    //Match map based on selection
    Mapmatching(selected_map);

    //Set the number of enemy vehicles based on the number of lanes
    enemy_count = num_lanes - 1;
    if (enemy_count > MAX_ENEMY)
    {
        enemy_count = MAX_ENEMY;
    }

    Enemy_Init();
    Reward_Init(); 

    while(remaining_lives > 0)
    {
        //Reset player position
        Initialize_Player_Center();
        
        HAL_Delay(300); // Brief pause before starting
        while (1) {
            char lives_text[32];
            char score_text[32];

            //Read Joystick
            Joystick_Read(&joystick_cfg, &joystick_data);
            Movement_to_Joystick(&joystick_data);

            uint32_t frame_start = HAL_GetTick();
        
            // Read input
            Input_Read();
        
            // Check if button was pressed to return to menu
            if (current_input.btn3_pressed) 
            {
                exit_state = MENU_STATE_HOME;
                break;  // Exit game loop
            }
        
            //Overtaking bonus points

            for (int i = 0; i < enemy_count; i++)
            {
                enemies[i].y += enemies[i].speed;
                if (enemies[i].y > ROAD_BOTTOM)
                {
                    enemies[i].y = ROAD_TOP;
                    enemies[i].x = lane_x[Random_Car()];

                }
            }

            //The road surface rolls with the speed of the vehicle
            road_offset += CAR_SPEED;   
            if (road_offset >= 20)
            {
                road_offset = 0;
            }

        


            for (int i = 0; i < enemy_count; i++)
            {
                if (Circles_Overlap(player_x, player_y, PLAYER_RADIUS,enemies[i].x, enemies[i].y, ENEMY_RADIUS))
                {
                    remaining_lives--;

                    buzzer_tone(&buzzer_cfg, 400, 60);
                    HAL_Delay(120);
                    buzzer_off(&buzzer_cfg);

                    Initialize_Player_Center(); 

                    enemies[i].y = ROAD_TOP;
                    enemies[i].x = lane_x[Random_Car()];
                    break;
                }
            }   

            
            // Player collects reward
            
            if (Circles_Overlap(player_x, player_y, PLAYER_RADIUS,reward.x, reward.y, 4))
            {
                score += REWARD_SCORE_GAIN;
                Reward_Init();   
            }


            if (score >= WIN_SCORE)
            {
                win = 1;
                exit_state = MENU_STATE_HOME;
                break;
            }


            //Game over
            if(remaining_lives == 0)
            {   
                win = 0;
                exit_state = MENU_STATE_HOME;
                break;
            }
           
        // RENDER: Draw to LCD
        LCD_Fill_Buffer(0);
        Reward_Update();

        Draw_Game2_Map();

        Draw_Player();

        Draw_Reward();

        
    for (int i = 0; i < enemy_count;
         i++)
    {
        LCD_printString("V", enemies[i].x, enemies[i].y, 1, 2);
    }

        
    sprintf(lives_text, "Lives: %lu", (unsigned long)remaining_lives);
    sprintf(score_text, "Score: %lu", (unsigned long)score);
    LCD_printString(lives_text, 10, 10, 2, 2);
    LCD_printString(score_text, 130, 10, 2, 2);

        
        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME2_FRAME_TIME_MS) {
            HAL_Delay(GAME2_FRAME_TIME_MS - frame_time);
        }
    }
  
     if (win || remaining_lives == 0)
            break;
    }
     
           
// Victory celebration
if (win)
{
    for (int i = 0; i < 6; i++)
    {
        LCD_Fill_Buffer(0);
        LCD_printString("YOU WIN!", 50, 100, 1, 3);
        LCD_printString("SCORE 200+", 40, 140, 1, 2);
        LCD_Refresh(&cfg0);

        buzzer_tone(&buzzer_cfg, 1500, 40);
        HAL_Delay(200);
        buzzer_off(&buzzer_cfg);

        HAL_Delay(200);
    }
}

    //Game over
    // Play game over sounds and lights
    if (!win)
{
    buzzer_note(&buzzer_cfg, NOTE_A4, 60);
    HAL_Delay(1000);
    buzzer_off(&buzzer_cfg);

    for (int i = 0; i < 4; i++)
    {
        LCD_Fill_Buffer(0);
        LCD_printString("GAME OVER!!!", 40, 120, 1, 3);
        LCD_Refresh(&cfg0);
        HAL_Delay(300);

        LCD_Fill_Buffer(0);
        LCD_Refresh(&cfg0);
        HAL_Delay(300);
    }
}

    LCD_Fill_Buffer(0);
    LCD_printString("BACK TO MENU...", 40, 120, 1, 2);
    LCD_Refresh(&cfg0);

    HAL_Delay(4000);

    
    return exit_state;  // Tell main where to go next
}
