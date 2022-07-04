/*
 * Copyright (c) 2014 by ELECTRONIC ASSEMBLY <technik@lcd-module.de>
 * EA DOG Graphic (ST7565R) software library for arduino.
 * 
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <Arduino.h>
#include <SPI.h>
#include <avr/pgmspace.h>

#include "DogDisplay.h"

/**
 * Initializes the SPI Hardware/Software and the DOG Display.
 * 
 * Performs a reset to the Display and sets the pin numbers of the controller pins which are connected to the display.
 * 
 * @param csPin the pin connected to the display CS0 (Chip Select) Pin.
 * @param siPin the pin connected to the display SDA/MOSI Pin.
 * @param clkPin the pin connected to the display CSK/CLK Pin.
 * @param cdPin the pin connected to the display CD Pin.
 * @param resetPin the number of the pin conected to the display RST pin.
 */
void DogDisplay::initialize(uint8_t csPin, uint8_t siPin, uint8_t clkPin, uint8_t cdPin, uint8_t resetPin) 
{
	DogDisplay::cdPin = cdPin;
	pinMode(cdPin, OUTPUT);
	initializeSpi(csPin, siPin, clkPin);

	// perform a Reset
	digitalWrite(resetPin, LOW);
	pinMode(resetPin, OUTPUT);
	delay(1);
	digitalWrite(resetPin, HIGH);
	delay(200);

  // init display
	commandMode();
	sendToSpiWithChipSelect(init_sequence, INITLEN);

	clear();
}

/**
 * Clears the entire display.
 */
void DogDisplay::clear() 
{
	for (uint8_t yInBytes = 0; yInBytes < DISPLAY_HEIGHT_IN_BYTES; yInBytes++)
	{
		position (0, yInBytes);
		chipSelect();
		dataMode();
		
		for (uint16_t x = 0; x < DISPLAY_WIDTH_IN_PX; x++) //clear the whole line
		{
      sendToSpi(0x00);
		}
		
		chipDeselect();
	}
}

/**
 * Draws a battery symbol displaying how full the battery is.
 * 
 * The battery symbol is 3*8 pixel high and BATTERY_SYMBOL_WIDTH pixel wide, the highest 2 y pixel are left blank.
 * The symbol is not clipped to display size.
 * 
 * @param xInPixel (0 .. DISPLAY_WIDTH_IN_PX - BATTERY_SYMBOL_WIDTH - 1) : the start column of the battery symbol in pixel
 * @param yInBytes (0 .. DISPLAY_HEIGHT_IN_BYTES - 4) :  the start y of the battery symbol in units of bytes (8 pixel)
 * @param full (0 .. 255) how full the battery is. 0 if the battery empty (device will switch off any moment), 
 *         255 if the battery is completely full, or a value in between.
 */
void DogDisplay::drawBatterySymbol(uint16_t xInPixel, uint8_t yInBytes, uint8_t full)
{
  uint8_t filledRectangleHeight = full / 16; // how high the filled rectangle inside the battery is (in px)

  // First line: 6 MSB: battery symbol head, 2 LSB: Data MSB
  position(xInPixel, yInBytes);
  uint8_t line1[] = {0xE0,0x20,0x39,0x3C,0x3C,0x3C,0x3C,0x39,0x20,0xE0};
  uint8_t pixelData;
  if (filledRectangleHeight >= 14)
  {
    pixelData = fillBitsInByteStartingWithMsb(filledRectangleHeight - 14);
    for (uint8_t i = 2; i < BATTERY_SYMBOL_WIDTH - 2; i++)
    {
      line1[i] |= pixelData;
    }
  }
  sendToSpiInDataMode(line1, BATTERY_SYMBOL_WIDTH);

  // Second line: Data middle bytes
  position(xInPixel, yInBytes + 1);
  uint8_t line2[] = {0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};
  if (filledRectangleHeight >= 6)
  {
    pixelData = fillBitsInByteStartingWithMsb(filledRectangleHeight - 6);
    for (uint8_t i = 2; i < BATTERY_SYMBOL_WIDTH - 2; i++)
    {
      line2[i] = pixelData;
    }
  }
  sendToSpiInDataMode(line2, BATTERY_SYMBOL_WIDTH);

  // Third line: 6 MSB: Data LSB, 2 LSB: Battery bottom
  position(xInPixel, yInBytes + 2);
  uint8_t line3[] = {0xFF,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xFF};
  pixelData = fillBitsInByteStartingWithMsb(filledRectangleHeight) >> 2;
  for (uint8_t i = 2; i < BATTERY_SYMBOL_WIDTH - 2; i++)
  {
    line3[i] |= pixelData;
  }
  sendToSpiInDataMode(line3, BATTERY_SYMBOL_WIDTH);
}

