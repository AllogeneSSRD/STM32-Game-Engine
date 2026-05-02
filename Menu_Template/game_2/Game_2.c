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
#include "main.h"


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
#define CAR1_SPEED 4
#define CAR2_SPEED 5
#define SPEED 1
#define PLAYER_RADIUS 6
#define ENEMY_RADIUS  6
#define ENEMY_SPACING 100 //Minimum distance between enemy vehicles
#define REWARD_SCORE_GAIN   5
#define REWARD_SCORE_LOSS   3
#define WIN_SCORE 100
#define FOLLOW_DISTANCE  60   // Follow the car deceleration distance
#define BTN3_HOLD_EXIT_MS   3000
#define POWER_HEAL    0   
#define POWER_INVINC  1   
#define INVINCIBLE_TIME_MS 3000
#define BLINK_INTERVAL_MS 200  
#define CAR_H 11 
#define CAR_W 7
#define ITEM_SAFE_DISTANCE  (ENEMY_RADIUS + 8)

static const  uint8_t car_X[CAR_H][CAR_W] =
{
    {0,0,1,1,1,0,0},   
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},

    {0,1,1,1,1,1,0},   
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {0,1,1,1,1,1,0},

    {0,1,0,0,0,1,0},  
    {0,1,0,0,0,1,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}
};

static const  uint8_t car_W[CAR_H][CAR_W] =
{
    {0,1,1,1,1,1,0},
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},

    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},

    {1,1,0,0,0,1,1},
    {1,1,0,0,0,1,1},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}
};

static const uint8_t car_V[CAR_H][CAR_W] =
{
    {1,1,1,1,1,1,1},   
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1},   
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},

    {1,0,1,0,1,0,1},   
    {1,0,1,0,1,0,1},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}
};


static const uint8_t *car_sprites[] =
{
    (const uint8_t *)car_V,  
    (const uint8_t *)car_W,  
    (const uint8_t *)car_X   
};



// number of lanes
static int num_lanes = 3;
// The X-coordinate of each lane
static int lane_x[MAX_LANES];

static int player_x;
static int player_y;

static uint8_t invincible = 0;
static uint32_t invincible_start_ms = 0;

typedef struct
{
    int x;
    int y;
    int speed;
    int base_speed;
} EnemyCar;

static EnemyCar enemies[MAX_ENEMY];
static int enemy_count = 1;

typedef struct
{
    int x;
    int y;
} Reward;

static Reward reward; 

typedef struct
{
    int x;
    int y;
    int type;   // POWER_HEAL or POWER_INVINC
} PowerUp;

static PowerUp powerup;


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
        case DIR_UP:player_y -= CAR_SPEED;break;

        case DIR_DOWN:player_y += CAR_SPEED;break;

        case DIR_LEFT:player_x -= CAR_SPEED;break;

        case DIR_RIGHT:player_x += CAR_SPEED;break;

        default:break;
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
    // If it is in an invincible state, the car will flash
    if (invincible)
    {
        uint32_t time = HAL_GetTick() - invincible_start_ms;
        if ((time / BLINK_INTERVAL_MS) % 2 == 0)
        {
            LCD_printString("A", player_x, player_y, 3, 2);
        }

    }
    else
    {
        LCD_printString("A", player_x, player_y, 1, 2);
    }
}

static void Draw_Reward(void)
{
    LCD_printString("*", reward.x, reward.y, 5, 2);
}

static void Draw_PowerUp(void)
{
    if (powerup.type == POWER_HEAL)
        LCD_printString("+", powerup.x, powerup.y, 2, 2);  
    else
        LCD_printString("#", powerup.x, powerup.y, 4, 2); 
}

void Draw_Enemies(void)
{
    for (int i = 0; i < enemy_count; i++)
    {
        int cx = enemies[i].x - CAR_W / 2;
        int cy = enemies[i].y - CAR_H / 2;

        const uint8_t *sprite;

        if (enemies[i].base_speed == CAR_SPEED)
            sprite = car_sprites[0];   
        else if (enemies[i].base_speed == CAR1_SPEED)
            sprite = car_sprites[1];   
        else
            sprite = car_sprites[2];  

        LCD_Draw_Sprite(cx, cy, CAR_W, CAR_H, sprite);
    }
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
 
        
        if (i == 0)
        {
            enemies[i].speed = CAR_SPEED;   // 3
        }
        else if (i == 1)
            enemies[i].speed = CAR1_SPEED;  // 4
        else
            enemies[i].speed = CAR2_SPEED;  // 5
        enemies[i].base_speed = enemies[i].speed;
    }
}

static void PowerUp_Init(void)
{
    powerup.x = lane_x[Random_Car()];
    powerup.y = ROAD_TOP;
    powerup.type = Random_Car() % 2;
}

static void Reward_Init(void)
{
    reward.x = lane_x[Random_Car()];
    reward.y = ROAD_TOP;
}

