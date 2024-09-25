import RPi.GPIO as GPIO
import time
import subprocess


DATA_PIN = 17
CLK_PIN = 19

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.IN)
GPIO.setup(CLK_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)


def receive_byte():
    bits = ""
    for _ in range(8):  # Receive each bit to form a byte
        while GPIO.input(CLK_PIN) == GPIO.LOW:
            pass  # Wait for the clock to go high
        bits += str(GPIO.input(DATA_PIN))
        while GPIO.input(CLK_PIN) == GPIO.HIGH:
            pass  # Wait for the clock to go low
    return int(bits, 2)


def receive_data():
    length = receive_byte()  # First receive the length of the string
    received_bytes = []
    for _ in range(length):
        received_bytes.append(receive_byte())
    return bytes(received_bytes).decode()


def run_command(cmd_args):
    """
    Function to execute a given command.
    """
    try:
        command = command_args[0]
        cs_pin = command_args[1]
        pairs = " ".join(
            command_args[2:]
        )  # Join all pairs elements to form a single second argument

        print([command, cs_pin, pairs.strip('"')])

        subprocess.run([command, cs_pin, pairs.strip('"')], check=True)
        print("Command executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while executing command: {e}")


try:
    while True:
        received_str = receive_data()
        print(f"Received string: {received_str}")

        command_args = ["./build/control"]

        if received_str == "exit":
            break
        elif received_str == "up":
            run_command(command_args + ["0", "0,3 1,10"])
            time.sleep(1)
            run_command(command_args + ["0", "2,3 2,10"])
        elif received_str == "down":
            run_command(command_args + ["0", "1,3 0,10"])
            time.sleep(1)
            run_command(command_args + ["0", "2,3 2,10"])
        elif received_str == "forward":
            run_command(command_args + ["0", "0,4 1,6"])
            time.sleep(1)
            run_command(command_args + ["0", "2,4 2,6"])
        elif received_str == "backward":
            run_command(command_args + ["0", "0,4 1,9"])
            time.sleep(1)
            run_command(command_args + ["0", "2,4 2,9"])
        elif received_str == "left":
            run_command(command_args + ["0", "0,4 1,7"])
            time.sleep(1)
            run_command(command_args + ["0", "2,4 2,7"])
        elif received_str == "right":
            run_command(command_args + ["0", "0,4 1,5"])
            time.sleep(1)
            run_command(command_args + ["0", "2,4 2,5"])
        else:
            pass

        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
