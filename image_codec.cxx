#include "sokoh.hxx"
#include "jpeg-9a/jpeglib.h"
#include <setjmp.h>

extern "C"{
  struct my_error_mgr{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
  };

  typedef struct my_error_mgr *my_error_ptr;

  METHODDEF(void) my_error_exit(j_common_ptr cinfo){
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
  }
};


std::shared_ptr<buffered_image> jpeg_image_codec::decode(const std::string& file) const{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  FILE* in_file;
  JSAMPARRAY buffer;
  int row_stride, line_index(0), pix_index(0);
  std::shared_ptr<buffered_image> image(nullptr);
  if((in_file = fopen(file.c_str(), "rb")) == NULL){
    fprintf(stderr, "cannot open %s\n", file.c_str());
    return nullptr;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if(setjmp(jerr.setjmp_buffer)){
    jpeg_destroy_decompress(&cinfo);
    fclose(in_file);
    return nullptr;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, in_file);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  image.reset(new buffered_image(cinfo.output_width, cinfo.output_height));
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
  while(cinfo.output_scanline < cinfo.output_height){
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    pix_index = 0;
    for(int i = 0; i < row_stride; i += 3){
      argb_t pix(0);
      pix |= (((argb_t)buffer[0][i]) << 16);
      pix |= (((argb_t)buffer[0][i + 1]) << 8);
      pix |= (buffer[0][i + 2]);
      image->set_pix(pix_index ++, line_index, pix);
    }

    line_index ++;
  }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(in_file);
  return image;
}

void jpeg_image_codec::encode(const std::shared_ptr<buffered_image>& image, const std::string& file) const{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE* outfile;
  JSAMPROW row_pointer[1];

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  if((outfile = fopen(file.c_str(), "wb")) == NULL){
    fprintf(stderr, "cannot open %s\n", file.c_str());
    throw sparta_exception("failed to open file to save image");
  }
  
  jpeg_stdio_dest(&cinfo, outfile);
  cinfo.image_width = image->width();
  cinfo.image_height = image->height();
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, this->_quality, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
  row_pointer[0] = new JSAMPLE[image->width() * 3];
  while(cinfo.next_scanline < cinfo.image_height){
    memset(row_pointer[0], 0, sizeof(JSAMPLE) * 3 * image->width());
    for(int i = 0; i < image->width(); ++i){
      argb_t pix = image->pix(i, cinfo.next_scanline);
      row_pointer[0][i * 3] = (JSAMPLE)((pix >> 16) & (JSAMPLE)(0xFF));
      row_pointer[0][i * 3 + 1] = (JSAMPLE)(((pix << 16) >> 24) & (JSAMPLE)(0xFF));
      row_pointer[0][i * 3 + 2] = (JSAMPLE)(pix & (JSAMPLE)(0xFF));
    }

    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  delete[] row_pointer[0];
  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}
