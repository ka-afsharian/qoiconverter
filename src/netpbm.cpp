#include "converter/netpbm.hpp"
#include <bitset>
#include <charconv>


using namespace converter::netpbm;

pbmreader::pbmreader(const std::string& filename)
  :filename_(filename), file_(filename,std::ios::binary){
  header_={"","","","",""};
}

int pbmreader::open(std::string& filename){
  if(this->is_open()){
    this->close();
  }
  filename_=filename;
  file_.open(filename_,std::ios::binary);
  if(!this->is_open()){
    //open fail
    return -1;
  }
  return 0;
}

int pbmreader::read_header(){
  if(!is_open()){
    //not open
    return -1;
  }
  file_.seekg(0, std::ios::beg);

  char magicchars[3];
  magicchars[2]=0;//null char to help convert array to string
  if(!file_.read(magicchars,2)){
    //file too small
    return -1;
  }
  std::string magic_number=std::string(magicchars);
  char ch;
  //if there isn't a space character after magic number, header invalid
  if(!std::isspace(static_cast<unsigned char>(file_.get()))){
    //header incorrect
    return -1;
  }
  //skip over all spaces
  while(std::isspace(static_cast<unsigned char>(file_.peek()))){
    file_.get();
  }

  std::string line;
  std::string comments{""};
  //grab all lines starting with # as a comment
  while(static_cast<unsigned char>(file_.peek())=='#'){
    std::getline(file_,line);
    comments += line;
    comments +='\n';
  }
  if(comments.size()){
  comments.pop_back(); //remove last \n
  }

  std::string width;
  std::string height;
  std::string max_value;
  //depending on magic number, header members are different
  if(magic_number=="P2" ||
     magic_number=="P5" ||
     magic_number=="P3" ||
     magic_number=="P6"){
    file_ >> width;
    file_ >> height;
    file_ >> max_value;
    for(auto& e : {width,height,max_value}){
      int int_test;
      auto [ptr,err] = std::from_chars(e.data(),e.data()+e.size(),int_test);
      if(ptr!=e.data()+e.size()){
        return -1;
      }
    }
    if(width.size()==0 ||
       height.size()==0||
       max_value.size()==0){
      //header incorrect
      return -1;
    }
  }else if(magic_number=="P1" ||
     magic_number=="P4"){
    file_ >> width;
    file_ >> height;
    for(auto& e : {width,height}){
      int int_test;
      auto [ptr,err] = std::from_chars(e.data(),e.data()+e.size(),int_test);
      if(ptr!=e.data()+e.size()){
        return -1;
      }
    }
    if(width.size()==0 ||
       height.size()==0){
      return -1;
    }
  }else{
    return -1;
  }
  //finally write to header member, no return -1 after this point
  header_={magic_number,comments,width,height,max_value};
  //consume char, usually newline or space
  file_.get(ch);
  //tellg should now be at beginning of data
  is_read_header_=true;
  data_offset_=file_.tellg();
  file_.seekg(0, std::ios::beg);
  return 1;
}

int pbmreader::read_data(){
  if(!is_read_header_ || !this->is_open()){
    return -1;
  }
  file_.seekg(data_offset_);
  if(header_.magic_number=="P6"){
    if(this->read_P6()==-1){
      return -1;
    };
  }else if(header_.magic_number=="P5"){
    if(this->read_P5()==-1){
      return -1;
    };
  }else if(header_.magic_number=="P4"){
    if(this->read_P4()==-1){
      return -1;
    };
  }else if(header_.magic_number=="P3"){
    if(this->read_P3()==-1){
      return -1;
    }
  }else if(header_.magic_number=="P2"){
    if(this->read_P2()==-1){
      return -1;
    }
  }else if(header_.magic_number=="P1"){
    if(this->read_P1()==-1){
      return -1;
    }
  }else{
    //write for other cases later
    return -1;
  }

  is_read_data_=true;
  return 0;
}


