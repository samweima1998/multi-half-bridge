# Master

```sh
cd /multi-half-bridge
python apps/bridge/master.py
```

# Slave

```sh
cd /multi-half-bridge
python apps/bridge/slave.py
```

# Wiring

Connect Pin 11 (GPIO 17) to Pin 11 (GPIO 17) between both Raspberry Pis.
Connect Pin 35 (GPIO 19) to Pin 35 (GPIO 19) between both Raspberry Pis.
Connect GND between both Raspberry Pis.