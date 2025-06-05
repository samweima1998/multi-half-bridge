/**
 * @file        tle94112-platf-rpi.hpp
 * @brief       TLE94112 Raspberry Pi Hardware Platforms
 * @copyright   Copyright (c) 2019-2020 Infineon Technologies AG
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TLE94112_PLATF_RPI_HPP_
#define TLE94112_PLATF_RPI_HPP_

#include <bcm2835.h>

/**
 * @addtogroup platfRpi
 * @{
 *
 * @brief Raspberry Pi Hardware Platform Pins
 */

/*!
 * Standard chip select pin for first TLE94112 shield
 */
#define TLE94112_PIN_CS0 RPI_V2_GPIO_P1_24

/*!
 * Standard chip select pin for second TLE94112 shield
 * To use a second shield with different CS pin you have
 * to change the position of the Jumper
 */
#define TLE94112_PIN_CS1 RPI_V2_GPIO_P1_26 //GPIO 7
// #define TLE94112_PIN_CS2 RPI_V2_GPIO_P1_22 //GPIO 25 //for TLE94112ES prototype
#define TLE94112_PIN_CS2 RPI_V2_GPIO_P1_36 //GPIO 16 //for magnetStage
#define TLE94112_PIN_CS3 RPI_V2_GPIO_P1_15 //GPIO 22
#define TLE94112_PIN_CS4 RPI_V2_GPIO_P1_12 //GPIO 18
#define TLE94112_PIN_CS5 RPI_V2_GPIO_P1_16 //GPIO 23
#define TLE94112_PIN_CS6 RPI_V2_GPIO_P1_18 //GPIO 24
#define TLE94112_PIN_CS7 RPI_V2_GPIO_P1_13 //GPIO 27

/*!
 * Standard TLE94112 enable pin
 */
#define TLE94112_PIN_EN RPI_V2_GPIO_P1_37 //GPIO 26

/** @} */

#endif /** TLE94112_PLATF_RPI_HPP_ **/