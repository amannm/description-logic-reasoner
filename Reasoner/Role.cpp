#include "Role.h"

Role* Role_create() {
	return new Role();
}

void Concept_destroy(Concept* c) {
	delete c;
}
void Role_destroy(Role* r) {
	delete r;
}