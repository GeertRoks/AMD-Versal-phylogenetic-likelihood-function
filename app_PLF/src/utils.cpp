#include "include.h"

std::string kernel_name(std::string kernel, unsigned int it) {
  std::ostringstream name;
  name << kernel << ":{" << kernel << "_" << it << "}";
  return name.str();
}

bool prerun_check() {
  char user_response;
  bool validInput = false;
  bool ready_for_run = false;

  while (!validInput) {
    std::cout << std::endl;
    std::cout << "Ready to continue with test? [Y/n]: ";
    std::cin >> user_response;

    user_response = tolower(user_response);

    if (user_response == 'y' || user_response == 'n' || std::cin.fail()) {
      validInput = true;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (user_response == 'y' || std::cin.fail()) {
        ready_for_run = true;
      }
    } else {
      std::cout << "You may only type 'y' or 'n'.\n";
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }

  if (!ready_for_run) {
    return false;
  }
  std::cout << std::endl;
  return true;
}

unsigned int findNumAieIOInXclbin(const std::string& input) {
    size_t pos = input.find('x');
    if (pos == std::string::npos) {
        // 'x' not found, return 0 or handle as needed
        return 0;
    }
    std::string valueStr = input.substr(pos + 1);
    unsigned int value = std::stoi(valueStr);
    return value;
}

