#pragma once
#include <vector>
#include <concepts>
#include <type_traits>
#include <initializer_list>
#include <cstdint>
#include <limits>
#include <ostream>



namespace converter::pixbuff{

struct sRGB_tag{
  static const size_t channels = 3;
};

struct greyscale_tag{
  static const size_t channels = 1;
};

struct yes_alpha_tag{};
struct no_alpha_tag{};

template <typename colorspace_tmpl, typename enable_alpha_tmpl, std::integral bitdepth_tmpl>
class picture_data{
public:
  using colorspace = colorspace_tmpl;
  using alpha_tag = enable_alpha_tmpl;
  using component_type = bitdepth_tmpl;

  static constexpr component_type max_value(){
    return std::numeric_limits<component_type>::max();
  }
  static constexpr size_t channels() {
    if constexpr (std::is_same_v<alpha_tag, yes_alpha_tag>) {
      return colorspace::channels +1;
    } else if constexpr (std::is_same_v<alpha_tag, no_alpha_tag>){
      return colorspace::channels;
    } else { static_assert(false," template error : invalid alpha tag");
      return 0;
    }
  }

  struct pixel{
    component_type p_data_[channels()];
    pixel(std::initializer_list<component_type> init) {
      std::fill(std::begin(p_data_),std::end(p_data_),0);
      if(init.size()>channels())[[unlikely]]{
        throw std::out_of_range("Initializer list wrong size");
      }
      std::copy(init.begin(),init.end(),p_data_);
    }

    pixel() : pixel(std::initializer_list<component_type>{0,0,0,0}) {}

    component_type& operator[](size_t index){
      if (index>channels())[[unlikely]]{
        throw std::out_of_range("Index out of range");
      }
      return p_data_[index];
    }

    const component_type& operator[](size_t index) const{
      if (index>channels())[[unlikely]]{
        throw std::out_of_range("Index out of range");
      }
      return p_data_[index];
    }


    template<typename... Args>
    pixel(Args... args) : pixel{static_cast<component_type>(args)...} {
    }

    void print(){
      for(auto& e : p_data_){
        std::cout << static_cast<int64_t>(e) << " ";
      }
    }
  };

  picture_data(size_t width,size_t height)
    : width_(width), height_(height), capacity_(width_*height_){
    data_.reserve(capacity_);
  };

  picture_data()
    : width_(0), height_(0), capacity_(0){
  };

  size_t get_width() const{
    return width_;
  }

  size_t get_height() const{
    return height_;
  }

  size_t get_size() const{
    return data_.size();
  }

  size_t get_capacity() const{
    return capacity_;
  }

  pixel& operator[](size_t index){
    if (index>capacity_)[[unlikely]]{
      throw std::out_of_range("Index out of range");
    }
    return data_[index];
  }

  const pixel& operator[](size_t index) const{
    if (index>capacity_)[[unlikely]]{
      throw std::out_of_range("Index out of range");
    }
    return data_[index];
  }

  template <typename... Args>
  int emplace_back(Args&&... args) {
    if (get_size()>=capacity_)[[unlikely]]{
      return -1;
    }
    data_.emplace_back(std::forward<Args>(args)...);
    return 0;
  }

  void print(){
      std::cout << "Width: " << width_ << " Height: " << height_ << std::endl;
    for(int i = 0; i != data_.size();++i){
      std::cout << "Pixel " << i << ": ";
      data_[i].print();
      std::cout << std::endl;
    }
  }


  std::vector<pixel> data_;
  private:
  size_t width_;
  size_t height_;
  size_t capacity_;


};



template <typename picture_data>
struct picture_data_traits{
  using colorspace = typename picture_data::colorspace;
  using alpha_tag = typename picture_data::alpha_tag;
  using component_type = typename picture_data::component_type; //example uint8_t uint16_t, bits per colorspace component
  using pixel_type = typename picture_data::pixel;
};


using rgba32 = picture_data<sRGB_tag, yes_alpha_tag, uint8_t>;


} //end namespace


