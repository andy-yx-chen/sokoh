#ifndef FONT_HXX_
#define FONT_HXX_

class font {
public:
	font(const std::string& font_file);
	~font();

	void set_size(int size, int resolution);
	void render_text(buffered_image& image, int base_x, int base_y, const std::wstring& text, argb_t color = 0);
	void render_text(buffered_image& image, int base_x, int base_y, const std::wstring& text, int rotate_degree, argb_t color = 0);

private:
	static void copy_to_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, buffered_image& image, argb_t color);
private:
	FT_Face _face;
};
#endif //FONT_HXX_
