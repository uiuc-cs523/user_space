#ifndef OBSERVABLE_MEMORY_H_
#define OBSERVABLE_MEMORY_H_

#include <vector>
#include <string>
#include "memory_observer.h"
#include "memory_defines.h"

class ObservableMemory
{
  private:
    MemoryState _previously_known_state;
    unsigned int _vmrss;
    unsigned int _vmhwm;
    unsigned int _med_threshold;
    unsigned int _high_threshold;
    unsigned int _max_available_memory;
    unsigned int _polling_period_ms;

    std::vector<MemoryObserver> observers;
    void notifyObservers(MemoryState cur_state);
    void refreshStatus();
    void log();
    unsigned int getProcTotal(std::string trend);
    MemoryState getCalculatedPressure(unsigned int total_rss);

  public:
    void addObserver(MemoryObserver observer);
    void start();

    ObservableMemory(unsigned int med_threshold, unsigned int high_threshold,
                     unsigned int max_available_memory, unsigned int polling_period_ms)
      : _med_threshold(med_threshold), _high_threshold(high_threshold),
        _max_available_memory(max_available_memory),
        _polling_period_ms(polling_period_ms) {}
};

#endif
