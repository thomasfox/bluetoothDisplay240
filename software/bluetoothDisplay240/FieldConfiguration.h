/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#ifndef FIELD_CONFIGURATION_H
#define FIELD_CONFIGURATION_H

#include <gfxfont.h>

#include "DogDisplay.h"

// A StringFieldConfiguration displays a value on the display as String
// It occupies a rectangle on the sceen defined by xOffset, yOffset, width and height
// It contains a font to display the value
// It has a label (e.g. for displaying the name or the unit of the value), which is always displayed at the right of the field, using a bitmap
class StringFieldConfiguration
{
  public:
  String unit;                // the text to display as the unit of the quantity in the field
  uint16_t xOffsetInPx = 0;   // the X distance in pixel from display start to the start of the field, 0 if the field starts right where the display starts
  uint8_t yOffsetInBytes = 0; // the Y distance in bytes (8 pixel) from display start to the start of the field, 0 if the field starts right where the display starts
  uint16_t widthInPx;         // the X dimension of the field in pixel
  uint8_t heightInBytes;      // the Y dimension of the field in bytes (8 px)
  GFXfont *gfxValueFont;      // the font used to display the value of the field
  uint8_t labelValueXDistanceInPx = 4; // Distance between the label and the displayed value, in x Direction, in px
  uint8_t labelBitmapWithInPx;         // the with of the label bitmap in px
  uint8_t labelBitmapHeightInBytes;    // the height of the label bitmap in Bytes (== height in px / 8)
  const uint8_t *labelBitmap;          // the bitmap containing the label (e.g containing the unit) of the field, as dog type bitmap (first row followed by second row ...)
  DogDisplay display;                  // the display where the field is displayed on
  
  void displayValue(String toDisplay); // displays the passed String in the field
  void displayUnit();                  // displays the unit bitmap of the field 
};

// A BarFieldConfiguration displays a value on the display as a bar
// It occupies a rectangle on the sceen defined by xOffset, yOffset, width and height=16px
class BarFieldConfiguration
{
  public:
  String unit;                // the text to display as the unit of the quantity in the field
  uint16_t xOffsetInPx = 0;   // the X distance in pixel from display start to the start of the field, 0 if the field starts right where the display starts
  uint8_t yOffsetInBytes = 0; // the Y distance in bytes (8 pixel) from display start to the start of the field, 0 if the field starts right where the display starts
  uint16_t widthInPx = 201;   // the X dimension of the field in pixel
  uint8_t heightInBytes = 2;  // the Y dimension of the field in bytes (i.e. 8 px)
  boolean additionalZeroExtension = true; // whether to use another y byte at the zero mark to extend the zero mark upwards;
  DogDisplay display;         // the display where the field is displayed on
  
  void displayValue(String toDisplay); // parses the passed String as int and displays the parsed value on the bar graph. Negative values can be displayed as well.

  private:
  const int16_t ZERO_BAR_MAX_X = 2;                                           // how far the zero bar in the graph extends in x direction
  void displayYSegment(int16_t startX, int16_t endX, uint8_t yOffsetInBytes); // displays a y segment (8 px high)of the graph
  void displayZeroExtension(int16_t xRelativeToMiddleX);                      // display the y segment of the graph belonging to the elongated zero bar
  int16_t getMiddleX();                                                       // returns the x coordinate of the pixel in the middle of the field rectangle
};

#endif
