#include "sdl.h"

#include <Arduino_GFX_Library.h>

#include "SPI.h"

#define RED 0xF800

#define _cs 5
#define _dc 16
#define _mosi 23
#define _sclk 18
#define _rst 4
#define _miso -1  // Not connected
#define _led 21

#define _left GPIO_NUM_26
#define _right GPIO_NUM_25
#define _up GPIO_NUM_15
#define _down GPIO_NUM_27
#define _select GPIO_NUM_14
#define _start GPIO_NUM_32
#define _a GPIO_NUM_33
#define _b GPIO_NUM_13

Arduino_DataBus *bus = new Arduino_ESP32SPI(_dc, _cs, _sclk, _mosi, _miso);
Arduino_GFX *tft = new Arduino_ST7735(bus, _rst, 3 /* rotation */, false /* IPS */, 128, 160, 0, 0, 0, 0, true /* bgr */);


void backlighting(bool state) {
  if (!state) {
    digitalWrite(_led, LOW);
  } else {
    digitalWrite(_led, HIGH);
  }
}

#define GAMEBOY_HEIGHT 144
#define GAMEBOY_WIDTH 160
#define DRAW_HEIGHT 144
#define DRAW_WIDTH 160
#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 160

#define SPI_FREQ 27000000

static uint8_t *frame_buffer;

static int button_start, button_select, button_a, button_b, button_down,
    button_up, button_left, button_right;

static volatile bool frame_ready = false;
TaskHandle_t draw_task_handle;



void draw_task(void *parameter) {
  uint16_t color_palette[] = {0xffff, (16 << 11) + (32 << 5) + 16,
                              (8 << 11) + (16 << 5) + 8, 0x0000};

  int h_offset = (SCREEN_WIDTH - DRAW_WIDTH) / 2;
  int skip_lines = (DRAW_HEIGHT - SCREEN_HEIGHT) / 2; // 8 líneas
  
  while (true) {
    while (!frame_ready) {
      delay(1);
    }
    frame_ready = false;
    
    // Pasamos el puntero del framebuffer adelantado para omitir las líneas superiores y evitar dibujar fuera de límites
    tft->drawIndexedBitmap(h_offset, 0, frame_buffer + skip_lines * DRAW_WIDTH, color_palette,
                           DRAW_WIDTH, SCREEN_HEIGHT);
  }
}

void sdl_init(void) {
  frame_buffer = new uint8_t[DRAW_WIDTH * DRAW_HEIGHT];
  // GFX_EXTRA_PRE_INIT();
  tft->begin(SPI_FREQ);
  pinMode(_led, OUTPUT);
  backlighting(true);
  tft->fillScreen(RED);

  gpio_num_t gpios[] = {_left, _right, _down, _up, _start, _select, _a, _b};
  for (gpio_num_t pin : gpios) {
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    // uncomment to use builtin pullup resistors
    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
  }
}

void sdl_start_draw_task(void) {
  xTaskCreatePinnedToCore(draw_task,  /* Function to implement the task */
                          "drawTask", /* Name of the task */
                          10000,      /* Stack size in words */
                          NULL,       /* Task input parameter */
                          0,          /* Priority of the task */
                          &draw_task_handle, /* Task handle. */
                          0); /* Core where the task should run */
}

// button_id mapping: 0=Up, 1=Down, 2=Left, 3=Right, 4=A, 5=B
bool sdl_read_button(int button_id) {
  switch(button_id) {
    case 0: return !gpio_get_level(_up);
    case 1: return !gpio_get_level(_down);
    case 2: return !gpio_get_level(_left);
    case 3: return !gpio_get_level(_right);
    case 4: return !gpio_get_level(_a);
    case 5: return !gpio_get_level(_b);
  }
  return false;
}

int sdl_update(void) {
  button_up = !gpio_get_level(_up);
  button_left = !gpio_get_level(_left);
  button_down = !gpio_get_level(_down);
  button_right = !gpio_get_level(_right);

  button_a = !gpio_get_level(_a);
  button_b = !gpio_get_level(_b);

  static uint32_t rotation_hold_start = 0;
  static bool rotation_triggered = false;
  static uint8_t current_rotation = 3;

  if (current_rotation == 3) {
    int temp = button_a; button_a = button_b; button_b = temp;
    temp = button_left; button_left = button_right; button_right = temp;
    temp = button_up; button_up = button_down; button_down = temp;
  }

  // Rotar pantalla si se mantienen LEFT+RIGHT+A+B durante 3 segundos
  if (button_left && button_right && button_a && button_b) {
    if (rotation_hold_start == 0) {
      rotation_hold_start = millis();
    } else if (!rotation_triggered && (millis() - rotation_hold_start >= 3000)) {
      rotation_triggered = true;
      current_rotation = (current_rotation == 3) ? 1 : 3;
      tft->setRotation(current_rotation);
    }
    // Evitar que estos botones hagan cosas raras en el emulador mientras rotamos
    button_up = button_down = button_left = button_right = button_a = button_b = false;
    button_start = false;
    button_select = false;
  } else {
    rotation_hold_start = 0;
    rotation_triggered = false;

    // Botones virtuales: Start (A + B + UP) y Select (A + B + DOWN)
    if (button_a && button_b && button_up) {
      button_start = true;
      button_select = !gpio_get_level(_select);
      button_up = false;
      button_a = false;
      button_b = false;
    } else if (button_a && button_b && button_down) {
      button_start = !gpio_get_level(_start);
      button_select = true;
      button_down = false;
      button_a = false;
      button_b = false;
    } else {
      button_start = !gpio_get_level(_start);
      button_select = !gpio_get_level(_select);
    }
  }

  sdl_frame();
  return 0;
}

unsigned int sdl_get_buttons(void) {
  unsigned int buttons =
      (button_start * 8) | (button_select * 4) | (button_b * 2) | button_a;
  return buttons;
}

unsigned int sdl_get_directions(void) {
  return (button_down * 8) | (button_up * 4) | (button_left * 2) | button_right;
}

uint8_t *sdl_get_framebuffer(void) { return frame_buffer; }

void sdl_frame(void) { frame_ready = true; }
