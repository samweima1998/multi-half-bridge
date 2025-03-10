#include "tle94112-rpi.hpp"
#include <bcm2835.h>
#include <iostream>
#include <sstream>
#include <string>

class BipolarStepper {
public:
    enum direction { FORWARD = 0, BACKWARD };

    Tle94112 *mDriver;
    Tle94112::HalfBridge hb_a1, hb_a2, hb_b1, hb_b2;
    int8_t state = 0;

    BipolarStepper(Tle94112 &mDriver, Tle94112::HalfBridge a1, Tle94112::HalfBridge a2, 
                   Tle94112::HalfBridge b1, Tle94112::HalfBridge b2)
        : mDriver(&mDriver), hb_a1(a1), hb_a2(a2), hb_b1(b1), hb_b2(b2) {}

    void disableOutputs() {
        mDriver->configHB(hb_a1, mDriver->TLE_FLOATING, mDriver->TLE_NOPWM);
        mDriver->configHB(hb_a2, mDriver->TLE_FLOATING, mDriver->TLE_NOPWM);
        mDriver->configHB(hb_b1, mDriver->TLE_FLOATING, mDriver->TLE_NOPWM);
        mDriver->configHB(hb_b2, mDriver->TLE_FLOATING, mDriver->TLE_NOPWM);
    }

    void fullStep(direction dir) {
        switch (state) {
            case 0:
                mDriver->configHB(hb_a1, mDriver->TLE_HIGH, mDriver->TLE_NOPWM);
                mDriver->configHB(hb_a2, mDriver->TLE_LOW, mDriver->TLE_NOPWM);
                break;
            case 1:
                mDriver->configHB(hb_b1, mDriver->TLE_HIGH, mDriver->TLE_NOPWM);
                mDriver->configHB(hb_b2, mDriver->TLE_LOW, mDriver->TLE_NOPWM);
                break;
            case 2:
                mDriver->configHB(hb_a2, mDriver->TLE_HIGH, mDriver->TLE_NOPWM);
                mDriver->configHB(hb_a1, mDriver->TLE_LOW, mDriver->TLE_NOPWM);
                break;
            case 3:
                mDriver->configHB(hb_b2, mDriver->TLE_HIGH, mDriver->TLE_NOPWM);
                mDriver->configHB(hb_b1, mDriver->TLE_LOW, mDriver->TLE_NOPWM);
                break;
        }
        state = (dir == FORWARD) ? (state + 1) % 4 : (state - 1 + 4) % 4;
    }
};

int main() {
    Tle94112Rpi controller;
    controller.begin();
    controller.clearErrors();

    BipolarStepper stepper(controller, controller.TLE_HB1, controller.TLE_HB5, 
                           controller.TLE_HB7, controller.TLE_HB9);
    stepper.disableOutputs();
    std::cout << "READY" << std::endl;

    std::string input;
    while (std::getline(std::cin, input)) {  // Listen for commands from stdin
        if (input.empty()) continue;

        std::istringstream input_stream(input);
        std::string direction_str;
        int steps;

        if (!(input_stream >> direction_str >> steps)) {
            std::cout << "ERROR: Invalid command format\nEND" << std::endl;
            continue;
        }

        BipolarStepper::direction dir;
        if (direction_str == "FORWARD") dir = BipolarStepper::FORWARD;
        else if (direction_str == "BACKWARD") dir = BipolarStepper::BACKWARD;
        else {
            std::cout << "ERROR: Invalid direction\nEND" << std::endl;
            continue;
        }
        for (int i = 0; i < steps; i++) {
            stepper.fullStep(dir);
            // bcm2835_delay(400 / 200); // Assuming 200 steps per revolution
        }

        std::cout << "SUCCESS: Moved " << steps << " steps in " << direction_str << " direction\nEND" << std::endl;
        stepper.disableOutputs();
    }

    std::cout << "SHUTDOWN" << std::endl;
    stepper.disableOutputs();
    return 0;
}
