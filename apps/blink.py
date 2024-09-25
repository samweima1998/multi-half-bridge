import subprocess
import time


def main():
    # Ask the user for the amount of milliseconds to keep the LED on
    millis = input("Enter the duration in milliseconds to keep the LED on: ")
    try:
        duration = int(millis)
    except ValueError:
        print("Please enter a valid number.")
        return

    # Command to turn on LEDs
    # Example: "./build/control 0 "1,1 0,2""
    # where "1,1" is state "High" for half bridge pin 1, and "0,2" is state "Low" for half bridge pin 2
    activate_leds_cmd = ["./build/control", "0", "1,1 0,2"]

    # Command to turn off LEDs (all floating i.e., not connected)
    # Example: "./build/control 0 "2,1 2,2""
    deactivate_leds_cmd = ["./build/control", "0", "2,1 2,2"]

    # Execute command to turn on the LEDs
    print("Turning on the LEDs...")
    subprocess.run(activate_leds_cmd)

    # Wait for the given amount of milliseconds
    time.sleep(duration / 1000.0)

    # Execute command to turn off the LEDs
    print("Turning off the LEDs...")
    subprocess.run(deactivate_leds_cmd)
    print("Process complete.")


if __name__ == "__main__":
    main()
