/*********************************************************************
  TALK TO ME - ECAL 2025 - AB.
  Serial communication via Web USB
  Some heplers functions for Buttons, Potentimeter, Leds
  All the logic is done on the Web side via javascript

  Support:
  10  digital inputs (Buttons)
  1   analog input (potentimeter)
  1-99  Neopixels as output

**********************************************************************/

#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>

// USB WebUSB object
Adafruit_USBD_WebUSB usb_web;

// Landing Page: scheme (0: http, 1: https), url
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "ecal-mid.ch/talktome/app.html");

unsigned long previousMillis;
unsigned long currentMillis;

// Buttons & Potetiemeter
int led_pin = LED_BUILTIN;
int nr_of_pins = 10;
int btn_pins[] = {15, 14, 13, 12, 11, 16, 17, 18, 19, 20};
int btn_states[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int last_btn_states[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int analog_pin = A0;
int analog_value = 0;
int analog_last_value = 0;

// Neo pixel

#define PIN 21
#define NUMPIXELS 99

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// color dictionary

uint32_t black = pixels.Color(0, 0, 0);       // 0
uint32_t white = pixels.Color(255, 255, 255); // 1
uint32_t red = pixels.Color(255, 0, 0);       // 2
uint32_t green = pixels.Color(0, 255, 0);     // 3
uint32_t blue = pixels.Color(0, 0, 255);      // 4
uint32_t magenta = pixels.Color(255, 0, 255); // 5
uint32_t yellow = pixels.Color(255, 210, 0);  // 6
uint32_t cyan = pixels.Color(0, 255, 255);    // 7
uint32_t orange = pixels.Color(255, 96, 3);   // 8
uint32_t purple = pixels.Color(128, 0, 255);  // 9
uint32_t pink = pixels.Color(255, 0, 128);    // 10

uint32_t colors[20] = {black, white, red, green, blue, magenta, yellow, cyan, orange, purple, pink};

uint32_t allLedsPixels[NUMPIXELS];
bool blink_state = 0;
bool ledMustBlink[NUMPIXELS];
bool ledMustPulse[NUMPIXELS];
bool ledMustPulseSpeak[NUMPIXELS];
unsigned long ledPreviousMillis;
int time_base = 200;
int sin_modifier;

// the setup function runs once when you press reset or power the board
void setup()
{
#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
  TinyUSB_Device_Init(0);
#endif

  for (int i = 0; i < nr_of_pins; i++)
  {
    if (btn_pins[i] != 0)
    {
      pinMode(btn_pins[i], INPUT_PULLUP);
    }
  }
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  usb_web.setLandingPage(&landingPage);
  usb_web.setLineStateCallback(line_state_callback);
  // usb_web.setStringDescriptor("TinyUSB WebUSB");
  usb_web.begin();

  // wait until device mounted
  while (!TinyUSBDevice.mounted())
    delay(1);
  Serial.begin(115200);
  delay(20);
  Serial.println("pico started");
  // Neopixels INIT
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  initAllLedsPixels();
}

// function to echo to both Serial and WebUSB
void echo_all(char chr)
{
  Serial.write(chr);
  // print extra newline for Serial
  if (chr == '\r')
    Serial.write('\n');

  // usb_web.write(chr);
}

void setLedPixel(String command)
{

  if (command.substring(1, 2).equals("x"))
  {
    // turn all off command Lx000
    initAllLedsPixels();
    Serial.println("all Leds Off");
    return;
  }

  int led_index = command.substring(1, 3).toInt();  // 00-99
  int color_code = command.substring(3, 5).toInt(); // 00 - 99
  int led_effect = command.substring(5, 6).toInt(); // 0-9

  if (led_index >= NUMPIXELS)
    return; // avoid offset

  pixels.setPixelColor(led_index, colors[color_code]);
  allLedsPixels[led_index] = colors[color_code];
  ledMustBlink[led_index] = ledMustPulse[led_index] = ledMustPulseSpeak[led_index] = 0;

  if (led_effect == 1)
    ledMustBlink[led_index] = 1;
  if (led_effect == 2)
    ledMustPulse[led_index] = 1;
  if (led_effect == 3)
    ledMustPulseSpeak[led_index] = 1;

  pixels.show();
}

// use for realtime setting of leds via hex rgb value
void setLedPixelHEX(String command)
{

  int led_index = command.substring(1, 3).toInt();   // 0-99
  String color_code = command.substring(3, 9);       // Hex rgb value
  int led_effect = command.substring(9, 10).toInt(); // 0-9

  if (led_index >= NUMPIXELS)
    return; // avoid offset
  // long number = (long)strtol(&color_code[1], NULL, 16);
  long number = hstol(color_code);
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;
  pixels.setPixelColor(led_index, pixels.Color(r, g, b));
  allLedsPixels[led_index] = pixels.Color(r, g, b);
  ledMustBlink[led_index] = ledMustPulse[led_index] = ledMustPulseSpeak[led_index] = 0;
  if (led_effect == 1)
    ledMustBlink[led_index] = 1;
  if (led_effect == 2)
    ledMustPulse[led_index] = 1;
  if (led_effect == 3)
    ledMustPulseSpeak[led_index] = 1;

  pixels.show();
}

long hstol(String recv)
{
  char c[recv.length() + 1];
  recv.toCharArray(c, recv.length() + 1);
  return strtol(c, NULL, 16);
}

void ledAnimationBlink()
{
  if (currentMillis - ledPreviousMillis > 500)
  {
    ledPreviousMillis = currentMillis;
    blink_state = !blink_state;
    for (int i = 0; i < NUMPIXELS; i++)
    {
      if (ledMustBlink[i] == 1)
      {
        if (blink_state == 0)
        {
          pixels.setPixelColor(i, colors[0]); // black
        }
        else
        {
          pixels.setPixelColor(i, allLedsPixels[i]); // saved set color
        }
      }
    }
    pixels.show();
  }
}

void ledAnimationPulseSpeak()
{
  float in, out;
  time_base = random(2, 5) * 100;
  int time_ref = (currentMillis % time_base) + sin_modifier;
  in = map(time_ref, 0, time_base, 0, TWO_PI * 100);
  out = sin(in / 100) * 127.5 + 127.5;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    if (ledMustPulseSpeak[i] == 1)
    {
      // map(value, fromLow, fromHigh, toLow, toHigh)
      uint8_t outR = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'r'));
      uint8_t outG = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'g'));
      uint8_t outB = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'b'));

      pixels.setPixelColor(i, pixels.Color(outR, outG, outB)); // saved set color
    }
  }
  pixels.show();
}

