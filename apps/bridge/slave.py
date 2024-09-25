import RPi.GPIO as GPIO
import time

DATA_PIN = 17
CLK_PIN = 19

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.IN)
GPIO.setup(CLK_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)


def receive_data():
    byte_data = []
    for _ in range(4):  # Receive 4 bytes
        bits = ""
        for _ in range(8):  # Receive each bit
            while GPIO.input(CLK_PIN) == GPIO.LOW:
                pass  # Wait for the clock to go high
            bits += str(GPIO.input(DATA_PIN))
            while GPIO.input(CLK_PIN) == GPIO.HIGH:
                pass  # Wait for the clock to go low
        byte_data.append(int(bits, 2))
    # Convert byte array back to integer
    return int.from_bytes(byte_data, byteorder="big", signed=True)


try:
    while True:
        received_int = receive_data()
        print(f"Received integer: {received_int}")
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
