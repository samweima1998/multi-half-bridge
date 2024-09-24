#include "tle94112-rpi.hpp"
#include "tle94112-platf-rpi.hpp"
#include <cstdio>
#include <bcm2835.h>

#include <cstdlib> // For atoi()
#include <vector>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <cs_pin> \"<state1,hb_pin1> <state2,hb_pin2> ... <stateN,hb_pinN>\"\n", argv[0]);
        printf("cs_pin: 0-7 (for CS0 to CS7)\n");
        printf("<stateN,hb_pinN>: Each pair separated by space, state 0 for Low, 1 for High, 2 for Floating\n");
        return -1;
    }

    // Evaluate chip select pin
    int csPinIndex = atoi(argv[1]);
    if (csPinIndex < 0 || csPinIndex > 7)
    {
        printf("Invalid chip select pin number.\n");
        return -1;
    }

    uint8_t csPins[] = {TLE94112_PIN_CS0, TLE94112_PIN_CS1, TLE94112_PIN_CS2, TLE94112_PIN_CS3, TLE94112_PIN_CS4, TLE94112_PIN_CS5, TLE94112_PIN_CS6, TLE94112_PIN_CS7};
    uint8_t csPin = csPins[csPinIndex];

    // Create an instance of the TLE94112 controller
    Tle94112Rpi controller(csPin);
    controller.begin(); // Initialize the controller

    // Parse state and half-bridge pairs
    std::istringstream pairStream(argv[2]);
    std::string pair;
    while (std::getline(pairStream, pair, ' '))
    {
        std::istringstream singlePair(pair);
        std::string stateStr, hbStr;
        if (!std::getline(singlePair, stateStr, ',') || !std::getline(singlePair, hbStr))
        {
            printf("Error parsing pair: %s\n", pair.c_str());
            continue;
        }
        int state = atoi(stateStr.c_str());
        int hbPinIndex = atoi(hbStr.c_str()) - 1;

        if (state < 0 || state > 2 || hbPinIndex < 0 || hbPinIndex > 11)
        {
            printf("Invalid state or half bridge pin in pair: %s\n", pair.c_str());
            continue;
        }

        Tle94112::HBState states[] = {Tle94112::TLE_LOW, Tle94112::TLE_HIGH, Tle94112::TLE_FLOATING};
        Tle94112::HalfBridge hbPins[] = {
            Tle94112::TLE_HB1, Tle94112::TLE_HB2, Tle94112::TLE_HB3, Tle94112::TLE_HB4,
            Tle94112::TLE_HB5, Tle94112::TLE_HB6, Tle94112::TLE_HB7, Tle94112::TLE_HB8,
            Tle94112::TLE_HB9, Tle94112::TLE_HB10, Tle94112::TLE_HB11, Tle94112::TLE_HB12};

        controller.configHB(hbPins[hbPinIndex], states[state], Tle94112::TLE_NOPWM);
    }

    // The program will terminate but the controller maintains the state since we don't call end() or clear configuration.
    return 0;
}