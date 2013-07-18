struct Concept;
struct Role;

typedef struct {
	Concept** superConcepts;
	int size;
} Results;

Concept* Concept_ExistentialRoleRestriction_create(Role*, Concept*);
Concept* Concept_Conjunction_create(Concept**, int);
Concept* Concept_Atomic_create();
void Concept_destroy(Concept*);

Role* Role_create();
void Role_destroy(Role*);

void Axiom_ConceptInclusion_apply(Concept*, Concept*);
void Axiom_ConceptEquivalence_apply(Concept*, Concept*);
void Axiom_ConceptDisjointedness_apply(Concept*, Concept*);


void Axiom_RoleInclusion_apply(Concept*, Concept*);
void Axiom_RoleEquivalence_apply(Concept*, Concept*);

	
void Axiom_TransitiveRole_apply(Concept*, Concept*);
void Axiom_ReflexiveRole_apply(Concept*, Concept*);

void Axiom_DomainRestriction_apply(Concept*, Concept*);
void Axiom_RangeRestriction_apply(Concept*, Concept*);

void Reasoner_initialize(Concept*);
void Reasoner_start();
void Concept_Subsumers_query(Concept*, Results*);

