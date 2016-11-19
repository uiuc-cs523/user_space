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
    double _med_threshold;
    double _high_threshold;
    unsigned int _max_available_memory;
    unsigned int _polling_period_ms;
    //Initialized to -1
    double _memory_utilization;
    bool _enable_logging;
    bool _running;
    bool _thread_created;
    std::string _observer_scope;
    std::ostream* _log_ostream;
    pthread_t _memory_thread;
    std::string _observable_process;

    std::vector<MemoryObserver*> _observers;

    void _notifyObservers(MemoryState cur_state);
    void _refreshStatus();
    void _log();
    unsigned int _getProcTotal(std::string trend);
    static void* _thread_start(void* class_instance);
    MemoryState _getCalculatedPressure();

  public:
    void addObserver(MemoryObserver* observer);
    void removeObserver(MemoryObserver* observer);
    void start_create_thread();
    void stop_join_thread();
    void start_blocking();
    void stop_blocking();

    ObservableMemory(unsigned int med_threshold, unsigned int high_threshold,
                     unsigned int max_available_memory,
                     unsigned int polling_period_ms = 500,
                     std::string observer_scope = "SINGLE",
                     bool enable_logging = true,
                     std::ostream* log_ostream = (std::ostream*)&std::cerr
                    );
};

#endif
