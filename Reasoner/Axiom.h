
struct Concept;
struct Role;

struct ConceptInclusion {
	Concept* sub;
	Concept* super;
};
struct RoleInclusion {
	//uint32_t before this
	Role* sub;
	Role* super;
};