/*********************************************************************
  TALK TO ME - ECAL 2025 - AB.
  Serial communication via Web USB or Web BLE
  Some heplers functions for Buttons, Potentimeter, Leds
  All the logic is done on the Web side via javascript

  Support:
  10  digital inputs (Buttons)
  1   analog input (potentimeter)
  10  Neopixels as output
  1 servo as output

**********************************************************************/

#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include "CommunicationConfig.h"

Servo myservo; // create servo object to control a servo
#define SERVO_PIN 22

#ifdef USE_WEB_USB
               // USB WebUSB object
Adafruit_USBD_WebUSB usb_web;
#else
               // BLE Service and Characteristics
BLEService talkToMeService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic buttonCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214",
                                       BLERead | BLEWrite | BLENotify, 20);
BLECharacteristic ledCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214",
                                    BLERead | BLEWrite, 20);
BLECharacteristic servoCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214",
                                      BLERead | BLEWrite, 20);
#endif

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
#define NUMPIXELS 80

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
  init_communication();

  for (int i = 0; i < nr_of_pins; i++)
  {
    if (btn_pins[i] != 0)
    {
      pinMode(btn_pins[i], INPUT_PULLUP);
    }
  }
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  myservo.attach(SERVO_PIN, 540, 2700); // add min and max to allow full 180 sweep

  Serial.begin(115200);
  delay(20);
  Serial.println("pico started");
  // Neopixels INIT
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  initAllLedsPixels();

#ifdef USE_WEB_BLE
  // Set up BLE characteristic callbacks
  ledCharacteristic.setEventHandler(BLEWritten, onLedCharacteristicWrite);
  servoCharacteristic.setEventHandler(BLEWritten, onServoCharacteristicWrite);
#endif
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

  int led_index = command.substring(1, 2).toInt();  // 0-9
  int color_code = command.substring(2, 4).toInt(); // 00 - 99
  int led_effect = command.substring(4, 5).toInt(); // 0-9

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

  int led_index = command.substring(1, 3).toInt(); // 0-99
  String color_code = command.substring(3, 9);     // Hex rgb value
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
    ledMustBlink[i] = ledMustPulse[i] = 0;
  }
  pixels.show();
}

void setServo(String command)
{
  int angle = command.substring(2, 5).toInt();
  Serial.println(angle);
  myservo.write(angle);
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

#ifdef USE_WEB_USB
  if (usb_web.available() > 4)
  {
    String command = "";
    if (usb_web.available() > 6)
    {
      // Led HEX message
      char inputHEX[9];
      usb_web.readBytes(inputHEX, 9);
      command = String(inputHEX);
    }
    else
    {
      char input[5];
      usb_web.readBytes(input, 5);
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
    if (command_code == "M")
    { // Motor (servo)
      setServo(command);
    }
  }
#else
  // Handle BLE central device
  BLEDevice central = BLE.central();
  if (central && central.connected())
  {
    // Process button states
    for (int i = 0; i < nr_of_pins; i++)
    {
      btn_states[i] = digitalRead(btn_pins[i]);
      if (last_btn_states[i] != btn_states[i])
      {
        char btnMsg[3];
        sprintf(btnMsg, "%c%d", btn_states[i] == 0 ? 'B' : 'H', i);
        buttonCharacteristic.writeValue(btnMsg, strlen(btnMsg));
        last_btn_states[i] = btn_states[i];
        delay(50);
      }
    }
  }
#endif

  // READ BUTTONS STATES
  for (int i = 0; i < nr_of_pins; i++)
  {
    btn_states[i] = digitalRead(btn_pins[i]);
    if (last_btn_states[i] != btn_states[i])
    {
#ifdef USE_WEB_USB
      if (btn_states[i] == 0)
      {
        usb_web.println("B" + String(i));
        usb_web.flush();
      }
      if (btn_states[i] == 1)
      {
        usb_web.println("H" + String(i));
        usb_web.flush();
      }
#endif
      last_btn_states[i] = btn_states[i];
      delay(50);
    }
  }
}

#ifdef USE_WEB_BLE
void onLedCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic)
{
  const uint8_t *data = characteristic.value();
  size_t len = characteristic.valueLength();
  String command((char *)data, len);

  if (command[0] == 'L')
  {
    setLedPixel(command);
  }
  else if (command[0] == 'H')
  {
    setLedPixelHEX(command);
  }
}

void onServoCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic)
{
  const uint8_t *data = characteristic.value();
  size_t len = characteristic.valueLength();
  String command((char *)data, len);

  if (command[0] == 'M')
  {
    setServo(command);
  }
}
#endif
