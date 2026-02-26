# ELEC2645 - Joystick Circle Mapping with LCD Display
# ELEC2645 - 带液晶显示屏的操纵杆圆圈映射


This lab demonstrates advanced embedded systems programming on the STM32L476 Nucleo board, focusing on:
本实验演示了基于 STM32L476 Nucleo 开发板的高级嵌入式系统编程，重点在于：
- **Analog Input Processing** - Reading and processing joystick input from ADC channels
- **模拟输入处理** - 从 ADC 通道读取和处理操纵杆输入
- **Circle Mapping Algorithm** - Transforming square joystick input to circular output for uniform control feel
- **圆形映射算法** - 将方形摇杆输入转换为圆形输出，以实现统一的控制手感
- **Coordinate Transformations** - Converting between Cartesian and polar coordinate systems
- **坐标变换** - 笛卡尔坐标系和极坐标系之间的转换
- **LCD Screen Control** - Visualizing joystick data in real-time on the ST7789V2 SPI display
- **LCD 屏幕控制** - 在 ST7789V2 SPI 显示屏上实时显示操纵杆数据
- **Compass-Style Output** - Calculating and displaying directional headings (N, NE, E, SE, S, SW, W, NW)
- **罗盘式输出** - 计算并显示方向（北、东北、东、东南、南、西南、西、西北）


The most important file is [Core/Src/main.c](Core/Src/main.c) which contains the main application logic and the student activity.
最重要的文件是 [Core/Src/main.c](Core/Src/main.c)，其中包含主要应用程序逻辑和学生活动。


## The Project
## 项目


The program reads a 2-axis analog joystick, applies circle mapping transformation, and displays the results on an LCD screen. The display shows:
该程序读取双轴模拟操纵杆的输入，应用圆映射变换，并将结果显示在液晶显示屏上。显示屏显示：
1. **Raw ADC values** - The raw 12-bit ADC readings from the joystick X and Y axes
1. **原始 ADC 值** - 来自操纵杆 X 轴和 Y 轴的原始 12 位 ADC 读数
2. **Angle and Magnitude** - The compass heading (0-360°) and magnitude (0.0-1.0) from circle-mapped coordinates
2. **角度和大小** - 由圆坐标系得到的罗盘方位角（0-360°）和大小（0.0-1.0）
3. **Direction** - The discrete 8-direction compass output (N, NE, E, SE, S, SW, W, NW, or CENTRE)
3. **方向** - 离散的 8 方向罗盘输出（北、东北、东、东南、南、西南、西、西北或中心）
4. **Visual Representation** - A colored dot that moves within a circular frame, with color changing based on direction
4. **视觉表现** - 一个彩色圆点在一个圆形框架内移动，颜色根据方向而变化


The circle mapping algorithm ensures that pushing the joystick feels equally responsive in all directions, unlike traditional square-to-square mapping (used in previous activity) which makes diagonal movements feel less responsive.
圆形映射算法确保推动操纵杆在各个方向上的响应速度都相同，这与传统的方形映射（在之前的活动中使用）不同，后者使得对角线移动的响应速度较慢。


## The Assignment
## 任务


Your task is to enhance the program by adding **edge detection with display mode toggling**:
你的任务是通过添加**带有显示模式切换功能的边缘检测**来增强程序：


**Your Task:**
你的任务：
Add code to detect when the joystick reaches the edge (high magnitude) and toggle the LCD display mode:
添加代码以检测操纵杆何时到达边缘（高幅度），并切换 LCD 显示模式：
- When joystick magnitude is **≥ 0.9**: Call `LCD_inverseMode` to invert the display colors
- 当摇杆幅度 **≥ 0.9** 时：调用 `LCD_inverseMode` 反转显示颜色
- When joystick magnitude is **< 0.9**: Call `LCD_normalMode` to return to normal display
- 当摇杆幅度小于 0.9 时：调用 `LCD_normalMode` 返回正常显示模式