/**
 * Sets the position of the write cursor in the display.
 * 
 * @param xInPixel (0..MAX_X_IN_PIXEL - 1) : the x position of the write cursor.
 * @param yInBytes (0..MAX_Y_IN_BYTES - 1) : the y position of the write cursor.
 */
void DogDisplay::position(uint16_t xInPixel, uint8_t yInBytes)  
{
  commandMode();
  chipSelect();
	sendToSpi(0x10 + (xInPixel >> 4));   //MSB adress column
	sendToSpi(0x00 + (xInPixel & 0x0F)); //LSB adress column
  sendToSpi(0x70);                     //MSB adress page 
	sendToSpi(0x60 + (yInBytes & 0x0F)); //LSB adress page	
  chipDeselect();
}

/**
 * Selects the DOG Display chip by setting the CS Pin to low.
 */
void DogDisplay::chipSelect()
{
  digitalWrite(csPin, LOW);
}

/**
 * Deselects the DOG Display chip by setting the CS Pin to high.
 */
void DogDisplay::chipDeselect()
{
  digitalWrite(csPin, HIGH);
}

/**
 * Sets the DOG Display to command mode by setting the A0 Pin to low.
 */
void DogDisplay::commandMode()
{
  digitalWrite(cdPin, LOW);
}

/**
 * Sets the DOG Display to data mode by setting the A0 Pin to high.
 */
void DogDisplay::dataMode()
{
  digitalWrite(cdPin, HIGH);
}

/**
 * Writes a byte to the SPI.
 * 
 * @param toSend the byte to send.
 */
void DogDisplay::sendToSpi(uint8_t toSend) 
{
  if (useHardwareSpi) 
  {
    SPI.transfer(toSend);
  }
  else 
  {
    uint8_t i = 8;
    do 
    {
      if (toSend & 0x80) 
      {
        digitalWrite(siPin, HIGH);
      }
      else
      {
        digitalWrite(siPin, LOW);
      }
      digitalWrite(clkPin, LOW);
      toSend <<= 1;
      digitalWrite(clkPin, HIGH);
    } while (--i);
  }
}

/**
 * Sends data bytes to the display.
 * 
 * @param dataBytes the data bytes to send.
 * @param dataLength the numbver of data bytes to send.
 */
void DogDisplay::sendToSpiInDataMode(uint8_t *dataBytes, uint16_t dataLength) 
{
   dataMode();
   sendToSpiWithChipSelect(dataBytes, dataLength);
}

/** 
 * Initializes the SPI Hardware and Software.
 * 
 * @param csPin the number of the CS Pin
 * @param siPin the number of the MOSI Pin( == MOSI to use the hardware SPI.
 * @param clkPin the number of the SCK Pin, or == SCK to use the hardware SPI.
 */
void DogDisplay::initializeSpi(uint8_t csPin, uint8_t siPin, uint8_t clkPin) 
{
	DogDisplay::csPin = csPin;
  DogDisplay::siPin = siPin;
  DogDisplay::clkPin = clkPin;
	
	if (siPin == MOSI && clkPin == SCK)
	{
		useHardwareSpi = true;
	}
	else 
	{
		useHardwareSpi = false;
	}
	
	chipDeselect();
	pinMode(csPin, OUTPUT);
	pinMode(siPin, OUTPUT);
	
	// Set SPI-Mode 3: CLK idle high, rising edge, MSB first
	digitalWrite(clkPin, HIGH);
	pinMode(clkPin, OUTPUT);
	if (useHardwareSpi)
	{	
		SPI.begin();
		SPI.setBitOrder(MSBFIRST);
		SPI.setDataMode(SPI_MODE3);
		SPI.setClockDivider(SPI_CLOCK_DIV4);
	}
}

/**
 * Writes a byte array to the SPI, and performs a chipSelect/chipDeselect before and after.
 * 
 * @param toSend the bytes to send.
 * @üparam dataLength the number of bytes to send.
 */
void DogDisplay::sendToSpiWithChipSelect(uint8_t *toSend, uint16_t dataLength) 
{
	chipSelect();
	do
	{
		sendToSpi(*toSend++);
	}	while(--dataLength);
	chipDeselect();
}

/**
 * Fills bits (i.e sets them to one) in a byte starting with the most significant bit (MSB) (0x80).
 * 
 * The bits not filled are zeroed.
 * 
 * @param bitsToFill (0..255) : how many bits should be set to 1. If the value is >=7, all bits are set to one.
 */
uint8_t DogDisplay::fillBitsInByteStartingWithMsb(uint8_t bitsToFill)
{
  uint8_t result = 0x00;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (bitsToFill > i)
    {
      result |= 0x80 >> i;
    }
  }
  return result;
}
