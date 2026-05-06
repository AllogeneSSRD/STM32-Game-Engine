
/* utils_1.h */
#ifndef UTILS_1_H
#define UTILS_1_H

#include <stdint.h>   // uint16_t

int32_t Max(int32_t a, int32_t b);
int32_t Clamp(int32_t value, int32_t min_value, int32_t max_value);


uint8_t Circles_Overlap(uint16_t x1, uint16_t y1, uint16_t r1,
                        uint16_t x2, uint16_t y2, uint16_t r2);

uint8_t Circle_Rect_Overlap(uint16_t circle_x, uint16_t circle_y, uint16_t circle_r,
                            uint16_t rect_center_x, uint16_t rect_center_y,
                            uint16_t rect_w, uint16_t rect_h);
#endif
