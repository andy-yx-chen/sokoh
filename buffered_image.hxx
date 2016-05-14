#ifndef __BUFFERED_IMAGE_HXX__
#define __BUFFERED_IMAGE_HXX__

#define color_a(color) (((color) >> 24) & 0xFF)
#define color_r(color) (((color) >> 16) & 0xFF)
#define color_g(color) (((color) >> 8) & 0xFF)
#define color_b(color) ((color) & 0xFF)
#define color_argb(a, r, g, b)((argb_t)(a) << 24 | (argb_t)(r) << 16 | (argb_t)(g) << 8 | (argb_t)(b))
typedef uint32_t argb_t;

class buffered_image{
private:
  int _width;
  int _height;
  std::vector<std::vector<argb_t> > _image_data;

public:
  buffered_image(int width, int height)
    :_width(width), _height(height), _image_data(height, std::vector<argb_t>(width, 0)){
  }

public:
  std::shared_ptr<buffered_image> resize(int width, int height) const;

public:
  int width() const{
    return _width;
  }

  int height() const{
    return _height;
  }

  argb_t pix(int x, int y) const;


  void set_pix(int x, int y, argb_t pix);

private:
  argb_t resample_by_bilinear(int width, int height, int x, int y) const;
  argb_t resample_by_bicubic(int width, int height, int x, int y) const;
};
#endif
