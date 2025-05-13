#include <bcm2835.h>
#include <iostream>
#include <sstream>
#include <string>

#define DIR_PIN 25       // BCM GPIO 11
#define STEP_PIN 6       // BCM GPIO 6
#define ENABLE_PIN 5     // BCM GPIO 5 (adjust to your actual pin)

enum Direction { FORWARD, BACKWARD };

void stepMotor(Direction dir, int steps) {
    // Enable motor (active LOW)
    bcm2835_gpio_write(ENABLE_PIN, LOW);

    // Set direction
    bcm2835_gpio_write(DIR_PIN, dir == FORWARD ? HIGH : LOW);

    for (int i = 0; i < steps; ++i) {
        bcm2835_gpio_write(STEP_PIN, HIGH);
        bcm2835_delayMicroseconds(20);
        bcm2835_gpio_write(STEP_PIN, LOW);
        bcm2835_delayMicroseconds(100);
    }

    // Disable motor after movement
    // bcm2835_delay(500);  // Let motor hold position for 0.5 sec
    // bcm2835_gpio_write(ENABLE_PIN, HIGH);
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "ERROR: Failed to initialize bcm2835\n";
        return 1;
    }

    bcm2835_gpio_fsel(DIR_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(STEP_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(ENABLE_PIN, BCM2835_GPIO_FSEL_OUTP);

    // Initially disable motor
    bcm2835_gpio_write(ENABLE_PIN, HIGH);

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            // No command input, keep motor disabled
            bcm2835_gpio_write(ENABLE_PIN, HIGH);
            continue;
        }

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

        // Report status to server
        std::cout << "SUCCESS: Moved " << steps << " steps in " << direction_str << " direction" << std::endl;
        std::cout << "DONE" << std::endl;
    }

    bcm2835_gpio_write(ENABLE_PIN, HIGH);  // Ensure motor is disabled before exit
    bcm2835_close();
    return 0;
}
