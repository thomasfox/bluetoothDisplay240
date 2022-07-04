/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#include "StringDisplay.h"
#include "Font.h"

/* 
 * Constructor.
 * 
 * @param gfxfont the font to use
 * @param toDisplay the String to display
 * 
 * Fills the glyph array with the glyphs of the character to display
 * and the startX array with the sztart X positions of each glyph.
 */
StringDisplay::StringDisplay(const GFXfont *gfxfont, String toDisplay)
{
  this->font = new Font(gfxfont);
  toDisplayLength = min(toDisplay.length(), MAX_DISPLAYABLE_STRING_LENGTH);
  totalXWidthInPixel = 0;
  for (uint8_t i = 0; i < toDisplayLength; ++i)
  {
    startX[i] = totalXWidthInPixel;
    char c = toDisplay.charAt(i);
    GFXglyph glyph = font->getGlyph(c);
    glyphArray[i] = glyph;
    totalXWidthInPixel += glyph.xAdvance;
  }
}

StringDisplay::~StringDisplay()
{
  free(this->font);
}

/**
 * Calculates a bit of the bitmap of the string which should be displayed.
 * Returns 0 if the bit at the given position is not set,
 * or 1 if the bit at the given position is set.
 * x=0,y=0 is at the upper left corner of the String, x is positive to the right, y is positive to the bottom.
 * 
 * @param x the x coordinate of the pixel to calculate the bit at
 * @param y the y coordinate of the pixel to calculate the bit at
 */
uint8_t StringDisplay::getBitAt(int16_t x, int16_t y)
{
  if (x < 0 || x >= totalXWidthInPixel || y < 0 || y >= font->getYAdvance())
  {
    // outside of rectangle of the displayed string
    return 0;
  }

  // calculate which glyph to use for the given x position
  uint8_t glyphNumber = 0;
  while (startX[glyphNumber + 1] < x && glyphNumber < toDisplayLength - 1)
  {
    glyphNumber++;
  }
  GFXglyph glyph = glyphArray[glyphNumber];

  // calculate the x position within the space reserved for the character
  uint16_t xInChar = x - startX[glyphNumber];

  if (xInChar < glyph.xOffset // before character bitmap
      || xInChar >= glyph.xOffset + glyph.width // after character bitmap
      || y < glyph.yOffset - font->getYBaseline() // above character bitmap
      || y >= glyph.yOffset - font->getYBaseline() + glyph.height) // below character bitmap
  {
    // outside of bitmap for the glyph
    return 0;
  }

  // determine x,y position within bitmap of the glyph
  word xInCharBitmap = xInChar - glyph.xOffset;
  word yInCharBitmap = y - glyph.yOffset + font->getYBaseline();
  
  // determine bit and byte offset in bitmap of the glyph
  word charBitmapOffsetTotalBits = yInCharBitmap * glyph.width + xInCharBitmap;
  word charBitmapOffsetBytes = charBitmapOffsetTotalBits / 8;
  word charBitmapOffsetBits = charBitmapOffsetTotalBits % 8;

  // read the needed byte from the bitmap of the glyph
  uint8_t bitmapByte = font->getBitmapByte(glyph, charBitmapOffsetBytes);

  // read the needed bit out of the byte and return it
  uint8_t shiftedBits = bitmapByte << charBitmapOffsetBits;
  if (shiftedBits & 0x80)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
 * Calculates the width in pixel of the string which should be displayed.
 */
uint16_t StringDisplay::getWidth()
{
  return totalXWidthInPixel;
}
