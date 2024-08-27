#ifndef TIMING_H
#define TIMING_H

#include <chrono>
#include <iomanip>
#include <fstream>
#include <cfloat>

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
  void init(size_t num_blocks) {
    this->num_blocks = num_blocks;
    this->begin = new double[num_blocks];
    this->t1 = new double[num_blocks];
    this->t2 = new double[num_blocks];
    this->end = new double[num_blocks];
  }
  void destroy() {
    delete[] begin;
    delete[] t1;
    delete[] t2;
    delete[] end;
    begin = nullptr;
    t1 = nullptr;
    t2 = nullptr;
    end = nullptr;
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
  double max_msm() {
    double max_val = DBL_MIN;
    double temp = 0.0f;
    for (unsigned int i = 0; i < num_blocks; i++) {
      temp = t2[i] - t1[i];
      if (temp > max_val) {
        max_val = temp;
      }
    }
    return max_val;
  }
  double min_msm() {
    double min_val = DBL_MAX;
    double temp = 0.0f;
    for (unsigned int i = 0; i < num_blocks; i++) {
      temp = t2[i] - t1[i];
      if (temp < min_val) {
        min_val = temp;
      }
    }
    return min_val;
  }
};

double bandwidth_MBs(double time_ms, double data_size) {
  return ((data_size/1000000.0f) / (time_ms/1000.0f)); //MB per second
}
unsigned long int bandwidth_As(double time_ms, double alignments) {
  return (unsigned long int)((double)alignments / (time_ms/1000.0f)); //Alignments per second
}
void print_timing_data(timing_data d, timing_data r, double data_size, unsigned int total_alignments, unsigned int calls) {
  std::cout << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << "| Timing region                          | time (ms)  | bandwidth (MB/s) | bandwidth (alignments/s) |" << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << "| Host to ACAP memory:                   | " << std::setw(10) << d.hm() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(d.hm(), data_size)    << " | ";
  std::cout << std::setw(24) << bandwidth_As(d.hm(), total_alignments) << " |" << std::endl;
  if (d.msm() > 0.005) {
    std::cout << "| ACAP memory to PL/AIE to ACAP memory:  | ";
    std::cout << std::setw(10) << d.msm() <<  " | ";
    std::cout << std::setw(16) << bandwidth_MBs(d.msm(), data_size) << " | ";
    std::cout << std::setw(24) << bandwidth_As(d.msm(), total_alignments) << " |" << std::endl;
    std::cout << std::left;
    std::cout << "|   - slowest:                           | ";
    std::cout << std::setw(10) << d.max_msm() << " | ";
    std::cout << std::setw(16) << bandwidth_MBs(d.max_msm(), data_size/calls) << " | ";
    std::cout << std::setw(24) << bandwidth_As(d.max_msm(), total_alignments/calls) << " |" << std::endl;
    std::cout << "|   - fastest:                           | ";
    std::cout << std::setw(10) << d.min_msm() << " | ";
    std::cout << std::setw(16) << bandwidth_MBs(d.min_msm(), data_size/calls) << " | ";
    std::cout << std::setw(24) << bandwidth_As(d.min_msm(), total_alignments/calls) << " |";
    std::cout << std::right << std::endl;
  }
  std::cout << "| ACAP memory to host:                   | " << std::setw(10) << d.mh() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(d.mh(), data_size)    << " | ";
    std::cout << std::setw(24) << bandwidth_As(d.mh(), total_alignments) << " |" << std::endl;
  std::cout << "|----------------------------------------+------------+------------------+--------------------------|" << std::endl;
  std::cout << "| Total execution time:                  | " << std::setw(10) << d.total() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(d.total(), data_size) << " | ";
  std::cout << std::setw(24) << bandwidth_As(d.total(), total_alignments) << " |" << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << "| Reference:                             | " << std::setw(10) << r.msm() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(r.msm(), data_size) << " | ";
  std::cout << std::setw(24) << bandwidth_As(r.msm(), total_alignments) << " |" << std::endl;
  std::cout << "|----------------------------------------+------------+------------------+--------------------------|" << std::endl;
  std::cout << "| Speed up (excluding pcie transfer):    | ";
  std::cout << std::setw(56) << r.msm()/d.msm() << " |" << std::endl;;
  std::cout << "| Speed up (including pcie transfer):    | ";
  std::cout << std::setw(56) << r.msm()/d.total() << " |" << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << std::endl;
}

void write_to_csv(std::string filename, timing_data* d, unsigned int instances) {
  std::ofstream outputFile(filename);
  for (unsigned int i = 0; i < instances; i++) {
    outputFile << "hm" << i << ",";
  }
  for (unsigned int i = 0; i < instances; i++) {
    outputFile << "msasm" << i << ",";
  }
  for (unsigned int i = 0; i < instances; i++) {
    outputFile << "mh" << i;
    if (i != instances-1) {
      outputFile << ",";
    }
  }
  outputFile << std::endl;

  for (unsigned int call = 0; call < d[0].num_blocks; call++) {
    for (unsigned int i = 0; i < instances; i++) {
      outputFile << d[i].hm(call) << ",";
    }
    for (unsigned int i = 0; i < instances; i++) {
      outputFile << d[i].msm(call) << ",";
    }
    for (unsigned int i = 0; i < instances; i++) {
      outputFile << d[i].mh(call);
      if (i != instances-1) {
        outputFile << ",";
      }
    }
    outputFile << std::endl;
  }
  outputFile.close();
}

#endif
