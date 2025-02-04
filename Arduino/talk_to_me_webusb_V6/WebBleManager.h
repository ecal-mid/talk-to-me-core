#ifndef WEBBLEMANAGER_H
#define WEBBLEMANAGER_H

#include <ArduinoBLE.h>

// BLE Service and Characteristics
extern BLEService talkToMeService;
extern BLECharacteristic buttonCharacteristic;
extern BLECharacteristic ledCharacteristic;
extern BLECharacteristic servoCharacteristic;

// Function declarations
void initWebBle();
void echo_all_ble(char chr);
void line_state_callback_ble(bool connected);
void processBleLedCommand(const uint8_t* data, size_t len);
void processServoCommand(const uint8_t* data, size_t len);

// Function implementations
inline void initWebBle()
{
    if (!BLE.begin()) {
        Serial.println("Failed to initialize BLE!");
        while (1);
    }

    // Set advertised local name and service UUID
    BLE.setLocalName("TalkToMe");
    BLE.setAdvertisedService(talkToMeService);

    // Add characteristics to the service
    talkToMeService.addCharacteristic(buttonCharacteristic);
    talkToMeService.addCharacteristic(ledCharacteristic);
    talkToMeService.addCharacteristic(servoCharacteristic);

    // Add service
    BLE.addService(talkToMeService);

    // Set initial values
    buttonCharacteristic.writeValue((byte)0);
    ledCharacteristic.writeValue((byte)0);
    servoCharacteristic.writeValue((byte)90);

    // Start advertising
    BLE.advertise();
    Serial.println("BLE initialized and advertising!");
}

inline void echo_all_ble(char chr)
{
    Serial.write(chr);
    // print extra newline for Serial
    if (chr == '\r')
        Serial.write('\n');
}

inline void line_state_callback_ble(bool connected)
{
    if (connected)
    {
        Serial.println("BLE connected");
        // Send initial connection message
        const char* msg = "MConnected!";
        buttonCharacteristic.writeValue(msg, strlen(msg));
    }
    else
    {
        Serial.println("BLE disconnected");
        const char* msg = "MDisconnected!";
        buttonCharacteristic.writeValue(msg, strlen(msg));
    }
}

#endif // WEBBLEMANAGER_H
