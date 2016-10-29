#ifndef MEMORY_OBSERVER_H_
#define MEMORY_OBSERVER_H_

#include "memory_defines.h"

class MemoryObserver
{
  private:
    MemoryState _previously_known_state;

  public:
    virtual void lowPressure();
    virtual void medPressure();
    virtual void highPressure();

    MemoryObserver();
    virtual ~MemoryObserver() {}
};

#endif
