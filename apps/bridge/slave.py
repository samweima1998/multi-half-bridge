import RPi.GPIO as GPIO
import time

DATA_PIN = 15
CLK_PIN = 17

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DATA_PIN, GPIO.IN)
GPIO.setup(CLK_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)


def receive_data():
    data = ""
    while len(data) < 8:
        if GPIO.input(CLK_PIN) == GPIO.HIGH:
            data += str(GPIO.input(DATA_PIN))
            while GPIO.input(CLK_PIN) == GPIO.HIGH:
                pass  # Wait for clock to go low to prepare for next bit
    return int(data, 2)


try:
    while True:
        received = receive_data()
        print(f"Received: {received:02X}")
        time.sleep(1)  # Adjust sleep time if necessary
except KeyboardInterrupt:
    GPIO.cleanup()
