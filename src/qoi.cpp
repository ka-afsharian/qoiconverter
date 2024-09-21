#include "converter/qoi.hpp"
#include "converter/endian_utils.hpp"
#include <cstring>
#include <bitset>
#include <array>

using namespace converter::qoi;

qoireader::qoireader(const std::string& filename):filename_(filename),file_(filename,std::ios::binary){
}

int qoireader::open(const std::string& filename){
  if(is_open()){
    close();
  }
  filename_=filename;
  file_.open(filename_,std::ios::binary);
  if(!is_open()){
    //open fail
    return -1;
  }
  return 0;
}

void header::print(){
   std::cout.write(magic,sizeof(magic));
   std::cout << "\n";
   std::cout << "Width: "<< width << "\n"
     << "Height: "<< height << "\n"
     << "Channels: "<< static_cast<int>(channels) << "\n"
     << "Colorspace: "<< static_cast<int>(colorspace) << std::endl;
  }


int qoireader::read_header(){
  if(!is_open()){
    //not open
    return -1;
  }
  file_.seekg(0, std::ios::beg);

  std::vector<char> headerbuff;
  headerbuff.reserve(14);
  headerbuff.resize(14);
  file_.read(headerbuff.data(),14);
  if(!file_){
    return -1;
  }

  data_offset_=file_.tellg();

  std::memcpy(header_.magic,&headerbuff[0],4);
  std::memcpy(&header_.width,&headerbuff[4],4);
  std::memcpy(&header_.height,&headerbuff[8],4);
  std::memcpy(&header_.channels,&headerbuff[12],1);
  std::memcpy(&header_.colorspace,&headerbuff[13],1);
  header_.width = converter::endian_utils::from_big_endianness(header_.width);
  header_.height = converter::endian_utils::from_big_endianness(header_.height);

  is_read_header_=true;
  return 0;
  }

header qoireader::get_header(){
  if(read_header()==-1){
    return {};
  }
  return header_;
};


bool qoireader::is_open() const{
  return file_.is_open();
}

int qoireader::close(){
  header_={};
  filename_="";
  picture_={};
  file_.close();
  data_offset_=0;
  is_read_data_=false;
  is_read_header_=false;
  is_decompressed_=false;
  compressed_data_={};
  return 0;

}

size_t qoireader::get_file_size(){
  if(!is_open()){
    //no file open
    return 0;
  }
  file_.seekg(0, std::ios::end);
  auto filesize = file_.tellg();
  file_.seekg(0, std::ios::beg);
  return static_cast<size_t>(filesize);
}



size_t qoireader::get_data_size(){
  if(!is_open()){
    return 0;
  }
  if(!is_read_header_){
    return 0;
  }
  return get_file_size() - data_offset_ - 8; //-8 because last byte is end of file mark
}


int qoireader::read(){
  if(read_header() == -1){
    //invalid header;
    return -1;
  }else if(read_data() == -1){
    //invalid data
    return -1;
  }else if(decompress() == -1){
    return -1;
  }
  return 0;
}

converter::pixbuff::rgba32 qoireader::get_rgba32(){
  //copy don't move
  return picture_;
};

int qoireader::read_data(){
  if(!is_open()){
    return -1;
  }
  if(!is_read_header_){
    return -1;
  }

  size_t data_size=get_data_size();
  if(data_size == 0){
    return -1;
  }

  compressed_data_.reserve(data_size); //allocate memory for all data
  compressed_data_.resize(data_size);  //fill that memory to its allocated size

  file_.seekg(data_offset_);

  file_.read(reinterpret_cast<char*>(compressed_data_.data()),data_size);

  if(!file_){
    return -1;
  }

  is_read_data_=true;

  return 0;

};

inline int qoireader::pixhash(const pixbuff::rgba32::pixel& pixel){
  return (pixel[0]*3 +pixel[1]*5 + pixel[2]*7 + pixel[3]*11)%64;
}

std::string converter::qoi::qoi_op_string(qoi_op op) {
    switch (op) {
        case rgb:
            return "rgb";
        case rgba:
            return "rgba";
        case index:
            return "index";
        case diff:
            return "diff";
        case luma:
            return "luma";
        case run:
            return "run";
        case error:
            return "error";
        default:
            return "unknown";
    }
}

qoi_op converter::qoi::det_qoi_op(const char& by){
  char test = (by >> 6) & 0b00000011;
  switch(test){
  case 0b11:                   //run or rgb or rgba
    if (static_cast<unsigned char>(by)==0b11111110){ //cast is neccessary
      return qoi_op::rgb;
    }else if(static_cast<unsigned char>(by)==0b11111111){
      return qoi_op::rgba;
    }else{
      return qoi_op::run;
    }
  case 0b00:
    return qoi_op::index;
  case 0b01:
    return qoi_op::diff;
  case 0b10:
    return qoi_op::luma;
  default:
    return qoi_op::error;
  }
}

