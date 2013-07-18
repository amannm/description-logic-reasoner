#include "Concept.h"

struct ExistentialRoleRestriction : Concept {

	Role* role;
	Concept* roleFiller;

	ExistentialRoleRestriction(Role* r, Concept* c) :
		role(r),
		roleFiller(c) {}

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
	virtual void initialize() final override {
		if(attributes.negativeOccurences > 0) {
			role->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(roleFiller, this);
			roleFiller->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(role, this);
		}
	}
};
struct Conjunction : Concept {

	std::vector<Concept*> conjunctConcepts;

	Conjunction(std::vector<Concept*> && conjv) :
		conjunctConcepts(std::move(conjv)) {}

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
		for(auto c : conjunctConcepts) {
			sub->inferences.superConceptQueue.push(c);
		}
	}
	virtual void initialize() final override {
		if(attributes.negativeOccurences > 0) {
			for(int i = 0; i < conjunctConcepts.size(); ++i) {
				for(int x = i + 1; x < conjunctConcepts.size(); ++x) {
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
	virtual void initialize() final override {
		Saturation::addMessage(this);
	}
};


void Concept::process(Role* predicate, Concept* object) {
	std::lock_guard<std::mutex> lock(inferences.access);
	if(predicate != nullptr) {
		if(inferences.predecessors.emplace(predicate, object).second) {
			for(auto pair : predicate->attributes.negativelyOccurringExistentialRoleRestrictionUses) {
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
	return new Atomic();
}