void ledAnimationPulse()
{
  float in, out;
  int time_ref = currentMillis % 2000;
  in = map(time_ref, 0, 2000, 0, TWO_PI * 100);
  out = sin(in / 100) * 127.5 + 127.5;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    if (ledMustPulse[i] == 1)
    {
      // map(value, fromLow, fromHigh, toLow, toHigh)
      uint8_t outR = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'r'));
      uint8_t outG = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'g'));
      uint8_t outB = map(out, 0, 255, 0, splitColor(allLedsPixels[i], 'b'));

      pixels.setPixelColor(i, pixels.Color(outR, outG, outB)); // saved set color
    }
  }
  pixels.show();
}

void initAllLedsPixels()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    allLedsPixels[i] = black; // pixels.Color(0, 0, 0);
    pixels.setPixelColor(i, allLedsPixels[i]);
    ledMustBlink[i] = ledMustPulse[i] = ledMustPulseSpeak[1] = 0;
  }
  pixels.show();
}


/**
   splitColor() - Receive a uint32_t value, and spread into bits.
*/
uint8_t splitColor(uint32_t c, char value)
{
  switch (value)
  {
  case 'r':
    return (uint8_t)(c >> 16);
  case 'g':
    return (uint8_t)(c >> 8);
  case 'b':
    return (uint8_t)(c >> 0);
  default:
    return 0;
  }
}

void loop()
{

  if (previousMillis + (time_base + sin_modifier) <= millis())
  { // check every x iterations
    sin_modifier = random(8) * 100;
    previousMillis = millis();
  }
  currentMillis = millis(); // too use millis outside of the loop
  ledAnimationBlink();
  ledAnimationPulse();
  ledAnimationPulseSpeak();

  if (usb_web.available() > 4)
  {
    String command = "";
    if (usb_web.available() > 7)
    {
      // Led HEX message
      char inputHEX[10];
      usb_web.readBytes(inputHEX, 10);
      command = String(inputHEX);
    }
    else
    {
      char input[5];
      usb_web.readBytes(input, 6);
      command = String(input);
      // Serial.println(command);
    }
    String command_code = command.substring(0, 1);
    if (command_code == "L")
    { // LED
      setLedPixel(command);
    }
    if (command_code == "H")
    { // LED HEX
      setLedPixelHEX(command);
    }
    /*if (command_code == "M")
    { // Motor (servo)
      setServo(command);
    }*/
  }

  // // READ LDR value
  // if (previousMillis + 200 <= millis()) {  // check every x iterations
  //   previousMillis = millis();
  //   // READ ANALOG VALUE
  //   analog_value = analogRead(analog_pin);
  //   Serial.println(analog_value);
  //   analog_value = map(analog_value, 350, 730, 0, 1);

  //   if (analog_last_value != analog_value) {
  //     analog_last_value = analog_value;
  //     Serial.println(analog_value);
  //     usb_web.println("A" + String(analog_value));
  //   }
  // }

  // READ BUTTONS STATES
  for (int i = 0; i < nr_of_pins; i++)
  {
    btn_states[i] = digitalRead(btn_pins[i]);
    if (last_btn_states[i] != btn_states[i])
    {
      if (btn_states[i] == 0)
      {
        usb_web.println("B" + String(i));
        usb_web.flush(); // added 2023 otherwise it waits fur large buffer to send...
      }
      if (btn_states[i] == 1)
      {
        usb_web.println("H" + String(i));
        usb_web.flush();
      }
      last_btn_states[i] = btn_states[i];
      delay(50);
    }
  }
}

void line_state_callback(bool connected)
{
  digitalWrite(led_pin, connected);
  initAllLedsPixels();
  if (connected)
  {
    usb_web.println("MConnected!");
    usb_web.flush();
  }
}
