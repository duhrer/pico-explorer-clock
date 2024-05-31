#include <math.h>
#include <string.h>
#include <time.h>

#include "pico_explorer.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"

#include "drivers/button/button.hpp"

using namespace pimoroni;

ST7789 st7789(PicoExplorer::WIDTH, PicoExplorer::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));

// 8-bit colour
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

// 16-bit colour
// PicoGraphics_PenRGB565 graphics(st7789.width, st7789.height, nullptr);

Pen WHITE = graphics.create_pen(255, 255, 255);
Pen BLACK = graphics.create_pen(0, 0, 0);
Pen RED = graphics.create_pen(255, 0, 0);

const int CX = round(st7789.width / 2);
const int CY = round(st7789.height / 2);

const int OUTER_RADIUS = std::min(CX, CY) - 10;
const int OUTER_LINEWIDTH = 5;
const int TICKMARK_LINELENGTH = 30;
const int TICKMARK_LINEWIDTH = 5;

const Point CENTER = Point(CY, CY);

const int SECONDS_IN_HOUR = (60 * 60);
const int SECONDS_IN_DAY = (24 * SECONDS_IN_HOUR);

const int HOUR_HAND_LENGTH = round(OUTER_RADIUS / 2);
const int MINUTE_HAND_LENGTH = OUTER_RADIUS - OUTER_LINEWIDTH;
const int SECOND_HAND_LENGTH = OUTER_RADIUS - OUTER_LINEWIDTH;

const double PI = std::acos(-1);

Button button_a(PicoExplorer::A);
Button button_b(PicoExplorer::B);
Button button_x(PicoExplorer::X);
Button button_y(PicoExplorer::Y);

int seconds = (12 * 60 * 60);
bool twelve_hour_time = false;

bool updateClock(struct repeating_timer *t) {
    seconds = (seconds + 1) % SECONDS_IN_DAY;
    return true;
}

void drawFromCentre(int angle, int length) {
    float radians = angle * PI / 180;
    int x = CX - round(std::cos(radians) * length);
    int y = CY - round(std::sin(radians) * length);
    Point dest = Point(x, y);
    graphics.line(CENTER, dest);
}

bool drawClock(struct repeating_timer *t) {
    graphics.set_pen(BLACK);
    graphics.clear();

    // Draw outer ring
    graphics.set_pen(WHITE);
    graphics.circle(CENTER, OUTER_RADIUS);
    graphics.set_pen(BLACK);
    graphics.circle(CENTER, OUTER_RADIUS - OUTER_LINEWIDTH);

    // Draw tick marks
    graphics.set_pen(WHITE);
    for (int angle = 0; angle < 360; angle +=30) {
        drawFromCentre(angle, OUTER_RADIUS);
    }

    // Erase inner circle
    graphics.set_pen(BLACK);
    graphics.circle(CENTER, OUTER_RADIUS - TICKMARK_LINELENGTH);

    // Draw the hands
    graphics.set_pen(WHITE);
    int hour = std::floor(seconds / SECONDS_IN_HOUR);
    if (twelve_hour_time) {
        hour = hour == 0 ? 12 : (hour % 12);
    }
    int hourAngle = (90 + ((hour % 12) * 30)) % 360;
    drawFromCentre(hourAngle, HOUR_HAND_LENGTH);

    int minute = std::floor((seconds % SECONDS_IN_HOUR)/60);
    int minuteAngle = (90 + (minute * 6)) % 360;
    drawFromCentre(minuteAngle, MINUTE_HAND_LENGTH);

    graphics.set_pen(RED);
    int secondAngle = (90 + (seconds * 6)) % 360;
    drawFromCentre(secondAngle, SECOND_HAND_LENGTH);

    // Draw time (two digits per)
    graphics.set_font("bitmap8");
    char time_string [8];
    sprintf(time_string, "%02d:%02d:%02d", hour, minute, seconds % 60);
    int text_width = graphics.measure_text(time_string, 4, 1, true);

    int text_x = CX - round(text_width / 2);
    int text_y = st7789.height - 32;
    Point textLocation = Point(text_x, text_y);
    Rect text_box(text_x, text_y, text_width, 32);

    graphics.set_pen(BLACK);
    graphics.rectangle(text_box);

    graphics.set_pen(WHITE);
    graphics.text(time_string, textLocation, text_box.w, 4.0, 0.0, 1.0, true);

    st7789.update(&graphics);

    return true;
}

bool keep_running = true;
bool left_combo_held = false;
bool right_combo_held = false;

bool checkButtons(struct repeating_timer *t) {
    // Repeatable Buttons
    bool button_x_state = button_x.read();
    bool button_y_state = button_y.read();
    bool button_a_state = button_a.read();
    bool button_b_state = button_b.read();

    if(button_y_state && !button_x_state) {
        seconds = (seconds - 60 + SECONDS_IN_DAY) % SECONDS_IN_DAY;
    }

    if(button_a_state && !button_b_state) {
        seconds = (seconds + SECONDS_IN_HOUR) % SECONDS_IN_DAY;
    }

    if(button_b_state && !button_a_state) {
        seconds = (seconds - SECONDS_IN_HOUR + SECONDS_IN_DAY) % SECONDS_IN_DAY;
    }

    if(button_x_state && !button_y_state) {
        seconds = (seconds + 60) % SECONDS_IN_DAY;
    }

    // Raw, non-repeatable "combos"
    bool left_combo_pressed = button_a.raw() && button_b.raw();
    bool right_combo_pressed = button_x.raw() && button_y.raw();

    // Don't need to track repeats here as we stop running the first time we see both combos at once.
    if(left_combo_pressed && right_combo_pressed) {
        keep_running = false;
    }

    if (right_combo_pressed) {
        if (!right_combo_held) {
            seconds = seconds - (seconds % 60);
            right_combo_held = true;
        }
    }
    else {
        right_combo_held = false;
    }
 
    if (left_combo_pressed) {
        if (!left_combo_held) {
            twelve_hour_time = !twelve_hour_time;
            left_combo_held = true;
        }
    }
    else {
        left_combo_held = false;
    }

    return true;
}

int main() {
    st7789.set_backlight(75);
    graphics.set_pen(BLACK);
    graphics.clear();
    st7789.update(&graphics);

    struct repeating_timer clock_timer;
    add_repeating_timer_ms(1000, updateClock, NULL, &clock_timer);

    struct repeating_timer draw_timer;
    add_repeating_timer_ms(100, drawClock, NULL, &draw_timer);

    struct repeating_timer button_timer;
    add_repeating_timer_ms(50, checkButtons, NULL, &button_timer);

    while (keep_running) {
         sleep_ms(10000);
    }

    cancel_repeating_timer(&clock_timer);
    cancel_repeating_timer(&draw_timer);
    cancel_repeating_timer(&button_timer);
}