#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <TouchScreen.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "ict_board.h"
#include "multimeter_click.h"

#define VERSION_NUMBER      "V 0.1"

/* "after-glow" from https://gogh-co.github.io/Gogh/ */
#define bg                  0x2104  
#define color_0             0x4a69          // grey
#define color_1             0x10a2          // black
#define color_2             0xa1c4          
#define color_3             0x7488
#define color_4             0xcce9
#define color_5             0x6cb6
#define color_6             0x9a70
#define color_7             0x7e99
#define color_8             0xce79

/* LCD setup*/
#define LCD_CS              21              // Chip Select
#define LCD_CD              19              // Command/Data
#define LCD_WR              18              // LCD Write
#define LCD_RD              17              // LCD Read
#define LCD_RESET           16

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

/* touchscreen setup */
#define YP A1                               // must be an analog pin, use "An" notation!
#define XM A0                               // must be an analog pin, use "An" notation!
#define YM 14                               // can be a digital pin
#define XP 15                               // can be a digital pin

#define TS_MIN_X 920
#define TS_MAX_X 120

#define TS_MIN_Y 940
#define TS_MAX_Y 69

#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

/*  Fonts
    cursor should be set at baseline of characters
    characters start one pixel left of cursor
    (9-point) y_baseline=top row of pixel + 4
    bottom row of character is y_baseline + 9
    total height of a character is 14
    e.g. if baseline is set at (1,4): characters occupy rows 0~13 (no space left at top/bottom at all)

    for paragraphs with a single pixel break, lines should start at (1,5), and (1, 26), (1, 41), (1, 56), ... don't ask me why
*/

/* declare function with an optional parameter timeout */
// void touch(Adafruit_TFTLCD &tft, uint16_t x_0, uint16_t x_1, uint16_t y_0, uint16_t y_1, uint32_t timeout=0);
int touch(Adafruit_TFTLCD &tft, uint16_t x_0, uint16_t x_1, uint16_t y_0, uint16_t y_1, uint32_t timeout=0);

void remote_method_look_up(char* func_name);
int get_status(int dut_id);
int get_voltage(int dut_id);
int get_resistance(int dut_id);
int put_display(char* line);

struct remote_method_row_t {
    const char* name;
    int (*func)(int);
};

static struct remote_method_row_t remote_methods[] = {
    {"get status", get_status},
    {"get voltage", get_voltage},
    {"get resistance", get_resistance},
    {"put display", put_display}
};

struct DUT_t
{
    /* properties */
    uint8_t num;
    uint8_t power_relay;
    uint8_t J12_1;
    uint8_t J4_1;
    uint8_t led_green;
    uint8_t led_orange;
    uint8_t solenoid;
    uint8_t speaker_top;
    uint8_t speaker_bottom;

    /* test results*/
    int v_1;                                // voltage between J12.1 & J12.2 (5V)
    int v_2;                                // voltage between J12.1 & J4.1 (3.3V)
    int r_1;                                // resistance between J12.2 & J6.5(GND) (~200kOhm)
    uint8_t led_green_result;
    uint8_t led_orange_result;
};

struct DUT_t duts[] = {
    {1, 47, test_points[0], test_points[1], 37, 36, 43, A11, A10, 0, 0},
    {2, 46, test_points[2], test_points[3], 35, 34, 42, A9, A8, 0, 0},
    {3, 45, test_points[4], test_points[5], 33, 32, 41, A7, A6, 0, 0},
    {4, 44, test_points[6], test_points[7], 31, 30, 40, A5, A4, 0, 0}
};

SoftwareSerial debug_port(11, 10);          // (rx, tx)

void setup() {
    // struct DUT_t duts[] = {
    //     {1, 47, test_points[0], test_points[1], 37, 36, 43, A11, A10, 0, 0},
    //     {2, 46, test_points[2], test_points[3], 35, 34, 42, A9, A8, 0, 0},
    //     {3, 45, test_points[4], test_points[5], 33, 32, 41, A7, A6, 0, 0},
    //     {4, 44, test_points[6], test_points[7], 31, 30, 40, A5, A4, 0, 0}
    // };

    init_ict_board();
    init_system();
    multimeter_init();

    Serial.begin(115200);
    debug_port.begin(9600);

    set_sleep_mode(0);

    tft.reset();
    uint16_t identifier = tft.readID();
    tft.begin(identifier);
    tft.setFont(&FreeMonoBold9pt7b);

    /* start screen */
    tft.fillScreen(bg);

    tft.setCursor(60,50);
    tft.setTextSize(1);
    tft.setTextColor(color_7);
    tft.print("CONSERVATION LABS");

    tft.setCursor(40,80);
    tft.setTextSize(1);
    tft.setTextColor(color_7);
    tft.print("H2KNOW FUNCTIONAL TEST");

    tft.setCursor(140,150);
    tft.setTextSize(1);
    tft.setTextColor(color_7);
    tft.print(VERSION_NUMBER);

    tft.drawRoundRect(80, 261, 160, 40, 8, color_8);

    tft.setCursor(130,285);
    tft.setTextColor(color_8);
    tft.setTextSize(1);
    tft.print("START");

    tft.setCursor(40,350);
    tft.setTextSize(1);
    tft.setTextColor(color_7);
    tft.print("TOUCH START TO START");

    tft.setCursor(10,430);
    tft.setTextSize(1);
    tft.setTextColor(color_8);
    tft.print("MICROART SERVICES INC 2022");

    touch(tft, 80, 240, 261, 301, 0xffffffff);  // this function will return in 1193.046 hours!          
    tft.fillScreen(bg);
    tft.setCursor(0,0);
    // for (auto j=0;j<1000;j++) {
    //     tft.print("sudo apt");
    // }

    // for (auto i=0;i<100;i++) {
    //     RT68_ON
    //     delay(500);
    //     relay_call(test_points[0]);
    //     delay(500);
    //     int v1 = multimeter_read_voltage();
    //     float v1_f = v1 * 0.0083 - 17.0335;
    //     Serial.println(v1_f);
    //     delay(1000);
    //     relay_call(test_points[1]);
    //     delay(500);
    //     RT68_OFF
    //     delay(500);
    // }

    /* random tests */
    // Serial.println(Serial.available());
    // if (strstr("hello", "xe")) {
    //     Serial.println("found");
    // }
    // show_info(&DUT_1);

    char message[100] = {0};
    uint8_t i = 0;
    uint32_t t_0 = millis();

    for (;;) {
        while (Serial.available()) {
            char u = Serial.read();            
            if (u == '\r') {
                // Serial.print("message received: ");
                debug_port.println(message);
                /* remote procedure call */
                // might be a good idea to retain parse_message() to decode an entire message frame (func+parameter)
                // remote_method_look_up(message);
                memset(message, 0, sizeof(message)/sizeof(message[0]));
                i=0;
                break;
            }
            message[i++] = u;
        }
        sleep_mode();
    }
}

