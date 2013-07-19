#include "AtomicConcept.h"
#include "Saturation.h"

AtomicConcept::AtomicConcept() {}

void AtomicConcept::addNegativeOccurence() {
	++attributes.negativeOccurences;
}
void AtomicConcept::addPositiveOccurence() {
	++attributes.positiveOccurences;
}
void AtomicConcept::addSubClass(Concept* sub) {
	sub->taxonomy.superConcepts.insert(this);
}
void AtomicConcept::initialize() {
	Saturation::addMessage(this);
}
