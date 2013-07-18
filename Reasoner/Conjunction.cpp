#include "Conjunction.h"

Conjunction::Conjunction(std::vector<Concept*> && conjv) :
	conjunctConcepts(std::move(conjv)) {}

void Conjunction::addNegativeOccurence() {
	++attributes.negativeOccurences;
	for(auto c : conjunctConcepts) {
		c->addPositiveOccurence();
	}
}
void Conjunction::addPositiveOccurence() {
	++attributes.positiveOccurences;
	for(auto c : conjunctConcepts) {
		c->addPositiveOccurence();
	}
}
void Conjunction::addSubClass(Concept* sub) {
	for(auto c : conjunctConcepts) {
		sub->inferences.superConceptQueue.push(c);
	}
}
void Conjunction::initialize() {
	if(attributes.negativeOccurences > 0) {
		for(size_t i = 0; i < conjunctConcepts.size(); ++i) {
			for(size_t x = i + 1; x < conjunctConcepts.size(); ++x) {
				conjunctConcepts[i]->attributes.negativelyOccurringConjugationUses.emplace(conjunctConcepts[x], this);
				conjunctConcepts[x]->attributes.negativelyOccurringConjugationUses.emplace(conjunctConcepts[i], this);
			}
		}
	}
}
