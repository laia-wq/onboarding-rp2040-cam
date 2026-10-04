#ifndef FRAME_PROCESSING_H
#define FRAME_PROCESSING_H
#include <stdint.h>
#include <stddef.h>

enum { CAMERA_WIDTH = 324, CAMERA_HEIGHT = 324,
       DISPLAY_WIDTH = 240, DISPLAY_HEIGHT = 135 };

static inline uint16_t grayscale_to_lcd(uint8_t gray) {
    uint16_t rgb = (uint16_t)(((gray & 0xf8u) << 8) |
                             ((gray & 0xfcu) << 3) |
                             ((gray & 0xf8u) >> 3));
    // LCD driver sends raw bytes: store the high RGB565 byte first.
    return (uint16_t)((rgb >> 8) | (rgb << 8));
}

static inline void prepare_display_frame(const uint8_t *camera, uint16_t *display) {
    // Match the tutorial's top-left crop and vertical flip, including row 0.
    for (size_t y = 0; y < DISPLAY_HEIGHT; ++y) {
        size_t source_row = DISPLAY_HEIGHT - 1 - y;
        for (size_t x = 0; x < DISPLAY_WIDTH; ++x) {
            display[y * DISPLAY_WIDTH + x] =
                grayscale_to_lcd(camera[source_row * CAMERA_WIDTH + x]);
        }
    }
}
#endif
