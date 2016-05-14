#ifndef __IMAGE_CODEC_HXX__
#define __IMAGE_CODEC_HXX__

class image_codec{
public:
  virtual ~image_codec(){
  }

  virtual std::shared_ptr<buffered_image> decode(const std::string& file) const = 0;

  virtual void encode(const std::shared_ptr<buffered_image>& image, const std::string& file) const = 0;
};

class jpeg_image_codec : public image_codec{
private:
  int _quality;

public:
  explicit jpeg_image_codec(int quality)
    :_quality(quality){
  }

  virtual ~jpeg_image_codec(){
  }

public:
  virtual std::shared_ptr<buffered_image> decode(const std::string& file) const;
  virtual void encode(const std::shared_ptr<buffered_image>& image, const std::string& file) const;
};

#endif
