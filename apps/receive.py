import RPi.GPIO as GPIO
import time

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


try:
    while True:
        received_str = receive_data()
        print(f"Received string: {received_str}")
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
