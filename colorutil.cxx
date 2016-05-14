#include "sokoh.hxx"

void rgb_to_hsl(argb_t color, double* h, double* s, double* l)
{
	double r = color_r(color) / 255.0;
	double g = color_g(color) / 255.0;
	double b = color_b(color) / 255.0;
	double max = 0.0;
	double min = 0.0;
	double delta = 0.0;
	double mini_delta = 0.00000001;
	max = std::max(r, g);
	max = std::max(max, b);
	min = std::min(r, g);
	min = std::min(min, b);
	delta = max - min;
	*l = (max + min) / 2.0;
	if (abs(delta - 0) < mini_delta) {
		*s = 0.0;
		return;
	}

	*s = delta / (1 - abs(2 * *l - 1));

	if (abs(max - r) < mini_delta) {
		*h = ((int)((g - b) / delta) % 6) / 6.0;
	}
	else if (abs(max - g) < mini_delta) {
		*h = ((b - r) / delta + 2.0) / 6.0;
	}
	else {
		*h = ((r - g) / delta + 4.0) / 6.0;
	}
}

argb_t hsl_to_rgb(double h, double s, double l)
{
	double c = (1.0 - abs(2.0 * l - 1)) * s;
	double x = c * (1 - abs((int)(h * 60) % 2 - 1.0));
	double m = l - c / 2.0;
	double unit = 1.0 / 6.0;
	double r(0.0), g(0.0), b(0.0);
	if (h < unit && h >= 0.0) {
		r = c;
		g = x;
	}
	else if (h >= unit && h < 2.0 * unit) {
		r = x;
		g = c;
	}
	else if (h >= 2.0 * unit && h < 3.0 * unit) {
		g = c;
		b = x;
	}
	else if (h >= 3.0 * unit && h < 4.0 * unit) {
		g = x;
		b = c;
	}
	else if (h >= 4.0 * unit && h < 5.0 * unit) {
		r = x;
		b = c;
	}
	else {
		r = c;
		b = x;
	}

	return color_argb(0, 255.0 * (r + m), 255.0 * (g + m), 255.0 * (b + m));
}
