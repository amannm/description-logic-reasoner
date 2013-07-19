extern "C" {
#include "Reasoner.h"
}
#include "Logger.h"
#include "Concept.h"
#include "Role.h"
#include "Individual.h"
#include "Indexing.h"
#include "Saturation.h"

#include <thread>
#include <unordered_map>
#include <unordered_set>


namespace std {
	template<typename T, typename U>
	class hash<pair<T*, U*>> {
	public:
		size_t operator()(const pair<T*, U*>& other) const {
			return hash<T*>()(other.first) ^ hash<U*>()(other.second);
		}
	};
}


namespace {

	struct ABox {
		std::vector<Individual*> individuals;
		std::vector<std::tuple<Role*, Individual*, Individual*>> roleAssertion;
		std::vector<std::pair<Concept*, Individual*>> conceptAssertion;
	} abox;

	struct RBox {
		std::vector<Role*> atomic;
		std::unordered_map<std::pair<Role*, Role*>, Role*> composition;

		std::vector<std::pair<Role*, Role*>> roleInclusion;
		std::vector<std::pair<Role*, Concept*>> domainRestriction;
		std::vector<std::pair<Role*, Concept*>> rangeRestriction;
	} rbox;

	struct CBox {
		std::vector<Concept*> atomic;
		Concept* top;
		Concept* bottom;
		std::unordered_map<std::pair<Concept*, Concept*>, Concept*> intersection;
		std::unordered_map<std::pair<Role*, Concept*>, Concept*> existentialRestriction;
		std::unordered_map<Individual*, Concept*> nominal;
		//std::vector<Concept*> concreteDomain;

		std::vector<std::pair<Concept*, Concept*>> conceptInclusion;
	} cbox;

	char rcount = 'R';
	char ccount = 'A';

	DL determineDL() {
		DL dl = DL::EL;
		if(rbox.roleInclusion.size() > 0) {
			dl  = DL::ELH;
			if(rbox.composition.size() > 0) {
				dl = DL::ELR;
				if(cbox.nominal.size() > 0) {
					dl = DL::ELRO;
				}
			}
		}
		else {
			if(cbox.nominal.size() > 0) {
				dl = DL::ELO;
			}
		}
		return dl;
	}

	int queryInclusions(Concept* c, Concept*** list) {
		int size = c->inferences.superConcepts.size();
		Concept** newList = new Concept*[size];
		int i = 0;
		for(auto p : c->inferences.superConcepts) {
			newList[i] = p;
			i++;
		}
		*list = newList;
		return size;
	}
	int queryInclusions(Role* r, Role*** list) {
		int size = r->inferences.superRoles.size();
		Role** newList = new Role*[size];
		int i = 0;
		for(auto p : r->inferences.superRoles) {
			newList[i] = p;
			i++;
		}
		*list = newList;
		return size;
	}
	int queryPredecessors(Concept* c, Role* r, Concept*** list) {

		if(c->inferences.predecessors.find(r) != c->inferences.predecessors.end()) {
			int size =  c->inferences.predecessors[r].size();
			Concept** newList = new Concept*[size];
			int i = 0;
			for(auto p : c->inferences.predecessors[r]) {
				newList[i] = p;
				i++;
			}
			*list = newList;
			return size;
		}
		else {
			*list = nullptr;
			return 0;
		}
	}
	int queryBackwardRoles(Concept* c, Role*** list) {
		int size = c->inferences.predecessors.size();
		Role** newList = new Role*[size];
		int i = 0;
		for(auto p : c->inferences.predecessors) {
			newList[i] = p.first;
			i++;
		}
		*list = newList;
		return size;
	}
	int querySuccessors(Concept* c, Role* r, Concept*** list) {

		if(c->inferences.successors.find(r) != c->inferences.successors.end()) {
			int size =  c->inferences.successors[r].size();
			Concept** newList = new Concept*[size];
			int i = 0;
			for(auto p : c->inferences.successors[r]) {
				newList[i] = p;
				i++;
			}
			*list = newList;
			return size;
		}
		else {
			*list = nullptr;
			return 0;
		}
	}
	int queryForwardRoles(Concept* c, Role*** list) {
		int size = c->inferences.successors.size();
		Role** newList = new Role*[size];
		int i = 0;
		for(auto p : c->inferences.successors) {
			newList[i] = p.first;
			i++;
		}
		*list = newList;
		return size;
	}
}


