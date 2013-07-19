#pragma once

#include "Logger.h"

#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>


struct Concept;

struct Individual {

	std::mutex access;
	std::string name;

	enum class Form {
		Unreferenced,
		Named,
		Anonymous,
	} form;

	struct {
	} attributes;

	struct {
	} inferences;

	Individual() : 
		form(Form::Unreferenced) {}

	void log(const std::string& s) {
		Logger::print("<Individual " + name + "> " + s);
	}
};