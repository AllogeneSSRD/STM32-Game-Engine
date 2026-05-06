// standard library 
#include <stdio.h>
#include <stdbool.h>

// STM32 / HAL
#include "stm32l4xx_hal.h"
#include "main.h"
#include "rng.h"
#include "tim.h"

// Hardware drivers
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "InputHandler.h"

// System modules
#include "Menu.h"
#include "Utils.h"

// Game modules
#include "Game_1.h"
#include "SubMenu_1.h"
#include "Utils_1.h"


extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control
extern volatile uint32_t g_tim6_ticks;
extern InputState current_input;

/**
 * @brief Game 1 Implementation - Student can modify
 * 
 */

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

// ===== Const Definitions =====
// LCD display dimensions
#define LCD_WIDTH 240
#define LCD_HEIGHT 240
#define PLAY_AREA_Y0 25  // Leave space at top for title
#define HUD_OFFSET_X 5
#define HUD_OFFSET_Y 5

// Player parameters 
#define PLAYER_RADIUS 6
#define PLAYER_COLOR 2
#define PLAYER_HP 10
// Player Movement parameters
#define MOVE_SPEED 2      // Pixels to move per update
#define MOVE_DELAY_MS 30  // Milliseconds between movement updates

// Enemy parameters
#define TARGET_RADIUS 4
#define TARGET_COLOR 6
#define TARGET_COUNT 8
#define TARGET_FALL_SPEED 1
#define TARGET_MOVE_DELAY_MS 66

#define TARGET_ADVANCED_RADIUS 6
#define TARGET_ADVANCED_COLOR 5
#define TARGET_ADVANCED_HP 3

#define TARGET_BOSS_RADIUS 9
#define TARGET_BOSS_COLOR 1
#define TARGET_BOSS_HP 8

#define TARGET_ITEM_WIDTH 10
#define TARGET_ITEM_HEIGHT 10
#define TARGET_ITEM_FALL_SPEED 1
#define TARGET_ITEM_HEAL_COLOR 7
#define TARGET_ITEM_SPREAD_COLOR 14
#define TARGET_ITEM_HEAL_HP 1
#define TARGET_ITEM_SPREAD_HP 1

#define TARGET_SCORE_NORMAL 1
#define TARGET_SCORE_ADVANCED 5
#define TARGET_SCORE_BOSS 20

// Bullet parameters
#define BULLET_RADIUS 2
#define BULLET_SPEED 3
#define BULLET_FIRE_INTERVAL_MS 300
#define MAX_BULLETS 32

#define SPREAD_POWERUP_DURATION_MS 7000
#define SPREAD_BULLET_COUNT 5
#define SPREAD_BULLET_OFFSET 10

#define MAX_ENEMY_BULLETS 8
#define ENEMY_BULLET_RADIUS 2
#define ENEMY_BULLET_SPEED 2
#define ENEMY_BULLET_COLOR 7

#define EASY_WIN_SCORE 100
#define HARD_WIN_SCORE 200

#define BTN3_HOLD_MS 1500

// ===== Game Function Prototypes =====
typedef struct {
    const char* mode_name;
    uint8_t target_fall_speed;
    uint16_t target_move_delay_ms;
    uint16_t bullet_fire_interval_ms;
    uint16_t boss_fire_interval_ms;
    uint8_t advanced_spawn_chance;
    uint8_t boss_spawn_chance;
    uint8_t heal_spawn_chance;
    uint8_t spread_spawn_chance;
    uint16_t win_score; // 0 means endless mode
} GameDifficulty;

typedef enum {
    TARGET_TYPE_NORMAL = 0,
    TARGET_TYPE_ADVANCED,
    TARGET_TYPE_BOSS,
    TARGET_TYPE_ITEM_HEAL,
    TARGET_TYPE_ITEM_SPREAD
} TargetType;


