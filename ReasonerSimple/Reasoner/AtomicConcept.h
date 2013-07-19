#pragma once

#include "Concept.h"

struct AtomicConcept : Concept {
	AtomicConcept();
	virtual void addNegativeOccurence() final override;
	virtual void addPositiveOccurence() final override;
	virtual void addSubClass(Concept*) final override;
	virtual void initialize() final override;
};