#include "Adafruit_APDS9960.h"
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#include "hsv.h"

//the pin that the interrupt is attached to
#define WELCOME_STRING "Hello Anna!!"
#define PROXIMITY_INTERRUPT_PIN 12 // FOR THE RECEIVER
#define LED_RING_PIN 14 // FOR X RING CONTROL
#define NB_LEDS 12

enum Modes {
    MODE_WHITE,
    MODE_STILL,
    MODE_RAINBOW,
    MODE_COLOR,
    NB_MODES
};

#define PRESS_FORCE 15 // HIGHER == less sensitive
#define PRESS_SLOWDOWN 100000.0
#define MAX_PRESSTIME 360.0
#define DEFAULT_COLOR 0x080833

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

#define RGB2Color(rgb) strip.Color(rgb.b*255, rgb.r*255, rgb.g*255) // LED=BGR format

struct ProximityInputState {
    // key handling
    unsigned long keypress = 0; // is user currently interacting ? (running timestamp)

    int long_press = 0; // "magic formula" press-duration related value
    int previous_long_press = 0; // previous value, not reset on release

    unsigned long press_duration = 0; // current keypress duration, updated during press
    unsigned long last_duration = 0; // previous keypress event duration, updated on release

    // for double click detection
    unsigned long last_ts = 0; // timestamp of previous press event
} inp; // input

#define DBLCLICK_PREV_DURATION 300
#define DBLCLICK_PREV_INTERVAL 100

void INT_ProximityHandler(void); // see at the end of the file for input handling

// state machine
unsigned char loop_mode = 0; // current mode to display

// lamp configuration

struct LampSettings {
    double brightness;
    double hue;
    double saturation;
    bool is_dirty; // should the display be updated ? (for non animated modes)
} lamp_settings = {0.7, 0.0, 1.0, false}; // XXX: hue=[0-360.0], others=[0-1.0]

// breakboards objects
Adafruit_APDS9960 apds;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NB_LEDS, LED_RING_PIN, NEO_GRB + NEO_KHZ800);

#define paint(c) { \
    for(uint16_t i=0; i<NB_LEDS; i++) { \
        strip.setPixelColor(i, c); \
    } \
    strip.show(); \
};

#define CALC_HUE fmod(inp.press_duration/30.0, 360.0)
#define CALC_INT inp.long_press/MAX_PRESSTIME


void loop() {
    // set dt relative to keypress or to miliseconds counter instead
    double dt = (inp.keypress? \
        fmod((pow(inp.press_duration/10, 2)/1000.0), 360.0) : \
        fmod((millis()/(1000.0/((inp.last_duration/100.0)+1.0))), 360.0) );

    switch(loop_mode) {
        case MODE_WHITE: // white light (variable intensity)
            if (lamp_settings.is_dirty) paint(RGB2Color(hsv2rgb({0, 0, inp.long_press?CALC_INT:lamp_settings.brightness})));
            break;
        case MODE_STILL: // single color (variable color)
            if (lamp_settings.is_dirty) { paint( RGB2Color(hsv2rgb({
                            (double) inp.long_press?CALC_HUE:lamp_settings.hue,
                            lamp_settings.saturation, lamp_settings.brightness})));
            }
            break;
        case MODE_RAINBOW: // rotating rainbow (variable speed)
            uint16_t i;
            for(i=0; i< NB_LEDS; i++) {
                strip.setPixelColor(i,
                        RGB2Color(hsv2rgb({fmod(i*360.0 / NB_LEDS + dt, 360.0), lamp_settings.saturation, lamp_settings.brightness}))
                        );
            }
            strip.show();
            break;
        case MODE_COLOR: // ever changing color (variable speed)
            paint( RGB2Color(hsv2rgb({dt, lamp_settings.saturation, lamp_settings.brightness})));
            break;
    }

    if(!digitalRead(PROXIMITY_INTERRUPT_PIN)) {
        INT_ProximityHandler();
        lamp_settings.is_dirty = true; // if user interacted the display is probably dirty
    } else {
        lamp_settings.is_dirty = false; // reset value before testing
    }
}

void setup() {
    // init devices

    // 12 leds, pin 14
    strip.begin();
    paint(DEFAULT_COLOR);

    Serial.begin(115200);
    WiFi.mode(WIFI_OFF);

    pinMode(PROXIMITY_INTERRUPT_PIN, INPUT_PULLUP);

    if(!apds.begin()) Serial.println("failed to initialize device! Please check your wiring.");
    else Serial.println("Device initialized!");

    apds.enableGesture(false);
    apds.enableColor(false);
    apds.enableProximity(true);

    apds.setProximityInterruptThreshold(0, PRESS_FORCE/4);
    apds.enableProximityInterrupt();

    Serial.println(WELCOME_STRING);
}

void on_double_click(unsigned long this_time) {
    loop_mode = (loop_mode+1)%NB_MODES;

    inp.long_press = 0;
    inp.last_ts = 0;
    inp.last_duration = 0;
    inp.previous_long_press = 0;
}

void on_release(unsigned long this_time) {
    if (inp.long_press) { // store settings of current mode
        switch(loop_mode) {
            case MODE_WHITE:
                lamp_settings.brightness = MAX(0.004, CALC_INT);
                break;
            case MODE_STILL:
                lamp_settings.hue = CALC_HUE;
                break;
            default:
                break;
        }
    }
    inp.last_ts = this_time;
    inp.last_duration = this_time - inp.keypress;
    inp.keypress = 0;
    inp.long_press = 0;
}

void on_press(unsigned long this_time) {
    // long_press is an exponential value
    // used to set light level
    inp.long_press = MIN(MAX_PRESSTIME, (int)(pow((float)(this_time - inp.keypress), 2)/PRESS_SLOWDOWN));
    inp.press_duration = this_time - inp.keypress;
    if(inp.long_press) inp.previous_long_press = inp.long_press;
}

void on_idle(unsigned long ts) {
    // TODO reset some states in case some delay is over and RELEASE didn't happen
}

void INT_ProximityHandler(void) {
    unsigned long this_time = millis(); // current timestamp

    unsigned char cur = apds.readProximity(); // current proximity value
    unsigned char is_pressed = cur > PRESS_FORCE;
    unsigned char is_release = cur < PRESS_FORCE;

    if (inp.keypress == 0 && is_pressed) { // new entrance after a release
        inp.keypress = this_time;
        if (this_time - inp.last_ts < DBLCLICK_PREV_DURATION && inp.last_duration < DBLCLICK_PREV_INTERVAL) {
            on_double_click(this_time);
        }
    } else if (inp.keypress) {
        if (is_release) {
            on_release(this_time);
        } else {
            on_press(this_time);
        }
    } else {
        on_idle(this_time);
    }
    apds.clearInterrupt();
}