struct pbmreader::pixel_data_print_visit{
  void operator()(P6_pixel_data data ){
    data.print();
  }
  void operator()(P5_pixel_data data ){
    data.print();
  }
  void operator()(P4_pixel_data data ){
    data.print();
  }
  void operator()(P3_pixel_data data ){
    data.print();
  }
  void operator()(P2_pixel_data data ){
    data.print();
  }
  void operator()(P1_pixel_data data ){
    data.print();
  }
};

int pbmreader::read_P1(){
  size_t size_of_data;
  size_of_data=(std::stoul(header_.width)*std::stoul(header_.height));
  std::string black;
  P1_pixel_data datatemp(stoul(header_.height),stoul(header_.width));
  datatemp.data_.reserve(size_of_data);
  for(size_t i = size_of_data; i > 0 ; --i){
    file_ >> black;
    if(file_.fail()){
      return -1;
    }
    P1_pixel pixeltemp{black};
    datatemp.data_.push_back(pixeltemp);
  }
  //uncomment later
  pixel_data_=std::move(datatemp);
  return 0;
}

int pbmreader::read_P2(){
  size_t size_of_data;
  size_of_data=(std::stoul(header_.width)*std::stoul(header_.height));
  std::string grey;
  P2_pixel_data datatemp(stoul(header_.height),stoul(header_.width),stoul(header_.max_value));
  datatemp.data_.reserve(size_of_data);
  for(size_t i = size_of_data; i > 0 ; --i){
    file_ >> grey;
    if(file_.fail()){
      return -1;
    }
    P2_pixel pixeltemp{grey};
    datatemp.data_.push_back(std::move(pixeltemp));
  }
  //uncomment later
  pixel_data_=std::move(datatemp);
  return 0;
}


int pbmreader::read_P3(){
  size_t size_of_data;
  size_of_data=(std::stoul(header_.width)*std::stoul(header_.height));
  std::string red, green, blue;
  P3_pixel_data datatemp(stoul(header_.height),stoul(header_.width),stoul(header_.max_value));
  datatemp.data_.reserve(size_of_data);
  for(size_t i = size_of_data; i > 0 ; --i){
    file_ >> red >> green >> blue;
    if(file_.fail()){
      return -1;
    }
    P3_pixel pixeltemp{red,green,blue};
    datatemp.data_.push_back(pixeltemp);
  }
  //uncomment later
  pixel_data_=std::move(datatemp);
  return 0;
}

int pbmreader::read_P4(){
  //this function adds extra "pixels" onto the end of rows if pixels in rows doesn't divide evenly by 8
  size_t size_of_row=(std::stoul(header_.width)+7)/8;
  size_t size_of_array=size_of_row*std::stoul(header_.height);

  std::vector<char> charbuff(size_of_array);
  if(!file_.read(charbuff.data(),size_of_array)){
    //fail read
    return -1;
  }
  //the datatemp will still contain true width
  P4_pixel_data datatemp(stoul(header_.height),stoul(header_.width));
  datatemp.data_.reserve(size_of_array*8);
  for(auto it = charbuff.cbegin(); it!=charbuff.cend();it++){
    std::bitset<8> bits(*it);
    for(size_t i = bits.size(); i > 0; --i){
      P4_pixel pixeltemp{bits[i-1]};
      datatemp.data_.push_back(pixeltemp);
    }
  }
  pixel_data_=std::move(datatemp);
  //std::visit(pixel_data_print_visit{},pixel_data_);
  return 0;
}

int pbmreader::read_P5(){
  size_t size_of_array;
  size_of_array=std::stoul(header_.width)*std::stoul(header_.height);
  std::vector<char> charbuff(size_of_array);
  if(!file_.read(charbuff.data(),size_of_array)){
    //fail read
    return -1;
  }
  P5_pixel_data datatemp(stoul(header_.height),stoul(header_.width),stoul(header_.max_value));
  datatemp.data_.reserve(size_of_array);
  for(auto it = charbuff.cbegin(); it!=charbuff.cend();it++){
    //emplace_back is a little faster
    //P5_pixel pixeltemp{static_cast<uint8_t>(*it)};
    //datatemp.data_.push_back(pixeltemp);
    datatemp.data_.emplace_back(static_cast<uint8_t>(*it));
  }
  pixel_data_=std::move(datatemp);
  return 0;
}



