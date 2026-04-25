#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "SubMenu_1.h"
#include "Joystick.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "rng.h"
#include "tim.h"
#include "main.h"


extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control
extern volatile uint32_t g_tim6_ticks;
extern InputState current_input;

/**
 * @brief Game 1 Implementation - Student can modify
 * 
 * EXAMPLE: Shows how to use PWM LED for visual feedback
 * This is a placeholder with a bouncing animation that changes LED brightness.
 * Replace this with your actual game logic!
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
#define PLAYER_LIVES 5
// Player Movement parameters
#define MOVE_SPEED 2      // Pixels to move per update
#define MOVE_DELAY_MS 30  // Milliseconds between movement updates

// Enemy parameters
#define TARGET_RADIUS 4
#define TARGET_COLOR 6
#define TARGET_COUNT 5
#define TARGET_FALL_SPEED 1
#define TARGET_MOVE_DELAY_MS 50

// Bullet parameters
#define BULLET_RADIUS 2
#define BULLET_SPEED 3
#define BULLET_FIRE_INTERVAL_MS 300
#define MAX_BULLETS 32

#define BTN3_HOLD_MS 1500


// static const char* mode_options[] = {
//     "Easy", // 模式内部显示
//     "Hard",
//     "Infinite"
// };

// ===== Game Function Prototypes =====
uint8_t Circles_Overlap(uint16_t x1, uint16_t y1, uint16_t r1,
                        uint16_t x2, uint16_t y2, uint16_t r2);

void Place_Target(uint8_t index,
                  uint16_t *target_x,
                  uint16_t *target_y,
                  uint16_t player_x,
                  uint16_t player_y);

// ===== Game Function Implementations =====

static uint16_t Random_U16(uint16_t max)
{
  uint32_t rnd = 0;
  HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
  return (uint16_t)(rnd % max);
}

// Reference: Unit 3.2 Joystick

/**
 * @brief Check if two circles overlap (for collision detection)
 * 
 * Two circles collide when the distance between their centers is less than
 * the sum of their radii. This function calculates the squared distance to
 * avoid expensive sqrt() operations - we compare squared distances instead.
 * 
 * @param x1, y1 Center coordinates of first circle
 * @param x2, y2 Center coordinates of second circle
 * @param r1, r2 Radii of the two circles
 * @return 1 if circles overlap, 0 otherwise
 */
