#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>
#include <variant>
#include <converter/pixbuff.hpp>

namespace converter::qoi{

enum qoi_op{
  rgb,
  rgba,
  index,
  diff,
  luma,
  run,
  error
};

qoi_op det_qoi_op(const char& by);

std::string qoi_op_string(qoi_op op);

struct header{
  char magic[4];
  uint32_t width;
  uint32_t height;
  uint8_t channels;
  uint8_t colorspace;

  void print();
};

class qoireader{
public:

  qoireader() = default;
  qoireader(const qoireader&)=delete;
  qoireader& operator=(const qoireader&)=delete;
  qoireader(qoireader&&) = delete;
  qoireader& operator=(qoireader&&)=delete;

  qoireader(const std::string& filename);
  int open(const std::string& filename);
  int read();
  int close();

  converter::pixbuff::rgba32 get_rgba32();

  bool is_open() const;
  size_t get_file_size();
  size_t get_data_size();

  void print();

  header get_header();

private:
  std::string filename_;
  std::ifstream file_;
  header header_;
  size_t data_offset_=0;
  bool is_read_data_=false;
  bool is_read_header_=false;
  bool is_decompressed_=false;
  converter::pixbuff::rgba32 picture_;
  std::vector<char> compressed_data_;
  int read_header();
  int read_data();
  int decompress();
  inline int pixhash(const converter::pixbuff::rgba32::pixel& pixel);
};





}//end namespace converter::qoi
