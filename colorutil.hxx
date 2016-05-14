#ifndef COLOR_UTIL_HXX
#define COLOR_UTIL_HXX

void rgb_to_hsl(argb_t color, double* h, double* s, double* l);

argb_t hsl_to_rgb(double h, double s, double l);
#endif //COLOR_UTIL_HXX
