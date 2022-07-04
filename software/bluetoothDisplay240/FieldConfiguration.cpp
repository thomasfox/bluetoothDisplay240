/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#include <Arduino.h>
#include <gfxfont.h>

#include "FieldConfiguration.h"
#include "DogDisplay.h"
#include "StringDisplay.h"

/**
 * Displays a string in the field.
 * 
 * The upper left corner of the displaywindow for the string is defined by (xOffsetInPx, yOffsetInBytes). 
 * The display window is widthInPx pixel long and heightInBytes bytes high.
 * 
 * @param toDisplay the string to display
 */
void StringFieldConfiguration::displayValue(String toDisplay)
{
  StringDisplay stringDisplay(gfxValueFont, toDisplay);
  word xOffsetInField = widthInPx - stringDisplay.getWidth() - labelBitmapWithInPx - labelValueXDistanceInPx;
  for (uint16_t yByteCount = yOffsetInBytes; yByteCount < yOffsetInBytes + heightInBytes; yByteCount++) 
  {
    display.position(xOffsetInPx, yByteCount);
    display.dataMode();
    display.chipSelect();
    for (word x = 0; x < widthInPx - labelBitmapWithInPx - labelValueXDistanceInPx; x++) 
    {
      uint8_t toSend = 0;
      for (uint16_t yBit = 0; yBit < 8; yBit++)
      {
        uint16_t y = yByteCount * 8 + yBit;
        if (stringDisplay.getBitAt(x - xOffsetInField, y))
        {
          toSend |= 0x01 << yBit;
        }
      }
      display.sendToSpi(toSend);
    }
    display.chipDeselect();
  }
}

/**
 * Displays the unit (label) of the field.
 * The upper left corner of the displaywindow for the unit is defined by (xOffsetInPx + widthInPx - labelBitmapWithInPx, yOffsetInBytes). 
 * 
 * @param toDisplay the string to display
 */
void StringFieldConfiguration::displayUnit()
{
  for (uint8_t yByteCount = 0; yByteCount < labelBitmapHeightInBytes; yByteCount++) 
  {
    display.position(xOffsetInPx + widthInPx - labelBitmapWithInPx, yOffsetInBytes + yByteCount);
    display.dataMode();
    display.chipSelect();
    for (uint8_t x = 0; x < labelBitmapWithInPx; x++) 
    {      
      uint8_t toSend = pgm_read_byte(labelBitmap + yByteCount * labelBitmapWithInPx + x);
      display.sendToSpi(toSend);
    }
    display.chipDeselect();
  }
}

/**
 * Parses the passed String as int and displays the parsed value on the bar graph. 
 * 
 * @param value the value to display. Must be parseable as int. 
 *        The displayable range is -(widthInPx - 1) / 2 ... (widthInPx - 1) / 2.
 *        Smaller or larger values are clipped.
 */
void BarFieldConfiguration::displayValue(String value)
{
  int16_t intValue = value.toInt();
  uint16_t middleX = getMiddleX();
  int16_t startX;
  int16_t endX;
  if (intValue < 0)
  {
    startX = middleX + intValue;
    if (startX < 0)
    {
      startX = 0;
    }
    endX = middleX; 
  }
  else 
  {
    startX = middleX;
    endX = middleX + intValue;
    if (endX >= widthInPx)
    {
      endX = widthInPx - 1;
    }
  }
  for (uint8_t yInBytes = 0; yInBytes < heightInBytes; yInBytes++)
  {
    displayYSegment(startX, endX, yInBytes);
  }
  if (additionalZeroExtension)
  {
    displayZeroExtension(intValue);    
  }
}

/**
 * Displays a y segment (8 px) of the graph which displays the elongated zero bar.
 * 
 * @param startX the x value where the displayed bar begins, relative to xOffsetInPx
 * @param endX the x value where the displayed bar ends, relative to xOffsetInPx. Must be >= startX.
 * @param yInBytes the y position of the segment to display in bytes (8 px) relative to yOffsetInBytes
 */
// display the y segment of the graph which displays the elongated zero bar
void BarFieldConfiguration::displayYSegment(int16_t startX, int16_t endX, uint8_t yInBytes)
{
  uint8_t nonModulusValue;
  if (yInBytes * 2 + 1 > heightInBytes)
  {
    nonModulusValue = 0xFF;
  }
  else if (yInBytes * 2 + 1 == heightInBytes)
  {
    nonModulusValue = 0xF0;
  }
  else
  {
    nonModulusValue = 0x00;
  }

  int16_t middleX = getMiddleX();
  display.position(xOffsetInPx, yOffsetInBytes + yInBytes);
  display.dataMode();
  display.chipSelect();
  for (int16_t x = 0; x < widthInPx; x++)
  {
    uint8_t toSend;
    if (x < startX || x > endX)
    {
      // outside displayed bar
      toSend = 0x00;
    }
    else
    {
      // inside displayed bar
      uint8_t modulusX = abs(x - middleX) % 20;
      if (modulusX <= ZERO_BAR_MAX_X || modulusX >= 20 - ZERO_BAR_MAX_X)
      {
        toSend = 0xFF;
      }
      else
      {
        toSend = nonModulusValue;
      }
    }
    display.sendToSpi(toSend);
  }
  display.chipDeselect();
}

/**
 * Displays the y segment of the graph belonging to the elongated zero bar.
 * To distinguish the different near-zero values from each other, the elongation is thicker than the zero bar
 * for near-zero values. For values far from the zero bar, the elongation is as thick as the zero bar.
 * 
 * @param xRelativeToMiddleX the parsed value of the field which should be displayed.
 */
void BarFieldConfiguration::displayZeroExtension(int16_t xRelativeToMiddleX)
{
  // all x values are relative to middleX + xOffsetInPx here
  
  int16_t startX = xRelativeToMiddleX - 2 * ZERO_BAR_MAX_X;
  if (startX > 0)
  {
    startX = 0;
  }
  if (startX < -ZERO_BAR_MAX_X)
  {
    startX = -ZERO_BAR_MAX_X;
  }

  int16_t endX = xRelativeToMiddleX + 2 * ZERO_BAR_MAX_X;
  if (endX < 0)
  {
    endX = 0;
  }
  if (endX > ZERO_BAR_MAX_X)
  {
    endX = ZERO_BAR_MAX_X;
  }

  display.position(xOffsetInPx + getMiddleX() - ZERO_BAR_MAX_X, yOffsetInBytes - 1);
  display.dataMode();
  display.chipSelect();
  for (int16_t x = -ZERO_BAR_MAX_X; x <= ZERO_BAR_MAX_X; x++)
  {
    if (x < startX || x > endX)
    {
      display.sendToSpi(0x00);
    }
    else
    {
      display.sendToSpi(0xFF);
    }
  }
  display.chipDeselect();
}

/**
 * Returns the x coordinate of the pixel in the middle of the display, relative to the field's x offset.
 */
inline int16_t BarFieldConfiguration::getMiddleX()
{
  return (widthInPx - 1) / 2;
}
