
struct Concept;
struct Role;
struct Individual;

class Reasoner {
	Concept* getAtomicConcept();
	Concept* getTopConcept();
	Concept* getBottomConcept();
	Concept* getNominalConcept(Individual*);
	Concept* getExistentialRestrictionConcept(Role*, Concept*);
	Concept* getIntersectionConcept(Concept*, Concept*);

	Role* getAtomicRole();
	Role* getCompositionRole(Role*, Role*);

	Individual* getIndividual();


	//Axiom applicators
	void setInclusion(Concept*, Concept*);
	void setEquivalence(Concept*, Concept*);

	void setInclusion(Role*, Role*);
	void setEquivalence(Role*, Role*);
	void setDomain(Role*, Concept*);
	void setRange(Role*, Concept*);

	void setAssertion(Role*, Individual*, Individual*);
	void setAssertion(Concept*, Individual*);
};