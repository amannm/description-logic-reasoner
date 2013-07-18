#include "Saturation.h"

#include <condition_variable>
#include <tuple>

void Saturation::addMessage(Concept* ce1, Role* ope, Concept* ce2) {
	queueAccess.lock();
	messageQueue.emplace(ce1, ope, ce2);
	queueAccess.unlock();
	newMessage.notify_one();
}

void Saturation::addMessage(Concept* ce1, Concept* ce2) {
	queueAccess.lock();
	messageQueue.emplace(ce1, nullptr, ce2);
	queueAccess.unlock();
	newMessage.notify_one();
}

void Saturation::addMessage(Concept* ce) {
	queueAccess.lock();
	messageQueue.emplace(ce, nullptr, ce);
	queueAccess.unlock();
	newMessage.notify_one();
}

void Saturation::start(int n) {
	std::tuple<Concept*, Role*, Concept*> message;
	saturationStatus.lock();
	++numActive;
	saturationStatus.unlock();
	while(true) {
		//Working room
		while(true) {
			queueAccess.lock();
			if(messageQueue.size() != 0) {
				message = messageQueue.front();
				messageQueue.pop();
				queueAccess.unlock();
			}
			else {
				queueAccess.unlock();
				break;
			}
			std::get<0>(message)->process(std::get<1>(message), std::get<2>(message));
		}
		//Waiting room
		std::unique_lock<std::mutex> waitingStatus(saturationStatus);
		if(--numActive == 0) {
			finished = true;
			waitingStatus.unlock();
			newMessage.notify_all();
			break;
		}
		else {
			newMessage.wait(waitingStatus);
			if(finished) {
				break;
			}
		}
		++numActive;
	}
}
