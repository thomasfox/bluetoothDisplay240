/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include "DogDisplay.h"
#include "StringDisplay.h"
#include "gfxfont.h"
#include "FreeSansBold24pt7b.h"
#include "FieldConfiguration.h"

const uint8_t BLUETOOTH_BUFFER_SIZE = 20;

const GFXfont *gfxfont = &FreeSansBold24pt7b;

const uint8_t ktsBitmap[] PROGMEM = {
  0xE7, 0xE7, 0x76, 0x3E, 0x1E, 0x3E, 0xF6, 0xF6, 0x06, 0x07, 
  0x07, 0xE1, 0xF1, 0x30, 0x30, 0x30, 0x30, 0xF8, 0xF8, 0x30, 
  0x30, 0x30, 0x83, 0x87, 0x84, 0x00, 0x80, 0x80, 0x83, 0x03, 
  0x00, 0x00, 0x00, 0x1F, 0x7F, 0x61, 0x7E, 0x1F, 0x61, 0x7F, 
  0x7E, 0x00, 0x00, 0x00}; // created using gfxutils with font FreeMonoBold9pt7

const uint8_t degBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0xFC, 0xFE, 0xC7, 0x83, 0x83, 0xC7, 
  0xFE, 0xBC, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xE3, 
  0xF3, 0x19, 0xF9, 0xF9, 0x19, 0xF1, 0xE1, 0x01, 0x01, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x07, 0x8F, 0xC0, 0xCF, 0xCF, 0xCE, 
  0x87, 0x03, 0x00, 0x00, 0x00, 0x1F, 0x3F, 0x30, 0x30, 0x37, 
  0x3F, 0x39, 0x30, 0x30, 0x39, 0x7F, 0x7F, 0x00, 0x00, 0x00}; // created using gfxutils with font FreeMonoBold9pt7

const String VELOCITY_FIELD_PREFIX = "f1:";

const String DIRECTION_STRING_FIELD_PREFIX = "f2:";

const String DIRECTION_BAR_FIELD_PREFIX = "f3:";

const uint8_t FIELD_PREFIX_LENGTH = 3;

DogDisplay display;

SoftwareSerial bluetooth(5, 6); // RX, TX

char bluetoothReceiveBuffer[BLUETOOTH_BUFFER_SIZE];
 
uint8_t bluetoothReceiveBufferCursor = 0;

int16_t knots = 0;

StringFieldConfiguration velocityField;

StringFieldConfiguration directionStringField;

BarFieldConfiguration directionBarField;

void setup()
{
  bluetooth.begin(9600);
  display.initialize(10, MOSI, SCK, 9, 3);   //CS = 10, SI = MOSI ,CLK = SCK, CD = 9, RESET = 3

  velocityField.widthInPx = 107;
  velocityField.heightInBytes = 6;
  velocityField.gfxValueFont = gfxfont;
  velocityField.labelBitmap = &ktsBitmap[0];
  velocityField.labelBitmapWithInPx = 11;
  velocityField.labelBitmapHeightInBytes = 4;
  velocityField.display = display;
  
  directionStringField.xOffsetInPx = 130;
  directionStringField.widthInPx = 110;
  directionStringField.heightInBytes = 6;
  directionStringField.gfxValueFont = gfxfont;
  directionStringField.labelBitmap = &degBitmap[0];
  directionStringField.labelBitmapWithInPx = 15;
  directionStringField.labelBitmapHeightInBytes = 4;
  directionStringField.display = display;

  directionBarField.yOffsetInBytes = 6;
  directionBarField.widthInPx = 201;
  directionBarField.heightInBytes = 2;
  directionBarField.display = display;

  displayNoBluetoothConnection();
  
  velocityField.displayUnit();
  
  directionStringField.displayUnit();
  
  directionBarField.displayValue("0");
  
  bluetooth.write("AT+NAMELCDDISPLAY\r\n");
  delay(1000);
  receiveBluetooth();
  clearBluetoothReceiveBuffer();
}

//main loop
void loop()
{
  uint8_t batteryFull = batteryChargingState();
  display.drawBatterySymbol(230, 5, batteryFull);

  receiveBluetooth();
}

