#include <bcm2835.h>
#include <iostream>
#include <sstream>
#include <string>

#define DIR_PIN RPI_GPIO_P1_11   // Replace with your actual DIR pin (e.g., GPIO17)
#define STEP_PIN RPI_GPIO_P1_6  // Replace with your actual STEP pin (e.g., GPIO27)

enum Direction { FORWARD, BACKWARD };

void stepMotor(Direction dir, int steps) {
    // Set direction
    bcm2835_gpio_write(DIR_PIN, dir == FORWARD ? HIGH : LOW);

    for (int i = 0; i < steps; ++i) {
        bcm2835_gpio_write(STEP_PIN, HIGH);
        bcm2835_delayMicroseconds(20); // 10-20 us high
        bcm2835_gpio_write(STEP_PIN, LOW);
        bcm2835_delayMicroseconds(1000); // Step interval (1ms = 1000Hz stepping)
    }
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "ERROR: Failed to initialize bcm2835\n";
        return 1;
    }

    bcm2835_gpio_fsel(DIR_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(STEP_PIN, BCM2835_GPIO_FSEL_OUTP);

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty()) continue;

        std::istringstream input_stream(input);
        std::string direction_str;
        int steps;

        if (!(input_stream >> direction_str >> steps)) {
            std::cout << "ERROR: Invalid command format\nEND" << std::endl;
            continue;
        }

        Direction dir;
        if (direction_str == "FORWARD") dir = FORWARD;
        else if (direction_str == "BACKWARD") dir = BACKWARD;
        else {
            std::cout << "ERROR: Invalid direction\nEND" << std::endl;
            continue;
        }

        stepMotor(dir, steps);

        std::cout << "SUCCESS: Moved " << steps << " steps in " << direction_str << " direction\nEND" << std::endl;
    }

    bcm2835_close();
    return 0;
}
