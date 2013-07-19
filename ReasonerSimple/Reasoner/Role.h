#pragma once

struct Concept;
struct ExistentialRoleRestriction;

#include <unordered_map>
#include <unordered_set>


struct Role {
	struct {
		std::unordered_map<Concept*, ExistentialRoleRestriction*> negativelyOccurringExistentialRoleRestrictionUses;
		std::unordered_set<Role*> subRoleUses;
		std::unordered_map<Role*, Role*> roleChainUses;
	} attributes;
};
