extern "C" {
#include "DL.h"
}

#include "Concept.h"
#include "ExistentialRoleRestriction.h"
#include "Conjunction.h"
#include "AtomicConcept.h"

#include "Role.h"

#include "Saturation.h"

#include <thread>


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

//Concept Interface

Concept* Concept_ExistentialRoleRestriction_create(Role* r, Concept* c) {
	return new ExistentialRoleRestriction(r, c);
}
Concept* Concept_Conjunction_create(Concept** conjuncts, int length) {
	std::vector<Concept*> conjv;
	while(--length != -1) {
		conjv.push_back(conjuncts[length]);
	}
	return new Conjunction(std::move(conjv));
}
Concept* Concept_Atomic_create() {
	return new AtomicConcept();
}
void Concept_destroy(Concept* c) {
	delete c;
}


//Role Interface

Role* Role_create() {
	return new Role();
}
void Role_destroy(Role* r) {
	delete r;
}