## Setup Instructions
## 安装说明


### Prerequisites
### 先决条件


1. **Completed previous labs**
1. **已完成之前的实验**
- Blinky and LCD Test to ensure the hardware is working first!
- 首先进行闪烁和液晶屏测试，以确保硬件正常工作！
- Understanding of ADC and basic GPIO operations
- 了解 ADC 和基本 GPIO 操作


2. **Configure the Project**
2. 配置项目
- Open the `Unit_3_2_Joystick_Circle` folder in VS Code
- 在 VS Code 中打开 `Unit_3_2_Joystick_Circle` 文件夹
- When prompted "*Would you like to configure discovered CMake project as STM32Cube project*", click **Yes**
当提示“*是否要将发现的 CMake 项目配置为 STM32Cube 项目*”时，单击**是**
- Allow the STM32 extension to complete initialization
- 允许 STM32 扩展完成初始化
- Select **Debug** configuration when prompted
- 出现提示时，选择**调试**配置


3. **Verify Hardware Connection**
3. **验证硬件连接**
- Connect the Nucleo board via USB
- 通过 USB 连接 Nucleo 开发板
- Check that the board appears under "STM32CUBE Devices and Boards" in the Run and Debug sidebar
- 检查开发板是否出现在“运行和调试”侧边栏的“STM32CUBE 设备和开发板”下。
- Test with the **Blink** function to verify communication
- 使用**闪烁**功能进行测试以验证通信情况


4. **Build and Run**
4. **构建和运行**
- Click **Build** in the bottom status bar to verify compilation
- 点击底部状态栏中的“构建”按钮以验证编译情况
- Open Run and Debug panel (`Ctrl+Shift+D`)
- 打开“运行和调试”面板（`Ctrl+Shift+D`）
- Select **"STM32Cube: STLink GDB Server"** and click Run
- 选择“STM32Cube: STLink GDB 服务器”，然后单击“运行”。
- Continue past breakpoints with **F5** or the play button
- 使用 **F5** 或播放按钮继续跳过断点


### Troubleshooting
### 故障排除


- **Board not detected**: Ensure ST Link drivers are installed and USB cable is connected
- **未检测到开发板**：请确保已安装 ST Link 驱动程序并已连接 USB 线缆。
- **Build errors**: Check that the joystick library header files are included correctly
- **构建错误**：请检查是否正确包含了操纵杆库头文件。
- **Joystick not responding**: Verify ADC channels are configured for X (ADC_CHANNEL_1) and Y (ADC_CHANNEL_2)
- **操纵杆无响应**：请检查 ADC 通道是否已配置为 X (ADC_CHANNEL_1) 和 Y (ADC_CHANNEL_2)
- **Joystick not centred correctly**: Ensure you connect the 5V Pin on the Joystick to *3.3V* on the Nucleo *not* 5V
- **摇杆未正确居中**：请确保将摇杆上的 5V 引脚连接到 Nucleo 上的 *3.3V* 引脚，而不是 *5V* 引脚。
- **LCD display issues**: Ensure the SPI2 and GPIO pins are properly initialized
- **LCD 显示问题**：请确保 SPI2 和 GPIO 引脚已正确初始化。



## Hardware Configuration
## 硬件配置


### Analog Joystick Connection
### 模拟摇杆连接


| Joystick Pin | Signal | Nucleo Pin | Purpose |
| 操纵杆引脚 | 信号 | Nucleo 引脚 | 用途 |
|-------------|--------|-----------|---------|
| VCC | Power (3.3V) | VDD | Joystick power supply |
| VCC | 电源 (3.3V) | VDD | 游戏手柄电源 |
| GND | Ground | GND | Ground reference |
| GND | 接地 | GND | 接地参考点 |
| X-axis | Analog X | PA1 (ADC1 CH1) | Horizontal position |
| X 轴 | 模拟 X | PA1 (ADC1 CH1) | 水平位置 |
| Y-axis | Analog Y | PA2 (ADC1 CH2) | Vertical position |
| Y 轴 | 模拟 Y 轴 | PA2（ADC1 通道 2） | 垂直位置 |
| Button | (Optional) | - | Can be added for digital input |
| 按钮 | （可选） | - | 可用于添加数字输入 |


