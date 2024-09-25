# RPI ZERO

```sh
sudo apt-get update
sudo apt-get install git
git clone https://github.com/tjbck/multi-half-bridge.git
```

# RPI 5

```sh
sudo apt-get update
sudo raspi-config # Enable Remote GPIO
sudo apt remove python3-rpi.gpio
pip3 install rpi-lgpio --break-system-packages
git clone https://github.com/tjbck/multi-half-bridge.git
```
