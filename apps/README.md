# RPI ZERO

```sh
sudo apt-get update
sudo apt-get install git
sudo apt-get install python3-pip

git clone https://github.com/tjbck/multi-half-bridge.git

cd multi-half-bridge
pip3 install -r apps/requirements.txt

# Run Server
python apps/server.py



```

# RPI 5

```sh
sudo apt-get update
sudo raspi-config # Enable Remote GPIO
sudo apt remove python3-rpi.gpio
pip3 install rpi-lgpio --break-system-packages
git clone https://github.com/tjbck/multi-half-bridge.git
```


# PermissionError: [Errno 13] Permission denied: './build/control'
```sh
chmod +x ./build/control
```