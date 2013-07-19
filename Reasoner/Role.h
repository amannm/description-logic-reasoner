#pragma once

#include "Logger.h"

#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>


struct Concept;

struct Role {

	std::mutex access;
	std::string name;

	enum class Form {
		Unreferenced,
		Top,
		Bottom,
		Named,
		Inverse,
		Composition
	} form;

	union {
		Role* inverse;
		Role* left;
	};
	Role* right;

	struct {
		std::vector<Role*> subRoleUses;
		std::vector<std::pair<Concept*, Concept*>> subsumedExistentialUses;
		std::vector<std::pair<Role*, Role*>> leftRoleCompositionUses; 
		std::vector<std::pair<Role*, Role*>> rightRoleCompositionUses; 
	} attributes;

	struct {
		std::vector<Role*> superRoleQueue;
		std::unordered_set<Role*> superRoles;
	} inferences;

	Role() : 
		left(nullptr), right(nullptr), form(Role::Form::Unreferenced) {}

	Role(Form f) : 
		left(nullptr), right(nullptr), form(f) {}

	void log(const std::string& s) {
		Logger::print("<Role " + name + "> " + s);
	}
};