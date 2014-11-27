/*
 * LED controlled by infrared remote control
 * Daniel Mendler 2014 <mail@daniel-mendler.de>
 */

enum {
        IR_PIN = 6,
        RED_PIN = 9,
        GREEN_PIN = 10,
        BLUE_PIN = 8,
        LED_PIN = 13,
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
        TEUFEL_OFFSET = 3000,
        TEUFEL_MUTE = TEUFEL_OFFSET + 28,
        TEUFEL_CHANNEL = TEUFEL_OFFSET + 20,
        TEUFEL_MINUS = TEUFEL_OFFSET + 23,
        TEUFEL_PLUS = TEUFEL_OFFSET + 19,
};
volatile int command = 0;
volatile int repeated = 0;

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
        case TEUFEL_MUTE: return "TEUFEL_MUTE";
        case TEUFEL_CHANNEL: return "TEUFEL_CHANNEL";
        case TEUFEL_PLUS: return "TEUFEL_PLUS";
        case TEUFEL_MINUS: return "TEUFEL_MINUS";
        default: return "UNKNOWN";
        }
}

inline unsigned int timer1() {
        unsigned int t = TCNT1;
        TCNT1 = 0;
        return t;
}

inline unsigned int timer3() {
        unsigned int t = TCNT3;
        TCNT3 = 0;
        return t;
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

        unsigned int elapsed = timer1();

        switch (state) {
        case IDLE:
                state = HEAD_HIGH;
                break;
        case HEAD_HIGH:
                state = elapsed >= MS(8.8) && elapsed <= MS(9.2) ? HEAD_LOW : IDLE;
                break;
        case HEAD_LOW:
                if (elapsed >= MS(4.3) && elapsed <= MS(4.7)) {
                        state = DATA_HIGH;
                        bits = 0;
                        data[0] = data[1] = data[2] = data[3] = 0;
                } else if (elapsed >= MS(2.05) && elapsed <= MS(2.45)) {
                        state = REPEAT;
                } else {
                        state = IDLE;
                }
                break;
        case REPEAT:
                state = IDLE;
                command = last_command;
                repeated = 1;
                break;
        case DATA_LOW:
                data[bits >> 3] |= elapsed >= MS(1.2) ? (1 << (bits & 7)) : 0;
                ++bits;
                state = DATA_HIGH;
                break;
        case DATA_HIGH:
                if (bits == 32) {
                        state = IDLE;
                        if (data[2] == 0xFF ^ data[3]) {
                                if (data[0] == 0 && data[1] == 0xFF)
                                        command = MUSIC_OFFSET + data[2];
                                else if (data[0] == 128 && data[1] == 127)
                                        command = TEUFEL_OFFSET + data[2];
                                else if (data[0] == 0 && data[1] == 239)
                                        command = LIGHT_OFFSET + data[2];
                                else
                                        command = 0;
                                last_command = command;
                        }
                } else {
                        state = DATA_LOW;
                }
                break;
        }

        digitalWrite(LED_PIN, state != IDLE);
}

inline void clamp(float* x, float a, float b) {
        if (*x < a)
                *x = a;
        if (*x > b)
                *x = b;
}

/*
 * hue, saturation, value/brightness
 */
void hsv2rgb(float h, float s, float v, float* rp, float* bp, float* gp) {
        float r, g, b;

        while (h < 0)
                h += 1;
        while (h >= 1)
                h -= 1;
        clamp(&s, 0, 1);
        clamp(&v, 0, 1);

	float c = v * s;
	float x = c * (1 - fabs(fmod(6 * h, 2) - 1));
        switch ((int)(6 * h)) {
        case 0: r = c; g = x; b = 0; break;
        case 1: r = x; g = c; b = 0; break;
        case 2: r = 0; g = c; b = x; break;
        case 3: r = 0; g = x; b = c; break;
        case 4: r = x; g = 0; b = c; break;
        case 5: r = c; g = 0; b = x; break;
        }

        *rp = r + v - c;
        *gp = g + v - c;
        *bp = b + v - c;
}

