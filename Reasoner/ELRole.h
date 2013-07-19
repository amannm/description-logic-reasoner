#pragma once

#include "Common.h"

#include <vector>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace EL {

	struct Concept;
	struct ExistentialConcept;

	struct Role : DLRole {

		struct {
			std::vector<std::pair<Concept*, ExistentialConcept*>> subsumedExistentialUses;
		} attributes;
	};
}