void loop() {
}

void init_system() {
    // rt67_on();                              // this shorts DUT_GND & CONTROLLER_GND, for serial communication only
}

// void touch(Adafruit_TFTLCD &tft, uint16_t x_0, uint16_t x_1, uint16_t y_0, uint16_t y_1, uint32_t timeout=0) {
//     Serial.println(timeout);
// }

int touch(Adafruit_TFTLCD &tft, uint16_t x_0, uint16_t x_1, uint16_t y_0, uint16_t y_1, uint32_t timeout=0) {
    uint32_t t_0 = millis();
    while (millis() - t_0 < timeout) {
        TSPoint p = ts.getPoint();
        pinMode(XM,OUTPUT);
        pinMode(YP,OUTPUT);
        if (p.z>MINPRESSURE && p.z<MAXPRESSURE) {
            p.x = map(p.x, TS_MIN_X, TS_MAX_X, tft.width(), 0);
            p.y = map(p.y, TS_MIN_Y, TS_MAX_Y, tft.height(), 0);
            if (p.x>x_0 && p.x<x_1 && p.y>y_0 && p.y <y_1) {
                break;
            }
        }
    }
    return 0;
}

void remote_method_look_up(char* func_name) {
    for (auto i=0;i<sizeof(remote_methods)/sizeof(remote_methods[0]);i++) {
        if (strcmp(remote_methods[i].name, func_name) == 0) {
            (remote_methods[i].func)(0);
            break;
        }
    }
}

int get_status(int dut_id) {
    // Serial.println("inside get_status()");
    // Serial.print("arg = ");
    // Serial.println(dut_id);
    Serial.print("OK\n");
}

int get_voltage(int dut_id) {
    /* this is for a function that takes a int as parameter, it works */
    relay_call(duts[dut_id].J12_1);
    delay(500);
    duts[dut_id].v_1 = multimeter_read_voltage();
    relay_call(duts[dut_id].J4_1);
    delay(500);
    duts[dut_id].v_2 = multimeter_read_voltage();
    Serial.write((byte*)&(duts[dut_id].v_1), 2);
    Serial.write((byte*)&(duts[dut_id].v_2), 2);

    /* code below is for a function where duts[i] is passed as pointer */
    // relay_call(dut -> J12_1);
    // delay(500);
    // dut -> v_1 = multimeter_read_voltage();
    // // dut->v_1 = 0x6162;                   // TEST

    // relay_call(dut -> J4_1);
    // delay(500);
    // dut -> v_2 = multimeter_read_voltage();
    // // dut->v_2 = 0x4142;                   // TEST

    // Serial.write((byte*)&(dut->v_1), 2);    // lower byte sent first
    // Serial.write((byte*)&(dut->v_2), 2);    // lower byte sent first
}

int get_resistance(int dut_id) {
    //
    Serial.println("inside get_resistance()");
    Serial.print("arg = ");
    Serial.println(dut_id);
}

int put_display(char* line) {
    while (Serial.available()) {

    }
}
/* pass by reference, its easier to modify variable inside structure */
// void show_info(struct DUT* dut) {
//     Serial.println(dut->num);

//     dut->led_green_result = 1;
//     Serial.println(dut->led_green_result);
// }

    /* console text*/
    // tft.setCursor(1,5);
    // tft.setTextColor(color_2);
    // tft.setTextSize(1);
    // tft.setFont(&FreeMonoBold9pt7b);
    // tft.print("[device] ");
    // tft.setTextColor(color_3);
    // tft.print("init\n");

    // tft.setTextColor(color_2);
    // tft.setTextSize(1);
    // tft.setFont(&FreeMonoBold9pt7b);
    // tft.print("[device] ");
    // tft.setTextColor(color_3);
    // tft.print("successful\n");

    // tft.setTextColor(color_2);
    // tft.setTextSize(1);
    // tft.setFont(&FreeMonoBold9pt7b);
    // tft.print("[host] ");
    // tft.setTextColor(color_3);
    // tft.print("ready\n");

    // tft.setTextColor(color_2);
    // tft.setTextSize(1);
    // tft.setFont(&FreeMonoBold9pt7b);
    // tft.print("[device] ");
    // tft.setTextColor(color_3);
    // tft.print("press anywhere to start\n");