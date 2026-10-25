# Smart Home Automation Using IoT

A starter project for demonstrating smart-home monitoring and low-voltage device control using an ESP32, optional sensors, and a browser dashboard.

## Features
- Browser dashboard to switch two demo outputs ON/OFF.
- Optional PIR motion sensor input and DHT22 temperature/humidity readings.
- ESP32 serves a small JSON API and a static dashboard over Wi-Fi.
- Dashboard refreshes device and sensor status periodically.
- Designed for **LEDs or other low-voltage demonstration loads** first.

## Hardware (suggested)
- ESP32 development board
- 2 LEDs with suitable current-limiting resistors, or a low-voltage output module
- Optional PIR sensor (digital output)
- Optional DHT22 sensor + suitable pull-up resistor/module
- USB power and Wi-Fi network

### Pin mapping (edit in `firmware/SmartHomeAutomation.ino`)
- Output 1: GPIO 18
- Output 2: GPIO 19
- PIR sensor: GPIO 27
- DHT22 data: GPIO 4

**Important:** GPIOs must not directly power a relay coil, lamp, or mains appliance. Use correctly rated driver/relay hardware and qualified installation for mains voltage. For first testing, use LEDs/low-voltage loads only.

## Run the dashboard with ESP32
1. Install Arduino IDE.
2. Install ESP32 board support in Boards Manager.
3. Install the `DHT sensor library` by Adafruit if you want to enable the DHT22.
4. Open `firmware/SmartHomeAutomation.ino`.
5. Replace `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD` with your local Wi-Fi credentials. Do not commit real credentials to a public repository.
6. Select your ESP32 board and COM port, then upload.
7. Open Serial Monitor at 115200 baud to see the device IP address.
8. Connect your computer/phone to the same Wi-Fi and open `http://ESP32_IP/`.

### Optional sensor switches
In the sketch, set:
- `#define USE_PIR 1` to enable PIR (default enabled)
- `#define USE_DHT 0` to disable DHT by default; set to `1` after installing the library and connecting the sensor.

## Test checklist
- Dashboard opens at the ESP32 IP address.
- Output 1 and Output 2 buttons change the respective GPIO state.
- PIR state appears when motion is detected (if enabled and connected).
- DHT values appear only if DHT is enabled and connected.
- Disconnect/reconnect Wi-Fi and document observed behavior.

## API
- `GET /api/status` — current outputs and optional sensor values
- `POST /api/output` — JSON body `{"id":1,"state":true}` or `{"id":2,"state":false}`

## Limitations
- This is an educational starter implementation, not a certified home-automation product.
- The API is intended for a trusted local demo network and has no authentication. Do not expose it to the public internet.
- No cloud persistence, user accounts, scheduling, or energy measurement is included yet.
