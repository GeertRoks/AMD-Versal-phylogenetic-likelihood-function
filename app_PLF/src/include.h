#pragma once

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <random>
#include <cstdlib>
#include <regex>
#include <cassert>

#include "xrt/xrt_device.h"
#include "experimental/xrt_xclbin.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_graph.h"
#include "xrt/xrt_bo.h"
#include "experimental/xrt_profile.h"

#include "plf.h"

std::string kernel_name(std::string kernel, unsigned int it);
bool prerun_check();
unsigned int findNumAieIOInXclbin(const std::string& input);

class acap_info {
  public:
    acap_info(char* filename) {
      this->xclbinFile = filename;
      set_xclbin();
      set_target();
      set_device();
      load_xclbin();
    }
    std::string get_target() { return this->target; }
    xrt::device* get_device() { return &this->device; }
    std::vector<xrt::xclbin::kernel> get_kernels() { return this->kernels; }
    xrt::uuid* get_uuid() { return &this->xclbin_uuid; }
    std::string get_app_name() { return splitFilename(0); }
    std::string get_pl_name() { return splitFilename(1); }
    std::string get_aie_name() { return splitFilename(2); }

  private:
    std::string xclbinFile;
    xrt::device device;
    xrt::xclbin xclbin;
    std::string target;
    xrt::uuid xclbin_uuid;
    std::vector<xrt::xclbin::kernel> kernels;

    void load_xclbin() {
      this->xclbin_uuid = this->device.load_xclbin(this->xclbin);
    }
    void set_xclbin() {
      this->xclbin = xrt::xclbin(this->xclbinFile);
      this->kernels = this->xclbin.get_kernels();
    }
    void set_device() {
      if (this->target == "sw_emu" || this->target == "hw_emu") {
        this->device = xrt::device(0);
      } else if (this->target == "hw") {
        this->device = xrt::device("0000:5e:00.1");
      }
      if(this->device == nullptr) {
        throw std::runtime_error("No valid device handle found. Run `xbutil examine` and look for the correct BDF.\n If xbutil is not found, then source the xrt setup file: `source /opt/xilinx/xrt/setup.sh`\n");
      }
    }
    void set_target() {
      switch(this->xclbin.get_target_type()) {
        case xrt::xclbin::target_type::sw_emu:
          this->target = "sw_emu";
          break;
        case xrt::xclbin::target_type::hw_emu:
          this->target = "hw_emu";
          break;
        case xrt::xclbin::target_type::hw:
          this->target = "hw";
          break;
        default:
          throw std::runtime_error("Invalid target");
      }
    }
    std::string splitFilename(size_t index) {
      std::string filename = this->xclbinFile;
      size_t pos_xclbin = filename.find(".xclbin");
      filename.erase(pos_xclbin, 7);
      std::vector<std::string> parts;
      size_t pos = 0;
      std::string delimiter = "_";

      // Find the last occurrence of the delimiter to get the filename
      size_t lastSlashPos = filename.find_last_of("/\\");
      if (lastSlashPos != std::string::npos) {
        filename = filename.substr(lastSlashPos + 1);
      }

      // Split the filename into parts
      while ((pos = filename.find(delimiter)) != std::string::npos) {
        parts.push_back(filename.substr(0, pos));
        filename.erase(0, pos + delimiter.length());
      }
      parts.push_back(filename); // Add the last part

      // Check if the index is within the bounds of the parts vector
      if (index >= 0 && index < parts.size()) {
        return parts[index];
      } else {
        // Return an empty string if the index is out of bounds
        return "";
      }
    }
};

typedef enum {ONE_INEV, TWO_IN} layout_t;
typedef enum {STREAM, WINDOW} aie_t;

struct testbench_info {
  unsigned int parallel_instances = 1;
  unsigned int alignment_sites;
  unsigned int elements_per_alignment = 16;
  unsigned int plf_calls;
  unsigned int window_size = 1024;
  unsigned int alignments_per_window() { return (this->window_size>>4); };
  layout_t input_layout;
  aie_t aie_type;

  unsigned long long int data_size() { return (unsigned long long int) this->data_elements() * this->word_size; }
  unsigned long long int data_elements() { return (unsigned long long int) this->elements_per_instance() * this->parallel_instances * this->plf_calls; }
  unsigned int word_size = sizeof(float);

  unsigned long long int acap_mem_usage() {
    unsigned long long int ram_per_plf = (this->buffer_size_left()+this->buffer_size_right()+this->buffer_size_out());
    return ram_per_plf*this->plf_calls;
  }
  unsigned long long int host_mem_usage() {
    unsigned long long int ram_per_plf = (this->buffer_size_left()+this->buffer_size_right()+this->buffer_size_out()+this->buffer_size_out()+64+64+16+(2*(this->elements_per_plf()+this->alignments_padding_elements())));
    return ram_per_plf*this->plf_calls;
  }

  unsigned int buffer_size_left()  { return this->word_size * this->buffer_elements_left(); }
  unsigned int buffer_size_right() { return this->word_size * this->buffer_elements_right(); }
  unsigned int buffer_size_out() { return this->word_size * this->buffer_elements_out(); }

  unsigned int instance_size_left() { return this->word_size * this->instance_elements_left(); }
  unsigned int instance_size_right() { return this->word_size * this->instance_elements_right(); }
  unsigned int instance_size_out() { return this->word_size * this->instance_elements_out(); }

  unsigned int alignments_per_instance(unsigned int instance) {
    return this->alignments_per_instance() - ((instance == this->parallel_instances-1) * this->alignments_padding());
  }
  unsigned int alignments_per_instance() {
    return std::ceil((double)alignment_sites/(double)parallel_instances);
  }
  unsigned int alignments_padding() {
    return (unsigned int)(this->alignments_per_instance()*this->parallel_instances) - this->alignment_sites;
  }
  unsigned int alignments_padding_elements() {
    return this->alignments_padding() * this->elements_per_alignment;
  }

  unsigned int buffer_elements_left()  {
    return this->instance_elements_left() * this->parallel_instances;
  }
  unsigned int buffer_elements_right() {
    return this->instance_elements_right() * this->parallel_instances;
  }
  unsigned int buffer_elements_out() {
    return this->instance_elements_out() * this->parallel_instances;
  }

  unsigned int instance_elements_left() {
    return this->elements_per_instance() + 5*16;
  }
  unsigned int instance_elements_right() {
    unsigned int result = this->elements_per_instance();
    switch (input_layout) {
      case TWO_IN:
        result += 4*16;
        break;
      case ONE_INEV:
        result += 5*16;
        break;
    }
    return result;
  }
  unsigned int instance_elements_out() {
    return this->elements_per_instance();
  }

  unsigned int elements_per_plf() {
    return this->alignment_sites * this->elements_per_alignment;
  }
  unsigned int elements_per_instance() {
    unsigned int result = 0;
    switch (aie_type) {
      case STREAM:
        result = this->alignments_per_instance() + this->stream_padding();
        break;
      case WINDOW:
        result = this->num_windows_per_instance() * this->alignments_per_window();
        break;
    }
    return result * this->elements_per_alignment;
  }
  unsigned int stream_padding() {
    return this->alignment_sites & 1;
  }
  unsigned int num_windows_per_instance() {
    unsigned int num_full_windows = this->alignments_per_instance()/this->alignments_per_window();
    unsigned int remainder = this->alignments_per_instance() - (num_full_windows*this->alignments_per_window());
    return num_full_windows + (remainder > 0);
  }

};
