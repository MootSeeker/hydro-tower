# 🌱 Hydroponic Tower Firmware

A **firmware for an ESP32-S3-based hydroponic tower controller**. This project enables **sensor monitoring, actuator control, and data transmission via MQTT** for integration with **Home Assistant**. 

[![forthebadge](https://forthebadge.com/images/badges/powered-by-coffee.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/open-source.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/license-mit.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/works-on-my-machine.svg)](https://forthebadge.com)

---

## 📌 Features
- **FreeRTOS-based multitasking** – Efficient real-time execution 
- **ESP32-S3 support** – Optimized for dual-core processing 
- **WiFi & MQTT integration** – Connects with Home Assistant 
- **LVGL UI on IPS LCD** – Displays real-time system data 
- **Sensor & actuator management** – Read & control hydroponic system 
---

# 🚀 Building & Flashing the Firmware

## 1️⃣ Set up ESP-IDF environment  
Before compiling the project, make sure the **ESP-IDF environment** is correctly loaded in your terminal.

- **For Linux/macOS:** Run the ESP-IDF export script.
- **For Windows (PowerShell):** Execute the ESP-IDF export command.

## 2️⃣ Configure the firmware  
To modify firmware settings (e.g., WiFi credentials, MQTT broker), use the configuration menu.

Run the following command in the project root directory to open the **ESP-IDF menuconfig** interface.

## 3️⃣ Build the firmware  
Compile the project using ESP-IDF’s build system. This will generate the firmware binary files.

## 4️⃣ Flash the firmware  
Once the build is complete, flash the firmware to the ESP32-S3 using the connected USB interface.

## 5️⃣ Monitor serial output  
After flashing, start monitoring the ESP32-S3 log output to debug and verify execution.

This will display real-time logs, including sensor readings, MQTT messages, and system status updates.


---

## 🛠️ Used Components

### 🔌 Microcontroller
- **ESP32-S3 (Waveshare) with IPS LCD**  
  The main controller responsible for managing sensors, actuators, and communication. Features dual-core processing and integrated WiFi.

### 📟 Display
- **IPS LCD Display (LVGL-based UI)**  
  A graphical display for real-time monitoring and control, using the **LVGL library** for rendering.

### 🌡️ Sensors
- **DFRobot Capacitive Water Level Sensor**  
  Used to monitor the water level in the reservoir and prevent pump dry-runs.
- **Elecrow G1/2" Water Flow Sensor**  
  Measures water flow to detect pump failures or blockages.
- **Adafruit DS18B20 Temperature Sensor**  
  Monitors water temperature to ensure optimal conditions for plant growth.
- **pH & EC Sensors (Future Enhancement)**  
  For monitoring the nutrient solution quality in the hydroponic system.

### 💧 Actuators
- **12V Water Pump**  
  Circulates nutrient-rich water through the hydroponic tower.
- **Solenoid Valves**  
  Used for automated control of water distribution.
- **LED Grow Lights (Future Enhancement)**  
  Provides additional lighting to optimize plant growth in low-light environments.

### 📡 Communication Modules
- **WiFi (ESP32 Built-in)**  
  Enables wireless communication with **Home Assistant** via MQTT.
- **MQTT Protocol**  
  Used for sending sensor data and receiving control commands.

### 🔋 Power Supply
- **12V Power Adapter (for Pump & LEDs)**  
  Provides power for the pump and future LED expansion.
- **5V Regulator**  
  Steps down voltage to power the ESP32-S3 and sensors.

---

## 🏆 License
This project is licensed under the **MIT License** – Feel free to use, modify & share! 🌿

---

💬 **Feedback or questions?** Open an issue! 🚀
