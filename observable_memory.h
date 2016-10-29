#ifndef OBSERVABLE_MEMORY_H_
#define OBSERVABLE_MEMORY_H_

#include <vector>
#include "memory_observer.h"

class ObservableMemory {
	private:
		std::vector<MemoryObserver> observers;
		void notifyObservers();
		void refreshStatus();
		void log();

	
	public:
		void addObserver(MemoryObserver in_observer);

		ObservableMemory(unsigned int low, unsigned int med, unsigned int high, unsigned int polling_period_ms);
};

#endif
