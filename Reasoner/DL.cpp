extern "C" {
#include "DL.h"
}

#include "Concept.h"

#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>












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
