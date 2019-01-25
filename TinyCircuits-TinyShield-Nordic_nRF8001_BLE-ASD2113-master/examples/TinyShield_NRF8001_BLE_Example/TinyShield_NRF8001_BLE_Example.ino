/*
  TinyCircuits NRF8001 UART Example Sketch

  This demo sets up the NRF8001 for Nordic's BLE virtual UART connection, and
  can pass data between the Arduino serial monitor and Nordic nRF UART V2.0 app
  or another compatible BLE terminal. Now supports TinyScreen+/SAMD21 boards.

  Written 27 May 2016
  By Ben Rose
  Modified 07 January 2019
  By Hunter Hykes

  https://TinyCircuits.com
*/

//when using this project in the Arduino IDE, set the following to false and rename file UART.h to UART.ino
#define CODEBENDER false

#if defined(ARDUINO_ARCH_SAMD)
  #define SerialMonitorInterface SerialUSB
#else
  #define SerialMonitorInterface Serial
#endif

#define BLE_DEBUG true

#include <SPI.h>
#include "lib_aci.h"
#include "aci_setup.h"
#include "uart_over_ble.h"
#include "services.h"

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;

#if CODEBENDER
#include "UART.h"
#endif



/**************************************************************************************************************************************
  setup is a run-once function and that the void before hand denotes that the function will not be returning any values/variables
  A void is nothingness.  It indicates there is no value, no datatype, nothing.  I would guess that setup() is equivalent to setup(void) 
  as far as the code it generates, basically no values pushed onto the stack when the function is called.  Explicitly stating 'void' 
  is nice clarification/documentation of the fact there should be no value there.

  Written 25 Jan 2019
  By Henry Do
*/


void setup(void)
{
  SerialMonitorInterface.begin(9600);
  //while (!SerialMonitorInterface);//Optional- On TinyScreen+/SAMD21 boards this will block until the serial monitor is opened
  delay(100);
  BLEsetup();
}


/**************************************************************************************************************************************
  BOOKMARK: Goal - Find the variable representing values sent to  iOS device via BLE and replace with accelerometer values.

  A function with void in front is a void function and so will not return a value.  
  The void after means that the function will accept no arguments. Loop() and setup() are functions. 
  Loop() is where the code that runms ovewr and over goes (your program). Setup() is the function that 
  only runs once upon controller reset.  You put your setup code there (like serial.begin() or
  wire.begin()).

  Written 25 Jan 2019
  By Henry Do
*/

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  if (ble_rx_buffer_len) {//Check if data is available
    SerialMonitorInterface.print(ble_rx_buffer_len);
    SerialMonitorInterface.print(" : ");
    SerialMonitorInterface.println((char*)ble_rx_buffer);
    ble_rx_buffer_len = 0;//clear afer reading
  }
  if (SerialMonitorInterface.available()) {//Check if serial input is available to send
    delay(50);//should catch full input
    uint8_t sendBuffer[21];
    uint8_t sendLength = 0;
    while (SerialMonitorInterface.available() && sendLength < 19) {
      sendBuffer[sendLength] = SerialMonitorInterface.read();
      sendLength++;
    }
    if (SerialMonitorInterface.available()) {
      SerialMonitorInterface.print(F("Input truncated, dropped: "));
      while (SerialMonitorInterface.available()) {
        SerialMonitorInterface.write(SerialMonitorInterface.read());
      }
      SerialMonitorInterface.println();
    }
    sendBuffer[sendLength] = '\0'; //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
    {
      SerialMonitorInterface.println(F("TX dropped!"));
    }
  }

}
