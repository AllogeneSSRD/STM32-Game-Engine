#ifndef GAME_3_MENU_H
#define GAME_3_MENU_H

#include "Menu.h"

typedef enum {
    DIFFICULTY_EASY = 0, //Easy mode
    DIFFICULTY_NORMAL,  //Normal mode
    DIFFICULTY_HARD,    //Hard mode
} DifficultyState;

// Difficulty system structure
typedef struct {

    uint8_t selected_option;    // Which menu option is highlighted (0-2)

} DifficultySystem;

// ==============================================
// INITIALIZATION AND STATE MANAGEMENT
// ==============================================

/**
 * @brief Initialize the difficulty system
 */
void Difficulty_Init(DifficultySystem* diffi); 

/**
 * @brief Run the difficulty - displays menu and waits for selection
 * 
 * Runs its own loop and returns the selected game state.
 * 
 * @return DifficultyState - The difficulty that was selected (easy, normal, hard)
 */
DifficultyState Difficulty_Run(DifficultySystem* diffi);

#endif // GAME_3_MENU_H