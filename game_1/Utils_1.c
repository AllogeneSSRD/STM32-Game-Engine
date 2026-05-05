
#include "utils.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_rng.h"


int32_t Max(int32_t a, int32_t b)
{
    return (a > b) ? a : b;
}

int32_t Clamp(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
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

uint8_t Circle_Rect_Overlap(uint16_t circle_x, uint16_t circle_y, uint16_t circle_r,
                            uint16_t rect_center_x, uint16_t rect_center_y,
                            uint16_t rect_w, uint16_t rect_h)
{
    int32_t half_w      = rect_w / 2;
    int32_t half_h      = rect_h / 2;
    int32_t rect_left   = (int32_t)rect_center_x - half_w;
    int32_t rect_top    = (int32_t)rect_center_y - half_h;
    int32_t rect_right  = (int32_t)rect_center_x + half_w;
    int32_t rect_bottom = (int32_t)rect_center_y + half_h;

    int32_t closest_x = Clamp((int32_t)circle_x, rect_left, rect_right);
    int32_t closest_y = Clamp((int32_t)circle_y, rect_top, rect_bottom);

    int32_t dx = (int32_t)circle_x - closest_x;
    int32_t dy = (int32_t)circle_y - closest_y;
    return ((dx * dx) + (dy * dy) <= (int32_t)(circle_r * circle_r)) ? 1 : 0;
}