int pbmreader::read_P6(){
  size_t size_of_array;
  size_of_array=std::stoul(header_.width)*std::stoul(header_.height)*3;
  std::vector<char> charbuff(size_of_array);
  if(!file_.read(charbuff.data(),size_of_array)){
    //fail read
    return -1;
  }
  P6_pixel_data datatemp(stoul(header_.height),stoul(header_.width),stoul(header_.max_value));
  datatemp.data_.reserve(size_of_array/3);
  for(auto it = charbuff.cbegin(); it!=charbuff.cend();){
    P6_pixel pixeltemp{static_cast<uint8_t>(*it),
                       static_cast<uint8_t>(*(it+1)),
                       static_cast<uint8_t>(*(it+2))};
    datatemp.data_.push_back(pixeltemp);
    std::advance(it,3);
  }
  pixel_data_=std::move(datatemp);
  //std::visit(pixel_data_print_visit{},pixel_data_);
  return 0;
}

int pbmreader::read(){
  if(this->read_header() == -1){
    //invalid header;
    return -1;
  }else if(this->read_data() == -1){
    //invalid data
    return -1;
  }
  return 0;
}

header pbmreader::get_header(){
  return header_;
}

int pbmreader::close(){
  header_={"","","","",""};
  filename_="";
  data_offset_=0;
  is_read_data_=false;
  is_read_header_=false;
  pixel_data_={};
  file_.close();
  return 0;
}

bool pbmreader::is_open() const{
  return file_.is_open();

}



size_t pbmreader::get_file_size(){
  if(!is_open()){
    //no file open
    return 0;
  }
  file_.seekg(0, std::ios::end);
  auto filesize = file_.tellg();
  file_.seekg(0, std::ios::beg);
  return static_cast<size_t>(filesize);
}



struct pbmreader::pixel_data_to_P6_visit{
  const header& headertemp;

  generic_pbm operator()(P6_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t valuered = static_cast<uint8_t>(e.red*scale_factor);
      uint8_t valuegreen = static_cast<uint8_t>(e.green*scale_factor);
      uint8_t valueblue = static_cast<uint8_t>(e.blue*scale_factor);
      temp.data_.emplace_back(valuered,valuegreen,valueblue);
    }
    return {std::move(headertemp),std::move(temp)};
  }

  generic_pbm operator()(const P5_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    for(auto& e : data.data_){
      uint8_t value = static_cast<uint8_t>(255*static_cast<double>(e.grey)/static_cast<double>(data.max_val_));
      temp.data_.emplace_back(value,value,value);
    }
    return {std::move(headertemp),std::move(temp)};
  }

  generic_pbm operator()(P4_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    for(auto& e : data.data_){
      uint8_t value = (255*(!e.black));
      temp.data_.emplace_back(value,value,value);
    }
    return {std::move(headertemp),std::move(temp)};
  }

  generic_pbm operator()(P3_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    for(auto& e : data.data_){
      uint8_t valuered = static_cast<uint8_t>(255*static_cast<double>(stoul(e.red))/static_cast<double>(data.max_val_));
      uint8_t valuegreen = static_cast<uint8_t>(255*static_cast<double>(stoul(e.green))/static_cast<double>(data.max_val_));
      uint8_t valueblue = static_cast<uint8_t>(255*static_cast<double>(stoul(e.blue))/static_cast<double>(data.max_val_));
      temp.data_.emplace_back(valuered,valuegreen,valueblue);
    }
    return {std::move(headertemp),std::move(temp)};
  }

  //fix other functions like this one, with scale factor and emplace_back
  generic_pbm operator()(const P2_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t valuegrey = static_cast<uint8_t>(static_cast<double>(stoul(e.grey))*scale_factor);
      temp.data_.emplace_back(valuegrey,valuegrey,valuegrey);
    }
    return {std::move(headertemp),std::move(temp)};
  }

  generic_pbm operator()(P1_pixel_data& data ){
    P6_pixel_data temp{data.height_,data.width_,255};
    size_t length = temp.height_*temp.width_;
    temp.data_.reserve(length);
    for(auto& e : data.data_){
      uint8_t valuegrey = static_cast<uint8_t>(255*(stoul(e.black)));
      temp.data_.emplace_back(valuegrey);
    }
    return {std::move(headertemp),std::move(temp)};
  }
};

