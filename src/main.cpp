#include "Adafruit_APDS9960.h"
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include "hsv.h"

//the pin that the interrupt is attached to
#define INT_PIN 12 // FOR THE RECEIVER
#define NB_MODES 4

#define PRESS_FORCE 15 // replace with 30 or 100 if too sensitive
#define PRESS_SLOWDOWN 100000.0
#define MAX_PRESSTIME 360.0
#define DEFAULT_COLOR 0x080833

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)
#define RGB2Color(rgb) strip.Color(rgb.b*255, rgb.r*255, rgb.g*255)

// key handling
unsigned long keypress = 0;
int long_press = 0;
int previous_long_press = 0;

// time
unsigned long prev_time;

// state machine
unsigned long press_duration = 0;
unsigned long last_duration = 0;
unsigned long last_ts = 0;
unsigned char loop_mode = 0;

bool is_dirty = false;

double brightness = 0.7;
double hue = 0.0;
double saturation = 1.0;

// breakboards objects
Adafruit_APDS9960 apds;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, 14, NEO_GRB + NEO_KHZ800); // 12 leds, pin 13

#define paint(c) { for(uint16_t i=0; i<strip.numPixels(); i++) { strip.setPixelColor(i, c); } strip.show(); };

void setup() {
    strip.begin();
    paint(DEFAULT_COLOR);

    Serial.begin(115200);
    WiFi.mode(WIFI_OFF);
    pinMode(INT_PIN, INPUT_PULLUP);

    if(!apds.begin()) Serial.println("failed to initialize device! Please check your wiring.");
    else Serial.println("Device initialized!");

    //apds.enableGesture(true);
    apds.enableProximity(true);
    apds.setProximityInterruptThreshold(0, PRESS_FORCE/4);
    apds.enableProximityInterrupt();

    Serial.println("Hello Anna!!");
}


void loop() {
    double dt = fmod((millis()/(1000.0/((long_press/10.0)+1))), 360.0);

    switch(loop_mode) {
        case 0: // white light (variable intensity)
            if (is_dirty) paint(RGB2Color(hsv2rgb({0, 0, long_press?long_press/MAX_PRESSTIME:brightness})));
            break;
        case 1: // single color (variable color)
            if (is_dirty) { paint( RGB2Color(hsv2rgb({
                            (double) keypress?fmod(press_duration/30.0,360.0):hue,
                            saturation, brightness})));
            }
            break;
        case 2: // rotating rainbow (variable speed)
            uint16_t i;
            for(i=0; i< strip.numPixels(); i++) {
                strip.setPixelColor(i,
                        RGB2Color(hsv2rgb({fmod(i*360.0 / strip.numPixels() + dt, 360.0), saturation, brightness}))
                        );
            }
            strip.show();
            break;
        case 3: // ever changing color (variable speed)
            paint( RGB2Color(hsv2rgb({dt, saturation, brightness})));
            break;
    }

    is_dirty = false; // reset value before testing
    if(!digitalRead(INT_PIN)) {
        unsigned long this_time = millis();
        unsigned char cur = apds.readProximity();
        unsigned char is_pressed = cur > PRESS_FORCE;
        unsigned char is_release = cur < PRESS_FORCE;

        if (keypress == 0 && is_pressed) {
            if (this_time - last_ts < 300 && last_duration < 100) {
                // DOUBLE CLICK
                loop_mode = (loop_mode+1)%NB_MODES;

                // Handle state transition HERE
                if (previous_long_press && previous_long_press > 0.5)
                    switch(loop_mode) {
                        case 1:
                            brightness = MAX(1/255.0, previous_long_press/MAX_PRESSTIME);
                            break;
                        case 2:
                            hue = fmod(press_duration/29.0,360.0);
                            break;

                        default:
                            break;
                    }
                Serial.println("Keypress");
                long_press = 0;
                last_ts = 0;
                last_duration = 0;
                previous_long_press = 0;
                is_dirty = true;
                return;
            }
            keypress = this_time;
        } else if (keypress) {
            if (is_release && keypress != 0) {
                // RELEASE
                last_ts = this_time;
                long_press = 0;
                last_duration = this_time - keypress;
                Serial.print("Duration ");
                Serial.println(last_duration/1000.0);
                keypress = 0;
            } else {
                // PRESS
                Serial.println("press press press");
                long_press = MIN(MAX_PRESSTIME, (int)(((float)(this_time - keypress)*(this_time-keypress))/PRESS_SLOWDOWN));
                press_duration = this_time - keypress;
                if(long_press) {
                    previous_long_press = long_press;
                    is_dirty = true;
                }
            }
        }
        //Serial.println(apds.readGesture());
        apds.clearInterrupt();
        prev_time = this_time;
    }
}
