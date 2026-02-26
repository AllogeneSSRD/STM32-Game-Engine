# ELEC2645 - Joystick Circle Mapping with LCD Display

This lab demonstrates advanced embedded systems programming on the STM32L476 Nucleo board, focusing on:
- **Analog Input Processing** - Reading and processing joystick input from ADC channels
- **Circle Mapping Algorithm** - Transforming square joystick input to circular output for uniform control feel
- **Coordinate Transformations** - Converting between Cartesian and polar coordinate systems
- **LCD Screen Control** - Visualizing joystick data in real-time on the ST7789V2 SPI display
- **Compass-Style Output** - Calculating and displaying directional headings (N, NE, E, SE, S, SW, W, NW)

The most important file is [Core/Src/main.c](Core/Src/main.c) which contains the main application logic and the student activity.

## The Project

The program reads a 2-axis analog joystick, applies circle mapping transformation, and displays the results on an LCD screen. The display shows:
1. **Raw ADC values** - The raw 12-bit ADC readings from the joystick X and Y axes
2. **Angle and Magnitude** - The compass heading (0-360°) and magnitude (0.0-1.0) from circle-mapped coordinates
3. **Direction** - The discrete 8-direction compass output (N, NE, E, SE, S, SW, W, NW, or CENTRE)
4. **Visual Representation** - A colored dot that moves within a circular frame, with color changing based on direction

The circle mapping algorithm ensures that pushing the joystick feels equally responsive in all directions, unlike traditional square-to-square mapping (used in previous activity) which makes diagonal movements feel less responsive. 

## The Assignment

Your task is to enhance the program by adding **edge detection with display mode toggling**:

**Your Task:**
Add code to detect when the joystick reaches the edge (high magnitude) and toggle the LCD display mode:
- When joystick magnitude is **≥ 0.9**: Call `LCD_inverseMode` to invert the display colors
- When joystick magnitude is **< 0.9**: Call `LCD_normalMode` to return to normal display


## Setup Instructions

### Prerequisites

1. **Completed previous labs**
   - Blinky and LCD Test to ensure the hardware is working first!
   - Understanding of ADC and basic GPIO operations

2. **Configure the Project**
   - Open the `Unit_3_2_Joystick_Circle` folder in VS Code
   - When prompted "*Would you like to configure discovered CMake project as STM32Cube project*", click **Yes**
   - Allow the STM32 extension to complete initialization
   - Select **Debug** configuration when prompted

3. **Verify Hardware Connection**
   - Connect the Nucleo board via USB
   - Check that the board appears under "STM32CUBE Devices and Boards" in the Run and Debug sidebar
   - Test with the **Blink** function to verify communication

4. **Build and Run**
   - Click **Build** in the bottom status bar to verify compilation
   - Open Run and Debug panel (`Ctrl+Shift+D`)
   - Select **"STM32Cube: STLink GDB Server"** and click Run
   - Continue past breakpoints with **F5** or the play button

### Troubleshooting

- **Board not detected**: Ensure ST Link drivers are installed and USB cable is connected
- **Build errors**: Check that the joystick library header files are included correctly
- **Joystick not responding**: Verify ADC channels are configured for X (ADC_CHANNEL_1) and Y (ADC_CHANNEL_2)
- **Joystick not centred correctly**: Ensure you connect the 5V Pin on the Joystick to *3.3V* on the Nucleo *not* 5V
- **LCD display issues**: Ensure the SPI2 and GPIO pins are properly initialized



## Hardware Configuration

### Analog Joystick Connection

| Joystick Pin | Signal | Nucleo Pin | Purpose |
|-------------|--------|-----------|---------|
| VCC         | Power (3.3V) | VDD | Joystick power supply |
| GND         | Ground | GND | Ground reference |
| X-axis      | Analog X | PA1 (ADC1 CH1) | Horizontal position |
| Y-axis      | Analog Y | PA2 (ADC1 CH2) | Vertical position |
| Button      | (Optional) | - | Can be added for digital input |

### LCD Display Connection (ST7789V2)

| LCD Pin | Signal | Nucleo Pin | Purpose |
|---------|--------|-----------|---------|
| VDD     | Power (3.3V-5V) | VDD | Display power supply |
| GND     | Ground | GND | Ground reference |
| MOSI    | Serial Data | PB15 | SPI Master Output Slave Input |
| SCK     | Clock | PB13 | SPI Clock signal |
| CS      | Chip Select | PB12 | SPI Chip Select |
| DC      | Data/Command | PB11 | Command vs Data mode |
| BL      | Backlight | PB1 | Backlight control (active high) |
| RST     | Reset | PB2 | Display reset (active low) |

### Serial Communication
- **UART Interface**: USB connection via ST Link debugger
- **Baud Rate**: 115200
- **Purpose**: Debug output via `printf()` and viewing calibration information