static GameDifficulty Get_Difficulty_From_Mode(SubMenuState selected_mode);
static TargetType Pick_Target_Type(const GameDifficulty *difficulty);
static uint8_t Get_Target_Radius(TargetType type);
static uint8_t Get_Target_Color(TargetType type);
static int16_t Get_Target_Initial_HP(TargetType type);
static uint16_t Get_Target_Kill_Score(TargetType type);
static uint8_t Is_Target_Item(TargetType type);
static void Draw_Target_Entity(uint16_t x, uint16_t y, TargetType type, uint8_t color);
static void Place_Target(uint8_t index,
                  uint16_t *target_x,
                  uint16_t *target_y,
                  TargetType *target_type,
                  int16_t *target_hp,
                  uint32_t *target_last_fire_tick,
                  uint16_t player_x,
                  uint16_t player_y,
                  const GameDifficulty *difficulty);

// ===== Game Function Implementations =====

static GameDifficulty Get_Difficulty_From_Mode(SubMenuState selected_mode)
{
    GameDifficulty cfg;

    if (selected_mode == SUBMENU_1_STATE_1)
    {
        cfg.mode_name = "Easy";
        cfg.target_fall_speed = TARGET_FALL_SPEED;
        cfg.target_move_delay_ms = TARGET_MOVE_DELAY_MS;
        cfg.bullet_fire_interval_ms = BULLET_FIRE_INTERVAL_MS;
        cfg.boss_fire_interval_ms = 1200;
        cfg.advanced_spawn_chance = 20;
        cfg.boss_spawn_chance = 5;
        cfg.heal_spawn_chance = 10;
        cfg.spread_spawn_chance = 10;
        cfg.win_score = EASY_WIN_SCORE;
    }
    else if (selected_mode == SUBMENU_1_STATE_2)
    {
        // Hard: faster enemies and slightly slower player shooting
        cfg.mode_name = "Hard";
        cfg.target_fall_speed = 2;
        cfg.target_move_delay_ms = TARGET_MOVE_DELAY_MS;
        cfg.bullet_fire_interval_ms = BULLET_FIRE_INTERVAL_MS;
        cfg.boss_fire_interval_ms = 700;
        cfg.advanced_spawn_chance = 35;
        cfg.boss_spawn_chance = 12;
        cfg.heal_spawn_chance = 5;
        cfg.spread_spawn_chance = 8;
        cfg.win_score = HARD_WIN_SCORE;
    }
    else
    {
        cfg.mode_name = "Infinite";
        cfg.target_fall_speed = TARGET_FALL_SPEED;
        cfg.target_move_delay_ms = TARGET_MOVE_DELAY_MS;
        cfg.bullet_fire_interval_ms = BULLET_FIRE_INTERVAL_MS;
        cfg.boss_fire_interval_ms = 1000;
        cfg.advanced_spawn_chance = 30;
        cfg.boss_spawn_chance = 10;
        cfg.heal_spawn_chance = 5;
        cfg.spread_spawn_chance = 5;
        cfg.win_score = 0;
    }

    return cfg;
}

static TargetType Pick_Target_Type(const GameDifficulty *difficulty)
{
    uint16_t r = Random_U16(100);
    if (r < difficulty->boss_spawn_chance)
        return TARGET_TYPE_BOSS;
    r -= difficulty->boss_spawn_chance;

    if (r < difficulty->advanced_spawn_chance)
        return TARGET_TYPE_ADVANCED;
    r -= difficulty->advanced_spawn_chance;

    if (r < difficulty->heal_spawn_chance)
        return TARGET_TYPE_ITEM_HEAL;
    r -= difficulty->heal_spawn_chance;

    if (r < difficulty->spread_spawn_chance)
        return TARGET_TYPE_ITEM_SPREAD;

    return TARGET_TYPE_NORMAL;
}

static uint8_t Get_Target_Radius(TargetType type)
{
    if (type == TARGET_TYPE_ADVANCED)       return TARGET_ADVANCED_RADIUS;
    if (type == TARGET_TYPE_BOSS)           return TARGET_BOSS_RADIUS;
    if (type == TARGET_TYPE_ITEM_HEAL || type == TARGET_TYPE_ITEM_SPREAD)
        return Max(TARGET_ITEM_WIDTH, TARGET_ITEM_HEIGHT) / 2; // Treat items as circles for collision
    else                                    return TARGET_RADIUS;
}

