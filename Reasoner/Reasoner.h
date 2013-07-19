
struct Concept;
struct Role;
struct Individual;

Concept* Concept_Atomic_create();
Concept* Concept_Top();
Concept* Concept_Bottom();
Concept* Concept_Nominal_create(Individual*);
Concept* Concept_ExistentialRestriction_create(Role*, Concept*);
Concept* Concept_Intersection_create(Concept*, Concept*);

Role* Role_Atomic_create();
Role* Role_Composition_create(Role*, Role*);

Individual* Individual_create();


//Axiom applicators
void Axiom_Concept_Inclusion_apply(Concept*, Concept*);
void Axiom_Concept_Equivalence_apply(Concept*, Concept*);

void Axiom_Role_Inclusion_apply(Role*, Role*);
void Axiom_Role_Equivalence_apply(Role*, Role*);
void Axiom_Role_Domain_apply(Role*, Concept*);
void Axiom_Role_Range_apply(Role*, Concept*);

void Axiom_Assertion_Role_apply(Role*, Individual*, Individual*);
void Axiom_Assertion_Concept_apply(Concept*, Individual*);

//Reasoner Interface
void Reasoner_start(int);

//Results
int Concept_Inclusions_query(Concept*, Concept***);
int Concept_ForwardLinkedConcepts_query(Concept*, Role*, Concept***);
int Concept_ForwardLinkedRoles_query(Concept*, Role***);
int Concept_BackwardLinkedConcepts_query(Concept*, Role*, Concept***);
int Concept_BackwardLinkedRoles_query(Concept*, Role***);
int Role_Inclusions_query(Role*, Role***);

