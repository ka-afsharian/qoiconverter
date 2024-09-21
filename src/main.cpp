#include "converter/converter.hpp"
#include "converter/utils.hpp"
#include <cassert>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <future>
#include <filesystem>


using namespace converter;
int func(const std::string& qoifile,const std::string& ppmdest){
  //auto start = std::chrono::high_resolution_clock::now();
  netpbm::pbmwriter pbmw;
  qoi::qoireader qoir;
  qoir.open(qoifile);
  if(qoir.read()==0){
    auto g = qoir.get_rgba32();
    pbmw.open(ppmdest,std::move(g));
    return pbmw.write_rgba32_to_p6();
  }
  return -1;
}

int main(int argc, char* argv[]){
if (argc < 2 || argc > 3){
  std::cout << "Incorrect Input";
  return -1;
}

if(argc==2){
  std::filesystem::path path = argv[1];
    std::cout << path.string() << std::endl;

  if(!std::filesystem::exists(path)){
    std::cout << "Incorrect path" << std::endl;
    return -1;
  }

  if(std::filesystem::is_directory(path)){
    std::vector<std::future<int>> results;
    results.reserve(std::thread::hardware_concurrency());
    for(const auto& e : std::filesystem::directory_iterator(path)){//maybe sort by file size before iterating
      std::string filename = e.path().string();
      if(e.path().extension() == ".qoi"){
        results.emplace_back(std::async(std::launch::async,func,filename,
                                        (e.path().parent_path()/e.path().stem()).string()+".ppm"));
      }
    }
    int i =0;
    for(auto& e : results){
      if(e.get()==-1)
        i++;
    }
    std::cout << i << " ERROR(S)" << std::endl;
  }else{
    if(func(path.string(),(path.parent_path()/path.stem()).string()+".ppm")==-1){
    std::cout <<"ERROR" << std::endl;
    }
  }
 }
 return 0;
}


































//  utils::timer timea;
//  timea.set();

//  std::future<void> result = std::async(std::launch::async,func,"./sun.qoi","./sun.ppm");

//  result.get();
  //result2.get();
//  std::cout << *timea.get() << std::endl;
  //auto he=test.get_header();
  //he.print();
  //auto g = test.get_rgba32();
  //test2.write_rgba32_to_p6();
  /*
  t.read();
  pixbuff::rgba32 g = t.get_rgba32();
  //auto g = test.get_generic_pbm();
  test2.write_rgba32_to_p6();
  */
  /*
  {
    using namespace converter::pixbuff; //for printing
  std::cout << a[0][1] << std::endl;;
  std::cout << t;
  }
  */
  //std::cout << static_cast<int>(a.max_value());
  //std::cout << a.data_.size();
 //std::cout << endian_utils::sys_endianness;
 //int a = 1;
 //auto b = endian_utils::reverse_byte_order(a);
// uint16_t c = 0x234;
// auto d = endian_utils::to_sys_endianness<endian_utils::endianness::BIG>(c);
// std::cout <<  std::hex << d << std::endl;
//
//
//

/*
utils::timer timee{};
timee.set();
auto i = test.read();

auto g = test.convert_to_P6();
netpbm::pbmwriter writer{};
writer.write();
}
auto endt = timee.get();
std::cout << endt.value() << std::endl;
test.close();
writer.close();
*/
