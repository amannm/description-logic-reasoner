extern "C" {
#include "DL.h"
}

#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


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

struct ExistentialRoleRestriction;
struct Conjunction;

struct Role {
	struct {
		std::unordered_map<Concept*, ExistentialRoleRestriction*> negativelyOccurringExistentialRoleRestrictionUses;
		std::unordered_set<Role*> subRoleUses;
		std::unordered_map<Role*, Role*> roleChainUses;
	} attributes;
};

struct Concept {

	struct {
		std::mutex access;
		std::unordered_set<Concept*> subConceptUses;
		std::unordered_map<Role*, ExistentialRoleRestriction*> negativelyOccurringExistentialRoleRestrictionUses;
		std::unordered_map<Concept*, Conjunction*> negativelyOccurringConjugationUses;
		int negativeOccurences;
		int positiveOccurences;
	} attributes;

	struct {
		std::mutex access;
		std::queue<Concept*> superConceptQueue;
		std::unordered_set<Concept*> superConcepts;
		std::unordered_map<Role*, Concept*> predecessors;
	} inferences;

	struct {
		std::mutex access;
		std::unordered_set<Concept*> superConcepts;
		std::unordered_set<Concept*> directSuperConcepts;
		std::unordered_set<Concept*> equivalentConcepts;
	} taxonomy;

	virtual void addNegativeOccurence() = 0;
	virtual void addPositiveOccurence() = 0;
	virtual void addSubClass(Concept*) = 0;
	virtual void initialize() = 0;

	void process(Role*, Concept*);
};

struct ExistentialRoleRestriction : Concept {

	Role* role;
	Concept* roleFiller;

	ExistentialRoleRestriction(Role* r, Concept* c) :
		role(r),
		roleFiller(c) {
	}

	virtual void addNegativeOccurence() final override {
		++attributes.negativeOccurences;
		roleFiller->addNegativeOccurence();
	}
	virtual void addPositiveOccurence() final override {
		++attributes.positiveOccurences;
		roleFiller->addPositiveOccurence();
	}
	virtual void addSubClass(Concept* sub) final override {
		Saturation::addMessage(roleFiller, role, sub);
	}
	virtual void initialize() final override  {
		if(attributes.negativeOccurences > 0) {
			role->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(roleFiller, this);
			roleFiller->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(role, this);
		}
	}
};
struct Conjunction : Concept {

	std::vector<Concept*> conjunctConcepts;

	Conjunction(std::vector<Concept*>&& conjv) : 
		conjunctConcepts(std::move(conjv)) {
	}

	virtual void addNegativeOccurence() final override {
		++attributes.negativeOccurences;
		for(auto c : conjunctConcepts) {
			c->addPositiveOccurence();
		}
	}
	virtual void addPositiveOccurence() final override {
		++attributes.positiveOccurences;
		for(auto c : conjunctConcepts) {
			c->addPositiveOccurence();
		}
	}
	virtual void addSubClass(Concept* sub) final override {
		for(auto c : conjunctConcepts){
			sub->inferences.superConceptQueue.push(c);
		}
	}
	virtual void initialize() final override  {
		if(attributes.negativeOccurences > 0) {
			for(int i = 0; i < conjunctConcepts.size(); ++i) {
				for(int x = i+1; x < conjunctConcepts.size(); ++x) {
					conjunctConcepts[i]->attributes.negativelyOccurringConjugationUses.emplace(conjunctConcepts[x], this);
					conjunctConcepts[x]->attributes.negativelyOccurringConjugationUses.emplace(conjunctConcepts[i], this);
				}
			}
		}
	}
};
struct Atomic : Concept {
	virtual void addNegativeOccurence() final override {
		++attributes.negativeOccurences;
	}
	virtual void addPositiveOccurence() final override {
		++attributes.positiveOccurences;
	}
	virtual void addSubClass(Concept* sub) final override {
		sub->taxonomy.superConcepts.insert(this);
	}
	virtual void initialize() final override  {
		Saturation::addMessage(this);
	}
};

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

void Concept::process(Role* predicate, Concept* object) {
	std::lock_guard<std::mutex> lock(inferences.access);
	if(predicate != nullptr) {
		if(inferences.predecessors.emplace(predicate, object).second) {
			for(auto pair : predicate->attributes.negativelyOccurringExistentialRoleRestrictionUses)  {
				if(inferences.superConcepts.find(pair.first) != inferences.superConcepts.end()) {
					Saturation::addMessage(object, pair.second);
				}
			}
		}
		inferences.superConceptQueue.push(this);
	}
	else {
		inferences.superConceptQueue.push(object);
	}
	while(inferences.superConceptQueue.size() != 0) {
		Concept* superConcept = inferences.superConceptQueue.front();
		inferences.superConceptQueue.pop();
		if(inferences.superConcepts.insert(superConcept).second) {
			superConcept->addSubClass(this);
			for(auto superSuper : superConcept->attributes.subConceptUses)
				inferences.superConceptQueue.push(superSuper);
			for(auto pair : superConcept->attributes.negativelyOccurringConjugationUses) 
				if(inferences.superConcepts.find(pair.first) != inferences.superConcepts.end())
					inferences.superConceptQueue.push(pair.second);
			for(auto pair : superConcept->attributes.negativelyOccurringExistentialRoleRestrictionUses) 
				for(auto range = inferences.predecessors.equal_range(pair.first); range.first != range.second; ++range.first) 
					Saturation::addMessage(pair.second->roleFiller, pair.second->role, range.first->second);
		}
	}
}


Concept* Concept_ExistentialRoleRestriction_create(Role* r, Concept* c) {
	return new ExistentialRoleRestriction(r,c);
}
Concept* Concept_Conjunction_create(Concept** conjuncts, int length) {
	std::vector<Concept*> conjv;
	while(--length != -1) {
		conjv.push_back(conjuncts[length]);
	} 
	return new Conjunction(std::move(conjv));
}
Concept* Concept_Atomic_create() {
	return new Atomic();
}

Role* Role_create() {
	return new Role();
}

void Concept_destroy(Concept* c) {
	delete c;
}
void Role_destroy(Role* r) {
	delete r;
}

void Axiom_Subsumption_apply(Concept* sub, Concept* super) {
	if(sub->attributes.subConceptUses.insert(super).second) {
		sub->addNegativeOccurence();
	}
	sub->initialize();
	super->initialize();
}

void Reasoner_initialize(Concept* c) {
	c->initialize();
}
void Reasoner_start() {
	std::thread worker1(Saturation::start, 0);
	std::thread worker2(Saturation::start, 0);
	worker1.join();
	worker2.join();
}
void Concept_Subsumers_query(Concept* c, Results* res) {
	std::vector<Concept*> cv;
	for(auto p : c->inferences.superConcepts) {
		cv.push_back(p);
	}
	res->size = cv.size();
	res->superConcepts = cv.data();
}