static void Reward_Update(void)
{
    reward.y += SPEED;

    for (int i = 0; i < enemy_count; i++)
    {
        if (abs(enemies[i].x - reward.x) < CAR_W)
        {
            int dy = enemies[i].y - reward.y;

            if (dy > 0 && dy < ITEM_SAFE_DISTANCE)
            {
                reward.y -= SPEED;
            }
        }
    }

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

static void PowerUp_Update(void)
{
    powerup.y += SPEED;

    for (int i = 0; i < enemy_count; i++)
    {
        if (abs(enemies[i].x - powerup.x) < CAR_W)
        {
            int dy = enemies[i].y - powerup.y;

            if (dy > 0 && dy < ITEM_SAFE_DISTANCE)
            {
                powerup.y -= SPEED;
            }
        }
    }

    if (powerup.y > ROAD_BOTTOM)
    {
        PowerUp_Init();
    }
}



MenuState Game2_Run(void) {
    // Initialize game state
    int win = 0;
    int game_playing = 0; 
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

    
    if (selected_map == BACK)
    { 
        return MENU_STATE_HOME;
    }

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
    PowerUp_Init(); 

    uint32_t btn3_exit = 0;

    while(remaining_lives > 0)
    {
        if (btn3_exit) break;

        LCD_Fill_Buffer(0);
        LCD_Refresh(&cfg0);
        buzzer_tone(&buzzer_cfg, 3000, 30);  // 1kHz at 30% volume
        HAL_Delay(50);  // Brief beep duration
        buzzer_off(&buzzer_cfg);  // Stop the buzzer


        game_playing = 1;
        //Reset player position
        Initialize_Player_Center();
        
        HAL_Delay(300); // Brief pause before starting

        // Initialize button hold tracking for returning to submenu
        uint32_t btn3_press_start_ms = 0;   // Press start time
        uint32_t btn3_hold_ms = 0;          // Time held down
        

        while (1) 
        {
            uint32_t frame_start = HAL_GetTick();

            // Read input
            Input_Read();

            // Press Instant ->Record Start Time
            if (current_input.btn3_pressed) btn3_press_start_ms = frame_start;
            if (btn3_press_start_ms != 0) 
            {
                if (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
                {
                    btn3_hold_ms = frame_start - btn3_press_start_ms;

                    buzzer_tone(&buzzer_cfg, 2000, 30);  // 1kHz at 30% volume
                    HAL_Delay(23);  // Brief beep duration
                    buzzer_off(&buzzer_cfg);  // Stop the buzzer

                    if (btn3_hold_ms >= 2000) 
                    {
                        btn3_exit = 1;
                        break;
                    }
                }
                else //Release ->Reset
                {
                    btn3_press_start_ms = 0;
                    btn3_hold_ms = 0;
                }
            }
            

            char lives_text[32];
            char score_text[32];

            //Read Joystick
            Joystick_Read(&joystick_cfg, &joystick_data);
            Movement_to_Joystick(&joystick_data);
        
            // Read input
            Input_Read();
        
            // Check if button was pressed to return to menu
            
            if (current_input.btn3_pressed && !game_playing)
            {
                exit_state = MENU_STATE_HOME;
                break;
            }


            for (int i = 0; i < enemy_count; i++)
            {
                enemies[i].speed = enemies[i].base_speed;

                for (int j = 0; j < enemy_count; j++)
                {
                    if (i != j) 
                    {
                        if (enemies[i].x == enemies[j].x && enemies[j].y > enemies[i].y)
                        {
                            int distance = enemies[j].y - enemies[i].y;
                            if (distance < FOLLOW_DISTANCE)
                            {
                                if (enemies[i].speed > enemies[j].speed)
                                {
                                    enemies[i].speed = enemies[j].speed;
                                }
                            }   
                        }
                    }
                }
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
                if (!invincible && Circles_Overlap(player_x, player_y, PLAYER_RADIUS,enemies[i].x, enemies[i].y, ENEMY_RADIUS))
                {
                    remaining_lives--;

                    buzzer_tone(&buzzer_cfg, 400, 60);
                    HAL_Delay(120);
                    buzzer_off(&buzzer_cfg);

                    invincible = 1;
                    invincible_start_ms = HAL_GetTick();

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


            
            // Player collects power-up
            if (Circles_Overlap(player_x, player_y, PLAYER_RADIUS,powerup.x, powerup.y, 4))
            {
                if (powerup.type == POWER_HEAL)
                {
                    if (remaining_lives < 3)
                    {
                        remaining_lives++;
                    }
                }
                else if (powerup.type == POWER_INVINC)
                {
                    invincible = 1;
                    invincible_start_ms = HAL_GetTick();  
                }

                PowerUp_Init();   
            }

            if (score >= WIN_SCORE)
            {
                win = 1;
                exit_state = MENU_STATE_HOME;
                break;
            }


            if (invincible)
            {
                if (HAL_GetTick() - invincible_start_ms >= INVINCIBLE_TIME_MS)
                {
                    invincible = 0;  
                }
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
        PowerUp_Update();

        Draw_Game2_Map();

        Draw_Player();

        Draw_Reward();

        Draw_PowerUp();
        Draw_Enemies();
        
    

        
    sprintf(lives_text, "Lives: %lu", (unsigned long)remaining_lives);
    sprintf(score_text, "Score: %lu", (unsigned long)score);
    LCD_printString(lives_text, 10, 10, 2, 2);
    LCD_printString(score_text, 130, 10, 2, 2);

        
        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME2_FRAME_TIME_MS) 
        {
            HAL_Delay(GAME2_FRAME_TIME_MS - frame_time);
        }
        }
  
        if (win || remaining_lives == 0)
        {
            break;
        }
            
    }
     
           
    // Victory celebration
    if (win)
    {
        for (int i = 0; i < 6; i++)
        {
            LCD_Fill_Buffer(0);
            LCD_printString("YOU WIN!", 50, 100, 2, 3);
            LCD_printString("SCORE 100+", 40, 140, 1, 2);
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

    HAL_Delay(2000);

    
    return exit_state;  // Tell main where to go next
}