static uint8_t Get_Target_Color(TargetType type)
{
    if (type == TARGET_TYPE_ADVANCED)       return TARGET_ADVANCED_COLOR;
    if (type == TARGET_TYPE_BOSS)           return TARGET_BOSS_COLOR;
    if (type == TARGET_TYPE_ITEM_HEAL)      return TARGET_ITEM_HEAL_COLOR;
    if (type == TARGET_TYPE_ITEM_SPREAD)    return TARGET_ITEM_SPREAD_COLOR;
else                                        return TARGET_COLOR;
}

static int16_t Get_Target_Initial_HP(TargetType type)
{
    if (type == TARGET_TYPE_ADVANCED)       return TARGET_ADVANCED_HP;
    if (type == TARGET_TYPE_BOSS)           return TARGET_BOSS_HP;
    if (type == TARGET_TYPE_ITEM_HEAL)      return TARGET_ITEM_HEAL_HP;
    if (type == TARGET_TYPE_ITEM_SPREAD)    return TARGET_ITEM_SPREAD_HP;
    else                                    return 1;
}

static uint16_t Get_Target_Kill_Score(TargetType type)
{
    if (type == TARGET_TYPE_ADVANCED)       return TARGET_SCORE_ADVANCED;
    if (type == TARGET_TYPE_BOSS)           return TARGET_SCORE_BOSS;
    else                                    return TARGET_SCORE_NORMAL;
}

static uint8_t Is_Target_Item(TargetType type)
{
    return (type == TARGET_TYPE_ITEM_HEAL || type == TARGET_TYPE_ITEM_SPREAD) ? 1 : 0;
}


static void Draw_Target_Entity(uint16_t x, uint16_t y, TargetType type, uint8_t color)
{
    if (Is_Target_Item(type))
    {
        uint16_t draw_x = (x > (TARGET_ITEM_WIDTH / 2)) ? (x - (TARGET_ITEM_WIDTH / 2)) : 0;
        uint16_t draw_y = (y > (TARGET_ITEM_HEIGHT / 2)) ? (y - (TARGET_ITEM_HEIGHT / 2)) : PLAY_AREA_Y0;
        LCD_Draw_Rect(draw_x, draw_y, TARGET_ITEM_WIDTH, TARGET_ITEM_HEIGHT, color, 1);
    }
    else
    {
        LCD_Draw_Circle(x, y, Get_Target_Radius(type), color, 1);
    }
}


/**
 * @brief Place a target at a random screen location that doesn't collide with player or other targets
 * 
 * This function uses a trial-and-error approach to find a valid spawn location:
 * 1) Generate random X/Y pixel coordinates within screen bounds
 * 2) Check if the location collides with the player using circle overlap detection
 * 3) Check if the location collides with any other targets
 * 4) If no collision, place the target and draw it on screen
 * 5) If collision detected, try again (up to 100 attempts)
 * 
 * The collision checking uses circle overlap: two circles collide if the distance
 * between their centers is less than the sum of their radii. This creates smooth,
 * natural-feeling collisions rather than harsh grid-based detection.
 * 
 * @param index Which target slot (0 to TARGET_COUNT-1) to place
 * @param target_x Array holding X pixel positions of all targets
 * @param target_y Array holding Y pixel positions of all targets
 * @param player_x Current player X position (to avoid spawning on player)
 * @param player_y Current player Y position
 */
