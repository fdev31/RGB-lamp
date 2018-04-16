# ESP8266 X-Ring RGB led contact-less lamp

Code & 3D models for a simple hand controlled RGB desktop lamp

Demo here: https://youtu.be/k0g8ce4BeVI

## Lamp made from

- 12 Bits WS2812B RGB LED ring (Adafruit-like)
- APDS9960 (Sparkfun)

## Needed material

- 3D printed part
    - .blend & .stl provided
- ~ 10 small wires
    - soldering iron + tin
- Wemos D1 mini (default platformio.ini config)
    - or any Arduino compatible microcontroller with 5v pin

## Needed pins

- 5v source
- 3v source
- interrupt
- GND
- 1 wire (for RGB leds, pin 13 by default)
- I2C (for the gesture sensor)
    - SDA
    - SCL

## Pinout


| X-Ring    | Wemos D1 |  APDS9960 |
|-----------|----------|-----------|
| GND       | G        |  GND      |
| 5V        | 5V       |           |
|           | 3.3V     |  VCC      |
| TXD       | D5  (14) |           |
|           | D6       |  INT      |
|           | TX (D1)  |  SCL      |
|           | RX (D2)  |  SDA      |

## Upload the code

You will need platformio

    pio update
    make up

## Bugs and limitations

Once assembled the distance from the motion sensor is very reduced, expect 1cm with very sensitive settings.
For that reason no gesture recognition is used but a simple "pass" detector is implemented with two features:

- "double click" (swipe your finger in both direction)
- "press" (let your finger near the receiver and hold)

