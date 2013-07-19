
#include "Role.h"
#include "Saturation.h"

#include "Logger.h"

void AtomicRole::index() {
	log("Indexing Atomic Role");
	Saturation::ELR::addRoleInitialization(this);
}

CompositionRole::CompositionRole(Role* r, Role* l) :
	right(r),
	left(l) {
}
void CompositionRole::index() {
	log("Indexing Composition Role");
	Saturation::ELR::addRoleInitialization(this);
	right->attributes.rightRoleCompositionUses.emplace_back(left, this);
}