aux_source_directory(. DIR_CAM_SRCS)
include_directories(../Config)
add_library(CAM ${DIR_CAM_SRCS})
target_link_libraries(CAM PUBLIC pico_stdlib hardware_dma hardware_i2c hardware_pio hardware_pwm)
