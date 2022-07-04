/*
 * Copyright (c) 2014 by ELECTRONIC ASSEMBLY <technik@lcd-module.de>
 * EA DOG Graphic (ST7565R) software library for arduino.
 * 
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#ifndef DOG_DISPLAY_H
#define DOG_DISPLAY_H

#define DISPLAY_WIDTH_IN_PX 240   // The width of the display in pixel 
#define DISPLAY_HEIGHT_IN_BYTES 8 // The height of the display in Bytes (i.e. 8 px)

#define BATTERY_SYMBOL_WIDTH 10   // The width of the display in pixel 
#define INITLEN 14

class DogDisplay
{
  public:
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
	void initialize(uint8_t p_cs, uint8_t p_si, uint8_t p_clk, uint8_t p_a0, uint8_t p_res);
  
  /**
   * Clears the entire display.
   */
	void clear();

  /**
   * Draws a battery symbol displaying how full the battery is.
   * 
   * The battery symbol is 3*8 pixel high and BATTERY_SYMBOL_WIDTH pixel wide, the highest 2 y pixel are left blank.
   * The symbol is not clipped to display size, so do not use too high x and y values.
   * 
   * @param xInPixel (0 .. DISPLAY_WIDTH_IN_PX - BATTERY_SYMBOL_WIDTH - 1) : the start column of the battery symbol in pixel
   * @param yInBytes (0 .. DISPLAY_HEIGHT_IN_BYTES - 4) :  the start y of the battery symbol in units of bytes (8 pixel)
   * @param full (0 .. 255) how full the battery is. 0 if the battery empty (device will switch off any moment), 
   *         255 if the battery is completely full, or a value in between.
   */
  void drawBatterySymbol(uint16_t xInPixel, uint8_t yInBytes, uint8_t full);

  /**
   * Sets the position of the write cursor in the display.
   * 
   * @param xInPixel (0..MAX_X_IN_PIXEL - 1) : the x position of the write cursor.
   * @param yInBytes (0..MAX_Y_IN_BYTES - 1) : the y position of the write cursor.
   */
  void position(uint16_t xInPixel, uint8_t yInBytes);

  /**
   * Selects the DOG Display chip by setting the CS Pin to low.
   */
  void chipSelect();
  
  /**
   * Deselects the DOG Display chip by setting the CS Pin to high.
   */
  void chipDeselect();

  /**
   * Sets the DOG Display to data mode by setting the A0 Pin to high.
   */
  void dataMode();

  /**
   * Writes a byte to the SPI.
   * 
   * This is a plain SPI write, it does not perform a chip select and does not send command or data mode.
   * 
   * @param toSend the byte to send.
   */
  void sendToSpi(uint8_t toSend);

  private:
  uint8_t csPin;            // the pin connected to the display CS0 (Chip Select) Pin.
	uint8_t siPin;            // the pin connected to the display SDA/MOSI Pin.
	uint8_t clkPin;           // the pin connected to the display CSK/CLK Pin.
  uint8_t cdPin;            // the pin connected to the display CD Pin.
	boolean useHardwareSpi;   // true: use hardware SPI, false: use software SPI
  uint8_t init_sequence[INITLEN] = {0xF1, 0x3F, 0xF2, 0x00, 0xF3, 0x3F, 0x81, 0xB7, 0xC0, 0x02, 0xA3, 0xE9, 0xA9, 0xD1}; // the byte sequence used to initialize the display

	void initializeSpi(uint8_t csPin, uint8_t siPin, uint8_t clkPin);
	void sendToSpiWithChipSelect(uint8_t *toSend, uint16_t dataLength);
  void sendToSpiInDataMode(uint8_t *dataBytes, uint16_t dataLength);
  void commandMode();

  uint8_t fillBitsInByteStartingWithMsb(uint8_t bitsToFill);
};

#endif
