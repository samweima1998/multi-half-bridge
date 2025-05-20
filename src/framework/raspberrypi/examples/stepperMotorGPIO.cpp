#include <gpiod.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

#define CHIP_NAME "gpiochip0" // Typical on Raspberry Pi
#define DIR_PIN 16
#define STEP_PIN 6
#define ENABLE_PIN 5

enum Direction { FORWARD, BACKWARD };

void pulseStepPin(gpiod_line* step_line, int steps) {
    for (int i = 0; i < steps; ++i) {
        gpiod_line_set_value(step_line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(20));
        gpiod_line_set_value(step_line, 0);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

int main() {

    // Set up libgpiod chip and lines
    gpiod_chip* chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        std::cerr << "ERROR: Failed to open GPIO chip" << std::endl;
        return 1;
    }

    gpiod_line* dir_line = gpiod_chip_get_line(chip, DIR_PIN);
    gpiod_line* step_line = gpiod_chip_get_line(chip, STEP_PIN);
    gpiod_line* enable_line = gpiod_chip_get_line(chip, ENABLE_PIN);

    if (!dir_line || !step_line || !enable_line) {
        std::cerr << "ERROR: Failed to access one or more GPIO lines" << std::endl;
        return 1;
    }

    // Request all lines as outputs
    if (gpiod_line_request_output(dir_line, "stepper", 0) ||
        gpiod_line_request_output(step_line, "stepper", 0) ||
        gpiod_line_request_output(enable_line, "stepper", 1)) {
        std::cerr << "ERROR: Failed to request GPIO lines as outputs" << std::endl;
        return 1;
    }

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            gpiod_line_set_value(enable_line, 1); // disable motor
            continue;
        }

        std::istringstream input_stream(input);
        std::string direction_str;
        int steps;

        if (!(input_stream >> direction_str >> steps)) {
            std::cout << "ERROR: Invalid command format" << std::endl;
            std::cout << "DONE" << std::endl;
            std::cout.flush();
            continue;
        }

        Direction dir;
        if (direction_str == "FORWARD") dir = FORWARD;
        else if (direction_str == "BACKWARD") dir = BACKWARD;
        else {
            std::cout << "ERROR: Invalid direction" << std::endl;
            std::cout << "DONE" << std::endl;
            std::cout.flush();
            continue;
        }
        std::cout << "Got line: \"" << direction_str << "\"" << std::endl;
        std::cout.flush();

        gpiod_line_set_value(enable_line, 0); // Enable motor (active LOW)
        gpiod_line_set_value(dir_line, dir == FORWARD ? 1 : 0); // Set direction
        std::this_thread::sleep_for(std::chrono::microseconds(100)); // allow DIR line to settle

        // Step
        pulseStepPin(step_line, steps);

        gpiod_line_set_value(enable_line, 1); // Optionally disable motor

        std::cout << "SUCCESS: Moved " << steps << " steps in " << direction_str << " direction" << std::endl;
        std::cout << "DONE" << std::endl;
        std::cout.flush();
    }

    gpiod_chip_close(chip);
    return 0;
}