Concept* Concept_Atomic_create() {
	auto p = new Concept(Concept::Form::Named);
	p->name.push_back(ccount++);
	cbox.atomic.push_back(p);
	p->log("New Atomic Concept");
	return p;
}
Concept* Concept_Top() {
	if(cbox.top == nullptr) {
		cbox.top = new Concept(Concept::Form::Top);
		cbox.top->name = "Top";
		cbox.top->log("New Top");
	}
	return cbox.top;
}
Concept* Concept_Bottom() {
	if(cbox.bottom == nullptr) {
		cbox.bottom = new Concept(Concept::Form::Bottom);
		cbox.bottom->name = "Bottom";
		cbox.bottom->log("New Bottom");
	}
	return cbox.bottom;
}
Concept* Concept_Nominal_create(Individual* i) {
	return nullptr;
	//TODO
}
Concept* Concept_ExistentialRestriction_create(Role* r, Concept* c) {
	auto pair = std::make_pair(r, c);
	if(cbox.existentialRestriction.find(pair) == cbox.existentialRestriction.end()) {
		auto p = new Concept(Concept::Form::ExistentialRestriction);
		p->role= r;
		p->filler = c;
		p->name.push_back(ccount++);
		cbox.existentialRestriction.emplace(pair, p);
		p->log("New Existential Restriction Concept of role " + r->name + " and filler " + c->name );
		return p;
	}
	else {
		return cbox.existentialRestriction[pair];
	}
}
Concept* Concept_Intersection_create(Concept* a,  Concept* b) {
	auto pair = std::make_pair(a, b);
	if(cbox.intersection.find(pair) == cbox.intersection.end()) {
		auto p = new Concept(Concept::Form::Intersection);
		p->first = a;
		p->second = b;
		p->name.push_back(ccount++);
		cbox.intersection.emplace(pair, p);
		p->log("New Intersection Concept of " + a->name + " and " + b->name);
		return p;
	}
	else {
		return cbox.intersection[pair];
	}
}

Role* Role_Composition_create(Role* a, Role* b) {
	auto pair = std::make_pair(a, b);
	if(rbox.composition.find(pair) == rbox.composition.end()) {
		auto p = new Role(Role::Form::Composition);
		p->left = a;
		p->right = b;
		p->name.push_back(rcount++);
		rbox.composition.emplace(pair, p);
		p->log("New Composition");
		return p;
	}
	else {
		return rbox.composition[pair];
	}
}

Role* Role_Atomic_create() {
	auto p = new Role(Role::Form::Named);
	p->name.push_back(rcount++);
	rbox.atomic.push_back(p);
	p->log("New Atomic Role");
	return p;
}

void Axiom_Concept_Inclusion_apply(Concept* sub, Concept* super) {
	cbox.conceptInclusion.emplace_back(sub, super);
}
void Axiom_Concept_Equivalence_apply(Concept* a, Concept* b) {
	cbox.conceptInclusion.emplace_back(a, b);
	cbox.conceptInclusion.emplace_back(b, a);
}
void Axiom_Role_Inclusion_apply(Role* sub, Role* super) {
	rbox.roleInclusion.emplace_back(sub, super);
}
void Axiom_Role_Equivalence_apply(Role* a, Role* b) {
	rbox.roleInclusion.emplace_back(a, b);
	rbox.roleInclusion.emplace_back(b, a);
}



void Reasoner_start(int numThreads) {
	//auto initPair = Saturation::setDL(determineDL());
	Indexer::setDL(determineDL());
	for(auto pair : rbox.roleInclusion) {
		Indexer::addRoleInclusion(pair.first, pair.second);
	}
	for(auto pair : cbox.conceptInclusion) {
		Indexer::addConceptInclusion(pair.first, pair.second);
	}
	Indexer::start(numThreads);
	Saturation::saturate(numThreads);
}



int Concept_Inclusions_query(Concept* c, Concept*** list) {
	return queryInclusions(c, list);
}
int Role_Inclusions_query(Role* r, Role*** list) {
	return queryInclusions(r, list);
}
int Concept_BackwardLinkedConcepts_query(Concept* c, Role* r, Concept*** list) {
	return queryPredecessors(c, r, list);
}
int Concept_BackwardLinkedRoles_query(Concept* c, Role*** list) {
	return queryBackwardRoles(c, list);
}
int Concept_ForwardLinkedConcepts_query(Concept* c, Role* r, Concept*** list) {
	return querySuccessors(c, r, list);
}
int Concept_ForwardLinkedRoles_query(Concept* c, Role*** list) {
	return queryForwardRoles(c, list);
}