### LCD Display Connection (ST7789V2)
### LCD 显示屏连接 (ST7789V2)


| LCD Pin | Signal | Nucleo Pin | Purpose |
| LCD 引脚 | 信号 | Nucleo 引脚 | 用途 |
|---------|--------|-----------|---------|
| VDD | Power (3.3V-5V) | VDD | Display power supply |
| VDD | 电源 (3.3V-5V) | VDD | 显示屏电源 |
| GND | Ground | GND | Ground reference |
| GND | 接地 | GND | 接地参考点 |
| MOSI | Serial Data | PB15 | SPI Master Output Slave Input |
| MOSI | 串行数据 | PB15 | SPI 主输出从输入 |
| SCK | Clock | PB13 | SPI Clock signal |
| SCK | 时钟 | PB13 | SPI 时钟信号 |
| CS | Chip Select | PB12 | SPI Chip Select |
| CS | 片选 | PB12 | SPI 片选 |
| DC | Data/Command | PB11 | Command vs Data mode |
| DC | 数据/命令 | PB11 | 命令模式与数据模式 |
| BL | Backlight | PB1 | Backlight control (active high) |
| BL | 背光 | PB1 | 背光控制（高电平有效） |
| RST | Reset | PB2 | Display reset (active low) |
| RST | 复位 | PB2 | 显示复位（低电平有效） |


### Serial Communication
### 串行通信
- **UART Interface**: USB connection via ST Link debugger
- **UART 接口**：通过 ST Link 调试器进行 USB 连接
- **Baud Rate**: 115200
- **波特率**：115200
- **Purpose**: Debug output via `printf()` and viewing calibration information
- **用途**：通过 `printf()` 函数输出调试信息并查看校准信息


## Software Architecture
## 软件架构


### Key Source Files
### 主要源文件


| File | Purpose |
文件 | 用途 |
|------|---------|
| [Core/Src/main.c](Core/Src/main.c) | **Main application** - Joystick initialization, calibration, display loop |
| [Core/Src/main.c](Core/Src/main.c) | **主应用程序** - 操纵杆初始化、校准、显示循环 |
| [Joystick/Joystick.h](Joystick/Joystick.h) | Joystick driver header - API documentation and type definitions |
| [Joystick/Joystick.h](Joystick/Joystick.h) | 游戏杆驱动程序头文件 - API 文档和类型定义 |
| [Joystick/Joystick.c](Joystick/Joystick.c) | Joystick driver implementation - ADC reading, circle mapping, coordinate transformations |
| [Joystick/Joystick.c](Joystick/Joystick.c) | 游戏杆驱动程序实现 - ADC 读取、圆映射、坐标变换 |
| [Core/Src/adc.c](Core/Src/adc.c) | ADC peripheral initialization |
| [Core/Src/adc.c](Core/Src/adc.c) | ADC 外设初始化 |
| [Core/Src/gpio.c](Core/Src/gpio.c) | GPIO peripheral initialization |
| [Core/Src/gpio.c](Core/Src/gpio.c) | GPIO 外设初始化 |


### Joystick Library
### 操纵杆库


The joystick driver provides a complete abstraction for reading analog joystick input with circle mapping:
操纵杆驱动程序提供了一个完整的抽象层，用于读取带有圆形映射的模拟操纵杆输入：


**Key Data Structures:**
**关键数据结构：**