static void Place_Target(uint8_t index,
                  uint16_t *target_x,
                  uint16_t *target_y,
                  TargetType *target_type,
                  int16_t *target_hp,
                  uint32_t *target_last_fire_tick,
                  uint16_t player_x,
                  uint16_t player_y,
                  const GameDifficulty *difficulty)
{
    uint8_t tries = 0;
    TargetType picked_type = Pick_Target_Type(difficulty);
    uint8_t spawn_radius = Get_Target_Radius(picked_type);
    uint8_t spawn_color = Get_Target_Color(picked_type);
    const uint16_t min_spacing = (PLAYER_RADIUS + TARGET_BOSS_RADIUS);
    const uint16_t top_spawn_y = PLAY_AREA_Y0 + spawn_radius;

    while (tries < 100)
    {
        // Generate random position within play area, with margins for the target radius
        uint16_t x = Random_U16(LCD_WIDTH - 2 * spawn_radius) + spawn_radius;
        uint16_t y = top_spawn_y;

        // Check collision with player
        uint8_t collision = Circles_Overlap(x, y, min_spacing / 2, player_x, player_y, PLAYER_RADIUS);
        
        // Check collision with other targets
        if (!collision)
        {
            for (uint8_t i = 0; i < TARGET_COUNT; i++)
            {
                if (i == index) continue;
                if (target_y[i] != 0 &&
                    Circles_Overlap(x, y, spawn_radius, 
                                    target_x[i], target_y[i], 
                                    Get_Target_Radius(target_type[i])))
                {
                    collision = 1;
                    break;
                }
            }
        }

        if (!collision)
        {
            target_x[index] = x;
            target_y[index] = y;
            target_type[index] = picked_type;
            target_hp[index] = Get_Target_Initial_HP(picked_type);
            target_last_fire_tick[index] = HAL_GetTick();

            Draw_Target_Entity(x, y, picked_type, spawn_color);
            return;
        }

        tries++;
    }
}


