#ifndef MEMORY_OBSERVER_H_
#define MEMORY_OBSERVER_H_

class MemoryObserver {
  private:
		enum MemoryState { LOW, MEDIUM, HIGH };
		MemoryState previously_known_state;

	public:
		virtual void lowPressure();
		virtual void mediumPressure();
		virtual void highPressure();

		MemoryObserver();
};

#endif
