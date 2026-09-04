#include "pen_settings.h"

int ColorToMenuId(COLORREF color) {
    if (color == RGB(255, 0, 0)) return kIdColorRed;
    if (color == RGB(0, 0, 255)) return kIdColorBlue;
    if (color == RGB(0, 160, 0)) return kIdColorGreen;
    return kIdColorBlack;
}

int WidthToMenuId(int width) {
    if (width <= 1) return kIdWidthThin;
    if (width >= 6) return kIdWidthThick;
    return kIdWidthMedium;
}
