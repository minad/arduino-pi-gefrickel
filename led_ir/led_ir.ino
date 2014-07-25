/*
 * LED controlled by infrared remote control
 * Daniel Mendler 2014 <mail@daniel-mendler.de>
 */

enum {
        IR_PIN = 6,
        RED_PIN = 10,
        GREEN_PIN = 8,
        BLUE_PIN = 9,
};

#define MS(ms) ((int)((ms*16000L)/1024L))

enum {
        MUSIC_OFFSET   = 1000,
        MUSIC_ON = MUSIC_OFFSET + 69,
        MUSIC_MODE = MUSIC_OFFSET + 70,
        MUSIC_MUTE = MUSIC_OFFSET + 71,
        MUSIC_PLAY = MUSIC_OFFSET + 68,
        MUSIC_BACK = MUSIC_OFFSET + 64,
        MUSIC_FORWARD = MUSIC_OFFSET + 67,
        MUSIC_EQ = MUSIC_OFFSET + 7,
        MUSIC_MINUS = MUSIC_OFFSET + 21,
        MUSIC_PLUS = MUSIC_OFFSET + 9,
        MUSIC_0 = MUSIC_OFFSET + 22,
        MUSIC_RANDOM = MUSIC_OFFSET + 25,
        MUSIC_USD = MUSIC_OFFSET + 13,
        MUSIC_1 = MUSIC_OFFSET + 12,
        MUSIC_2 = MUSIC_OFFSET + 24,
        MUSIC_3 = MUSIC_OFFSET + 94,
        MUSIC_4 = MUSIC_OFFSET + 8,
        MUSIC_5 = MUSIC_OFFSET + 28,
        MUSIC_6 = MUSIC_OFFSET + 90,
        MUSIC_7 = MUSIC_OFFSET + 66,
        MUSIC_8 = MUSIC_OFFSET + 82,
        MUSIC_9 = MUSIC_OFFSET + 74,
        LIGHT_OFFSET = 2000,
        LIGHT_BRIGHTER = LIGHT_OFFSET,
        LIGHT_DARKER,
        LIGHT_OFF,
        LIGHT_ON,
        LIGHT_RED,
        LIGHT_GREEN,
        LIGHT_BLUE,
        LIGHT_WHITE,
        LIGHT_RED1,
        LIGHT_GREEN1,
        LIGHT_BLUE1,
        LIGHT_FLASH,
        LIGHT_RED2,
        LIGHT_GREEN2,
        LIGHT_BLUE2,
        LIGHT_STROBE,
        LIGHT_RED3,
        LIGHT_GREEN3,
        LIGHT_BLUE3,
        LIGHT_FADE,
        LIGHT_RED4,
        LIGHT_GREEN4,
        LIGHT_BLUE4,
        LIGHT_SMOOTH,
};
volatile int command = 0;

