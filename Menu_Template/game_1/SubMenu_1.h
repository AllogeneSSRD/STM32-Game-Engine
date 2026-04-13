#ifndef SUBMENU_1_H
#define SUBMENU_1_H

#include <stdint.h>

// ==============================================
// MENU STATE MACHINE
// ==============================================

typedef enum {
    SUBMENU_1_STATE_HOME = 0,        // Main menu - select game
    SUBMENU_1_STATE_1,          // Game mode 1
    SUBMENU_1_STATE_2,          // Game mode 2
    SUBMENU_1_STATE_3,          // Game mode 3
} SubMenuState;

// Menu system structure
typedef struct {
    uint8_t selected_option;    // Which menu option is highlighted (0-3)
} SubMenuSystem;

// ==============================================
// INITIALIZATION AND STATE MANAGEMENT
// ==============================================

/**
 * @brief Initialize the menu system
 */
void SubMenu_Init(SubMenuSystem* menu);

/**
 * @brief Run the menu - displays menu and waits for selection
 * 
 * Runs its own loop and returns the selected game state.
 * 
 * @return SubMenuState - The game that was selected (GAME_1, GAME_2, or GAME_3)
 */
SubMenuState SubMenu_Run(SubMenuSystem* menu);

#endif // SUBMENU_1_H
