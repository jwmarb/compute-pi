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
    Result capture(clock_t c_endtime, double w_endtime);
};

Benchmark::Benchmark(clock_t c_starttime, double w_starttime) {
  this->cpu_t = c_starttime;
  this->wall_t = w_starttime;
}

Benchmark::Result::Result(clock_t cpu_duration, double wall_duration) {
  this->cpu_duration = ((double) cpu_duration)/CLOCKS_PER_SEC;
  this->wall_duration = wall_duration;
}

void Benchmark::reset() {
  this->cpu_t = clock();
  this->wall_t = omp_get_wtime();
}

Benchmark::Result Benchmark::capture(clock_t c_endtime, double w_endtime) {
  return Benchmark::Result(c_endtime - this->cpu_t, w_endtime - this->wall_t);
}

long Benchmark::Result::cpu_hours() {
  return this->cpu_duration/3600;
}

long Benchmark::Result::cpu_mins() {
  return ((long) this->wall_duration/60)%60;
}

double Benchmark::Result::cpu_seconds() {
  return ((long) (this->cpu_duration * 1000.0) % 60000)/1000.0;
}

long Benchmark::Result::wall_hours() {
  return this->wall_duration/3600;
}

long Benchmark::Result::wall_mins() {
  return ((long) this->wall_duration/60)%60;
}

double Benchmark::Result::wall_seconds() {
  return ((long) (this->wall_duration * 1000.0) % 60000)/1000.0;
}