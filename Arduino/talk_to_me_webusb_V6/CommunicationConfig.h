#ifndef COMMUNICATIONCONFIG_H
#define COMMUNICATIONCONFIG_H

// Choose communication method (uncomment only one)
#define USE_WEB_USB
//#define USE_WEB_BLE

// Verify only one is defined
#if defined(USE_WEB_USB) && defined(USE_WEB_BLE)
#error "Cannot use both WebUSB and WebBLE at the same time. Please choose one in CommunicationConfig.h"
#endif

#if !defined(USE_WEB_USB) && !defined(USE_WEB_BLE)
#error "Please choose either WebUSB or WebBLE in CommunicationConfig.h"
#endif

// Include appropriate headers based on configuration
#ifdef USE_WEB_USB
    #include "WebUsbManager.h"
    #define init_communication initWebUsb
    #define communication_echo echo_all
    #define communication_callback line_state_callback
#else
    #include "WebBleManager.h"
    #define init_communication initWebBle
    #define communication_echo echo_all_ble
    #define communication_callback line_state_callback_ble
#endif

#endif // COMMUNICATIONCONFIG_H
