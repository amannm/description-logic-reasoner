#include "DL.h"

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <queue>

struct ExistentialRoleRestriction;
struct Conjunction;

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