- **Joystick_t** - Contains all joystick state:
- **Joystick_t** - 包含所有游戏手柄状态：
- `x_raw`, `y_raw` - Raw 12-bit ADC values
- `x_raw`、`y_raw` - 原始 12 位 ADC 值
- `coord` - Cartesian coordinates (-1.0 to 1.0) before circle mapping
- `coord` - 圆映射前的笛卡尔坐标（-1.0 到 1.0）
- `coord_mapped` - Cartesian coordinates after circle mapping (uniform control feel)
- `coord_mapped` - 圆映射后的笛卡尔坐标（统一的控制手感）
- `angle` - Compass heading 0-360° (0°=North, 90°=East, etc.)
- `angle` - 罗盘方位 0-360°（0°=北，90°=东，等等）
- `magnitude` - Distance from center 0.0-1.0
- `magnitude` - 距中心的距离 0.0-1.0
- `direction` - 8-direction enum (N, NE, E, SE, S, SW, W, NW, CENTRE)
- `direction` - 8 方向枚举 (N, NE, E, SE, S, SW, W, NW, CENTRE)


**Key Functions:**
**主要功能：**


- `Joystick_Init()` - Initialize ADC channels and configuration
- `Joystick_Init()` - 初始化 ADC 通道和配置
- `Joystick_Calibrate()` - Find center position (call while joystick is centered)
- `Joystick_Calibrate()` - 查找中心位置（在摇杆居中时调用）
- `Joystick_Read()` - Read current joystick state (call in main loop)
- `Joystick_Read()` - 读取当前游戏杆状态（在主循环中调用）


**Circle Mapping Algorithm:**
**圆映射算法：**


The circle mapping transformation converts square input range to circular output:
圆形映射变换将方形输入范围转换为圆形输出：
- Formula: `x' = x * sqrt(1 - y²/2)`, `y' = y * sqrt(1 - x²/2)`
- 公式：`x' = x * sqrt(1 - y²/2)`，`y' = y * sqrt(1 - x²/2)`
- Effect: Pushing the joystick corner feels same force as cardinal directions
- 效果：推动摇杆角落与推动四个方向的感觉相同
- Result: Maximizes effective range and provides uniform tactile feedback
结果：最大程度地扩大有效范围并提供均匀的触觉反馈


See [Joystick/Joystick.h](Joystick/Joystick.h) for complete API documentation.
有关完整的 API 文档，请参阅 [Joystick/Joystick.h](Joystick/Joystick.h)。


### LCD Driver
### LCD 驱动器


The project includes the ST7789V2 LCD driver for the 240×320 pixel display:
该项目包含用于 240×320 像素显示屏的 ST7789V2 LCD 驱动器：
- **Location**: [ST7789V2_Driver_STM32L4/](ST7789V2_Driver_STM32L4/)
- **位置**: [ST7789V2_Driver_STM32L4/](ST7789V2_Driver_STM32L4/)
- **Key Functions**:
- **主要功能**：
- `LCD_Draw_Circle(x, y, radius, color, fill)` - Draw a circle
- `LCD_Draw_Circle(x, y, radius, color, fill)` - 绘制一个圆
- `LCD_printString(x, y, text, size, color)` - Display text
- `LCD_printString(x, y, text, size, color)` - 显示文本
- `LCD_Fill_Buffer(color)` - Clear the display
- `LCD_Fill_Buffer(color)` - 清除显示
- `LCD_Refresh()` - Update the physical display
- `LCD_Refresh()` - 更新物理显示屏
- `LCD_inverseMode()` - Invert display colors (for your assignment!)
- `LCD_inverseMode()` - 反转显示颜色（供您作业使用！）
- `LCD_normalMode()` - Return to normal display mode
- `LCD_normalMode()` - 返回正常显示模式


## Understanding the Code
## 理解代码


### Initialization (Before Main Loop)
### 初始化（主循环之前）


```c
// Create joystick configuration
// 创建游戏杆配置
Joystick_cfg_t joystick_cfg = { ... };


// Initialize ADC and find center position
// 初始化 ADC 并找到中心位置
Joystick_Init(&joystick_cfg);
Joystick_Calibrate(&joystick_cfg);
```