generic_pbm pbmreader::convert_to_P6(){
  if(!is_read_data_){
    return {};
  }
  std::string magic_number = header_.magic_number;
  header headertmp;
  headertmp.magic_number = "P6";
  headertmp.comments = header_.comments;
  headertmp.width= header_.width;
  headertmp.height= header_.height;
  headertmp.max_value= "255";
  pixel_data_to_P6_visit visitor{headertmp};
  return std::visit(visitor,pixel_data_);
}

struct pbmreader::pixel_data_to_rgba32_visit{
  const header& headertemp;

  converter::pixbuff::rgba32 operator()(P6_pixel_data& data ){
    converter::pixbuff::rgba32 temp(data.width_,data.height_);
    //size_t length = data.height_*data.width_;
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t valuered = static_cast<uint8_t>(e.red*scale_factor);
      uint8_t valuegreen = static_cast<uint8_t>(e.green*scale_factor);
      uint8_t valueblue = static_cast<uint8_t>(e.blue*scale_factor);
      temp.emplace_back(valuered,valuegreen,valueblue,0);
    }
    return temp;//add move
  }

  converter::pixbuff::rgba32 operator()(const P5_pixel_data& data ){
    converter::pixbuff::rgba32 temp(data.width_,data.height_);
    //size_t length = data.height_*data.width_;
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t value = static_cast<uint8_t>(e.grey*scale_factor);
      temp.emplace_back(value,value,value,0);
    }
    return temp;
  }

  converter::pixbuff::rgba32 operator()(P4_pixel_data& data ){ //remember p4 pixel data is in wierd from with extra "pixels"
    size_t width = data.width_;  //true width
    size_t height = data.height_;
    size_t size_of_row=(width+7)/8; //bytes every row takes up
    //size_t size_of_array=size_of_row*data.height_; // total bytes
    converter::pixbuff::rgba32 temp(width,height);
    //size_t length = data.height_*data.width_;
    for(size_t i=0; i!=height ; ++i){                 //count what row im on
      for(size_t j=0; j!=size_of_row*8;++j){          //count what column im on, counting extra "pixels"
        if(j>=width){                              //if column is on these "pixels" skip, don't emplace
          continue;
        }
        uint8_t value = (255*(!data.data_[i*size_of_row*8 + j].black)); //get value in 0-255 form
        temp.emplace_back(value,value,value,0);                         //emplace value onto rgba32, which is in good form
      }
    }
    return temp;
  }

  converter::pixbuff::rgba32 operator()(P3_pixel_data& data ){
    converter::pixbuff::rgba32 temp(data.width_,data.height_);
    //size_t length = data.height_*data.width_;
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t valuered = static_cast<uint8_t>(static_cast<double>(stoul(e.red))*scale_factor);
      uint8_t valuegreen = static_cast<uint8_t>(static_cast<double>(stoul(e.green))*scale_factor);
      uint8_t valueblue = static_cast<uint8_t>(static_cast<double>(stoul(e.blue))*scale_factor);
      temp.emplace_back(valuered,valuegreen,valueblue,0);
    }
    return temp;
  }

  converter::pixbuff::rgba32 operator()(const P2_pixel_data& data ){
    converter::pixbuff::rgba32 temp(data.width_,data.height_);
    //size_t length = data.height_*data.width_;
    double scale_factor = 255.0/static_cast<double>(data.max_val_);
    for(auto& e : data.data_){
      uint8_t valuegrey = static_cast<uint8_t>(static_cast<double>(stoul(e.grey))*scale_factor);
      temp.emplace_back(valuegrey,valuegrey,valuegrey,0);
    }
    return temp;
  }

  converter::pixbuff::rgba32 operator()(P1_pixel_data& data ){
    converter::pixbuff::rgba32 temp(data.width_,data.height_);
    //size_t length = data.height_*data.width_;
    for(auto& e : data.data_){
      uint8_t valueblack = static_cast<uint8_t>(255*(stoul(e.black)));
      temp.emplace_back(valueblack,valueblack,valueblack,0);
    }
    return temp;
  }
};

