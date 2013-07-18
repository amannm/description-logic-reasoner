#pragma once

#include "Concept.h"

struct Conjunction : Concept {

	std::vector<Concept*> conjunctConcepts;

	Conjunction(std::vector<Concept*>&&);

	virtual void addNegativeOccurence() final override;
	virtual void addPositiveOccurence() final override;
	virtual void addSubClass(Concept*) final override;
	virtual void initialize() final override;
};