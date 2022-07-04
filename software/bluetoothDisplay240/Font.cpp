/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#include <Arduino.h>
#include <gfxfont.h>
#include <avr/pgmspace.h>
#include "Font.h"

/**
 * Constructor.
 * 
 * @param gfxfont the address of the GFX font definition in PGM space.
 */
Font::Font(const GFXfont *gfxfont)
{
  this->gfxfont = gfxfont;

  firstCharacterInFont = pgm_read_word(&(gfxfont->first));
  lastCharacterInFont = pgm_read_word(&(gfxfont->last));
  yAdvance = pgm_read_byte(&(gfxfont->yAdvance));
  bitmaps = pgm_read_ptr(&(gfxfont->bitmap));
  maxHeightAboveBaseline = 0;
  maxDescentBelowBaseline = 0;
  
  for (char c = firstCharacterInFont; c <= lastCharacterInFont; c++)
  {
    GFXglyph glyph = getGlyph(c);
    int8_t yOffset = glyph.yOffset;
    uint8_t height = glyph.height;
    if (-yOffset > maxHeightAboveBaseline)
    {
      maxHeightAboveBaseline = -yOffset;
    }
    if (yOffset + height > maxDescentBelowBaseline)
    {
      maxDescentBelowBaseline = yOffset + height;
    }
  }
  // the baseline is chosen such that the maximum height and the maximum descent
  // has the same distance from the line height limits
  yBaseline = -maxHeightAboveBaseline - (yAdvance - maxHeightAboveBaseline - maxDescentBelowBaseline) / 2;
}

/**
 * Returns the glyph (display definition) for the character c.
 * 
 * @param c the character to find the glyph for.
 *        Must be in the range firstCharacterInFont..lastCharacterInFont, this is NOT checked in the method.
 */
GFXglyph Font::getGlyph(char c)
{
  // calculate difference from first character of font (stored in firstCharacterInFont)
  c -= firstCharacterInFont;
  
  // get the pointer to the glyph (character) definition
  GFXglyph *glyphConst = &(((GFXglyph *)pgm_read_ptr(&gfxfont->glyph))[c]);

  // read the offset of the character bitmap in the global font bitmap
  uint16_t characterBitmapOffset = pgm_read_word(&(glyphConst->bitmapOffset));

  // read width and height of the character's bitmap
  uint8_t characterWidth = pgm_read_byte(&(glyphConst->width)), characterHeight = pgm_read_byte(&(glyphConst->height));

  // read the width of the space reserved for the character
  uint8_t characterXAdvance = pgm_read_byte(&(glyphConst->xAdvance));

  // read x and y offset of the bitmap within the place reserved by this character
  int8_t characterXOffset = pgm_read_byte(&(glyphConst->xOffset)),
         characterYOffset = pgm_read_byte(&(glyphConst->yOffset));

  GFXglyph glyph = {
      characterBitmapOffset, 
      characterWidth, 
      characterHeight,
      characterXAdvance,
      characterXOffset,
      characterYOffset};
  return glyph;
}

int8_t Font::getYBaseline() 
{
  return yBaseline;
}

int8_t Font::getYAdvance()
{
  return yAdvance;
}

uint8_t Font::getBitmapByte(GFXglyph gfxGlyph, uint16_t offsetInCharBitmap)
{
  return (uint8_t)(pgm_read_byte(bitmaps + gfxGlyph.bitmapOffset + offsetInCharBitmap));
}