converter::pixbuff::rgba32 pbmreader::get_rgba32(){
  if(!is_read_data_){
    return {};
  }
  pixel_data_to_rgba32_visit visitor{header_};
  return std::visit(visitor,pixel_data_);
}


generic_pbm pbmreader::get_generic_pbm(){
  return {get_header(),pixel_data_};
}

void pbmreader::print(){
  if(!this->is_read_data_ || !this->is_read_header_){
    std::cout << "No data";
  }else{
    this->get_header().print();
    std::visit(pixel_data_print_visit{},this->pixel_data_);
  }
}

//pbmwriter
pbmwriter::pbmwriter(){};

pbmwriter::pbmwriter(const std::string& filename,generic_pbm&& gen)
  :filename_(filename),file_(filename_),gen_(std::move(gen)),gen_input_(true){
}

pbmwriter::pbmwriter(const std::string& filename,const generic_pbm& gen)
  :filename_(filename),file_(filename_),gen_(gen),gen_input_(true){
}

pbmwriter::pbmwriter(const std::string& filename,converter::pixbuff::rgba32&& rgba32_buff)
  :filename_(filename),file_(filename_),rgba32_buff_(std::move(rgba32_buff)),rgba32_input_(true){
  rgba32_input_=true;
}

pbmwriter::pbmwriter(const std::string& filename,const converter::pixbuff::rgba32& rgba32_buff)
  :filename_(filename),file_(filename_),rgba32_buff_(rgba32_buff),rgba32_input_(true){
}

struct pbmwriter::pixel_data_write_gen_visit{
  std::ofstream& file_; //struct needs file_ so it can write to file

  void operator()(P6_pixel_data& varipd) {
    std::vector<char> buffer;
    buffer.reserve(varipd.data_.size() * 3);  // 3 bytes per pixel (RGB)
    for (const auto& e : varipd.data_) {
      buffer.push_back(static_cast<char>(e.red));
      buffer.push_back(static_cast<char>(e.green));
      buffer.push_back(static_cast<char>(e.blue));
    }
    file_.write(buffer.data(), buffer.size());
    file_ << " "<< std::flush;
  }

  void operator()(const P5_pixel_data& varipd ){
    std::vector<char> buffer;
    buffer.reserve(varipd.data_.size());  // 3 bytes per pixel (RGB)
    for(const auto& e : varipd.data_){
      buffer.push_back(static_cast<char>(e.grey));
    }
    file_.write(buffer.data(), buffer.size());
    file_ << " "<< std::flush;
  }

  void operator()(const P4_pixel_data& varipd ){
    //remember P4_pixel_data has extra white "pixels" at the right of each row, corresponding to missing pixels %8
    size_t length = varipd.height_*varipd.width_;
    std::vector<char> buffer;
    buffer.reserve(length);
    for(size_t i= 0 ; i != length ; ++i){
      std::bitset<8> bits;
      int j =0;
      for(auto e = varipd.data_.cbegin() + i*8; e != varipd.data_.cbegin() + (i*8) + 8;
          e++){
        if(e < varipd.data_.cend() && e->black){
          bits.set(7-j);
        }
        j++;
      }
      unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
      //file_.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
      buffer.push_back(static_cast<char>(byte));
    }
    file_.write(buffer.data(),buffer.size());
    file_ << std::flush;
  }

