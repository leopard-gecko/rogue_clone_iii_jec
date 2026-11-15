/* Shared UTF-8 text handling for option input and terminal cell widths.
 * Copyright (c) 2026 leopardgecko. See LICENSE.txt.
 * This module does not depend on curses, Windows APIs or the current locale.
 */
#ifndef JROGUE_UNICODE_TEXT_H
#define JROGUE_UNICODE_TEXT_H

#include <stddef.h>

/* Return the byte length of one valid Unicode scalar, or 0 for invalid input.
 * The buffer is not required to be NUL-terminated. */
size_t jrogue_utf8_scalar_length(const char *bytes, size_t available);

/* Positions are UTF-8 byte offsets; the returned offsets are whole
 * extended-character boundaries.  Invalid UTF-8 bytes stand alone. */
size_t jrogue_text_next(const char *text, size_t pos);
size_t jrogue_text_prev(const char *text, size_t pos);

/* Largest complete character prefix within max_bytes bytes. */
size_t jrogue_text_prefix(const char *text, size_t max_bytes);

/* Terminal columns, not code points or UTF-8 bytes.  The _n variant
 * measures complete characters in the first max_bytes bytes. */
int jrogue_text_width(const char *text);
int jrogue_text_width_n(const char *text, size_t max_bytes);

#endif /* JROGUE_UNICODE_TEXT_H */
