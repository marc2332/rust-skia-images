#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H

_Static_assert(FREETYPE_MAJOR == 2 && FREETYPE_MINOR == 14 && FREETYPE_PATCH == 3,
               "Expected FreeType 2.14.3 headers");

int main(void) {
    FT_Library library;
    FT_Int major, minor, patch;

    if (FT_Init_FreeType(&library)) {
        return 1;
    }

    FT_Library_Version(library, &major, &minor, &patch);
    printf("FreeType headers: %d.%d.%d, runtime: %d.%d.%d\n",
           FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH, major, minor, patch);
    FT_Done_FreeType(library);

    return major != FREETYPE_MAJOR || minor != FREETYPE_MINOR || patch != FREETYPE_PATCH;
}
