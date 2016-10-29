int main(void)
{
  unsigned int total_rss = -1;

  std::string command = "grep VmRSS /proc/*/status | grep -Eo '[[:space:]][[:digit:]]+' | awk '{total = total + $1} END {print total}'";
  FILE* cmd_fp = popen(command.c_str(), "r");

  if (cmd_fp == NULL) {
    std::cerr << "ERROR: Unable to execute command\n\t" << command << std::endl;
    throw;
  } else {
    //Max number of characters for a unsigned int should be 10 (4294967295)
    char result[16];
    if (fgets(result, 16, cmd_fp) != NULL) {
      total_rss = atoi(result);
      std::cout << total_rss << std::endl;
    }
  }

  if (total_rss < 0) {
    std::cerr << "ERROR: Failed to calculate totalRSS\n\t" << command << std::endl;
    throw;
  } else {
    return total_rss;
  }
}
