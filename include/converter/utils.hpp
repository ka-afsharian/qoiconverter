#pragma once
#include <cstdint>
#include <optional>


namespace converter::utils{

class timer{
public:
  using duration = double;
  using seconds = double;
  using frequency = double;
  using clock = uint64_t;

  timer();
  timer(const timer&)=delete;
  timer& operator=(const timer&)=delete;
  timer(timer&&) = delete;
  timer& operator=(timer&&)=delete;

  void set();

  std::optional<seconds> get();

  static double get_cpu_frequency();

private:
  clock start;
  clock end;
  frequency cpu_freq; // Get CPU frequency
};

}//endnamespace
