#include "sokoh.hxx"

void graphics::paint_background(argb_t background) {
	for (int x = 0; x < this->_image.width(); ++x) {
		for (int y = 0; y < this->_image.height(); ++y) {
			this->_image.set_pix(x, y, background);
		}
	}
}

void graphics::load_font(const std::string& font_name, const std::string& font_file) {
	std::shared_ptr<font> f(new font(font_file));
	this->_font_list.insert(std::make_pair(font_name, f));
}

void graphics::draw_text(int x, int y, const std::wstring& text, const std::string& font_name, int font_size, argb_t color) {
	font_list::const_iterator it = this->_font_list.find(font_name);
	if (it == this->_font_list.end()) {
		throw std::domain_error("font not found.");
	}

	it->second->set_size(font_size, 72);
	it->second->render_text(this->_image, x, y + font_size, text, color);
}

void graphics::draw_line(int x, int y, int x1, int y1, argb_t color, int width) {
	if (x < 0 || x1 < 0 || y < 0 || y1 < 0 || x >= this->_image.width() || x1 >= this->_image.width() || y >= this->_image.height() || y1 >= this->_image.height()) {
		throw std::out_of_range("points' coordinators are out of range");
	}

	int base_x(0), base_y(0);
	int target_x(0), target_y(0);
	if (x <= x1) {
		base_x = x;
		target_x = x1;
		base_y = y;
		target_y = y1;
	}
	else {
		base_x = x1;
		target_x = x;
		base_y = y1;
		target_y = y;
	}

	if (x == x1) {
		if (y1 > y) {
			target_y = y1;
			base_y = y;
		}
		else {
			target_y = y;
			base_y = y1;
		}

		for (int i = base_y; i <= target_y; ++i) {
			for (int j = x - (width - 1) / 2; j <= x + width / 2 && j >= 0 && j < this->_image.width(); ++j) {
				this->_image.set_pix(j, i, color);
			}
		}
	}
	else {
		double k = (double)(target_y - base_y) / (double)(target_x - base_x);
		for (int i = base_x; i <= target_x; ++i) {
			int point_y = base_y + (int)(k * i);
			for (int j = point_y - (width - 1) / 2; j <= point_y + width / 2 && j >= 0 && j < this->_image.height(); ++j) {
				this->_image.set_pix(i, j, color);
			}
		}
	}
}