  void operator()(const P3_pixel_data& varipd ){
    std::string buffer;
    buffer.reserve(varipd.data_.size()*12);//3 each color + 3 space
    for(const auto& e : varipd.data_){
      buffer += e.red + " " + e.green + " " + e.blue + " ";
      buffer += ' ';
    }
    file_ << buffer;
    file_ << std::flush;
  }
  void operator()(const P2_pixel_data& varipd ){
    std::string buffer;
    buffer.reserve(varipd.data_.size()*4);//4 arbirary
    for(const auto& e : varipd.data_){
      buffer += e.grey;
      buffer += ' ';
    }
    file_ << buffer;
    file_ << std::flush;
  }
  void operator()(const P1_pixel_data& varipd ){
    std::string buffer;
    buffer.reserve(varipd.data_.size()*2);
    for(const auto& e : varipd.data_){
      buffer += e.black;
      buffer += ' ';
    }
    file_ << buffer;
    file_ << std::flush;
  }
};

int pbmwriter::write_gen(){
  if(!this->is_open()){
    //failed open
    return -1;
  }

  if(gen_input_==false){
    return -1;
  }

  std::string magic_number = gen_.header_.magic_number;
  file_ << magic_number << "\n";
  if(!gen_.header_.comments.empty()){
    file_ << gen_.header_.comments << "\n";
  }
  if(magic_number=="P2" ||
     magic_number=="P5" ||
     magic_number=="P3" ||
     magic_number=="P6"){
    file_ << gen_.header_.width << " " << gen_.header_.height << "\n" <<
      gen_.header_.max_value << "\n";
  }else if(magic_number=="P1" ||
     magic_number=="P4"){
    file_ << gen_.header_.width << " " << gen_.header_.height << "\n";
  }

  pixel_data_write_gen_visit visitor_gen(file_);// create visitor
  pixels_data& varipd = gen_.pixel_data_; // create variant
  std::visit(visitor_gen,varipd);

  return 0;
}


int pbmwriter::write_rgba32_to_p6(){
  file_ << "P6" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  file_ << static_cast<int>(rgba32_buff_.max_value()) << "\n";
  std::vector<char> buffer;
  buffer.reserve(rgba32_buff_.get_size() * 3);  // 3 bytes per pixel (RGB)
  for (size_t i = 0; i != rgba32_buff_.get_size(); ++i) {
    buffer.push_back(static_cast<char>(rgba32_buff_[i][0]));
    buffer.push_back(static_cast<char>(rgba32_buff_[i][1]));
    buffer.push_back(static_cast<char>(rgba32_buff_[i][2]));
  }
  file_.write(buffer.data(), buffer.size());
  file_ << " "<< std::flush;
  return 0;
}

int pbmwriter::write_rgba32_to_p5(){
  file_ << "P5" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  file_ << static_cast<int>(rgba32_buff_.max_value()) << "\n";
  std::vector<char> buffer;
  buffer.reserve(rgba32_buff_.get_size());  // 3 bytes per pixel (RGB)
  for (size_t i = 0; i != rgba32_buff_.get_size(); ++i) {
    buffer.push_back(static_cast<char>((rgba32_buff_[i][0]+rgba32_buff_[i][1]+rgba32_buff_[i][2])/3));
  }
  file_.write(buffer.data(), buffer.size());
  file_ << " "<< std::flush;
  return 0;
}

// this sucks
int pbmwriter::write_rgba32_to_p4(){
  size_t size_of_row=(rgba32_buff_.get_width()+7)/8; //number of bytes per row
  size_t size_of_array=size_of_row*rgba32_buff_.get_height(); //number of bytes total;
  file_ << "P4" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  size_t cutoff = rgba32_buff_.max_value()/2 -13;
  std::vector<char> buffer;
  buffer.reserve(size_of_array);  //prevents vector reallocate
  size_t tindex=0;                //true index in terms of rgba32 doesn't count extra "pixels"
  for (size_t i = 0; i != rgba32_buff_.get_height(); ++i) {         //counts row im on
      for(size_t j = 0; j != size_of_row; ++j){                     //counts which byte im on of column
        std::bitset<8> bits;
        for(size_t k = 0; k !=8; ++k){                              //iterate through bits of byte
          size_t rindex= j*8 +k;                                 //bit index of row, index counts extra "pixels"
          if(rindex >= rgba32_buff_.get_width()){                //if rindex is on extra pixels, skip iter
            continue;
          }
          if(tindex < rgba32_buff_.get_size() &&                //checks if true index will not throw, maybe not neccessary
             (static_cast<size_t>(rgba32_buff_[tindex][0]+rgba32_buff_[tindex][1]+rgba32_buff_[tindex][2])/3) < cutoff){
            bits.set(7-k);
          }
          ++tindex; //iterate true index now that we used it
        }
        buffer.emplace_back(static_cast<char>(bits.to_ulong()));
      }
  }
  file_.write(buffer.data(), buffer.size());
  file_ << " "<< std::flush;
  return 0;
}

