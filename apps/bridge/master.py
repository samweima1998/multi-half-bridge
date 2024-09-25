import RPi.GPIO as GPIO
import time

DATA_PIN = 17
CLK_PIN = 19

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.OUT)
GPIO.setup(CLK_PIN, GPIO.OUT)


def send_data(data):
    for bit in "{:08b}".format(data):  # Send 8-bit data
        GPIO.output(DATA_PIN, int(bit))
        GPIO.output(CLK_PIN, GPIO.HIGH)
        time.sleep(0.01)
        GPIO.output(CLK_PIN, GPIO.LOW)
        time.sleep(0.01)


try:
    while True:
        send_data(0xA5)  # Example: Send 0xA5
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