uint8_t Circles_Overlap(uint16_t x1, uint16_t y1, uint16_t r1,
                        uint16_t x2, uint16_t y2, uint16_t r2)
{
  int32_t dx = (int32_t)x2 - (int32_t)x1;
  int32_t dy = (int32_t)y2 - (int32_t)y1;
  int32_t dist_squared = (dx * dx) + (dy * dy);
  int32_t radii_sum = r1 + r2;
  int32_t radii_sum_squared = radii_sum * radii_sum;
  
  return (dist_squared <= radii_sum_squared) ? 1 : 0;
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
void Place_Target(uint8_t index,
                  uint16_t *target_x,
                  uint16_t *target_y,
                  uint16_t player_x,
                  uint16_t player_y)
{
  uint8_t tries = 0;
  const uint16_t min_spacing = (PLAYER_RADIUS + TARGET_RADIUS) * 2;  // Minimum distance from player/targets
  const uint16_t top_spawn_y = PLAY_AREA_Y0 + TARGET_RADIUS;

  while (tries < 100)
  {
    // Generate random position within play area, with margins for the target radius
    uint16_t x = Random_U16(LCD_WIDTH - 2 * TARGET_RADIUS) + TARGET_RADIUS;
    uint16_t y = top_spawn_y;

    // Check collision with player
    uint8_t collision = Circles_Overlap(x, y, min_spacing / 2, player_x, player_y, PLAYER_RADIUS);
    
    // Check collision with other targets
    if (!collision)
    {
      for (uint8_t i = 0; i < TARGET_COUNT; i++)
      {
        if (i == index)
        {
          continue;
        }

        if (Circles_Overlap(x, y, TARGET_RADIUS, target_x[i], target_y[i], TARGET_RADIUS))
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

      LCD_Draw_Circle(x, y, TARGET_RADIUS, TARGET_COLOR, 1);
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

    // 外部循环：子菜单选择模式；游戏内BT3返回这里。
    // 内部循环：运行实际游戏，直到BT3按下返回子菜单。
    // Outer loop: submenu selects mode; gameplay BT3 returns here.
    // Inner loop: runs the actual game until BT3 is pressed to return to submenu.
    while (1) {
        selected_mode = SubMenu_Run(&submenu);
        if (selected_mode == SUBMENU_1_STATE_HOME || selected_mode == SUBMENU_1_STATE_RETURN_MAIN) {
            return MENU_STATE_HOME;
        }

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
        uint32_t last_target_move_tick = HAL_GetTick();
        for (uint8_t i = 0; i < TARGET_COUNT; i++)
        {
            Place_Target(i, target_x, target_y, player_x, player_y);
        }

        // Initialize bullet
        uint16_t bullet_x[MAX_BULLETS] = {0};
        int16_t bullet_y[MAX_BULLETS] = {0};
        uint8_t bullet_active[MAX_BULLETS] = {0};
        uint32_t last_bullet_move_tick = HAL_GetTick();

        // Initialize score & life
        uint16_t score = 0;
        int16_t lives = PLAYER_LIVES;
        char hud_str[64];
        sprintf(hud_str, "Score: %2d Lives: %2d", score, lives);
        LCD_Draw_Rect(0, 0, LCD_WIDTH, 25, 0, 1);
        LCD_printString(hud_str, HUD_OFFSET_X, HUD_OFFSET_Y, 1, 2);

        // Initialize button hold tracking for returning to submenu
        uint32_t btn3_press_start_ms = 0;   // 按下开始时间
        uint32_t btn3_hold_ms = 0;          // 已按住的时间
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

            // 按下瞬间 -> 记录起始时间
            if (current_input.btn3_pressed) btn3_press_start_ms = frame_start;
            // 如果已经开始计时
            if (btn3_press_start_ms != 0) 
            {
                // Read GPIO 是否仍然按住
                if (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
                {
                    btn3_hold_ms = frame_start - btn3_press_start_ms;
                    sprintf(btn_hold_str,
                            "Hold BT3: %lu / %u ms",
                            btn3_hold_ms, BTN3_HOLD_MS);

                    LCD_Draw_Rect(0, LCD_HEIGHT - 20, LCD_WIDTH, 20, 0, 1);
                    LCD_printString(btn_hold_str, 10, 220, 1, 1);

                    if (btn3_hold_ms >= BTN3_HOLD_MS) {
                        break;
                    }
                }
                else // 松手 -> 清零
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
                // 将位置限制在屏幕边界内（防止玩家离开屏幕）
                // 保持玩家半径远离边缘，以便整个圆圈始终可见
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
            // 仅当玩家实际移动时才重新绘制（避免不必要的 LCD 操作）
            if (player_x != prev_player_x || player_y != prev_player_y)
            {
                // Erase player at old position (draw circle in background color)
                // Draw player at new position (draw circle in player color)
                // 擦除玩家上一帧的图像（以背景色绘制圆圈）
                // 在新位置绘制玩家（用玩家颜色绘制圆圈）                
                LCD_Draw_Circle(prev_player_x, prev_player_y, PLAYER_RADIUS, 0, 1);
                LCD_Draw_Circle(player_x, player_y, PLAYER_RADIUS, PLAYER_COLOR, 1);

                prev_player_x = player_x;
                prev_player_y = player_y;
            }

            // ===== STEP 4: Bullet Create and Movement =====
            // Update - fire
            bool fired = false;
            if ((frame_start - last_bullet_move_tick) >= BULLET_FIRE_INTERVAL_MS)
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
            if ((frame_start - last_target_move_tick) >= TARGET_MOVE_DELAY_MS)
            {
                for (uint8_t i = 0; i < TARGET_COUNT; i++)
                {
                    LCD_Draw_Circle(target_x[i], target_y[i], TARGET_RADIUS, 0, 1);

                    uint16_t new_target_y = target_y[i] + TARGET_FALL_SPEED;
                    if (new_target_y >= (LCD_HEIGHT - TARGET_RADIUS))
                    {
                        Place_Target(i, target_x, target_y, player_x, player_y);
                        continue;
                    }
                    target_y[i] = new_target_y;
                    LCD_Draw_Circle(target_x[i], target_y[i], TARGET_RADIUS, 6, 1);
                }

            last_target_move_tick = frame_start;
            }

            // ===== STEP -2: Collision Detection =====
            // Circle overlap collision: check if circles touch or overlap
            // Two circles collide when distance between centers < sum of radii
            // 圆重叠碰撞检测：检查圆是否接触或重叠
            // 当两个圆心之间的距离小于半径之和时，这两个圆会发生碰撞。            
            for (uint8_t i = 0; i < TARGET_COUNT; i++)
            {
                // Check if player circle overlaps with target
                if (Circles_Overlap(player_x, player_y, PLAYER_RADIUS, 
                                    target_x[i], target_y[i], TARGET_RADIUS))
                {
                    // When the player collides with the enemy, Erase enemy
                    LCD_Draw_Circle(target_x[i], target_y[i], TARGET_RADIUS, 0, 1);
                    Place_Target(i, target_x, target_y, player_x, player_y);

                    score++;
                    lives--; // Decrease lives when player collides with target
                }

                // Check if any active bullets overlap with target
                for (uint8_t b = 0; b < MAX_BULLETS; b++)
                {
                    if (!bullet_active[b]) continue;

                    if (Circles_Overlap(bullet_x[b], bullet_y[b], BULLET_RADIUS,
                                        target_x[i], target_y[i], TARGET_RADIUS))
                    {
                        // When the bullet collides with the enemy, Erase bullet
                        LCD_Draw_Circle(target_x[i], target_y[i], TARGET_RADIUS, 0, 1);
                        LCD_Draw_Circle(bullet_x[b], bullet_y[b], BULLET_RADIUS, 0, 1);
                        bullet_active[b] = 0;
                        score++;

                        Place_Target(i, target_x, target_y, player_x, player_y);
                        break;
                    }
                }
            }                        
            sprintf(hud_str, "Score: %2d Lives: %2d", score, lives);
            LCD_Draw_Rect(0, 0, LCD_WIDTH, 25, 0, 1);
            LCD_printString(hud_str, HUD_OFFSET_X, HUD_OFFSET_Y, 1, 2);


            // ===== STEP -1: Update Display =====
            // Transfer the frame buffer to the LCD hardware (makes all draws visible)
            LCD_Refresh(&cfg0);

            // // UPDATE: Game logic
            // animation_counter++;

            // // Simple animation: move object back and forth
            // moving_x += move_direction * move_step;
            // if (moving_x >= 200 || moving_x <= 0) {
            //     move_direction *= -1;
            // }

            // Example: Vary LED brightness based on animation
            // uint8_t brightness = (moving_x * 100) / 200;
            // PWM_SetDuty(&pwm_cfg, brightness);

            // // RENDER: Draw to LCD
            // LCD_Fill_Buffer(0);

            // // Title
            // LCD_printString("GAME 1", 60, 10, 1, 3);
            // LCD_printString((char*)mode_options[selected_mode - SUBMENU_1_STATE_1], 60, 45, 1, 2);

            // // Simple animated object (moving box)
            // LCD_printString("[*]", 20 + moving_x, 100, 1, 3);

            // // Display counter
            // char counter[32];
            // sprintf(counter, "Frame: %lu", animation_counter);
            // LCD_printString(counter, 50, 150, 1, 2);

            // // Show PWM LED usage
            // LCD_printString("LED: PWM Demo", 30, 180, 1, 1);
            // char pwm_str[32];
            // sprintf(pwm_str, "Brightness: %d%%", brightness);
            // LCD_printString(pwm_str, 30, 195, 1, 1);

            // // Instructions
            // LCD_printString("Press BT3 to", 40, 210, 1, 1);
            // LCD_printString("Back to Mode Menu", 20, 225, 1, 1);

            // LCD_Refresh(&cfg0);

            // Frame timing - wait for remainder of frame time
            uint32_t frame_time = HAL_GetTick() - frame_start;
            if (frame_time < GAME1_FRAME_TIME_MS) {
                HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
            }
        }
    }
}
