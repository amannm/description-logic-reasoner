
#include "ELSaturation.h"
#include "EL.h"
#include <mutex>
#include <queue>
#include <tuple>
#include <condition_variable>

namespace EL {

	namespace {
		std::mutex queueAccess; 
		std::queue<std::tuple<Concept*, Role*, Concept*>> messageQueue;

		std::condition_variable newMessage;
		std::mutex saturationStatus;

		int numActive = 0;
		bool finished = false;
	}

	void Saturation::addMessage(Concept* concept) {
		queueAccess.lock();
		messageQueue.emplace(concept, nullptr, concept);
		queueAccess.unlock();
		newMessage.notify_one();
	}
	void Saturation::addMessage(Concept* sub, Concept* super) {
		queueAccess.lock();
		messageQueue.emplace(sub, nullptr, super);
		queueAccess.unlock();
		newMessage.notify_one();
	}
	void Saturation::addMessage(Concept* concept, Role* role, Concept* predecessor) {
		queueAccess.lock();
		messageQueue.emplace(concept, role, predecessor);
		queueAccess.unlock();
		newMessage.notify_one();
	}

	void Saturation::start(int n) {
		saturationStatus.lock();
		++numActive;
		finished = false;
		saturationStatus.unlock();
		while(true) {
			//Working room
			while(true) {
				queueAccess.lock();
				if(messageQueue.size() != 0) {
					auto message = messageQueue.front();
					messageQueue.pop();
					queueAccess.unlock();
					std::get<0>(message)->process(std::get<1>(message), std::get<2>(message));
				}
				else {
					queueAccess.unlock();
					break;
				}
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
}