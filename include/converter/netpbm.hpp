#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>
#include <variant>
#include <converter/pixbuff.hpp>

namespace converter::netpbm{

struct header{
  //entire header are string values
  std::string magic_number;
  std::string comments;
  std::string width;
  std::string height;
  std::string max_value;

  void print(){
   std::cout << "magic number: "<< magic_number << std::endl
     << "commends: "<< comments << std::endl
     << "width: "<< width << std::endl
     << "height: "<< height << std::endl
     << "max_value: "<< max_value << std::endl;
  }
};

template<typename T>
class pixel_data{
  public:
  pixel_data()=default;
  pixel_data(size_t height,size_t width): height_(height),width_(width),max_val_(0){
  }
  pixel_data(size_t height,size_t width, size_t max_val): height_(height),width_(width),max_val_(max_val){
  }
  void print() const{
    for(auto& e : data_){
      e.print();
    }
    std::cout << "Non file data:" << std::endl
      << "pixel data width:" << width_ << std::endl
      << "pixel data height:" << height_ << std::endl;
  }
  size_t height_;
  size_t width_;
  size_t max_val_;
  std::vector<T> data_;
};

struct P1_pixel{
  std::string black;
  void print() const{
    std::cout << "black:" << black <<std::endl;
  }
};

struct P2_pixel{
  std::string grey;
  void print() const{
    std::cout << "grey:" << grey <<std::endl;
  }
};

struct P3_pixel{
  std::string red;
  std::string green;
  std::string blue;
  void print() const{
    std::cout << "red:" << red << std::endl
      << "green:" << green << std::endl
      << "blue:" << blue << std::endl;
  }
};

struct P4_pixel{
  bool black;
  void print() const{
    std::cout << "black:" << std::to_string(black) << std::endl;
  }
};

struct P5_pixel{
  uint8_t grey;
  void print() const{
    std::cout << "grey:" << std::to_string(grey) << std::endl;
  }
};

struct P6_pixel{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  void print() const{
    std::cout << "red:" << std::to_string(red) << std::endl
      << "green:" << std::to_string(green) << std::endl
      << "blue:" << std::to_string(blue) << std::endl;
  }
};

using P1_pixel_data = pixel_data<P1_pixel>;
using P2_pixel_data = pixel_data<P2_pixel>;
using P3_pixel_data = pixel_data<P3_pixel>;
using P4_pixel_data = pixel_data<P4_pixel>;
using P5_pixel_data = pixel_data<P5_pixel>;
using P6_pixel_data = pixel_data<P6_pixel>;

using pixels_data = std::variant<P6_pixel_data,P5_pixel_data,
                                 P4_pixel_data,P3_pixel_data,
                                 P2_pixel_data,P1_pixel_data>;

struct generic_pbm{
  header header_;
  pixels_data pixel_data_;
  void print(){
    header_.print();
    std::visit([](auto& value){
      value.print();
    },pixel_data_);
  }
};

class pbmreader{
public:

  pbmreader() = default;
  pbmreader(const pbmreader&)=delete;
  pbmreader& operator=(const pbmreader&)=delete;
  pbmreader(pbmreader&&) = delete;
  pbmreader& operator=(pbmreader&&)=delete;

  pbmreader(const std::string& filename);
  int open(std::string& filename);
  int read();
  int close();

  generic_pbm get_generic_pbm();
  converter::pixbuff::rgba32 get_rgba32();


  bool is_open() const;
  size_t get_file_size();

  generic_pbm convert_to_P6();
  void print();


private:
  std::string filename_;
  std::ifstream file_;
  pixels_data pixel_data_;
  header header_;
  size_t data_offset_=0;
  bool is_read_data_=0;
  bool is_read_header_=0;
  struct pixel_data_print_visit;
  struct pixel_data_to_P6_visit;
  struct pixel_data_to_rgba32_visit;
  header get_header();
  int read_header();
  int read_data();
  int read_P6();
  int read_P5();
  int read_P4();
  int read_P3();
  int read_P2();
  int read_P1();
};



class pbmwriter{
public:
  pbmwriter();
  pbmwriter(const pbmwriter&)=delete;
  pbmwriter& operator=(const pbmwriter&)=delete;
  pbmwriter(pbmwriter&&) = delete;
  pbmwriter& operator=(pbmwriter&&)=delete;

  pbmwriter(const std::string& filename,generic_pbm&& gen);
  pbmwriter(const std::string& filename,const generic_pbm& gen);
  pbmwriter(const std::string& filename,converter::pixbuff::rgba32&& rgba32_buff);
  pbmwriter(const std::string& filename,const converter::pixbuff::rgba32& rgba32_buff);
  int write_gen();
  int write_rgba32_to_p1();
  int write_rgba32_to_p2();
  int write_rgba32_to_p3();
  int write_rgba32_to_p4();
  int write_rgba32_to_p5();
  int write_rgba32_to_p6();

  int open(const std::string& filename,generic_pbm&& gen);
  int open(const std::string& filename, const generic_pbm& gen);
  int open(const std::string& filename,converter::pixbuff::rgba32&& rgba32_buff);
  int open(const std::string& filename, const converter::pixbuff::rgba32& rgba32_buff);
  bool is_open() const;
  int close();
private:
  std::string filename_;
  std::ofstream file_;
  generic_pbm gen_;
  converter::pixbuff::rgba32 rgba32_buff_;
  bool gen_input_{false};
  bool rgba32_input_{false};
  struct pixel_data_write_gen_visit;
};


}//endnamespace
