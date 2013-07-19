#include "Concept.h"
#include "Role.h"
#include "Axiom.h"

namespace EL {
	struct Ontology {
		Concept topConcept;

		uint32_t numNamedConcepts;
		Concept* namedConcepts;

		uint32_t numNamedRoles;
		Role* namedRoles;

		uint32_t numIntersectionConcepts;
		Concept* intersectionConcepts;

		uint32_t numExistentialRestrictionConcepts;
		Concept* existentialRestrictionConcepts;

		uint32_t numConceptInclusions;
		ConceptInclusion* conceptInclusions;
	};
}