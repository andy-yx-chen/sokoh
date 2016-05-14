#include "sokoh.hxx"
#define byte_of(v, n) (((v) >> (8 * n)) & 0xFF)

float lerp(float p1, float p2, float delta){
  return p1 + delta * (p2 - p1);
}

float blerp(float p1, float p2, float p3, float p4, float deltaX, float deltaY){
  return lerp(lerp(p1, p2, deltaX), lerp(p3, p4, deltaX), deltaY);
}

double cubic_interpolate(double p[4], double x){
  return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

double bicubic_interpolate(double p[4][4], double x, double y){
  double arr[4];
  arr[0] = cubic_interpolate(p[0], y);
  arr[1] = cubic_interpolate(p[1], y);
  arr[2] = cubic_interpolate(p[2], y);
  arr[3] = cubic_interpolate(p[3], y);
  return cubic_interpolate(arr, x);
}

argb_t buffered_image::resample_by_bicubic(int width, int height, int x, int y) const{
  double gx = x / (double) width * _width;
  double gy = y / (double) height * _height;
  int gxi = (int) gx;
  int gyi = (int) gy;
  double dx = gx - gxi;
  double dy = gy - gyi;
  argb_t result(0);
  for(int byte_index = 0; byte_index < 3; ++ byte_index){
    double p[4][4];
    for(int i = -1; i < 3; ++i){
      for(int j = -1; j < 3; ++j){
	    argb_t px = pix((gxi + i) >= 0 ? (gxi + i) < _width ? (gxi + i) : (_width - 1) : 0, (gyi + j) >= 0 ? (gyi + j) < _height ? (gyi + j) : (_height - 1) : 0);
	    p[i + 1][j + 1] = (double) byte_of(px, byte_index);
      }
    }

    double value = bicubic_interpolate(p, dx, dy);
    if(value > 255.0){
      value = 255.0;
    }

    argb_t b(0);
    if(value > 0){
      b = (argb_t) value;
    }

    result |= (argb_t)b << (byte_index * 8);
  }

  return result;
}

argb_t buffered_image::resample_by_bilinear(int width, int height, int x, int y) const{
  float gx = x / (float) width * (_width - 1);
  float gy = y / (float) height * (_height - 1);
  int gxi = (int) gx;
  int gyi = (int) gy;
  argb_t result(0);
  argb_t f00 = pix(gxi, gyi);
  argb_t f10 = pix(gxi + 1, gyi);
  argb_t f01 = pix(gxi, gyi + 1);
  argb_t f11 = pix(gxi + 1, gyi + 1);
  for(int i = 0; i < 3; ++i){
    result |= (argb_t)blerp(byte_of(f00, i), byte_of(f10, i), byte_of(f01, i), byte_of(f11, i), gx - gxi, gy - gyi) << (i * 8);
  }
  return result;
}

std::shared_ptr<buffered_image> buffered_image::resize(int width, int height) const{
  std::shared_ptr<buffered_image> img(new buffered_image(width, height));
  for(int x = 0; x < width; ++x){
    for(int y = 0; y < height; ++y){
      img->set_pix(x, y, resample_by_bicubic(width, height, x, y));
    }
  }

  return img;
}

argb_t buffered_image::pix(int x, int y) const{
  if(x > _width || y > _height){
    throw sparta_exception("out of image range!");
  }

  return _image_data[y][x];
}


void buffered_image::set_pix(int x, int y, argb_t pix){
  if(x > _width || y > _height){
    throw sparta_exception("out of image range!");
  }

  _image_data[y][x] = pix;
}


