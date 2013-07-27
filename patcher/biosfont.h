/* KallistiOS ##version##

   dc/biosfont.h
   (c)2000-2001 Dan Potter
   Japanese Functions (c)2002 Kazuaki Matsumoto

*/

/** \file   dc/biosfont.h
    \brief  BIOS font drawing functions.

    This file provides support for utilizing the font built into the Dreamcast's
    BIOS. These functions allow access to both the western character set and
    Japanese characters.

    \author Dan Potter
    \author Kazuaki Matsumoto
*/

#ifndef __DC_BIOSFONT_H
#define __DC_BIOSFONT_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include "types.h"

/** \brief  Set the font foreground color.

    This function selects the foreground color to draw when a pixel is opaque in
    the font. The value passed in for the color should be in whatever pixel
    format that you intend to use for the image produced.

    \param  c               The color to use.
    \return                 The old foreground color.
*/
uint32 bfont_set_foreground_color(uint32 c);

/** \brief  Set the font background color.

    This function selects the background color to draw when a pixel is drawn in
    the font. This color is only used for pixels not covered by the font when
    you have selected to have the font be opaque.

    \param  c               The color to use.
    \return                 The old background color.
*/
uint32 bfont_set_background_color(uint32 c);

/** \brief  Set the font to draw 32-bit color.

    This function changes whether the font draws colors as 32-bit or 16-bit. The
    default is to use 16-bit.

    \param  on              Set to 0 to use 16-bit color, 32-bit otherwise.
    \return                 The old state (1 = 32-bit, 0 = 16-bit).
*/
int bfont_set_32bit_mode(int on);

/* Constants for the function below */
#define BFONT_CODE_ISO8859_1    0   /**< \brief ISO-8859-1 (western) charset */
#define BFONT_CODE_EUC          1   /**< \brief EUC-JP charset */
#define BFONT_CODE_SJIS         2   /**< \brief Shift-JIS charset */

/** \brief  Set the font encoding.

    This function selects the font encoding that is used for the font. This
    allows you to select between the various character sets available.

    \param  enc             The character encoding in use
    \see    BFONT_CODE_ISO8859_1
    \see    BFONT_CODE_EUC
    \see    BFONT_CODE_SJIS
*/
void bfont_set_encoding(int enc);

/** \brief  Find an ISO-8859-1 character in the font.

    This function retrieves a pointer to the font data for the specified
    character in the font, if its available. Generally, you will not have to
    use this function, use one of the bfont_draw_* functions instead.

    \param  ch              The character to look up
    \return                 A pointer to the raw character data
*/
uint8 *bfont_find_char(int ch);

/** \brief  Find an full-width Japanese character in the font.

    This function retrieves a pointer to the font data for the specified
    character in the font, if its available. Generally, you will not have to
    use this function, use one of the bfont_draw_* functions instead.

    This function deals with full-width kana and kanji.

    \param  ch              The character to look up
    \return                 A pointer to the raw character data
*/
uint8 *bfont_find_char_jp(int ch);

/** \brief  Find an half-width Japanese character in the font.

    This function retrieves a pointer to the font data for the specified
    character in the font, if its available. Generally, you will not have to
    use this function, use one of the bfont_draw_* functions instead.

    This function deals with half-width kana only.

    \param  ch              The character to look up
    \return                 A pointer to the raw character data
*/
uint8 *bfont_find_char_jp_half(int ch);

/** \brief  Draw a single character to a buffer.

    This function draws a single character in the set encoding to the given
    buffer. Calling this is equivalent to calling bfont_draw_thin() with 0 for
    the final parameter.

    \param  buffer          The buffer to draw to (at least 12 x 24 pixels)
    \param  bufwidth        The width of the buffer in pixels
    \param  opaque          If non-zero, overwrite blank areas with black,
                            otherwise do not change them from what they are
    \param  c               The character to draw
*/
void bfont_draw(void *buffer, int bufwidth, int opaque, int c);

/** \brief  Draw a single thin character to a buffer.

    This function draws a single character in the set encoding to the given
    buffer. This only works with ISO-8859-1 characters and half-width kana.

    \param  buffer          The buffer to draw to (at least 12 x 24 pixels)
    \param  bufwidth        The width of the buffer in pixels
    \param  opaque          If non-zero, overwrite blank areas with black,
                            otherwise do not change them from what they are
    \param  c               The character to draw
    \param  iskana          Set to 1 if the character is a kana, 0 if ISO-8859-1
*/
void bfont_draw_thin(void *buffer, int bufwidth, int opaque, int c, int iskana);

/** \brief  Draw a single wide character to a buffer.

    This function draws a single character in the set encoding to the given
    buffer. This only works with full-width kana and kanji.

    \param  buffer          The buffer to draw to (at least 24 x 24 pixels)
    \param  bufwidth        The width of the buffer in pixels
    \param  opaque          If non-zero, overwrite blank areas with black,
                            otherwise do not change them from what they are
    \param  c               The character to draw
*/
void bfont_draw_wide(void *buffer, int bufwidth, int opaque, int c);

/** \brief  Draw a full string to a buffer.

    This function draws a NUL-terminated string in the set encoding to the given
    buffer. This will automatially handle mixed half and full-width characters
    if the encoding is set to one of the Japanese encodings.

    \param  buffer          The buffer to draw to
    \param  width           The width of the buffer in pixels
    \param  opaque          If non-zero, overwrite blank areas with black,
                            otherwise do not change them from what they are
    \param  str             The string to draw
*/
void bfont_draw_str(void *buffer, int width, int opaque, char *str);

__END_DECLS

#endif  /* __DC_BIOSFONT_H */
