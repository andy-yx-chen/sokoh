#include "sokoh.hxx"

struct font_system {
public:
	font_system();
	~font_system();
	FT_Library library;
};

font_system font_container;

font_system::font_system() : library(nullptr) {
	FT_Error error;
	error = FT_Init_FreeType(&library);
	if (error) {
		throw std::runtime_error("failed to initialize FT library");
	}
}

font_system::~font_system() {
	if (library != nullptr) {
		FT_Done_FreeType(library);
	}
}

font::font(const std::string& font_file) : _face(nullptr) {
	if (font_container.library == nullptr) {
		throw std::runtime_error("font system is not initialized");
	}

	FT_Error error = FT_New_Face(font_container.library, font_file.c_str(), 0, &this->_face);
	if (error) {
		throw std::domain_error("invalid font file.");
	}
}

font::~font() {
	if (this->_face != nullptr) {
		FT_Done_Face(this->_face);
	}
}

void font::set_size(int size, int resolution) {
	FT_Set_Char_Size(this->_face, size * 64, 0, resolution, 0);
}

void font::render_text(buffered_image& image, int base_x, int base_y, const std::wstring& text, argb_t color) {
	FT_Int pen_x = base_x;
	FT_Error error;
	for (size_t i = 0; i < text.length(); ++i) {
		error = FT_Load_Char(this->_face, text[i], FT_LOAD_RENDER);
		if (error) {
			continue;
		}

		font::copy_to_bitmap(&this->_face->glyph->bitmap, this->_face->glyph->bitmap_left + pen_x, base_y - this->_face->glyph->bitmap_top, image, color);
		pen_x += this->_face->glyph->advance.x >> 6;
	}
}

void font::render_text(buffered_image& image, int base_x, int base_y, const std::wstring& text, int rotate_degree, argb_t color) {
	FT_Matrix matrix;
	FT_Vector pen;
	FT_Error error;
	double angle = (rotate_degree / 180) * 3.14159;
	matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
	pen.x = base_x * 64;
	pen.y = base_y * 64;
	for (size_t i = 0; i < text.length(); ++i) {
		FT_Set_Transform(this->_face, &matrix, &pen);
		error = FT_Load_Char(this->_face, text[i], FT_LOAD_RENDER);
		if (error) {
			continue;
		}

		font::copy_to_bitmap(&this->_face->glyph->bitmap, this->_face->glyph->bitmap_left, base_y - this->_face->glyph->bitmap_top, image, color);
		
		/* increment pen position */
		pen.x += this->_face->glyph->advance.x;
		pen.y += this->_face->glyph->advance.y;
	}
}

void font::copy_to_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, buffered_image& image, argb_t color) {
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;
	double h, s, l;
	rgb_to_hsl(color, &h, &s, &l);
	for (FT_Int i = x, p = 0; i < x_max; ++i, ++p) {
		for (FT_Int j = y, q = 0; j < y_max; ++j, ++q) {
			if (i < 0 || j < 0 || i >= image.width() || j >= image.height()) {
				continue;
			}

			unsigned char bitmap_value = bitmap->buffer[q * bitmap->width + p];
			if (bitmap_value == 0) {
				continue;
			}

			unsigned char point = bitmap->buffer[q * bitmap->width + p];
			argb_t pix_value = 0;
			if (color == 0) {
				pix_value = ((255 - point) << 16) | ((255 - point) << 8) | (255 - point);
			}
			else {
				double new_l = l * (2.0 - point / 255.0);
				if (new_l > 1.0) {
					new_l = 1.0;
				}

				pix_value = hsl_to_rgb(h, s, new_l);
			}

			image.set_pix(i, j, pix_value);
		}
	}
}