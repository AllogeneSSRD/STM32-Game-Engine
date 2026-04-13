#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "SubMenu_1.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

/**
 * @brief Game 1 Implementation - Student can modify
 * 
 * EXAMPLE: Shows how to use PWM LED for visual feedback
 * This is a placeholder with a bouncing animation that changes LED brightness.
 * Replace this with your actual game logic!
 */

// Game state - customize for your game
static uint32_t animation_counter = 0;
static int16_t moving_x = 0;
static int8_t move_direction = 1;

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

static const char* mode_options[] = {
    "MODE 1", // 模式内部显示
    "MODE 2",
    "MODE 3"
};

MenuState Game1_Run(void) {
    SubMenuSystem submenu;
    SubMenuState selected_mode;

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

        // Initialize game state
        animation_counter = 0;
        moving_x = 0;
        move_direction = 1;

        // Only use to test different modes
        int16_t move_step = 2;
        if (selected_mode == SUBMENU_1_STATE_2) {
            move_step = 4;
        } else if (selected_mode == SUBMENU_1_STATE_3) {
            move_step = 6;
        }

        // Play a brief startup sound
        buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
        HAL_Delay(50);  // Brief beep duration
        buzzer_off(&buzzer_cfg);  // Stop the buzzer

        // Game loop
        while (1) {
            uint32_t frame_start = HAL_GetTick();

            // Read input
            Input_Read();

            if (current_input.btn3_pressed) {
                PWM_SetDuty(&pwm_cfg, 50);  // Reset LED when leaving mode
                while (current_input.btn3_pressed) {
                    HAL_Delay(10);
                    Input_Read();
                }
                break;
            }

            // UPDATE: Game logic
            animation_counter++;

            // Simple animation: move object back and forth
            moving_x += move_direction * move_step;
            if (moving_x >= 200 || moving_x <= 0) {
                move_direction *= -1;
            }

            // Example: Vary LED brightness based on animation
            uint8_t brightness = (moving_x * 100) / 200;
            PWM_SetDuty(&pwm_cfg, brightness);

            // RENDER: Draw to LCD
            LCD_Fill_Buffer(0);

            // Title
            LCD_printString("GAME 1", 60, 10, 1, 3);
            LCD_printString((char*)mode_options[selected_mode - SUBMENU_1_STATE_1], 60, 45, 1, 2);

            // Simple animated object (moving box)
            LCD_printString("[*]", 20 + moving_x, 100, 1, 3);

            // Display counter
            char counter[32];
            sprintf(counter, "Frame: %lu", animation_counter);
            LCD_printString(counter, 50, 150, 1, 2);

            // Show PWM LED usage
            LCD_printString("LED: PWM Demo", 30, 180, 1, 1);
            char pwm_str[32];
            sprintf(pwm_str, "Brightness: %d%%", brightness);
            LCD_printString(pwm_str, 30, 195, 1, 1);

            // Instructions
            LCD_printString("Press BT3 to", 40, 210, 1, 1);
            LCD_printString("Back to Mode Menu", 20, 225, 1, 1);

            LCD_Refresh(&cfg0);

            // Frame timing - wait for remainder of frame time
            uint32_t frame_time = HAL_GetTick() - frame_start;
            if (frame_time < GAME1_FRAME_TIME_MS) {
                HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
            }
        }
    }
}
