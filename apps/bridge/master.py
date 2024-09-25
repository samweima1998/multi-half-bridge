import RPi.GPIO as GPIO
import time

DATA_PIN = 17
CLK_PIN = 19

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.OUT)
GPIO.setup(CLK_PIN, GPIO.OUT)


def send_data(data):
    # Convert integer to a byte string
    byte_array = data.to_bytes(4, byteorder="big", signed=True)

    for byte in byte_array:
        for bit in "{:08b}".format(byte):
            GPIO.output(DATA_PIN, int(bit))
            GPIO.output(CLK_PIN, GPIO.HIGH)
            time.sleep(0.01)
            GPIO.output(CLK_PIN, GPIO.LOW)
            time.sleep(0.01)


try:
    while True:
        # Accept user input, convert it to integer and send
        data = int(input("Enter an integer to send: "))
        send_data(data)
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
except ValueError as e:
    print(f"Invalid input: {e}")
    GPIO.cleanup()
