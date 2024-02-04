#pragma once

// General
//#define ENABLE_UART
#define VERBOSE_OUTPUT

// Board
#define KCN_RP2040_BASE
//#define KCN_RP2040_REV1 // TEMPORARY. Final board is Rev2, this is just to account for minor pin changes between the two

// DO NOT EDIT BELOW THIS LINE

// Board-specific options
#ifdef KCN_RP2040_BASE

#define DEVICE_VENDOR "VE3KCN"
#ifndef KCN_RP2040_REV1
#define DEVICE_NAME "CATS Base Transceiver R2"

#else
#define DEVICE_NAME "CATS Base Transceiver R1"
#endif
#define USE_RP2040
#define USE_SPI

// Indicators
#ifdef KCN_RP2040_REV1
#define USB_LED_PIN 25 // TEMPORARY. FINAL BOARD IS 8!

#else

#define USB_LED_PIN 8
#endif
#define TX_LED_PIN 9
#define RX_LED_PIN 10

// Transceiver
#define USE_RF4463
// TEMPORARY. REMOVE THIS WHEN FINAL BOARD IS READY
#ifdef KCN_RP2040_REV1
#define RADIO_SCK_PIN 6
#define RADIO_TX_PIN 3
#define RADIO_RX_PIN 4
#define RADIO_CS_PIN 5
#define RADIO_IRQ_PIN 8
#define RADIO_SDN_PIN 7

#else // Rev 2 and beyond*

#define RADIO_SCK_PIN 18
#define RADIO_TX_PIN 19
#define RADIO_RX_PIN 20
#define RADIO_CS_PIN 21
#define RADIO_IRQ_PIN 13
#define RADIO_SDN_PIN 12
#define RADIO_GP0_PIN 16
#define RADIO_GP1_PIN 17

#endif
//#define RADIO_SCK_PIN 18
//#define RADIO_TX_PIN 19
//#define RADIO_RX_PIN 20
//#define RADIO_CS_PIN 21
//#define RADIO_IRQ_PIN 14
//#define RADIO_SDN_PIN 13

// GPIO Pins
// This will only be used if the finished firmware has scripting support,
// Otherwise there's no point, just edit the firwmare directly in that case.
#define NUM_GPIO_PINS 10

// Header 1
#define GPIO_0_PIN 3
#define GPIO_1_PIN 4
#define GPIO_2_PIN 5
#define GPIO_3_PIN 6
#define GPIO_4_PIN 7
// Header 2
#define GPIO_5_PIN 25
#define GPIO_6_PIN 26
#define GPIO_7_PIN 27
#define GPIO_8_PIN 28
#define GPIO_9_PIN 29

#endif