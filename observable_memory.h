#ifndef OBSERVABLE_MEMORY_H_
#define OBSERVABLE_MEMORY_H_

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
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
    bool _enable_logging;
    bool _running;
    bool _thread_created;
    std::ostream* _log_ostream;
    pthread_t _memory_thread;

    std::vector<MemoryObserver> observers;
    void notifyObservers(MemoryState cur_state);
    void refreshStatus();
    void log();
    unsigned int getProcTotal(std::string trend);
    static void* thread_start(void* class_instance);
    MemoryState getCalculatedPressure(unsigned int total_rss);

  public:
    void addObserver(MemoryObserver observer);
    void start_create_thread();
    void start_blocking();
    void stop();
    void stop_join_thread();

    ObservableMemory(unsigned int med_threshold, unsigned int high_threshold,
                     unsigned int max_available_memory,
                     unsigned int polling_period_ms,
                     std::ostream* log_ostream,
                     bool enable_logging = true
                    );
};

#endif
