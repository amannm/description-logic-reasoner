#pragma once

#include "Common.h"

#include <vector>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace EL {

	struct Concept;
	struct AtomicConcept;
	struct ExistentialConcept;
	struct IntersectionConcept;

	struct Role : DLRole {
		struct {
			std::vector<std::pair<Concept*, ExistentialConcept*>> subsumedExistentialUses;
		} attributes;
	};

	struct Concept : DLConcept {
		struct {
			std::vector<Concept*> subConceptUses;
			std::vector<std::pair<Role*, ExistentialConcept*>> subsumedExistentialUses;
			std::vector<std::pair<Concept*, IntersectionConcept*>> subsumedIntersectionUses;

			std::vector<Concept*> ancestralConcepts;
			int negativeOccurences;
		} attributes;

		struct {
			std::vector<Concept*> superConceptQueue;
			std::unordered_set<Concept*> superConcepts;
			std::unordered_map<Role*, std::unordered_set<Concept*>> predecessors;
		} inferences;

		Concept();

		virtual void addSubClass(Concept*) {}
		virtual void index() {}

		void process(Role*, Concept*);

		static Concept* getTop();
	};

	struct AtomicConcept : Concept {
		virtual void index() final override;
	};

	struct TopConcept : Concept {
	};

	struct ExistentialConcept : Concept {
		Role* role;
		Concept* roleFiller;

		ExistentialConcept(Role*, Concept*);

		virtual void addSubClass(Concept*) final override;
		virtual void index() final override;
	};

	struct IntersectionConcept : Concept {
		Concept* first;
		Concept* second;

		IntersectionConcept(Concept*, Concept*);

		virtual void addSubClass(Concept*) final override;
		virtual void index() final override;
	};
}