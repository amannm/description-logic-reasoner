#pragma once

#include "ExistentialRoleRestriction.h"
#include "Saturation.h"
#include "Role.h"

ExistentialRoleRestriction::ExistentialRoleRestriction(Role* r, Concept* c) :
	role(r),
	roleFiller(c) {}

void ExistentialRoleRestriction::addNegativeOccurence() {
	++attributes.negativeOccurences;
	roleFiller->addNegativeOccurence();
}
void ExistentialRoleRestriction::addPositiveOccurence() {
	++attributes.positiveOccurences;
	roleFiller->addPositiveOccurence();
}
void ExistentialRoleRestriction::addSubClass(Concept* sub) {
	Saturation::addMessage(roleFiller, role, sub);
}
void ExistentialRoleRestriction::initialize() {
	if(attributes.negativeOccurences > 0) {
		role->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(roleFiller, this);
		roleFiller->attributes.negativelyOccurringExistentialRoleRestrictionUses.emplace(role, this);
	}
}