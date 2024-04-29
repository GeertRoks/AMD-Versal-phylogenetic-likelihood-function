#ifndef TIMING_H
#define TIMING_H

#include <chrono>
#include <iomanip>
#include <fstream>

class Timer {
public:
  void reset() {
    m_beg = Clock::now();
  }
  double elapsed() const
  {
    return std::chrono::duration_cast<Millisecond>(Clock::now() - m_beg).count();
  }
private:
  using Clock = std::chrono::steady_clock;
  using Millisecond = std::chrono::duration<double, std::ratio<1,1000> >;

  std::chrono::time_point<Clock> m_beg { Clock::now() };
};

struct timing_data {
  timing_data(size_t num_blocks) {
    this->num_blocks = num_blocks;
    this->begin = new double[num_blocks];
    this->t1 = new double[num_blocks];
    this->t2 = new double[num_blocks];
    this->end = new double[num_blocks];
  }
  ~timing_data() {
    //NOTE: gives double free if deletes are enabled??
    //delete[] begin;
    //delete[] t1;
    //delete[] t2;
    //delete[] end;
    //begin = nullptr;
    //t1 = nullptr;
    //t2 = nullptr;
    //end = nullptr;
  }
  size_t num_blocks;
  double* begin;
  double* t1;
  double* t2;
  double* end;

  double hm(int i) { return t1[i] - begin[i]; };
  double msm(int i) { return t2[i] - t1[i]; };
  double mh(int i) { return end[i] - t2[i]; };
  double total(int i) { return end[i] - begin[i]; };
  double hm() {
    double result = 0.0f;
    for (unsigned int i = 0; i < num_blocks; i++) {
      result += t1[i] - begin[i];
    }
    return result;
  };
  double msm() {
    double result = 0.0f;
    for (unsigned int i = 0; i < num_blocks; i++) {
      result += t2[i] - t1[i];
    }
    return result;
  };
  double mh() {
    double result = 0.0f;
    for (unsigned int i = 0; i < num_blocks; i++) {
      result += end[i] - t2[i];
    }
    return result;
  };
  double total() {
    return end[num_blocks-1] - begin[0];
  };
};

double bandwidth(double time_ms, double data_size) {
  return ((double)data_size / (time_ms))/1000.0f; //MB per second
}
void print_timing_data(timing_data d, double data_size) {
  std::cout << std::endl;
  std::cout << "==========================================================================" << std::endl;
  std::cout << "| Timing region                          | time (ms)  | bandwidth (MB/s) |" << std::endl;
  std::cout << "==========================================================================" << std::endl;
  std::cout << "| Host to ACAP memory:                   | " << std::setw(10) << d.hm() << " | ";
  std::cout << std::setw(16) << bandwidth(d.hm(), data_size)    << " |" << std::endl;
  if (d.msm() > 0.005) {
    std::cout << "| ACAP memory to PL/AIE to ACAP memory:  | " << std::setw(10) << d.msm() << " | ";
    std::cout << std::setw(16) << bandwidth(d.msm(), data_size)   << " |" << std::endl;
  }
  std::cout << "| ACAP memory to host:                   | " << std::setw(10) << d.mh() << " | ";
  std::cout << std::setw(16) << bandwidth(d.mh(), data_size)    << " |" << std::endl;
  std::cout << "|----------------------------------------+-------------------------------|" << std::endl;
  std::cout << "| Total execution time:                  | " << std::setw(10) << d.total() << " | ";
  std::cout << std::setw(16) << bandwidth(d.total(), data_size) << " |" << std::endl;
  std::cout << "==========================================================================" << std::endl;
  std::cout << std::endl;
}

void write_to_csv(std::string filename, timing_data d) {
  std::ofstream outputFile(filename);
  outputFile << "hm" << "," << "msasm" << "," << "mh" << std::endl;
  for (unsigned int i = 0; i < d.num_blocks; i++) {
    outputFile << d.hm(i) << "," << d.msm(i) << "," << d.mh(i) << std::endl;
  }
  outputFile.close();
}

#endif
