#include "Menu.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings

#define NUM_DIFFICULTY_OPTIONS 3

// Frame rate for menu (in milliseconds)
#define MENU_FRAME_TIME_MS 30  // ~33 FPS
    

// Difficulty options
static const char* difficulty_options[] = {
        "Easy",
        "Normal",
        "Hard"
};

//Render difficulty menu
static void render_difficulty_menu(DifficultySystem* diffi) 
{
        LCD_Fill_Buffer(0);

        // Title
        LCD_printString("SELECT DIFFICULTY", 20, 10, 1, 3);

        // List options with highlight
        for (int i = 0; i < NUM_DIFFICULTY_OPTIONS; i++) {
            uint16_t y_pos = 110 + (i * 40);
            uint8_t text_size = 2;

            if (i == diffi->selected_option) {
                // Draw arrow to indicate selection
                LCD_printString(">", 40, y_pos, 1, text_size);
            }

            LCD_printString((char*)difficulty_options[i], 70, y_pos, 1, text_size);
        }

        // Instructions
        LCD_printString("Press BT3", 50, 240, 1, 1);

        LCD_Refresh(&cfg0);
}

//Initialize difficulty menu
void Difficulty_Init(DifficultySystem* diffi) 
{
    diffi->selected_option = 0;
}

//Run difficulty menu
DifficultyState Difficulty_Run(DifficultySystem* diffi)
{
    static Direction last_direction = CENTRE;  // Track last direction for debouncing
    DifficultyState selected_diffi = DIFFICULTY_EASY;  // Which difficulty was selected

    // Menu's own loop - runs until game is selected
    while (1) 
    {
        uint32_t frame_start = HAL_GetTick();

        // Read input
        Input_Read();

        // Read current joystick position
        Joystick_Read(&joystick_cfg, &joystick_data);

        // Handle joystick navigation (up/down to select option)
        Direction current_direction = joystick_data.direction;

        if (current_direction == S && last_direction != S) {  // Joystick pushed DOWN
            // Move selection down
            diffi->selected_option++;
            if (diffi->selected_option >= NUM_DIFFICULTY_OPTIONS) {
                diffi->selected_option = 0;  // Wrap around
            }
        } 
        else if (current_direction == N && last_direction != N) {  // Joystick pushed UP
            // Move selection up
            if (diffi->selected_option == 0) {
                diffi->selected_option = NUM_DIFFICULTY_OPTIONS - 1;  // Wrap around
            } else {
                diffi->selected_option--;
            }
        }

        last_direction = current_direction;

        // Handle button press to select current option
        if (current_input.btn3_pressed) {
            // User pressed button - select the highlighted option
            if (diffi->selected_option == 0) {
                selected_diffi = DIFFICULTY_EASY;
            } else if (diffi->selected_option == 1) {
                selected_diffi = DIFFICULTY_NORMAL;
            } else if (diffi->selected_option == 2) {
                selected_diffi = DIFFICULTY_HARD;
            }   
            break;  // Exit difficulty loop - game selected!
        }

        // Render difficulty
        render_difficulty_menu(diffi);

        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < MENU_FRAME_TIME_MS) {
            HAL_Delay(MENU_FRAME_TIME_MS - frame_time);
        }
    }

    return selected_diffi;  // Return which difficulty was selected
}