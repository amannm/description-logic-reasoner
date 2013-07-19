#pragma once

extern "C" {
#include "Reasoner.h"
}




namespace Index {
	Concept* createAtomicConcept();
	Concept* createExistentialConcept(Role*, Concept*);
	Concept* createIntersectionConcept(Concept*, Concept*);
	void destroyConcept(Concept*);
	
	Role* createCompositionRole(Role*, Role*);
	Role* createAtomicRole();
	void destroyRole(Role*);

	void initializeRoles();
	void initializeConcepts();
};