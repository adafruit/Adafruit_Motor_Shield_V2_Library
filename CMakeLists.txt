cmake_minimum_required(VERSION 3.14)
project(AdafruitMotorShieldV2 VERSION 1.1.4 LANGUAGES CXX)

add_library(AdafruitMotorShieldV2
    Adafruit_MotorShield.cpp
    utility/Adafruit_MS_PWMServoDriver.cpp
    utility/linux_i2c.cpp
)

target_include_directories(AdafruitMotorShieldV2
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utility
)

target_compile_features(AdafruitMotorShieldV2 PUBLIC cxx_std_11)

add_executable(motor-cli cli.cpp)
target_link_libraries(motor-cli PRIVATE AdafruitMotorShieldV2)
target_compile_features(motor-cli PRIVATE cxx_std_11)
