#include "converter/converter.hpp"
#include "converter/utils.hpp"
#include <cassert>
#include <string>
#include <limits>
#include <iostream>
#include <thread>
#include <vector>
#include <optional>
#include <experimental/net>
#include <experimental/netfwd>
#include <ranges>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <coroutine>
#include <x86intrin.h>
#include <array>
#include <random>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <streambuf>
#include <ostream>
#include <variant>
#include <array>
#include <future>
#include <filesystem>
#include <chrono>


using namespace converter;
void func(const std::string& qoifile,const std::string& ppmdest){
  auto start = std::chrono::high_resolution_clock::now();
  netpbm::pbmwriter pbmw;
  qoi::qoireader qoir;
  qoir.open(qoifile);
  if(qoir.read()==0){
    auto g = qoir.get_rgba32();
    pbmw.open(ppmdest,std::move(g));
    if(pbmw.write_rgba32_to_p6()==0){
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      std::cout << "FILE:"<< qoifile << " TO " << ppmdest << " TOTAL TIME: "<< elapsed.count() << std::endl;
    }else{
      std::cout << "FILE:"<< qoifile << " TO " << ppmdest << " FAILURE" << std::endl;
    }
  }
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
    //auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::future<void>> results;
    results.reserve(10);
    for(const auto& e : std::filesystem::directory_iterator(path)){
      std::string filename = e.path().string();
      if(e.path().extension() == ".qoi"){
        results.emplace_back(std::async(std::launch::async,func,filename,
                                        (e.path().parent_path()/e.path().stem()).string()+".ppm"));
      }
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
