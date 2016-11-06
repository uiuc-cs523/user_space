#include "stdlib.h"
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <unistd.h>

#include "observable_memory.h"

ObservableMemory::ObservableMemory(unsigned int med_threshold,
                                   unsigned int high_threshold,
                                   unsigned int max_available_memory,
                                   unsigned int polling_period_ms,
                                   std::ostream* log_ostream,
                                   bool enable_logging
                                  )
{
  _med_threshold = med_threshold;
  _high_threshold = high_threshold;
  _max_available_memory = max_available_memory;
  _polling_period_ms = polling_period_ms;
  _log_ostream = log_ostream;
  _enable_logging = enable_logging;
  _running = true;
  _thread_created = false;
}

void ObservableMemory::notifyObservers(MemoryState cur_state)
{
  for (std::vector<MemoryObserver>::iterator observer_it = observers.begin();
       observer_it != observers.end();
       observer_it++) {
    if (cur_state == LOW) {
      observer_it->lowPressure();
    } else if (cur_state == MED) {
      observer_it->medPressure();
    } else if (cur_state == HIGH) {
      observer_it->highPressure();
    } else {
      std::cerr << "ERROR: Unknown MemoryState: " << cur_state << std::endl;
      throw;
    }
  }
}

void ObservableMemory::refreshStatus()
{
  //Get sum of all rss
  unsigned int total_rss = getProcTotal("VmRSS");
  usleep(_polling_period_ms);
  unsigned int total_hwm = getProcTotal("VmHWM");
  _vmrss = total_rss;
  _vmhwm = total_hwm;

  //Get calculated memory pressure
  MemoryState cur_state = getCalculatedPressure(total_rss);

  //Update memory pressure status if transition occured and notify observers
  if (cur_state != _previously_known_state) {
    _previously_known_state = cur_state;

    notifyObservers(cur_state);
  }
}

void ObservableMemory::log()
{
  std::cout << "State: " << _previously_known_state << " total VmRSS: " <<
            _vmrss << " total VmHWM: " << _vmhwm << std::endl;
}

void ObservableMemory::addObserver(MemoryObserver observer)
{
  observers.push_back(observer);
}


void ObservableMemory::start_create_thread()
{
  if (!_thread_created) {
    _thread_created = true;
    pthread_create(&_memory_thread, 0, &ObservableMemory::thread_start, this);
  }
}

void* ObservableMemory::thread_start(void* class_instance)
{
  ((ObservableMemory*)class_instance)->start_blocking();

  return 0;
}

void ObservableMemory::start_blocking()
{
  //Get initial memory state and update all observers
  unsigned int total_rss = getProcTotal("VmRSS");
  MemoryState cur_state = getCalculatedPressure(total_rss);

  notifyObservers(cur_state);

  _previously_known_state = cur_state;

  while (_running) {
    usleep(_polling_period_ms);

    refreshStatus();
    log();
  }
}

void ObservableMemory::stop()
{
  _running = false;
}

void ObservableMemory::stop_join_thread()
{
  this->stop();
  pthread_join(_memory_thread, 0);
  _thread_created = false;
}

unsigned int ObservableMemory::getProcTotal(std::string trend)
{
  unsigned int total_trend = -1;

  std::string command = "grep ";
  command += trend;
  command += " /proc/*/status | grep -Eo '[[:space:]][[:digit:]]+' | awk '{total = total + $1} END {print total}'";
  FILE* cmd_fp = popen(command.c_str(), "r");

  if (cmd_fp == NULL) {
    std::cerr << "ERROR: Unable to execute command\n\t" << command << std::endl;
    throw;
  } else {
    //Max number of characters for a unsigned int should be 10 (4294967295)
    char result[16];
    if (fgets(result, 16, cmd_fp) != NULL) {
      total_trend = atoi(result);
      std::cout << total_trend << std::endl;
    }

    pclose(cmd_fp);
  }

  if (total_trend < 0) {
    std::cerr << "ERROR: Failed to calculate total " << trend << "\n\t"
              << command << std::endl;
    throw;
  } else {
    return total_trend;
  }
}

MemoryState ObservableMemory::getCalculatedPressure(unsigned int total_rss)
{
  MemoryState cur_state;

  double memory_utilization = (double) (total_rss / ((double) _max_available_memory));
  std::cout << "memory_utilization: " << memory_utilization << std::endl;

  if (memory_utilization < _med_threshold) {
    cur_state = LOW;
  } else if (memory_utilization < _high_threshold) {
    cur_state = MED;
  } else {
    cur_state = HIGH;
  }

  return cur_state;
}
