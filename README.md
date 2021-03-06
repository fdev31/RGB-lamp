# ESP8266 X-Ring RGB led contact-less lamp

Code & 3D models for a simple hand controlled RGB desktop lamp

[![Demo video](https://img.youtube.com/vi/k0g8ce4BeVI/0.jpg)](https://www.youtube.com/watch?v=k0g8ce4BeVI)

## Lamp made from

- 12 Bits WS2812B RGB LED ring (Adafruit-like)
- APDS9960 (Sparkfun)

## Needed material

- 3D printed part
    - .blend & .stl provided
- ~ 10 small wires
    - soldering iron + tin
- Some micro controller (tested with Wemos D1 & D1 mini)
    - 5v
    - 3.3v
    - I2C
    - 1 free interrupt pin
    - 1 PWM / 1 wire capable pin

## Pinout


| X-Ring    | Wemos D1 mini |  APDS9960 |
|-----------|---------------|-----------|
| GND       | G             |  GND      |
| 5V        | 5V            |           |
|           | 3.3V          |  VCC      |
| TXD       | D5  (14)      |           |
|           | D6            |  INT      |
|           | D1 / TX       |  SCL      |
|           | D2 / RX       |  SDA      |

## Upload the code

You will need platformio

    pio update
    make up

## Bugs and limitations

Once assembled the distance from the motion sensor is very reduced, expect 1cm with very sensitive settings.
For that reason no gesture recognition is used but a simple "pass" detector is implemented with two features:

- "double click" (swipe your finger in both direction)
- "press" (let your finger near the receiver and hold)

