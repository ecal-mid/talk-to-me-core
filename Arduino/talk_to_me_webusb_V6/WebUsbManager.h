#ifndef WEBUSBMANAGER_H
#define WEBUSBMANAGER_H

#include "Adafruit_TinyUSB.h"

// USB WebUSB object
extern Adafruit_USBD_WebUSB usb_web;

// Landing Page: scheme (0: http, 1: https), url
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "ecal-mid.ch/talktome/app.html");

// Function declarations
void initWebUsb();
void echo_all(char chr);
void line_state_callback(bool connected);

// Function implementations
inline void initWebUsb()
{
#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
    // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
    TinyUSB_Device_Init(0);
#endif

    usb_web.setLandingPage(&landingPage);
    usb_web.setLineStateCallback(line_state_callback);
    usb_web.begin();

    // wait until device mounted
    while (!TinyUSBDevice.mounted())
        delay(1);
}

inline void echo_all(char chr)
{
    Serial.write(chr);
    // print extra newline for Serial
    if (chr == '\r')
        Serial.write('\n');
}

inline void line_state_callback(bool connected)
{
    if (connected)
    {
        Serial.println("WebUSB connected");
        usb_web.println("MConnected!");
        usb_web.flush();
    }
    else
    {
        Serial.println("WebUSB disconnected");
        usb_web.println("MDisconnected!");
        usb_web.flush();
    }
}

#endif // WEBUSBMANAGER_H
