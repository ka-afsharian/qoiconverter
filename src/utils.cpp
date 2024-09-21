#include "converter/utils.hpp"
#include <x86intrin.h>
#include <thread>
#include <ctime>


namespace converter::utils{

timer::timer() : start(__rdtsc()), end(start), cpu_freq(0){}


void timer::set(){
    start=__rdtsc();
}

std::optional<timer::seconds> timer::get(){
    end = __rdtsc();
    cpu_freq=get_cpu_frequency();
    duration elapsed_cycles = static_cast<duration>(end - start);
    if(cpu_freq>0){
        seconds elapsed_time = elapsed_cycles / cpu_freq; // Time in seconds
        return elapsed_time;
    } else {
        return {};
    }
}

timer::frequency __attribute__((optimize("O0"))) timer::get_cpu_frequency(){
    auto start_chrono = std::chrono::high_resolution_clock::now();
    clock tsc_start = __rdtsc();

    for(int i=0;i<300;i++);


    auto end_chrono = std::chrono::high_resolution_clock::now();
    clock tsc_end = __rdtsc();
    std::chrono::duration<seconds> elapsed_seconds = end_chrono - start_chrono;
    return static_cast<frequency>(static_cast<duration>(tsc_end - tsc_start) / elapsed_seconds.count());
}

}//end namespace converter::utils
