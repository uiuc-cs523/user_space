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
                                   std::string observer_scope,
                                   bool enable_logging,
                                   std::ostream* log_ostream
                                  )
{
  _med_threshold = (double) med_threshold / 100;
  _high_threshold = (double) high_threshold / 100;
  _max_available_memory = max_available_memory;
  _polling_period_ms = polling_period_ms;
  _log_ostream = log_ostream;
  _enable_logging = enable_logging;
  _observer_scope = observer_scope;
  _running = true;
  _thread_created = false;
  _memory_utilization = -1;
  _vmrss = -1;
  _vmhwm = -1;

  //Either all processes or only the one running
  if (!_observer_scope.compare("SINGLE")) {
    std::ostringstream pid_stringstream;
    pid_stringstream << getpid();
    _observable_process = pid_stringstream.str();
  } else if (!_observer_scope.compare("ALL")) {
    _observable_process = "*";
  } else {
    std::cerr << "ERROR: invalid _observer_scope: " << _observer_scope
      << std::endl;
    std::cerr << "\tValid arguments are:" << std::endl;
    std::cerr << "\t\t\"ALL\" Monitor memory consumed by all processes"
      << std::endl;
    std::cerr << "\t\t\"SINGLE\" Monitor memory consumed by only this processes"
      << std::endl;
    throw;
  }
}

void ObservableMemory::_notifyObservers(MemoryState cur_state)
{
  for (std::vector<MemoryObserver*>::iterator observer_it = _observers.begin();
       observer_it != _observers.end();
       observer_it++) {
    if (cur_state == LOW) {
      (*observer_it)->lowPressure();
    } else if (cur_state == MED) {
      (*observer_it)->medPressure();
    } else if (cur_state == HIGH) {
      (*observer_it)->highPressure();
    } else {
      std::cerr << "ERROR: Unknown MemoryState: " << cur_state << std::endl;
      throw;
    }
  }
}

void ObservableMemory::_refreshStatus()
{
  MemoryState cur_state = _getCalculatedPressure();

  if (cur_state != _previously_known_state) {
    _previously_known_state = cur_state;

    _notifyObservers(cur_state);
  }
}

void ObservableMemory::_log()
{
  if (_enable_logging) {
    *_log_ostream << "State: " << _previously_known_state << " utilization: " <<
              _memory_utilization << " total VmRSS: " <<_vmrss <<
              " total VmHWM: " << _vmhwm << std::endl;
  }
}

void ObservableMemory::addObserver(MemoryObserver* observer)
{
  _observers.push_back(observer);
}

void ObservableMemory::removeObserver(MemoryObserver* observer)
{
  std::vector<MemoryObserver*>::iterator observer_it =
    find(_observers.begin(), _observers.end(), observer);
  if (observer_it != _observers.end()) {
    _observers.erase(observer_it, observer_it + 1);
  } else {
    std::cerr << "ERROR: Tried to remove an observer that didn't exist!"
      << std::endl;
    throw;
  }
}

void ObservableMemory::start_create_thread()
{
  if (!_thread_created) {
    _thread_created = true;
    pthread_create(&_memory_thread, 0, &ObservableMemory::_thread_start, this);
  }
}

void* ObservableMemory::_thread_start(void* class_instance)
{
  ((ObservableMemory*)class_instance)->start_blocking();

  return 0;
}

void ObservableMemory::start_blocking()
{
  MemoryState cur_state = _getCalculatedPressure();

  _notifyObservers(cur_state);

  _previously_known_state = cur_state;

  while (_running) {
    usleep(_polling_period_ms);

    _refreshStatus();
    _log();
  }
}

void ObservableMemory::stop_blocking()
{
  _running = false;
}

void ObservableMemory::stop_join_thread()
{
  this->stop_blocking();
  pthread_join(_memory_thread, 0);
  _thread_created = false;
}

unsigned int ObservableMemory::_getProcTotal(std::string trend)
{
  unsigned int total_trend = -1;

  std::string command = "grep ";
  command += trend;
  command += " /proc/";
  command += _observable_process;
  command += "/status | grep -Eo '[[:space:]][[:digit:]]+' | ";
  command += "awk '{total = total + $1} END {print total}'";
  FILE* cmd_fp = popen(command.c_str(), "r");

  if (cmd_fp == NULL) {
    std::cerr << "ERROR: Unable to execute command\n\t" << command << std::endl;
    throw;
  } else {
    //Max number of characters for a unsigned int should be 10 (4294967295)
    char result[16];
    if (fgets(result, 16, cmd_fp) != NULL) {
      total_trend = atoi(result);
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

MemoryState ObservableMemory::_getCalculatedPressure()
{
  _vmrss = _getProcTotal("VmRSS");
  _vmhwm = _getProcTotal("VmHWM");

  MemoryState cur_state;

  _memory_utilization = (double) (_vmrss / ((double) _max_available_memory));

  if (_memory_utilization < _med_threshold) {
    cur_state = LOW;
  } else if (_memory_utilization < _high_threshold) {
    cur_state = MED;
  } else {
    cur_state = HIGH;
  }

  return cur_state;
}
