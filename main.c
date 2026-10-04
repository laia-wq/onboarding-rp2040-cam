#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "tusb.h"
#include "cam.h"
#include "ImageData.h"
#include "LCD_1in14_V2.h"
#include "GUI_Paint.h"
#include "frame_processing.h"
uint8_t image_buf[CAMERA_WIDTH * CAMERA_HEIGHT];
static uint16_t display_buf[DISPLAY_WIDTH * DISPLAY_HEIGHT];

enum { CORE_READY = 1234, FRAME_READY = 1235, FRAME_DISPLAYED = 1236 };

static void expect_message(uint32_t expected) {
    uint32_t received = multicore_fifo_pop_blocking();
    if (received != expected) {
        panic("Unexpected inter-core message: %lu", (unsigned long)received);
    }
}

static void core1_entry(void) {
    if (DEV_Module_Init() != 0) {
        panic("Display module initialization failed");
    }
    LCD_1IN14_V2_Init(HORIZONTAL);
    Paint_NewImage((UBYTE *)display_buf, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, BLACK);
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(BLACK);
    Paint_DrawImage(realityLabsLogo, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    LCD_1IN14_V2_Display(display_buf);
    sleep_ms(1000);

    struct cam_config config;
    cam_config_struct(&config);
    cam_init(&config);

    multicore_fifo_push_blocking(CORE_READY);
    expect_message(CORE_READY);
    printf("Core 1: camera ready, Core 0 acknowledged\n");

    while (true) {
        cam_capture_frame(&config);
        prepare_display_frame(image_buf, display_buf);
        multicore_fifo_push_blocking(FRAME_READY);
        expect_message(FRAME_DISPLAYED);
    }
}

int main(void) {
    vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(250000, true);
    stdio_init_all();
    for (unsigned i = 0; i < 20 && !tud_cdc_connected(); ++i) {
        sleep_ms(100);
    }
    printf("USB CDC connected: %d\n", tud_cdc_connected() ? 1 : 0);
    multicore_launch_core1(core1_entry);
    expect_message(CORE_READY);
    multicore_fifo_push_blocking(CORE_READY);
    printf("Core 0: Core 1 acknowledged; displaying camera frames\n");

    while (true) {
        expect_message(FRAME_READY);
        LCD_1IN14_V2_Display(display_buf);
        multicore_fifo_push_blocking(FRAME_DISPLAYED);
    }
}
