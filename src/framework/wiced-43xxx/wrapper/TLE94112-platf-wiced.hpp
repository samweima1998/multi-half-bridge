/** 
 * @file        TLE94112-platf-wiced.hpp
 * @brief       TLE94112 Cypress WICED-43xxx Hardware Platforms
 * @date        Mai 2020
 * @copyright   Copyright (c) 2019-2020 Infineon Technologies AG
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef TLE94112_PLATF_WICED_HPP_
#define TLE94112_PLATF_WICED_HPP_

#include "../pal/TLE94112-pal-wiced.hpp"

#if (TLE94112_FRAMEWORK == TLE94112_FRMWK_WICED)

/**
 * @addtogroup tl94112wicedhw
 *
 * @brief Arduino Hardware Platform Pins
 */

/*!
 * Standard chip select pin for first TLE94112 shield
 */
#define TLE94112_PIN_CS1     10

/*!
 * Standard chip select pin for second TLE94112 shield
 * To use a second shield with different CS pin you have
 * to remove the 0 Ohm resistor R7 and solder it to R8
 */
#define TLE94112_PIN_CS2     9

/*!
 * Standard TLE94112 enable pin
 */
#define TLE94112_PIN_EN      8

/**
 * @}
 */

#endif /** TLE94112_FRAMEWORK **/
#endif /** TLE94112_PLATF_WICED_HPP_ **/