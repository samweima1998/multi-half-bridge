#include "tle94112-rpi.hpp"
#include "tle94112-platf-rpi.hpp"
#include <cstdio>
#include <bcm2835.h>

#include <cstdlib> // For atoi()
#include <vector>
#include <sstream>
#include <string>
#include <iostream>

int main()
{
    // Initialize the controller for all chip select pins
    uint8_t csPins[] = {TLE94112_PIN_CS0, TLE94112_PIN_CS1, TLE94112_PIN_CS2, TLE94112_PIN_CS3,
                        TLE94112_PIN_CS4, TLE94112_PIN_CS5, TLE94112_PIN_CS6, TLE94112_PIN_CS7};
    Tle94112Rpi controllers[8] = {Tle94112Rpi(csPins[0]), Tle94112Rpi(csPins[1]),
                                  Tle94112Rpi(csPins[2]), Tle94112Rpi(csPins[3]),
                                  Tle94112Rpi(csPins[4]), Tle94112Rpi(csPins[5]),
                                  Tle94112Rpi(csPins[6]), Tle94112Rpi(csPins[7])};

    Tle94112::HBState states[] = {Tle94112::TLE_FLOATING, Tle94112::TLE_LOW, Tle94112::TLE_HIGH};
    Tle94112::HalfBridge hbPins[] = {
        Tle94112::TLE_HB1, Tle94112::TLE_HB2, Tle94112::TLE_HB3, Tle94112::TLE_HB4,
        Tle94112::TLE_HB5, Tle94112::TLE_HB6, Tle94112::TLE_HB7, Tle94112::TLE_HB8,
        Tle94112::TLE_HB9, Tle94112::TLE_HB10, Tle94112::TLE_HB11, Tle94112::TLE_HB12};

    // combining the two loops below causes some controllers to be unresponsive
    for (int i = 0; i < 8; i++)
    {
        controllers[i].begin();
    }
    for (int i = 0; i < 8; i++)
    {
        controllers[i].cs->init();
    }
    for (int i = 0; i < 8; i++)
    {
        controllers[i].clearErrors();
        for (int hbPinIndex = 0; hbPinIndex < 12; hbPinIndex++)
        {
            controllers[i].configHB(hbPins[hbPinIndex], states[0],Tle94112::TLE_NOPWM);
        }
    }
    std::cout << "READY\n"
              << std::flush;

    std::string input;
    while (std::getline(std::cin, input)) // Read commands from stdin
    {
        if (input.empty())
            continue;

        std::istringstream input_stream(input);
        std::string cs_pin_str, state_hb_pairs;
        if (!(input_stream >> cs_pin_str) || !std::getline(input_stream, state_hb_pairs))
        {
            std::cout << "ERROR: Invalid command format\nEND\n"
                      << std::flush;
            continue;
        }

        int csPinIndex = std::atoi(cs_pin_str.c_str());
        if (csPinIndex < 0 || csPinIndex > 7)
        {
            std::cout << "ERROR: Invalid chip select pin number\nEND\n"
                      << std::flush;
            continue;
        }

        controllers[csPinIndex].cs->enable();

        std::istringstream pairStream(state_hb_pairs);
        std::string pair;
        bool error = false;
        std::getline(pairStream, pair, ' '); // to remove lead null
        while (std::getline(pairStream, pair, ' '))
        {

            std::istringstream singlePair(pair);
            std::string stateStr, hbStr;
            if (!std::getline(singlePair, stateStr, ',') || !std::getline(singlePair, hbStr))
            {
                std::cout << "ERROR: Error parsing pair: " << pair << "\nEND\n"
                          << std::flush;
                error = true;
                break;
            }

            int state = std::atoi(stateStr.c_str());
            int hbPin = std::atoi(hbStr.c_str()) - 1;

            if (state < 0 || state > 2 || hbPin < 0 || hbPin > 11)
            {
                std::cout << "ERROR: Invalid state or half-bridge pin in pair: " << pair << "\nEND\n"
                          << std::flush;
                error = true;
                break;
            }

            controllers[csPinIndex].configChip(hbPins[hbPin], states[state], Tle94112::TLE_NOPWM);

        }

        if (!error)
        {
            std::cout << "SUCCESS: Command processed for CS" << csPinIndex << "\nEND\n"
                      << std::flush;
        }
        controllers[csPinIndex].timer->delayMilli(1); /*! \brief time in milliseconds to wait for chipselect signal raised */
    }

    std::cout << "SHUTDOWN\n"
              << std::flush;

    for (int i = 0; i < 8; i++)
    {
        controllers[i].end();
    }
    return 0;
}