const char* command2string(int c) {
        switch (c) {
        case MUSIC_ON: return "MUSIC_ON";
        case MUSIC_MODE: return "MUSIC_MODE";
        case MUSIC_MUTE: return "MUSIC_MUTE";
        case MUSIC_PLAY: return "MUSIC_PLAY";
        case MUSIC_BACK: return "MUSIC_BACK";
        case MUSIC_FORWARD: return "MUSIC_FORWARD";
        case MUSIC_EQ: return "MUSIC_EQ";
        case MUSIC_MINUS: return "MUSIC_MINUS";
        case MUSIC_PLUS: return "MUSIC_PLUS";
        case MUSIC_0: return "MUSIC_0";
        case MUSIC_RANDOM: return "MUSIC_RANDOM";
        case MUSIC_USD: return "MUSIC_USD";
        case MUSIC_1: return "MUSIC_1";
        case MUSIC_2: return "MUSIC_2";
        case MUSIC_3: return "MUSIC_3";
        case MUSIC_4: return "MUSIC_4";
        case MUSIC_5: return "MUSIC_5";
        case MUSIC_6: return "MUSIC_6";
        case MUSIC_7: return "MUSIC_7";
        case MUSIC_8: return "MUSIC_8";
        case MUSIC_9: return "MUSIC_9";
        case LIGHT_BRIGHTER: return "LIGHT_BRIGHTER";
        case LIGHT_DARKER: return "LIGHT_DARKER";
        case LIGHT_OFF: return "LIGHT_OFF";
        case LIGHT_ON: return "LIGHT_ON";
        case LIGHT_RED: return "LIGHT_RED";
        case LIGHT_GREEN: return "LIGHT_GREEN";
        case LIGHT_BLUE: return "LIGHT_BLUE";
        case LIGHT_WHITE: return "LIGHT_WHITE";
        case LIGHT_RED1: return "LIGHT_RED1";
        case LIGHT_GREEN1: return "LIGHT_GREEN1";
        case LIGHT_BLUE1:return "LIGHT_BLUE1";
        case LIGHT_FLASH: return "LIGHT_FLASH";
        case LIGHT_RED2: return "LIGHT_RED2";
        case LIGHT_GREEN2: return "LIGHT_GREEN2";
        case LIGHT_BLUE2: return "LIGHT_BLUE2";
        case LIGHT_STROBE: return "LIGHT_STROBE";
        case LIGHT_RED3: return "LIGHT_RED3";
        case LIGHT_GREEN3: return "LIGHT_GREEN3";
        case LIGHT_BLUE3: return "LIGHT_BLUE3";
        case LIGHT_FADE: return "LIGHT_FADE";
        case LIGHT_RED4: return "LIGHT_RED4";
        case LIGHT_GREEN4: return "LIGHT_GREEN4";
        case LIGHT_BLUE4: return "LIGHT_BLUE4";
        case LIGHT_SMOOTH: return "LIGHT_SMOOTH";
        default: return "UNKNOWN";
        }
}

void decode() {
        static volatile int bits = 0;
        static volatile unsigned char data[4];
        static volatile int last_command = 0;

        static volatile enum {
                IDLE,
                HEAD_HIGH,
                HEAD_LOW,
                DATA_LOW,
                DATA_HIGH,
                REPEAT
        } state = IDLE;

        switch (state) {
        case IDLE:
                TCNT1 = 0;
                state = HEAD_HIGH;
                break;
        case HEAD_HIGH:
                if (TCNT1 >= MS(8.9) && TCNT1 <= MS(9.1)) {
                        TCNT1 = 0;
                        state = HEAD_LOW;
                } else {
                        state = IDLE;
                }
                break;
        case HEAD_LOW:
                if (TCNT1 >= MS(4.4) && TCNT1 <= MS(4.6)) {
                        state = DATA_HIGH;
                        bits = 0;
                        data[0] = data[1] = data[2] = data[3] = 0;
                } else if (TCNT1 >= MS(2.15) && TCNT1 <= MS(2.35)) {
                        state = REPEAT;
                } else {
                        state = IDLE;
                }
                break;
        case REPEAT:
                state = IDLE;
                command = last_command;
                break;
        case DATA_LOW:
                data[bits >> 3] |= TCNT1 >= MS(1.2) ? (1 << (bits & 7)) : 0;
                ++bits;
                state = DATA_HIGH;
                break;
        case DATA_HIGH:
                if (bits == 32) {
                        state = IDLE;
                        if (data[2] == 0xFF ^ data[3]) {
                                if (data[0] == 0 && data[1] == 0xFF)
                                        command = MUSIC_OFFSET + data[2];
                                else if (data[0] == 0 && data[1] == 239)
                                        command = LIGHT_OFFSET + data[2];
                                else
                                        command = 0;
                                last_command = command;
                        }
                } else {
                        TCNT1 = 0;
                        state = DATA_LOW;
                }
                break;
        }
}

/*
 * hue, saturation, value/brightness
 */
void hsv2rgb(float h, float s, float v, float* rp, float* bp, float* gp) {
        float r, g, b;

        if (h < 0 || h >= 1)
                h = 0;
        if (s < 0)
                s = 0;
        else if (s > 1)
                s = 1;
        if (v < 0)
                v = 0;
        else if (v > 1)
                v = 1;

        float x = 3 * h;
        switch ((int)x) {
        case 0:
                r = 1 - x;
                g = x;
                b = 0;
                break;
        case 1:
                x -= 1;
                r = 0;
                g = 1 - x;
                b = x;
                break;
        case 2:
                x -= 2;
                r = x;
                g = 0;
                b = 1 - x;
                break;
        }

        *rp = v * (1 - s + s * r);
        *gp = v * (1 - s + s * g);
        *bp = v * (1 - s + s * b);
}

