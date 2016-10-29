#ifndef SUBPROCESS_A_H_
#define SUBPROCESS_A_H_

#include "memory_observer.h"

class SubprocessA : public MemoryObserver {
	public:
		void lowPressure();
		void mediumPressure();
		void highPressure();

		SubprocessA();
};

#endif