## Software Architecture

### Key Source Files

| File | Purpose |
|------|---------|
| [Core/Src/main.c](Core/Src/main.c) | **Main application** - Joystick initialization, calibration, display loop |
| [Joystick/Joystick.h](Joystick/Joystick.h) | Joystick driver header - API documentation and type definitions |
| [Joystick/Joystick.c](Joystick/Joystick.c) | Joystick driver implementation - ADC reading, circle mapping, coordinate transformations |
| [Core/Src/adc.c](Core/Src/adc.c) | ADC peripheral initialization |
| [Core/Src/gpio.c](Core/Src/gpio.c) | GPIO peripheral initialization |

### Joystick Library

The joystick driver provides a complete abstraction for reading analog joystick input with circle mapping:

**Key Data Structures:**

- **Joystick_t** - Contains all joystick state:
  - `x_raw`, `y_raw` - Raw 12-bit ADC values
  - `coord` - Cartesian coordinates (-1.0 to 1.0) before circle mapping
  - `coord_mapped` - Cartesian coordinates after circle mapping (uniform control feel)
  - `angle` - Compass heading 0-360° (0°=North, 90°=East, etc.)
  - `magnitude` - Distance from center 0.0-1.0
  - `direction` - 8-direction enum (N, NE, E, SE, S, SW, W, NW, CENTRE)

**Key Functions:**

- `Joystick_Init()` - Initialize ADC channels and configuration
- `Joystick_Calibrate()` - Find center position (call while joystick is centered)
- `Joystick_Read()` - Read current joystick state (call in main loop)

**Circle Mapping Algorithm:**

The circle mapping transformation converts square input range to circular output:
- Formula: `x' = x * sqrt(1 - y²/2)`, `y' = y * sqrt(1 - x²/2)`
- Effect: Pushing the joystick corner feels same force as cardinal directions
- Result: Maximizes effective range and provides uniform tactile feedback

See [Joystick/Joystick.h](Joystick/Joystick.h) for complete API documentation.

### LCD Driver

The project includes the ST7789V2 LCD driver for the 240×320 pixel display:
- **Location**: [ST7789V2_Driver_STM32L4/](ST7789V2_Driver_STM32L4/)
- **Key Functions**:
  - `LCD_Draw_Circle(x, y, radius, color, fill)` - Draw a circle
  - `LCD_printString(x, y, text, size, color)` - Display text
  - `LCD_Fill_Buffer(color)` - Clear the display
  - `LCD_Refresh()` - Update the physical display
  - `LCD_inverseMode()` - Invert display colors (for your assignment!)
  - `LCD_normalMode()` - Return to normal display mode


## Understanding the Code

### Initialization (Before Main Loop)

```c
// Create joystick configuration
Joystick_cfg_t joystick_cfg = { ... };

// Initialize ADC and find center position
Joystick_Init(&joystick_cfg);
Joystick_Calibrate(&joystick_cfg);
```

### Main Loop

```c
while (1) {
    // Read all joystick data (raw ADC, processed, cartesian, polar, direction)
    Joystick_Read(&joystick_cfg, &joystick_data);
    
    // Use circle-mapped coordinates for uniform control
    float mapped_x = joystick_data.coord_mapped.x;
    float mapped_y = joystick_data.coord_mapped.y;
    
    // Your assignment: Use joystick_data.magnitude to toggle display mode
    // if (joystick_data.magnitude >= 0.9) { ... }
    
    // Display results
    LCD_Refresh(&cfg0);
}
```

### Available Data

When you need to check if the joystick is at the edge:
- `joystick_data.magnitude` - Value 0.0-1.0 indicating how far from center
  - 0.0 = joystick at center
  - 1.0 = joystick fully deflected
  - 0.9 = 90% of full range (good threshold for "at edge")

## Tips for Success

1. **Test Your Understanding**: Print `joystick_data.magnitude` to serial monitor to see the values before implementing the toggle
2. **Start Simple**: Get the if statement working with one display mode first
3. **Debug**: Use breakpoints or printf statements to verify your magnitude checks
4. **Edge Cases**: Think about what happens when magnitude is exactly 0.9 - does behavior feel right?
5. **Performance**: The display mode toggle happens every loop iteration - that's okay!

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| "Direction shows CENTRE when joystick is pushed" | Magnitude check was missing | Fixed - code now checks both angle AND magnitude |
| "Dot position is jerky or inverted" | Y-axis polarity issue | Circle frame Y-axis is inverted (py = cy - y_mapped) |
| "Colours don't look right" | Wrong color value in LCD_Draw_Circle | Each direction 0-8 maps to a palette color, verify palette |
| "Compilation errors about joystick.h" | Include path issue | Ensure joystick/ folder is in include path |
| "Display doesn't invert at the edge" | Assignment not implemented | Add your if/else code in the marked section of main.c |

