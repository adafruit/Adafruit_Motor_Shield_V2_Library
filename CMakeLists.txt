cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "Adafruit_MotorShield.cpp" "utility/Adafruit_MS_PWMServoDriver.cpp"
                    INCLUDE_DIRS "." "utility/"
                    REQUIRES arduino Adafruit_BusIO
                    )

project(Adafruit_Motor_Shield_V2_Library)

