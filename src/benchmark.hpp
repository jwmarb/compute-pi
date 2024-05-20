#pragma once
#include <ctime>
#include <omp.h>

class Benchmark {
  private:
    clock_t cpu_t;
    double wall_t;
  public:
    Benchmark(clock_t c_starttime, double w_starttime);
    class Result {
      public:
        double cpu_duration;
        double wall_duration;
        Result(clock_t cpu_duration, double wall_duration);
        long cpu_hours();
        long cpu_mins();
        double cpu_seconds();
        long wall_hours();
        long wall_mins();
        double wall_seconds();
    };
    
    void reset();
    Result capture(clock_t c_starttime, double w_endtime);
};