void setup() {
        Serial.begin(9600);
        pinMode(IR_PIN, INPUT);
        attachInterrupt(0, decode, CHANGE);

        pinMode(RED_PIN, OUTPUT);
        pinMode(GREEN_PIN, OUTPUT);
        pinMode(BLUE_PIN, OUTPUT);
        pinMode(LED_PIN, OUTPUT);

        TCCR1A = 0;
        TCCR1B = 5;
        TCCR1C = 0;
        TCNT1 = 0;
        TIMSK1 = 0;

        TCCR3A = 0;
        TCCR3B = 5;
        TCCR3C = 0;
        TCNT3 = 0;
        TIMSK3 = 0;

        Serial.println("Hello!");
}

void loop() {
        enum {
                MODE_STATIC,
                MODE_SMOOTH,
                MODE_FLASH,
                MODE_FADE_ALL,
                MODE_FADE_COLOR,
        };

        const float SMOOTH_FREQ = 0.05,
                    FADE_FREQ = 0.1,
                    STROBE_FREQ = 3,
                    FLASH_FREQ = 4;

        static int mode = MODE_SMOOTH, strobe_on = 0;
        static float brightness = 1, time = 0, strobe = 0,
                     a_h = 0, a_s = 0, a_v = 0,
                     b_h = 0, b_s = 0, b_v = 0;

        int cmd = command, rep = repeated;
        command = repeated = 0;
        /*
        if (cmd) {
                Serial.print("IR command: ");
                Serial.print(command2string(cmd));
                Serial.println(rep ? " (repeated)" : "");
        }
        */
        float elapsed = timer3() * (1024 / 1.6e7);
        time += elapsed;

        switch (cmd) {
        case LIGHT_RED:
                a_h = a_s = a_v = 1;
                mode = MODE_STATIC;
                break;
        case LIGHT_RED1:
                a_h = -0.015;
                a_s = 1;
                a_v = 0.8;
                mode = MODE_STATIC;
                break;
        case LIGHT_RED2:
                a_h = -0.03;
                a_s = 1;
                a_v = 0.7;
                mode = MODE_STATIC;
                break;
        case LIGHT_RED3:
                a_h = -0.045;
                a_s = 1;
                a_v = 0.6;
                mode = MODE_STATIC;
                break;
        case LIGHT_RED4:
                a_h = -0.06;
                a_s = 1;
                a_v = 0.5;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN:
                a_h = 2./3;
                a_s = 1;
                a_v = 1;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN1:
                a_h = 2./3 - 0.025;
                a_s = 1;
                a_v = 0.6;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN2:
                a_h = 2./3 - 0.05;
                a_s = 1;
                a_v = 0.5;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN3:
                a_h = 2./3 - 0.075;
                a_s = 1;
                a_v = 0.4;
                mode = MODE_STATIC;
                break;
        case LIGHT_GREEN4:
                a_h = 2./3 - 0.1;
                a_s = 1;
                a_v = 0.3;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE:
                a_h = 1./3;
                a_s = 1;
                a_v = 1;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE1:
                a_h = 1./3 - 0.05;
                a_s = 1;
                a_v = 0.7;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE2:
                a_h = 1./3 - 0.1;
                a_s = 1;
                a_v = 0.6;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE3:
                a_h = 1./3 - 0.15;
                a_s = 1;
                a_v = 0.5;
                mode = MODE_STATIC;
                break;
        case LIGHT_BLUE4:
                a_h = 1./3 - 0.2;
                a_s = 1;
                a_v = 0.4;
                mode = MODE_STATIC;
                break;
        case LIGHT_ON:
        case LIGHT_SMOOTH:
                if (mode != MODE_SMOOTH) {
                        mode = MODE_SMOOTH;
                        time = 0;
                }
                break;
        case LIGHT_OFF:
                a_h = a_s = a_v = 0;
                mode = MODE_STATIC;
                break;
        case LIGHT_BRIGHTER:
                brightness *= 1.1;
                if (brightness > 1)
                        brightness = 1;
                break;
        case LIGHT_DARKER:
                brightness /= 1.1;
                if (brightness < 0.005)
                        brightness = 0.005;
                break;
        case LIGHT_STROBE:
                if (!rep)
                        strobe_on = !strobe_on;
                break;
        case LIGHT_FLASH:
                if (mode != MODE_FLASH) {
                        mode = MODE_FLASH;
                        a_h = random(1e6)/1e6;
                        a_s = a_v = 1;
                        time = 0;
                }
                break;
        case LIGHT_FADE:
                if (mode != MODE_FADE_COLOR && mode != MODE_FADE_ALL) {
                        if (mode == MODE_STATIC) {
                                mode = MODE_FADE_COLOR;
                                b_h = a_h + (random(1e6)/1e6 - 0.5) / 60;
                                b_s = a_s;
                                b_v = a_v;
                        } else {
                                mode = MODE_FADE_ALL;
                                a_h = random(1e6)/1e6;
                                b_h = random(1e6)/1e6;
                                a_s = a_v = b_s = b_v = 1;
                        }
                        time = 0;
                }
                break;
        case LIGHT_WHITE:
                a_h = a_s = 0;
                a_v = 1;
                mode = MODE_STATIC;
                break;
        }

        /*
        while (Serial.available() >= 6) {
                char data[6];
                for (int i = 0; i < sizeof (data); ++i)
                        data[i] = Serial.read();
                if (data[0] == 'r' && data[1] == 'g' && data[2] == 'b') {
                        a_r = data[3] / 255.0;
                        a_g = data[4] / 255.0;
                        a_b = data[5] / 255.0;
                        mode = MODE_STATIC;
                } else {
                        Serial.flush();
                }
                //Serial.println(a_r);
                //Serial.println(a_g);
                //Serial.println(a_b);
                //Serial.println("RECEIVED");
        }
        */

        float r, g, b;
        switch (mode) {
        case MODE_SMOOTH:
                hsv2rgb(time * SMOOTH_FREQ, 1, brightness, &r, &g, &b);
                if (time >= 1 / SMOOTH_FREQ)
                        time = 0;
                break;
        case MODE_FADE_ALL:
        case MODE_FADE_COLOR:
                {
                        float fade = time * FADE_FREQ;
                        if (fade >= 1) {
                                if (mode == MODE_FADE_ALL) {
                                        a_h = b_h;
                                        b_h = random(1e6)/1e6;
                                } else {
                                        float next = a_h + (random(1e6)/1e6 - 0.5) / 60;
                                        a_h = b_h;
                                        b_h = next;
                                }
                                time = 0;
                        }
                        hsv2rgb(fade * b_h + (1 - fade) * a_h,
                                fade * b_s + (1 - fade) * a_s,
                                brightness * (fade * b_v + (1 - fade) * a_v), &r, &g, &b);
                }
                break;
        case MODE_FLASH:
                if (time >= 1 / FLASH_FREQ) {
                        a_h = random(1e6)/1e6;
                        a_s = a_v = 1;
                        time = 0;
                }
        case MODE_STATIC:
                hsv2rgb(a_h, a_s, brightness * a_v, &r, &g, &b);
                break;
        }

        if (strobe_on) {
                strobe += elapsed * STROBE_FREQ;
                r *= strobe;
                g *= strobe;
                b *= strobe;
                if (strobe >= 1)
                        strobe = 0;
        }

        analogWrite(RED_PIN, 255 * r);
        analogWrite(GREEN_PIN, 255 * g);
        analogWrite(BLUE_PIN, 255 * b);
}
