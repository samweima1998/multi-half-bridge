import sys
import RPi.GPIO as GPIO
import time

DATA_PIN = 17
CLK_PIN = 19

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.OUT)
GPIO.setup(CLK_PIN, GPIO.OUT)


def send_data(data):
    # Convert the string data to bytes
    bytes_data = data.encode()

    # Send the length of the bytes first
    send_byte(len(bytes_data))

    # Send each byte of the string
    for byte in bytes_data:
        send_byte(byte)


def send_byte(byte):
    for bit in "{:08b}".format(byte):  # Send each bit of the byte
        GPIO.output(DATA_PIN, int(bit))
        GPIO.output(CLK_PIN, GPIO.HIGH)
        time.sleep(0.01)
        GPIO.output(CLK_PIN, GPIO.LOW)
        time.sleep(0.01)


def main():
    try:
        # Join all arguments into a single string, separated by spaces
        all_input = " ".join(
            sys.argv[1:]
        )  # sys.argv[0] is the script name, ignored here
        send_data(all_input)
        time.sleep(1)
    except KeyboardInterrupt:
        GPIO.cleanup()
    except ValueError as e:
        print(f"Invalid input: {e}")
        GPIO.cleanup()


if __name__ == "__main__":
    main()
