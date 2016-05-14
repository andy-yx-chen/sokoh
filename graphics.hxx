#ifndef GRAPHICS_HXX_
#define GRAPHICS_HXX_

class font;
typedef std::unordered_map<std::string, std::shared_ptr<font>> font_list;

class graphics {
public:
	graphics(buffered_image& image)
		: _image(image), _font_list() {
	}

public:
	void paint_background(argb_t background);
	void load_font(const std::string& font_name, const std::string& font_file);
	void draw_text(int x, int y, const std::wstring& text, const std::string& font_name, int font_size, argb_t color = 0);
	void draw_line(int x, int y, int x1, int y1, argb_t color, int width = 1);

private:
	graphics(const graphics&);
	graphics& operator= (const graphics&);

private:
	buffered_image& _image;
	font_list _font_list;
};
#endif //GRAPHICS_HXX_