int qoireader::decompress(){
  if(!is_read_data_){
      return -1;
    }

  picture_={header_.width,header_.height};

  std::array<converter::pixbuff::rgba32::pixel,64> arr;

  converter::pixbuff::rgba32::pixel previous_pixel = pixbuff::rgba32::pixel{0,0,0,255};
  arr[pixhash(previous_pixel)]=previous_pixel;

  for(size_t i =0; i != compressed_data_.size();){
    size_t runlength,iindex;
    int diffgreen,drmdg,dbmdg,dr,dg,db;

    char val;
    try{
      val = compressed_data_.at(i);
    } catch (const std::out_of_range& ex){ picture_={};return -1;}
    qoi_op op = det_qoi_op(val);

    switch(op){
    case qoi_op::run:
      runlength = static_cast<size_t>(val & 0b00111111) + 1;
      //don't change previous pixel
      //no need to add value to arr
      for(size_t j=0;j!=runlength;j++)
        picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      ++i;
      break;
    case qoi_op::index:
      iindex = val & 0b00111111;
      previous_pixel = arr[iindex];
      //no need to add value to arr, already in it
      picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      ++i;
      break;
    case qoi_op::luma:
      diffgreen = static_cast<int>(val & 0b00111111) - 32;
      try{
        val = compressed_data_.at(i+1); //if there is no data after i then file is incorrect
        } catch (const std::out_of_range& ex){ picture_={};return -1;}
      drmdg = static_cast<int>((val & 0b11110000) >> 4) - 8;
      dbmdg = static_cast<int>(val & 0b00001111) - 8;
      previous_pixel[0] = static_cast<uint8_t>(previous_pixel[0] + diffgreen + drmdg); //curpxr = drmdg +diffgreen + prevpxr
      previous_pixel[1] = static_cast<uint8_t>(previous_pixel[1] + diffgreen);
      previous_pixel[2] = static_cast<uint8_t>(previous_pixel[2] + diffgreen + dbmdg); //curpxb = dbmdg + diffgreen + prevpxb
      //alpha no change
      arr[pixhash(previous_pixel)]=previous_pixel;
      picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      i += 2; // length of data read is 2 bytes
      break;
    case qoi_op::diff:
      dr = static_cast<int>((val & 0b00110000) >> 4) - 2;
      dg = static_cast<int>((val & 0b00001100) >> 2) - 2;
      db = static_cast<int>(val & 0b00000011) - 2;
      previous_pixel[0] = static_cast<uint8_t>(previous_pixel[0] + dr);
      previous_pixel[1] = static_cast<uint8_t>(previous_pixel[1] + dg);
      previous_pixel[2] = static_cast<uint8_t>(previous_pixel[2] + db);
      //alpha no change
      arr[pixhash(previous_pixel)]=previous_pixel;
      picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      i +=1;
      break;
    case qoi_op::rgb:
      try{
        val = compressed_data_.at(i+1); //if there is no data after i then file is incorrect
        previous_pixel[0] = val;
        val = compressed_data_.at(i+2); //if there is no data after 2 then file is incorrect
        previous_pixel[1] = val;
        val = compressed_data_.at(i+3); //if there is no data after 3 then file is incorrect
        } catch (const std::out_of_range& ex){ picture_={};return -1;}
      previous_pixel[2] = val;
      arr[pixhash(previous_pixel)]=previous_pixel;
      picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      i +=4;
      break;
    case qoi_op::rgba:
      try{
        val = compressed_data_.at(i+1); //if there is no data after 1 then file is incorrect
        previous_pixel[0] = val;
        val = compressed_data_.at(i+2); //if there is no data after 2 then file is incorrect
        previous_pixel[1] = val;
        val = compressed_data_.at(i+3); //if there is no data after 3 then file is incorrect
        previous_pixel[2] = val;
        val = compressed_data_.at(i+4); //if there is no data after 4 then file is incorrect
        } catch (const std::out_of_range& ex){ picture_={}; return -1;}
      previous_pixel[3] = val;
      arr[pixhash(previous_pixel)]=previous_pixel;
      picture_.emplace_back(previous_pixel[0],previous_pixel[1],previous_pixel[2],previous_pixel[3]);
      i +=5;
      break;
    default:
      picture_ = {};
      return -1;
    }
  }

  if(picture_.get_size()!=picture_.get_width()*picture_.get_height()){
    picture_ = {};
    return -1;
  }

  return 0;
}






