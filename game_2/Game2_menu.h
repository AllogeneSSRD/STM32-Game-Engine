#ifndef GAME2_MENU_H
#define GAME2_MENU_H

#include <stdint.h>

// ==============================================
// MENU STATE MACHINE
// ==============================================

typedef enum {
    MENU_GAME2_MAP1,          // Running Game 1
    MENU_GAME2_MAP2,          // Running MAP 2
    MENU_GAME2_MAP3, 
    BACK         // Running MAP 3
} MapState;

// Map system structure
typedef struct {
    uint8_t selected_option;    // Which menu option is highlighted (0-2)
} MapSystem;

// ==============================================
// INITIALIZATION AND STATE MANAGEMENT
// ==============================================

/**
 * @brief Initialize the menu system
 */
void Map_Init(MapSystem* map);

/**
 * @brief Run the menu - displays menu and waits for selection
 * 
 * Runs its own loop and returns the selected game state.
 * 
 * @return MapState - The game that was selected (GAME_1, GAME_2, or GAME_3)
 */
MapState Map_Run(MapSystem* map);

#endif // GAME2_MENU_H
