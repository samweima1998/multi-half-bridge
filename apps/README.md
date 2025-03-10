# RPI ZERO
```sh


# RPI ZERO

```sh
sudo apt-get update
sudo apt-get install git
sudo apt-get install python3-pip

git clone https://github.com/samweima1998/multi-half-bridge.git

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
git clone https://github.com/samweima1998/multi-half-bridge.git
```


# PermissionError: [Errno 13] Permission denied: './build/control'
```sh
chmod +x ./build/control
```

# Auto Launch Server on Boot


To have a Python server script launch automatically when your Raspberry Pi boots, you need to create a service that runs at startup. This can be achieved using `systemd`, which is the initialization system and service manager that is widely used in Linux distributions, including Raspberry Pi OS.

Below are the steps to create a systemd service that will start your Python server when the Raspberry Pi boots:

### Step 1: Create Your Python Server Script

Make sure the server script works as expected by running:

```bash
python /home/pi/multi-half-bridge/apps/server.py
```

Visit `http://<raspberrypi-ip>:7070` from a browser on the same network and check if it displays { "status": True }

### Step 3: Create a systemd Service File

1. Create a service file under `/etc/systemd/system/`. For example, name it `python_server.service`:
```bash
sudo nano /etc/systemd/system/python_server.service
```

2. Add the following content to the service file:
```ini
[Unit]
Description=Python Server
After=network.target

[Service]
ExecStart=/usr/bin/python /home/pi/multi-half-bridge/apps/server.py
WorkingDirectory=/home/pi
StandardOutput=inherit
StandardError=inherit
Restart=always
User=pi

[Install]
WantedBy=multi-user.target
```

3. Save and exit the file (using CTRL+X, then Y and Enter in nano).

### Step 4: Enable and Start the Service

1. Reload the systemd manager configuration:
```bash
sudo systemctl daemon-reload
```

2. Enable the new service to start on boot:
```bash
sudo systemctl enable python_server.service
```

3. Start the service immediately:
```bash
sudo systemctl start python_server.service
```

### Step 5: Check the Status of Your Service

To ensure that the service is active and running, you can use:
```bash
sudo systemctl status python_server.service
```

If everything is set up correctly, your Python server should now automatically start when your Raspberry Pi boots up.

### Troubleshooting

If your server doesn't start properly, you can check the logs for errors:
```bash
sudo journalctl -u python_server.service
```