MenuState Game1_Run(void) 
{
    SubMenuSystem submenu;
    SubMenuState selected_mode;
    Joystick_t joystick_data;

    SubMenu_Init(&submenu);

    // Outer loop: submenu selects mode; gameplay BT3 returns here.
    // Inner loop: runs the actual game until BT3 is pressed to return to submenu.
    while (1) {
        selected_mode = SubMenu_Run(&submenu);
        if (selected_mode == SUBMENU_1_STATE_HOME || selected_mode == SUBMENU_1_STATE_RETURN_MAIN) {
            return MENU_STATE_HOME;
        }

        GameDifficulty difficulty = Get_Difficulty_From_Mode(selected_mode);

        // Play a brief startup sound
        buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
        HAL_Delay(50);  // Brief beep duration
        buzzer_off(&buzzer_cfg);  // Stop the buzzer

        // ===== Initialize Game =====
        LCD_Fill_Buffer(0);
        
        // Initialize player at center of screen
        uint16_t player_x = LCD_WIDTH / 2;
        uint16_t player_y = LCD_HEIGHT / 2;
        uint16_t prev_player_x = player_x;
        uint16_t prev_player_y = player_y;
        uint32_t last_move_tick = HAL_GetTick();
        // Draw player
        LCD_Draw_Circle(player_x, player_y, PLAYER_RADIUS, PLAYER_COLOR, 1);

        // Initial targets
        uint16_t target_x[TARGET_COUNT] = {0};
        uint16_t target_y[TARGET_COUNT] = {0};
        int16_t target_hp[TARGET_COUNT] = {0};
        TargetType target_type[TARGET_COUNT] = {TARGET_TYPE_NORMAL};
        uint32_t target_last_fire_tick[TARGET_COUNT] = {0};
        uint32_t last_target_move_tick = HAL_GetTick();
        for (uint8_t i = 0; i < TARGET_COUNT; i++)
        {
            Place_Target(i, target_x, target_y, target_type, target_hp, target_last_fire_tick,
                         player_x, player_y, &difficulty);
        }

        // Initialize bullet
        uint16_t bullet_x[MAX_BULLETS] = {0};
        int16_t bullet_y[MAX_BULLETS] = {0};
        uint8_t bullet_active[MAX_BULLETS] = {0};
        uint32_t last_bullet_move_tick = HAL_GetTick();

        // Initialize boss bullets
        uint16_t enemy_bullet_x[MAX_ENEMY_BULLETS] = {0};
        int16_t enemy_bullet_y[MAX_ENEMY_BULLETS] = {0};
        uint8_t enemy_bullet_active[MAX_ENEMY_BULLETS] = {0};

        uint32_t spread_shot_until_ms = 0;

        // Initialize score & life
        uint16_t score = 0;
        int16_t lives = PLAYER_HP;
        char hud_str[64];
        if (difficulty.win_score > 0)
        {
            sprintf(hud_str, "%s  %2d/%3d  L:%2d", difficulty.mode_name, score, difficulty.win_score, lives);
        }
        else
        {
            sprintf(hud_str, "INF  %2d  L:%2d", score, lives);
        }
        LCD_Draw_Rect(0, 0, LCD_WIDTH, 25, 0, 1);
        LCD_printString(hud_str, HUD_OFFSET_X, HUD_OFFSET_Y, 1, 2);

        // Initialize button hold tracking for returning to submenu
        uint32_t btn3_press_start_ms = 0;   // Press start time
        uint32_t btn3_hold_ms = 0;          // Hold time duration
        char btn_hold_str[64];

        LCD_Draw_Rect(0, LCD_HEIGHT - 20, LCD_WIDTH, 20, 0, 1);
        LCD_printString("Hold BT3 to return", 10, 220, 1, 1);

        LCD_Refresh(&cfg0);

        // Game loop
        while (1) 
        {
            uint32_t frame_start = HAL_GetTick();

            // Read input
            Input_Read();

            // Button press moment -> record start time
            if (current_input.btn3_pressed) btn3_press_start_ms = frame_start;
            if (btn3_press_start_ms != 0) 
            {
                if (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
                {
                    btn3_hold_ms = frame_start - btn3_press_start_ms;
                    sprintf(btn_hold_str,
                            "Hold BT3: %lu / %u ms",
                            btn3_hold_ms, BTN3_HOLD_MS);

                    LCD_Draw_Rect(0, LCD_HEIGHT - 20, LCD_WIDTH, 20, 0, 1);
                    LCD_printString(btn_hold_str, 10, 220, 1, 1);

                    if (btn3_hold_ms >= BTN3_HOLD_MS) 
                    {
                        // Game over
                        char score_str[64];
                        sprintf(score_str, "Score: %2d  HP: %2d", score, lives);

                        buzzer_note(&buzzer_cfg, NOTE_A4, 60);
                        LCD_Fill_Buffer(0);
                        LCD_printString("GAME Exit", 40, 120, 15, 3);
                        LCD_printString(score_str, 20, 160, 1, 2);
                        LCD_Refresh(&cfg0);
                        HAL_Delay(3500);
                        buzzer_off(&buzzer_cfg);

                        LCD_Fill_Buffer(0);
                        LCD_printString("BACK TO MENU...", 20, 120, 1, 2);
                        LCD_Refresh(&cfg0);
                        HAL_Delay(1000);
                        break;
                    }
                }
                else // Button released before hold threshold -> reset start time
                {
                    btn3_press_start_ms = 0;
                    btn3_hold_ms = 0;
                    LCD_Draw_Rect(0, LCD_HEIGHT - 20, LCD_WIDTH, 20, 0, 1);
                    // LCD_printString("Hold BT3 to return", 10, 220, 1, 1);
                }
            }

            // ===== STEP 1: Read Joystick Input =====
            Joystick_Read(&joystick_cfg, &joystick_data);

            // ===== STEP 2: Player Movement (Rate-Limited) =====
            // Only allow movement every MOVE_DELAY_MS milliseconds to control game speed
            if ((frame_start - last_move_tick) >= MOVE_DELAY_MS && joystick_data.direction != CENTRE)
            {
            // Calculate movement delta based on joystick direction
            // dx/dy represent pixel movement in X and Y directions
            int16_t dx = 0;
            int16_t dy = 0;

            switch (joystick_data.direction)
            {
                case N:  dy = -MOVE_SPEED; break;
                case NE: dy = -MOVE_SPEED; dx =  MOVE_SPEED; break;
                case E:  dx =  MOVE_SPEED; break;
                case SE: dy =  MOVE_SPEED; dx =  MOVE_SPEED; break;
                case S:  dy =  MOVE_SPEED; break;
                case SW: dy =  MOVE_SPEED; dx = -MOVE_SPEED; break;
                case W:  dx = -MOVE_SPEED; break;
                case NW: dy = -MOVE_SPEED; dx = -MOVE_SPEED; break;
                default: break;
            }

            // Apply movement if joystick is deflected
            if (dx != 0 || dy != 0)
            {
                int32_t new_x = (int32_t)player_x + dx;
                int32_t new_y = (int32_t)player_y + dy;

                // Clamp position to screen boundaries (prevent player from leaving the display)
                // Keep player radius away from edges so the full circle stays visible
                if (new_x < PLAYER_RADIUS) new_x = PLAYER_RADIUS;
                if (new_x >= (LCD_WIDTH - PLAYER_RADIUS)) new_x = LCD_WIDTH - PLAYER_RADIUS - 1;
                if (new_y < (PLAY_AREA_Y0 + PLAYER_RADIUS)) new_y = PLAY_AREA_Y0 + PLAYER_RADIUS;
                if (new_y >= (LCD_HEIGHT - PLAYER_RADIUS)) new_y = LCD_HEIGHT - PLAYER_RADIUS - 1;

                player_x = (uint16_t)new_x;
                player_y = (uint16_t)new_y;
            }

            last_move_tick = frame_start;
            }

            // ===== STEP 3: Render Player Movement =====
            // Only redraw if player actually moved (avoids unnecessary LCD operations)
            if (player_x != prev_player_x || player_y != prev_player_y)
            {
                // Erase player at old position (draw circle in background color)
                // Draw player at new position (draw circle in player color)           
                LCD_Draw_Circle(prev_player_x, prev_player_y, PLAYER_RADIUS, 0, 1);
                LCD_Draw_Circle(player_x, player_y, PLAYER_RADIUS, PLAYER_COLOR, 1);

                prev_player_x = player_x;
                prev_player_y = player_y;
            }

            // ===== STEP 4: Bullet Create and Movement =====
            // Update - fire
            bool fired = false;
            if ((frame_start - last_bullet_move_tick) >= difficulty.bullet_fire_interval_ms)
            {
                if (spread_shot_until_ms > frame_start)
                {
                    for (int8_t spread = 0; spread < SPREAD_BULLET_COUNT; spread++)
                    {
                        for (uint8_t i = 0; i < MAX_BULLETS; i++)
                        {
                            if (!bullet_active[i])
                            {
                                int32_t bullet_spawn_x = (int32_t)player_x + ((spread - (SPREAD_BULLET_COUNT / 2)) * SPREAD_BULLET_OFFSET);
                                bullet_spawn_x = Clamp(bullet_spawn_x, BULLET_RADIUS, LCD_WIDTH - BULLET_RADIUS - 1);
                                bullet_x[i] = (uint16_t)bullet_spawn_x;
                                bullet_y[i] = (int16_t)player_y - PLAYER_RADIUS - BULLET_RADIUS;
                                bullet_active[i] = 1;
                                fired = true;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    for (uint8_t i = 0; i < MAX_BULLETS; i++)
                    {
                        if (!bullet_active[i])
                        {
                            bullet_x[i] = player_x;
                            bullet_y[i] = (int16_t)player_y - PLAYER_RADIUS - BULLET_RADIUS;
                            bullet_active[i] = 1;
                            fired = true;
                            break;
                        }
                    }
                }
                if (fired) last_bullet_move_tick = frame_start;
            }

            // Update - move
            for (uint8_t i = 0; i < MAX_BULLETS; i++)
            {
                // Old position for bullet (used to erase previous frame)
                if (!bullet_active[i]) continue;
                LCD_Draw_Circle(bullet_x[i], (uint16_t)bullet_y[i], BULLET_RADIUS, 0, 1);

                // New position for bullet (move up)
                bullet_y[i] -= BULLET_SPEED;
                if (bullet_y[i] <= (PLAY_AREA_Y0 + BULLET_RADIUS)) 
                {
                    bullet_active[i] = 0;
                }
                else
                {
                    LCD_Draw_Circle(bullet_x[i], (uint16_t)bullet_y[i], BULLET_RADIUS, 3, 1);
                }
            }

            // ===== STEP 5: Target Falling Movement =====
            if ((frame_start - last_target_move_tick) >= difficulty.target_move_delay_ms)
            {
                for (uint8_t i = 0; i < TARGET_COUNT; i++)
                {
                    uint8_t radius = Get_Target_Radius(target_type[i]);
                    uint8_t color = Get_Target_Color(target_type[i]);

                    Draw_Target_Entity(target_x[i], target_y[i], target_type[i], 0);

                    uint16_t new_target_y = target_y[i] + difficulty.target_fall_speed;
                    if (new_target_y >= (LCD_HEIGHT - radius))
                    {
                        Place_Target(i, target_x, target_y, target_type, target_hp, target_last_fire_tick,
                                     player_x, player_y, &difficulty);
                        continue;
                    }

                    target_y[i] = new_target_y;
                    Draw_Target_Entity(target_x[i], target_y[i], target_type[i], color);

                    // Boss bullet spawn
                    if (target_type[i] == TARGET_TYPE_BOSS &&
                        (frame_start - target_last_fire_tick[i]) >= difficulty.boss_fire_interval_ms)
                    {
                        for (uint8_t j = 0; j < MAX_ENEMY_BULLETS; j++)
                        {
                            if (!enemy_bullet_active[j])
                            {
                                enemy_bullet_x[j] = target_x[i];
                                enemy_bullet_y[j] = (int16_t)target_y[i] + radius + ENEMY_BULLET_RADIUS;
                                enemy_bullet_active[j] = 1;
                                target_last_fire_tick[i] = frame_start;
                                break;
                            }
                        }
                    }
                }

            last_target_move_tick = frame_start;
            }

            // Boss bullet movement and collision (same hit behavior: touch player costs life)
            for (uint8_t k = 0; k < MAX_ENEMY_BULLETS; k++)
            {
                if (!enemy_bullet_active[k]) continue;

                LCD_Draw_Circle(enemy_bullet_x[k], (uint16_t)enemy_bullet_y[k], ENEMY_BULLET_RADIUS, 0, 1);
                enemy_bullet_y[k] += ENEMY_BULLET_SPEED;

                if (enemy_bullet_y[k] >= (LCD_HEIGHT - ENEMY_BULLET_RADIUS))
                {
                    enemy_bullet_active[k] = 0;
                    continue;
                }

                if (Circles_Overlap(player_x, player_y, PLAYER_RADIUS,
                                    enemy_bullet_x[k], (uint16_t)enemy_bullet_y[k], ENEMY_BULLET_RADIUS))
                {
                    enemy_bullet_active[k] = 0;
                    lives--;
                    continue;
                }

                LCD_Draw_Circle(enemy_bullet_x[k], (uint16_t)enemy_bullet_y[k], 
                            ENEMY_BULLET_RADIUS, ENEMY_BULLET_COLOR, 1);
            }

            // ===== STEP -2: Collision Detection =====
            // Circle overlap collision: check if circles touch or overlap
            // Two circles collide when distance between centers < sum of radii        
            for (uint8_t i = 0; i < TARGET_COUNT; i++)
            {
                uint8_t target_radius = Get_Target_Radius(target_type[i]);

                // Check if player circle overlaps with target
                uint8_t player_hit = 0;
                if (Is_Target_Item(target_type[i]))
                {
                    player_hit = Circle_Rect_Overlap(player_x, player_y, PLAYER_RADIUS,
                                                     target_x[i], target_y[i],
                                                     TARGET_ITEM_WIDTH, TARGET_ITEM_HEIGHT);
                }
                else
                {
                    player_hit = Circles_Overlap(player_x, player_y, PLAYER_RADIUS,
                                                 target_x[i], target_y[i], target_radius);
                }

                if (player_hit)
                {
                    // When the player collides with the enemy, Erase enemy
                    Draw_Target_Entity(target_x[i], target_y[i], target_type[i], 0);

                    if (target_type[i] == TARGET_TYPE_ITEM_HEAL)
                    {
                        if (lives < PLAYER_HP * 2) // Prevent overheal
                        {
                            lives++;
                        }
                    }
                    else if (target_type[i] == TARGET_TYPE_ITEM_SPREAD)
                    {
                        spread_shot_until_ms = frame_start + SPREAD_POWERUP_DURATION_MS;
                    }
                    else
                    {
                        score++;
                        lives--; // Decrease lives when player collides with target
                    }

                    Place_Target(i, target_x, target_y, target_type, target_hp, target_last_fire_tick,
                                 player_x, player_y, &difficulty);
                }

                // Check if any active bullets overlap with target
                if (Is_Target_Item(target_type[i])) continue;

                for (uint8_t b = 0; b < MAX_BULLETS; b++)
                {
                    if (!bullet_active[b]) continue;

                    if (Circles_Overlap(bullet_x[b], bullet_y[b], BULLET_RADIUS,
                                        target_x[i], target_y[i], target_radius))
                    {
                        // When the bullet collides with the enemy, Erase bullet
                        Draw_Target_Entity(target_x[i], target_y[i], target_type[i], 0);
                        LCD_Draw_Circle(bullet_x[b], (uint16_t)bullet_y[b], BULLET_RADIUS, 0, 1);
                        bullet_active[b] = 0;

                        target_hp[i]--;

                        if (target_hp[i] <= 0)
                        {
                            score += Get_Target_Kill_Score(target_type[i]);
                            Place_Target(i, target_x, target_y, target_type, target_hp, target_last_fire_tick,
                                         player_x, player_y, &difficulty);
                        }
                        else
                        {
                            Draw_Target_Entity(target_x[i], target_y[i], target_type[i], Get_Target_Color(target_type[i]));
                        }

                        break;
                    }
                }
            }
            
            if (difficulty.win_score > 0)
            {
                sprintf(hud_str, "%s  %2d/%3d  L:%2d", difficulty.mode_name, score, difficulty.win_score, lives);
            }
            else
            {
                sprintf(hud_str, "INF  %2d  L:%2d", score, lives);
            }
            LCD_Draw_Rect(0, 0, LCD_WIDTH, 25, 0, 1);
            LCD_printString(hud_str, HUD_OFFSET_X, HUD_OFFSET_Y, 1, 2);


            // ===== STEP -1: Update Display =====
            // Transfer the frame buffer to the LCD hardware (makes all draws visible)
            LCD_Refresh(&cfg0);

            // Frame timing - wait for remainder of frame time
            uint32_t frame_time = HAL_GetTick() - frame_start;
            if (frame_time < GAME1_FRAME_TIME_MS) {
                HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
            }
            // Exit game loop when player has no lives left
            if (lives <= 0)
            {   
                // Game over
                char score_str[64];
                sprintf(score_str, "Score: %2d  HP: %2d", score, lives);

                buzzer_note(&buzzer_cfg, NOTE_A4, 60);
                LCD_Fill_Buffer(0);
                LCD_printString("GAME OVER", 40, 120, 15, 3);
                LCD_printString(score_str, 20, 160, 1, 2);
                LCD_Refresh(&cfg0);
                HAL_Delay(3000);
                buzzer_off(&buzzer_cfg);

                LCD_Fill_Buffer(0);
                LCD_printString("BACK TO MENU...", 20, 120, 1, 2);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);
                break;
            }

            // For easy/hard mode: exit loop when reaching target score
            if (difficulty.win_score > 0 && score >= difficulty.win_score)
            {
                buzzer_tone(&buzzer_cfg, 1500, 35);
                LCD_Fill_Buffer(0);
                LCD_printString("YOU WIN", 60, 110, 2, 3);
                LCD_printString((char*)difficulty.mode_name, 70, 145, 1, 2);
                LCD_Refresh(&cfg0);
                HAL_Delay(3000);
                buzzer_off(&buzzer_cfg);

                LCD_Fill_Buffer(0);
                LCD_printString("BACK TO MENU...", 20, 120, 1, 2);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);
                break;
            }
        }
    }
}
