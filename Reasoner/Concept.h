#pragma once

#include "Logger.h"

#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>

struct Role;
struct Individual;

struct Concept {

	enum class Form : uint32_t {
		Unreferenced,
		Top,
		Bottom,
		Named,
		Intersection,
		Union,
		Complement,
		Nominal,
		ExistentialRestriction,
		UniversalRestriction,
	} form;

	std::mutex access;
	std::string name;

	union {
		Individual* member;
		Role* role;
		Concept* first;
		Concept* complement;
	};

	union {
		Concept* second;
		Concept* filler;
	};

	struct {
		std::vector<Concept*> subConceptUses;
		std::vector<std::pair<Role*, Concept*>> subsumedExistentialUses;
		std::vector<std::pair<Concept*, Concept*>> subsumedIntersectionUses;
	} attributes;

	struct {
		std::vector<Concept*> superConceptQueue;
		std::unordered_set<Concept*> superConcepts;
		std::unordered_map<Role*, std::unordered_set<Concept*>> predecessors;
		std::unordered_map<Role*, std::unordered_set<Concept*>> successors;
	} inferences;

	Concept() :
		first(nullptr), second(nullptr), form(Form::Unreferenced) {}

	Concept(Form f) :
		first(nullptr), second(nullptr), form(f) {}

	void log(const std::string& s) {
		Logger::print("<Concept " + name + "> " + s);
	}
};