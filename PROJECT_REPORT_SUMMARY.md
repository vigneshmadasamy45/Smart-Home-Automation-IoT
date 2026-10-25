# Smart Home Automation — Project Summary

## Objective
Create an educational IoT prototype that lets a user control two low-voltage demo outputs from a browser dashboard and view optional sensor status.

## Architecture
- ESP32 handles input reading, output control, Wi-Fi, and HTTP API.
- Browser dashboard is served by the ESP32.
- Optional PIR motion sensor reports motion state.
- Optional DHT22 reports temperature and humidity when enabled.
- Outputs use GPIO 18 and 19; use LEDs with current-limiting resistors for initial testing.

## Endpoints
- `GET /api/status`
- `POST /api/output` with JSON: `{\"id\":1,\"state\":true}`

## Test plan
1. Upload firmware and open serial monitor at 115200 baud.
2. Join same Wi-Fi and open the printed dashboard URL.
3. Test each output ON/OFF with LEDs.
4. Verify PIR status if connected.
5. If DHT22 is enabled, verify values against a known reference if available.
6. Test Wi-Fi loss/recovery and document actual results.