void receiveBluetooth()
{
  if (!bluetooth.available())
  {
    delay(100);
    return;
  }
  
  while (bluetooth.available())
  {
    if (bluetoothReceiveBufferCursor >= BLUETOOTH_BUFFER_SIZE - 1) 
    {
      clearBluetoothReceiveBuffer();
    }
    char readFromBluetooth = bluetooth.read();
    //bluetooth.write(readFromBluetooth);
    if ((readFromBluetooth < '0' || readFromBluetooth > '9')
        && (readFromBluetooth < 'a' || readFromBluetooth > 'z')
        && (readFromBluetooth < 'A' || readFromBluetooth > 'Z')
        && readFromBluetooth!='.' 
        && readFromBluetooth!='-' 
        && readFromBluetooth!=';'
        && readFromBluetooth!=':')
    {
      // ignore all unknown characters
      continue;
    }
    if (readFromBluetooth == ';')
    {
      readFromBluetooth='\0';
    }
    bluetoothReceiveBuffer[bluetoothReceiveBufferCursor]=readFromBluetooth;
    String toDisplay = bluetoothReceiveBuffer;
    if (readFromBluetooth == '\0')
    {
      toDisplay = bluetoothReceiveBuffer;
      determineFieldAndDisplayString(toDisplay);
      clearBluetoothReceiveBuffer();
    }
    else
    {
      bluetoothReceiveBufferCursor++;
    }  
  }
  //bluetooth.write(bluetoothReceiveBuffer);
}

void clearBluetoothReceiveBuffer() 
{
  bluetoothReceiveBufferCursor = 0;
  for (uint8_t i = 0; i < BLUETOOTH_BUFFER_SIZE; i++)
  {
    bluetoothReceiveBuffer[i] = '\0';
  }
}

/**
 * Displays a prefixed String.
 * 
 * The field to display the value in is determined by the prefix, which should be any of the constants
 * <code>VELOCITY_FIELD_PREFIX</code>, <code>DIRECTION_STRING_FIELD_PREFIX</code>. 
 * or <code>DIRECTION_BAR_FIELD_PREFIX</code>.
 * 
 * The displayed value is then the passed String with the prefix removed.
 * If no prefix is recognized, the passed String is displayed as is in the directionStringField.
 * 
 * @param prefixedString the string to be displayed, consisting of a prefix for field selection
 *        and the value to be displayed. 
 */
void determineFieldAndDisplayString(String prefixedString)
{
  // Serial.print("prefixedString: " + prefixedString + "\r\n");
  if (prefixedString.startsWith(VELOCITY_FIELD_PREFIX)) 
  {
    velocityField.displayValue(prefixedString.substring(FIELD_PREFIX_LENGTH));
  }
  else if (prefixedString.startsWith(DIRECTION_STRING_FIELD_PREFIX))
  {
    directionStringField.displayValue(prefixedString.substring(FIELD_PREFIX_LENGTH));
  }
  else if (prefixedString.startsWith(DIRECTION_BAR_FIELD_PREFIX))
  {
    directionBarField.displayValue(prefixedString.substring(FIELD_PREFIX_LENGTH));
  }
//  else 
//  {
//    directionStringField.displayValue(prefixedString);
//  }
}

/**
 * Displays to the user that there is currently no bluetooth connection.
 * 
 * For this, the velocity field and the direction String field are used to display "No Conn"
 */
void displayNoBluetoothConnection()
{
  velocityField.displayValue("No");
  directionStringField.displayValue("Con");
}

/**
 * Returns the charging state of the battery.
 * 
 * @return 0 if the battery empty (device will switch off any moment), 
 *         255 if the battery is completely full,
 *         or a value in between describing how full the battery is.
 */
uint8_t batteryChargingState()
{
  // Battery charging state is read from analog channel 0. This input reads half the battery voltage.
  // The reference Voltage of 3.3V is equivalent to a reading of 1024.
  int16_t batteryAnalogRead = analogRead(0);

  // Battery charging state calculation
  // Battery empty: 3.5V. AnalogRead is then 3.5V/3.3V/2*1024=543
  // Battery full: 4.1V. AnalogRead is then 3.5V/3.3V/2*1024=636. Difference to BatteryEmpty is 93. Factor is then 255/93
  int16_t batteryAnalogReadAboveEmpty = batteryAnalogRead - 543;
  if (batteryAnalogReadAboveEmpty < 0)
  {
     batteryAnalogReadAboveEmpty = 0;
  }
  if (batteryAnalogReadAboveEmpty > 93)
  {
     batteryAnalogReadAboveEmpty = 93;
  }
  return (uint8_t)(batteryAnalogReadAboveEmpty*255/93);
}
