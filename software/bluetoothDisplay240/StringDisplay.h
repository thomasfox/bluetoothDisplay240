/*
 * Copyright (c) 2022 Thomas Fox
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */
#ifndef STRINGDISPLAY_H
#define STRINGDISPLAY_H

#include <Arduino.h>
#include <gfxfont.h>

#include "Font.h"

/**
 * Calculates the bitmap of a string which is displayed using a gfxfont.
 */
class StringDisplay {
  public:
    StringDisplay(const GFXfont *gfxfont, String toDisplay);
    ~StringDisplay();

    /**
     * Calculates a bit of the bitmap of the string which should be displayed.
     * Returns 0 if the bit at the given position is not set,
     * or 1 if the bit at the given position is set.
     * 
     * @param x the x coordinate of the pixel to calculate the bit at
     * @param y the y coordinate of the pixel to calculate the bit at
     */
    uint8_t getBitAt(int16_t x, int16_t y);

    /**
     * Calculates the width in pixel of the string which should be displayed.
     */
    uint16_t getWidth();

    /**
     * The maximum length in charcters of strings we can display.
     */
    static const uint8_t MAX_DISPLAYABLE_STRING_LENGTH = 20;
    
  private:
    // The font we use to display the string
    Font *font;
    
    // The length in characters of the string which we are displaying
    uint8_t toDisplayLength;
    
    // the width in pixel of the displayed string
    uint16_t totalXWidthInPixel;
    
    // array of the font glyphs which display the single characters in the string
    GFXglyph glyphArray[MAX_DISPLAYABLE_STRING_LENGTH];
    
    // array of x offsets of the spaces reserved for the characters in the string
    uint16_t startX[MAX_DISPLAYABLE_STRING_LENGTH];
};
#endif
