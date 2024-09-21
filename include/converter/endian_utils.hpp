#pragma once
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define SYSTEM_IS_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define SYSTEM_IS_BIG_ENDIAN
#else
    #error "Unknown endianness"
#endif

#include <concepts>
#include <cstring>

namespace converter::endian_utils{

enum endianness{
  LITTLE,
  BIG
};

constexpr endianness sys_endianness =
#ifdef SYSTEM_IS_LITTLE_ENDIAN
  endianness::LITTLE;
#elif SYSTEM_IS_BIG_ENDIAN
  endianness::BIG;
#else
  static_assert(false, "sys_endianness error");
#endif


template<std::integral T>
inline T reverse_byte_order(T value){
  T result;
  unsigned char buff[sizeof(T)];
  std::memcpy(buff,&value,sizeof(T));

  for(std::size_t i=0;i < sizeof(T); ++i){
    (reinterpret_cast<unsigned char*>(&result))[i] = buff[sizeof(T) - 1 - i];
  }
  return result;
}


template<endianness InputEndianness, std::integral T>
inline T to_sys_endianness(T value){
  if constexpr (InputEndianness == sys_endianness){
    return value;
  }
  else{
    return reverse_byte_order(value);
  };
}

template<typename T>
T from_big_endianness(T value){
  return to_sys_endianness<endianness::BIG>(value);
}

template<typename T>
T from_little_endianness(T value){
  return to_sys_endianness<endianness::LITTLE>(value);
}




}//end namespace converter::endian_utils
