/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#ifndef FONT_H
#define FONT_H

#include <Arduino.h>
#include <gfxfont.h>

class Font {
  public:
    /**
     * Constructor.
     * 
     * @param gfxfont the address of the GFX font definition in PGM space.
     */
    Font(const GFXfont *gfxfont);
    
    /**
     * Returns the glyph (display definition) for the character c.
     * 
     * @param c the character to find the glyph for.
     *        Must be in the range firstCharacterInFont..lastCharacterInFont, this is NOT checked in the method.
     */
    GFXglyph getGlyph(char c);

    /**
     * Return the baseline of the gfxfont. 
     * The returned value is negative and relative to the upmost line of the font.
     * Tthe baseline is chosen such that the maximum height and the maximum descent
     * has the same distance from the line height limits.
     */
    int8_t getYBaseline();

    /**
     * Return the yAdvance property of the gfxfont,
     * which tells by how many pixel lines should be separated in y direction.
     */
    int8_t getYAdvance();
    
    /**
     * Return the byte in the bitmap for the glyph gfxGlyph, 
     * which has the offset offsetInCharBitmap inside the character's bitmap.
     * I.e. a offsetInCharBitmap of zero returns the first byte of the character's bitmap.
     * No check is made whether one stays in the character's bitmap.
     */
    uint8_t getBitmapByte(GFXglyph gfxGlyph, uint16_t offsetInCharBitmap);
   
  private:
    // The addresss of the gfxfont used by this Font object.
    // It is assumed that this address is in PGM space
    const GFXfont *gfxfont;

    // From font definition: the first displayable character in the font
    uint16_t firstCharacterInFont;
    
    // From font definition: the last displayable character in the font
    uint16_t lastCharacterInFont;
    
    // From font definition: how much pixel should be between lines
    uint8_t yAdvance;
    
    // From font definition: the address of the stored bitmaps
    uint8_t* bitmaps;

    // Calculated: the maximum height in pixel of any glyph in the font above the baseline (positive)
    int8_t maxHeightAboveBaseline;
    
    // Calculated: the maximum descent in pixel of any glyph in the font below the baseline (positive)
    int8_t maxDescentBelowBaseline;

    // Calculated: the position of the baseline (negative)
    int8_t yBaseline;
};

#endif