### Main Loop
### 主循环


```c
while (1) {
// Read all joystick data (raw ADC, processed, cartesian, polar, direction)
// 读取所有游戏手柄数据（原始 ADC 数据、处理后的数据、笛卡尔坐标数据、极坐标数据、方向数据）
Joystick_Read(&joystick_cfg, &joystick_data);
// Use circle-mapped coordinates for uniform control
// 使用圆形映射坐标进行统一控制
float mapped_x = joystick_data.coord_mapped.x;
float mapped_y = joystick_data.coord_mapped.y;
// Your assignment: Use joystick_data.magnitude to toggle display mode
// 你的任务：使用 joystick_data.magnitude 来切换显示模式
// if (joystick_data.magnitude >= 0.9) { ... }
// 如果 (joystick_data.magnitude >= 0.9) { ... }
// Display results
// 显示结果
LCD_Refresh(&cfg0);
}
```


### Available Data
### 可用数据


When you need to check if the joystick is at the edge:
当您需要检查操纵杆是否位于边缘时：
- `joystick_data.magnitude` - Value 0.0-1.0 indicating how far from center
- `joystick_data.magnitude` - 值介于 0.0 和 1.0 之间，表示与中心点的距离。
- 0.0 = joystick at center
- 0.0 = 操纵杆位于中心位置
- 1.0 = joystick fully deflected
- 1.0 = 操纵杆完全偏转
- 0.9 = 90% of full range (good threshold for "at edge")
- 0.9 = 90% 的完整范围（边缘状态的良好阈值）


## Tips for Success
成功秘诀


1. **Test Your Understanding**: Print `joystick_data.magnitude` to serial monitor to see the values before implementing the toggle
1. **测试你的理解**：在实现切换功能之前，将 `joystick_data.magnitude` 打印到串口监视器以查看其值。
2. **Start Simple**: Get the if statement working with one display mode first
2. **从简单的开始**：先让 if 语句在一种显示模式下正常工作。
3. **Debug**: Use breakpoints or printf statements to verify your magnitude checks
3. **调试**：使用断点或 printf 语句来验证你的数值大小检查。
4. **Edge Cases**: Think about what happens when magnitude is exactly 0.9 - does behavior feel right?
4. **极端情况**：想想当幅度恰好为 0.9 时会发生什么——行为感觉是否正确？
5. **Performance**: The display mode toggle happens every loop iteration - that's okay!
5. **性能**：显示模式切换发生在每次循环迭代中 - 这没关系！


## Troubleshooting
## 故障排除


| Issue | Cause | Solution |
问题 | 原因 | 解决方案 |
|-------|-------|----------|
| "Direction shows CENTRE when joystick is pushed" | Magnitude check was missing | Fixed - code now checks both angle AND magnitude |
| “按下操纵杆时，方向显示为中心” | 缺少幅度检查 | 已修复 - 代码现在同时检查角度和幅度 |
| "Dot position is jerky or inverted" | Y-axis polarity issue | Circle frame Y-axis is inverted (py = cy - y_mapped) |
| “点的位置抖动或反转” | Y 轴极性问题 | 圆框 Y 轴反转（py = cy - y_mapped） |
| "Colours don't look right" | Wrong color value in LCD_Draw_Circle | Each direction 0-8 maps to a palette color, verify palette |
| “颜色看起来不对劲” | LCD_Draw_Circle 中的颜色值错误 | 每个方向 0-8 都对应一个调色板颜色，请检查调色板 |
| "Compilation errors about joystick.h" | Include path issue | Ensure joystick/ folder is in include path |
| “joystick.h 编译错误” | 包含路径问题 | 请确保 joystick/ 文件夹位于包含路径中 |
| "Display doesn't invert at the edge" | Assignment not implemented | Add your if/else code in the marked section of main.c |
| “显示边缘未反转” | 赋值未实现 | 请在 main.c 的标记部分添加您的 if/else 代码 |

