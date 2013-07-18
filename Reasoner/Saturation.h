#include "DL.h"

#include <mutex>
#include <queue>


namespace Saturation {
	std::mutex queueAccess;
	std::queue<std::tuple<Concept*, Role*, Concept*>> messageQueue;
	std::condition_variable newMessage;
	std::mutex saturationStatus;

	int numActive = 0;
	bool finished = false;

	void addMessage(Concept*, Role*, Concept*);
	void addMessage(Concept*, Concept*);
	void addMessage(Concept*);

	void start(int);
}