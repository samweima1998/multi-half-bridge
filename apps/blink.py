import subprocess
import time


def run_command(cmd):
    """Executes a shell command."""
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")


def blink_control(blink_time_ms, high_state, low_state):
    """Blinks the voltage by toggling between high and low states."""
    interval_sec = blink_time_ms / 1000.0

    print("Starting the blink process")
    while True:
        # Set high state
        print("Setting high state")
        run_command(high_state)

        # Wait for half the interval
        time.sleep(interval_sec / 2)

        # Set low state
        print("Setting low state")
        run_command(low_state)

        # Wait for half the interval
        time.sleep(interval_sec / 2)


if __name__ == "__main__":
    # Get user input for blink duration in milliseconds
    blink_time_ms = float(
        input("Enter the amount of milliseconds to blink the voltage: ")
    )

    # Commands to set high and low states
    high_state_command = ["../build/control", "0", "1,1 0,2"]
    low_state_command = ["../build/control", "0", "2,1 2,2"]

    try:
        blink_control(blink_time_ms, high_state_command, low_state_command)
    except KeyboardInterrupt:
        print("\nBlinking stopped by the user.")
        # If you want to ensure all processes are terminated, you might need additional clean-up here.
