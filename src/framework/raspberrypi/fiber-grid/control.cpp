#include "tle94112-rpi.hpp"
#include "tle94112-platf-rpi.hpp"

#include <cstdio>
#include <bcm2835.h>
#include <cstdlib> // Included for atoi()
#include <vector>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s <cs_pin> <state> <hb_pin1,hb_pin2,...,hb_pinN>\n", argv[0]);
        printf("cs_pin: 0-3 (for CS0 to CS3)\n");
        printf("state: 0 for Low, 1 for High, 2 for Floating\n");
        printf("hb_pin: Comma separated list of integers from 1-12 (for HB1 to HB12)\n");
        return -1;
    }

    // Evaluate chip select pin
    int csPinIndex = atoi(argv[1]);
    if (csPinIndex < 0 || csPinIndex > 3)
    {
        printf("Invalid chip select pin number.\n");
        return -1;
    }
    uint8_t csPins[] = {TLE94112_PIN_CS0, TLE94112_PIN_CS1, TLE94112_PIN_CS2, TLE94112_PIN_CS3, TLE94112_PIN_CS4, TLE94112_PIN_CS5, TLE94112_PIN_CS6, TLE94112_PIN_CS7};
    uint8_t csPin = csPins[csPinIndex];

    // Parsing comma-separated list of half bridge pins
    std::vector<int> hbPinIndices;
    std::string token;
    std::istringstream tokenStream(argv[3]);
    while (std::getline(tokenStream, token, ','))
    {
        int hbPinIndex = std::atoi(token.c_str());
        if (hbPinIndex < 1 || hbPinIndex > 12)
        {
            printf("Invalid half bridge pin number: %d\n", hbPinIndex);
            return -1;
        }
        hbPinIndices.push_back(hbPinIndex - 1); // convert 1-indexed to 0-indexed
    }

    // Evaluate state
    int stateIndex = atoi(argv[2]);
    if (stateIndex < 0 || stateIndex > 2)
    {
        printf("Invalid state input. Use 0 for Low, 1 for High, or 2 for Floating.\n");
        return -1;
    }
    Tle94112::HBState states[] = {Tle94112::TLE_LOW, Tle94112::TLE_HIGH, Tle94112::TLE_FLOATING};
    Tle94112::HBState hbState = states[stateIndex];

    // Create an instance of the TLE94112 controller
    Tle94112Rpi controller(csPin);
    controller.begin(); // Initialize the controller

    // Set the specified half bridges to the specified state
    for (int hbPinIndex : hbPinIndices)
    {
        Tle94112::HalfBridge hbPins[] = {
            Tle94112::TLE_HB1, Tle94112::TLE_HB2, Tle94112::TLE_HB3, Tle94112::TLE_HB4,
            Tle94112::TLE_HB5, Tle94112::TLE_HB6, Tle94112::TLE_HB7, Tle94112::TLE_HB8,
            Tle94112::TLE_HB9, Tle94112::TLE_HB10, Tle94112::TLE_HB11, Tle94112::TLE_HB12};
        Tle94112::HalfBridge hbPin = hbPins[hbPinIndex]; // convert 1-indexed to 0-indexed
        controller.configHB(hbPin, hbState, Tle94112::TLE_NOPWM);
    }

    // The program will terminate but the controller maintains the state since we don't call end() or clear configuration.
    return 0;
}