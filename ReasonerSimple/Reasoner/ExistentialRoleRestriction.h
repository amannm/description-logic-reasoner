#pragma once

#include "Concept.h"

struct ExistentialRoleRestriction : Concept {

	Role* role;
	Concept* roleFiller;

	ExistentialRoleRestriction(Role*, Concept*);

	virtual void addNegativeOccurence() final override;
	virtual void addPositiveOccurence() final override;
	virtual void addSubClass(Concept*) final override;
	virtual void initialize() final override;
};