int pbmwriter::write_rgba32_to_p3(){
  file_ << "P3" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  file_ << static_cast<int>(rgba32_buff_.max_value()) << "\n";
  for (size_t i = 0; i != rgba32_buff_.get_size(); ++i) {
    file_ << static_cast<int>(rgba32_buff_[i][0]) << " ";
    file_ << static_cast<int>(rgba32_buff_[i][1]) << " ";
    file_ << static_cast<int>(rgba32_buff_[i][2]) << " ";
  }
  return 0;
}

int pbmwriter::write_rgba32_to_p2(){
  file_ << "P2" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  file_ << static_cast<int>(rgba32_buff_.max_value()) << "\n";
  for (size_t i = 0; i != rgba32_buff_.get_size(); ++i) {
    file_ << static_cast<int>((rgba32_buff_[i][0]+rgba32_buff_[i][1]+rgba32_buff_[i][2])/3) << " ";
  }
  return 0;
}

int pbmwriter::write_rgba32_to_p1(){
  file_ << "P1" << "\n" << rgba32_buff_.get_width() << " " << rgba32_buff_.get_height() << "\n";
  size_t cutoff = rgba32_buff_.max_value()/2 -13;
  for (size_t i = 0; i != rgba32_buff_.get_size(); ++i) {
    if(static_cast<size_t>(rgba32_buff_[i][0]+rgba32_buff_[i][1]+rgba32_buff_[i][2])/3 < cutoff)
      file_ << 1 << " ";
    else
      file_ << 0 << " ";
  }
  return 0;
}

bool pbmwriter::is_open() const{
  return file_.is_open();
}


int pbmwriter::close(){
  this->filename_="";
  generic_pbm datatemp;
  this->gen_=datatemp;
  this->rgba32_buff_=converter::pixbuff::rgba32();
  this->file_.close();
  this->gen_input_=false;
  this->rgba32_input_=false;
  return 0;

}


int pbmwriter::open(const std::string& filename,generic_pbm&& gen){
  if(this->is_open()){
    this->close();
  }
  filename_=filename;
  file_.open(filename_);
  gen_input_=true;
  if(!this->is_open()){
    //open fail
    return -1;
  }
  gen_=std::move(gen);
  return 0;
}

int pbmwriter::open(const std::string& filename,const generic_pbm& gen){
  if(this->is_open()){
    this->close();
  }
  filename_=filename;
  file_.open(filename_);
  gen_input_=true;
  if(!this->is_open()){
    //open fail
    return -1;
  }
  gen_=gen;
  return 0;
}

int pbmwriter::open(const std::string& filename,converter::pixbuff::rgba32&& rgba32_buff){
  if(this->is_open()){
    this->close();
  }
  filename_=filename;
  file_.open(filename_);
  rgba32_input_=true;
  if(!this->is_open()){
    //open fail
    return -1;
  }
  rgba32_buff_=std::move(rgba32_buff);
  return 0;
}

int pbmwriter::open(const std::string& filename,const converter::pixbuff::rgba32& rgba32_buff){
  if(this->is_open()){
    this->close();
  }
  filename_=filename;
  file_.open(filename_);
  rgba32_input_=true;
  if(!this->is_open()){
    //open fail
    return -1;
  }
  rgba32_buff_=rgba32_buff;
  return 0;
}