enum {
        MODE_STATIC,
        MODE_SMOOTH,
        MODE_FLASH,
        MODE_FADE
};

void setup() {
        Serial.begin(9600);
        pinMode(IR_PIN, INPUT);
        attachInterrupt(0, decode, CHANGE);

        pinMode(RED_PIN, OUTPUT);
        pinMode(GREEN_PIN, OUTPUT);
        pinMode(BLUE_PIN, OUTPUT);

        TCCR1A = 0;
        TCCR1B = 5;
        TCCR1C = 0;
        TCNT1 = 0;
        TIMSK1 = 0;
}

void loop() {
        static int mode = MODE_STATIC, strobe_on = 0, flash = 0;
        static float smooth_hue = 0, brightness = 1, strobe = 0, fade = 0,
                        static_r = 0, static_g = 0, static_b = 0,
                        fade1_r = 0, fade1_g = 0, fade1_b = 0,
                        fade2_r = 0, fade2_g = 0, fade2_b = 0;

        int cmd = command;
        command = 0;

        switch (cmd) {
        case LIGHT_RED:
                static_r = 1;
                static_g = static_b = 0;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN:
                static_r = static_b = 0;
                static_g = 1;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE:
                static_r = static_g = 0;
                static_b = 1;
                mode = MODE_STATIC;
                break;
        case LIGHT_SMOOTH:
                mode = MODE_SMOOTH;
                break;
        case LIGHT_OFF:
                static_r = static_g = static_b = 0;
                mode = MODE_STATIC;
                break;
        case LIGHT_BRIGHTER:
                brightness += 0.05;
                if (brightness > 1)
                        brightness = 1;
                break;
        case LIGHT_DARKER:
                brightness -= 0.05;
                if (brightness < 0.05)
                        brightness = 0.05;
                break;
        case LIGHT_STROBE:
                strobe_on = !strobe_on;
                break;
        case LIGHT_FLASH:
                mode = MODE_FLASH;
                break;
        case LIGHT_FADE:
                mode = MODE_FADE;
                hsv2rgb(random(1e6)/1e6, 1, 1, &fade1_r, &fade1_g, &fade1_b);
                hsv2rgb(random(1e6)/1e6, 1, 1, &fade2_r, &fade2_g, &fade2_b);
                break;
        case 0:
                break;
        default:
                static_r = static_g = static_b = 1;
                mode = MODE_STATIC;
                break;
        }

        float r, g, b;
        switch (mode) {
        case MODE_SMOOTH:
                hsv2rgb(smooth_hue, 1, 1, &r, &g, &b);
                smooth_hue += 1e-5;
                if (smooth_hue >= 1)
                        smooth_hue = 0;
                break;
        case MODE_FADE:
                fade += 1e-4;
                if (fade >= 1) {
                        fade1_r = fade2_r;
                        fade1_g = fade2_g;
                        fade1_b = fade2_b;
                        hsv2rgb(random(1e6)/1e6, 1, 1, &fade2_r, &fade2_g, &fade2_b);
                        fade = 0;
                }
                r = fade * fade2_r + (1 - fade) * fade1_r;
                g = fade * fade2_g + (1 - fade) * fade1_g;
                b = fade * fade2_b + (1 - fade) * fade1_b;
                break;
        case MODE_FLASH:
                if (flash % 2000 == 0)
                        hsv2rgb(random(1e6)/1e6, 1, 1, &static_r, &static_g, &static_b);
                ++flash;
        case MODE_STATIC:
                r = static_r;
                g = static_g;
                b = static_b;
                break;
        }

        if (strobe_on) {
                r *= strobe;
                g *= strobe;
                b *= strobe;
                strobe += 0.0008;
                if (strobe > 1)
                        strobe = 0;
        }

        r *= brightness;
        g *= brightness;
        b *= brightness;

        analogWrite(RED_PIN, 255 * r);
        analogWrite(GREEN_PIN, 255 * g);
        analogWrite(BLUE_PIN, 255 * b);

        if (cmd)
                Serial.println(command2string(cmd));
}
