#include "Concept.h"
#include "Role.h"
#include "Individual.h"
#include "Job.h"
#include <unordered_map>



namespace EL {

	Job saturationJob;
	Job indexingJob;

	std::unordered_map<Concept*, int> conceptPositiveOccurenceMap;
	std::unordered_map<Concept*, int> conceptNegativeOccurenceMap;

	Concept topConcept;

	uint32_t numNamedConcepts;
	Concept* namedConcepts;

	uint32_t numNamedRoles;
	Role* namedRoles;

	uint32_t numIntersectionConcepts;
	Concept* intersectionConcepts;

	uint32_t numExistentialRestrictionConcepts;
	Concept* existentialRestrictionConcepts;

	uint32_t numConceptInclusions;
	ConceptInclusion* conceptInclusions;

	void initialize(Concept* c) {
		saturationJob.addTask(ConceptInitializationTask(c));
	}
	struct ConceptTask {
		Concept* context;
		ConceptTask(Concept* c) :
			context(c) {}
		void processQueue();
	};
	struct ConceptInitializationTask : ConceptTask {
		ConceptInitializationTask(Concept* c) :
			ConceptTask(c) {}
		void operator()();
	};
	struct PredecessorTask : ConceptTask {
		Role* role;
		Concept* predecessor;
		PredecessorTask(Concept* c, Role* r, Concept* p) :
			ConceptTask(c), role(r), predecessor(p) {}
		void operator()();
	};
	struct SuperConceptTask : ConceptTask {
		Concept* superConcept;
		SuperConceptTask(Concept* c, Concept* s) :
			ConceptTask(c), superConcept(s) {}
		void operator()();
	};

	void ConceptTask::processQueue() {	
		while(context->inferences.superConceptQueue.size() != 0) {
			Concept* superConcept = context->inferences.superConceptQueue.back();
			context->inferences.superConceptQueue.pop_back();
			if(context->inferences.superConcepts.insert(superConcept).second) {
				context->log("New SuperConcept Insertion: " + superConcept->name);

				switch(superConcept->form) {
				case Concept::Form::ExistentialRestriction: 
					context->log("Conclusion (Global): New Relation " + context->name + "-" +  superConcept->role->name + "->" + superConcept->filler->name);
					saturationJob.addTask(PredecessorTask(superConcept->filler, superConcept->role, context));
					break;
				case Concept::Form::Intersection: 
					context->log("Conclusion (Local): New SuperConcept " + superConcept->first->name);
					context->log("Conclusion (Local): New SuperConcept " + superConcept->second->name);
					context->inferences.superConceptQueue.push_back(superConcept->first);
					context->inferences.superConceptQueue.push_back(superConcept->second);
					break;
				}

				for(auto superSuper : superConcept->attributes.subConceptUses) {
					context->log("Conclusion (Local): New SuperConcept " + superSuper->name);
					context->inferences.superConceptQueue.push_back(superSuper);
				}
				for(auto pair : superConcept->attributes.subsumedIntersectionUses) {
					if(context->inferences.superConcepts.find(pair.first) != context->inferences.superConcepts.end()) {	
						context->log("Conclusion (Local): New Intersection SuperConcept " + pair.second->name);
						context->inferences.superConceptQueue.push_back(pair.second);
					}
				}
				for(auto pair : superConcept->attributes.subsumedExistentialUses) {
					auto it = context->inferences.predecessors.find(pair.first);
					if(it != context->inferences.predecessors.end()) {
						for(auto p : it->second) {
							context->log("Conclusion (Global): New Relation " + p->name + "-" + pair.second->role->name + "->" + pair.second->filler->name);
							saturationJob.addTask(PredecessorTask(pair.second->filler, pair.second->role, p));
						}
					}
				}
			}
		}
	}
	void ConceptInitializationTask::operator()() {
		context->log("Processing initialization");
		if(context->access.try_lock()) {
			context->inferences.superConceptQueue.push_back(context);
			processQueue();
			context->access.unlock();
		}
		else {
			saturationJob.addTask(*this);
		}
	}
	void PredecessorTask::operator()() {
		if(context->access.try_lock()) {
			context->log("Processing predecessor (" + role->name + " " + predecessor->name + ")");
			if(context->inferences.predecessors[role].insert(predecessor).second) {
				context->log("New predecessor insertion: ("+ predecessor->name + " " + role->name + ")");
				for(auto pair : role->attributes.subsumedExistentialUses)  {
					if(context->inferences.superConcepts.find(pair.first) != context->inferences.superConcepts.end()) {
						saturationJob.addTask(SuperConceptTask(predecessor, pair.second));
					}
				}
			}
			context->inferences.superConceptQueue.push_back(context);
			processQueue();
			context->access.unlock();
		}
		else {
			saturationJob.addTask(*this);
		}
	}
	void SuperConceptTask::operator()() {
		context->log("Processing SuperConcept (" + superConcept->name + ")");
		if(context->access.try_lock()) {
			//std::lock_guard<std::mutex> lock(context->access);
			context->inferences.superConceptQueue.push_back(superConcept);
			processQueue();
			context->access.unlock();
		}
		else {
			saturationJob.addTask(*this);
		}
	}

	struct IndexLeftConceptTask {
		Concept* context;
		IndexLeftConceptTask(Concept* c) :
			context(c) {
		}
		void operator()() {
			if(conceptNegativeOccurenceMap[context]++ == 0) {
				switch(context->form) {
				case Concept::Form::Named: 
					context->log("Indexing (-) Atomic Concept");
					initialize(context);
					break;
				case Concept::Form::ExistentialRestriction: 
					context->log("Indexing (-) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
					context->filler->attributes.subsumedExistentialUses.emplace_back(context->role, context);
					context->role->attributes.subsumedExistentialUses.emplace_back(context->filler, context);
					indexingJob.addTask(IndexLeftConceptTask(context->filler));
					break;
				case Concept::Form::Intersection: 
					context->log("Indexing (-) Intersection of " + context->first->name + " and " + context->second->name);
					context->first->attributes.subsumedIntersectionUses.emplace_back(context->second, context);
					context->second->attributes.subsumedIntersectionUses.emplace_back(context->first, context);
					indexingJob.addTask(IndexLeftConceptTask(context->first));
					indexingJob.addTask(IndexLeftConceptTask(context->second));
					break;
				}
			}
		}
	};
	struct IndexRightConceptTask {
		Concept* context;
		IndexRightConceptTask(Concept* c) :
			context(c) {
		}
		void operator()() {
			if(conceptPositiveOccurenceMap[context]++ == 0) {
				switch(context->form) {
				case Concept::Form::Named: 
					context->log("Indexing (+) Atomic Concept");
					initialize(context);
					break;
				case Concept::Form::ExistentialRestriction: 					
					context->log("Indexing (+) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
					indexingJob.addTask(IndexRightConceptTask(context->filler));
					break;
				case Concept::Form::Intersection: 
					context->log("Indexing (+) Intersection of " + context->first->name + " and " + context->second->name);
					indexingJob.addTask(IndexRightConceptTask(context->first));
					indexingJob.addTask(IndexRightConceptTask(context->second));
					break;
				}
			}
		}
	};

	Concept* getConceptReference(FILE* stream) {

		uint32_t type;
		fread(&type, 4, 1, stream);
		uint32_t index;
		fread(&index, 4, 1, stream);

		switch(type) {
		case Concept::Form::Named :
			namedConcepts[index].form == Concept::Form::Named;
			return &namedConcepts[index];
		case Concept::Form::Top :
			topConcept.form = Concept::Form::Top;
			return &namedConcepts[index];
		case Concept::Form::Intersection :
			intersectionConcepts[index].form = Concept::Form::Intersection;
			return &intersectionConcepts[index];
		case Concept::Form::ExistentialRestriction :
			existentialRestrictionConcepts[index].form = Concept::Form::ExistentialRestriction;
			return &existentialRestrictionConcepts[index];
		}
	}

	Role* getRoleReference(FILE* stream) {
		uint32_t index;
		fread(&index, 4, 1, stream);
		return &namedRoles[index];
	}

	void parseNamedConcepts(FILE* stream) {
		printf("Parsing %u Named Concepts", fread(&numNamedConcepts, 4, 1, stream));
		namedConcepts = new Concept[numNamedConcepts];
		for(uint32_t i = 0; i < numNamedConcepts; ++i) {
			char ch = fgetc(stream);
			while(ch > 0) {
				namedConcepts[i].name.push_back(ch);
				ch = fgetc(stream);
			}
			namedConcepts[i].form == Concept::Form::Named;
		}
	}

	void parseNamedRoles(FILE* stream) {
		printf("Parsing %u Named Roles", fread(&numNamedRoles, 4, 1, stream));
		namedRoles = new Role[numNamedRoles];
		for(uint32_t i = 0; i < numNamedRoles; ++i) {
			char ch = fgetc(stream);
			while(ch > 0) {
				namedRoles[i].name.push_back(ch);
				ch = fgetc(stream);
			}
			namedRoles[i].form == Role::Form::Named;
		}
	}

	void parseIntersectionConcepts(std::istream& stream) {
		printf("Parsing %u Intersection Concepts", fread(&numIntersectionConcepts, 4, 1, stream));
		intersectionConcepts = new Concept[numIntersectionConcepts];
		for(uint32_t i = 0; i < numIntersectionConcepts; ++i) {
			intersectionConcepts[i].first = getConceptReference(stream);
			intersectionConcepts[i].second = getConceptReference(stream);
			intersectionConcepts[i].form == Concept::Form::Intersection;
		}
	}

	void parseExistentialRestrictionConcepts(FILE* stream) {
		printf("Parsing %u Existential Restriction Concepts", fread(&numExistentialRestrictionConcepts, 4, 1, stream));
		existentialRestrictionConcepts = new Concept[numExistentialRestrictionConcepts];
		for(uint32_t i = 0; i < numExistentialRestrictionConcepts; ++i) {
			existentialRestrictionConcepts[i].role = getRoleReference(stream);
			existentialRestrictionConcepts[i].filler = getConceptReference(stream);
		}
	}

	void parseConceptInclusions(FILE* stream) {
		printf("Parsing %u Concept Inclusions", fread(&numConceptInclusions, 4, 1, stream));
		conceptInclusions = new ConceptInclusion[numConceptInclusions];
		for(uint32_t i = 0; i < numConceptInclusions; ++i) {
			conceptInclusions[i].sub = getConceptReference(stream);
			conceptInclusions[i].super = getConceptReference(stream);
		}
	}
}

//
//namespace {
//	Concept topConcept;
//	Concept bottomConcept;
//
//	Role topRole;
//	Role bottomRole;
//
//	uint32_t numNamedConcepts;
//	Concept* namedConcepts; //includes top and bottom as first two (reserved indices)
//	uint32_t numNamedRoles;
//	Role* namedRoles; 
//	uint32_t numNamedIndividuals;
//	Individual* namedIndividuals;
//	//
//	uint32_t numIntersectionConcepts;
//	Concept* intersectionConcepts;
//	uint32_t numUnionConcepts;
//	Concept* unionConcepts;
//	uint32_t numComplementConcepts;
//	Concept* complementConcepts;
//	uint32_t numNominalConcepts;
//	Concept* nominalConcepts;
//	uint32_t numUniversalRestrictionConcepts;
//	Concept* universalRestrictionConcepts;
//	uint32_t numExistentialRestrictionConcepts;
//	Concept* existentialRestrictionConcepts;
//
//	uint32_t numInverseRoles;
//	Role* inverseRoles;
//	//
//	uint32_t numConceptInclusions;
//	ConceptInclusion* conceptInclusions;
//	uint32_t numRoleInclusions;
//	RoleInclusion* roleInclusions;
//}
/*
Concept* getConceptReference(FILE* stream) {

	uint32_t type;
	fread(&type, 4, 1, stream);
	uint32_t index;
	fread(&index, 4, 1, stream);

	switch(type) {
	case Concept::Form::Named :
		namedConcepts[index].form == Concept::Form::Named;
		return &namedConcepts[index];
	case Concept::Form::Top :
		topConcept.form = Concept::Form::Top;
		return &namedConcepts[index];
	case Concept::Form::Bottom :
		bottomConcept.form = Concept::Form::Bottom;
		return &namedConcepts[index];
	case Concept::Form::Intersection :
		intersectionConcepts[index].form = Concept::Form::Intersection;
		return &intersectionConcepts[index];
	case Concept::Form::Union :
		unionConcepts[index].form = Concept::Form::Union;
		return &unionConcepts[index];
	case Concept::Form::Complement :
		complementConcepts[index].form = Concept::Form::Complement;
		return &complementConcepts[index];
	case Concept::Form::Nominal :
		nominalConcepts[index].form = Concept::Form::Nominal;
		return &nominalConcepts[index];
	case Concept::Form::UniversalRestriction :
		universalRestrictionConcepts[index].form = Concept::Form::UniversalRestriction;
		return &universalRestrictionConcepts[index];
	case Concept::Form::ExistentialRestriction :
		existentialRestrictionConcepts[index].form = Concept::Form::ExistentialRestriction;
		return &existentialRestrictionConcepts[index];
	}
}
Role* getRoleReference(FILE* stream) {

	uint32_t type;
	fread(&type, 4, 1, stream);
	uint32_t index;
	fread(&index, 4, 1, stream);

	switch(type) {
	case Role::Form::Named :
		return &namedRoles[index];
	case Role::Form::Top :
		return &topRole;
	case Role::Form::Bottom :
		return &bottomRole;
	case Role::Form::Inverse :
		return &inverseRoles[index];
	}
}
Individual* getIndividualReference(FILE* stream) {
	uint32_t index;
	fread(&index, 4, 1, stream);
	return &namedIndividuals[index];
}
RoleInclusion getRoleInclusion(FILE* stream) {
	RoleInclusion ri;

	uint32_t length;
	fread(&length, 4, 1, stream);

	uint32_t* indices = new uint32_t[length];
	fread(&indices, 4, length, stream);

	ri.sub = new Role(Role::Form::Composition);
	ri.sub->right = &namedRoles[indices[length--]];
	ri.sub->left = &namedRoles[indices[length--]];
	while(--length >= 0) {
		Role* leftSide = new Role(Role::Form::Composition);
		leftSide->right = ri.sub;
		leftSide->left = &namedRoles[indices[length]];
		ri.sub = leftSide;
	}
	ri.super = getRoleReference(stream);
	return ri;
}
*/
/*
void parseNamedConcepts(FILE* stream) {
	printf("Parsing %u Named Concepts", fread(&numNamedConcepts, 4, 1, stream));
	namedConcepts = new Concept[numNamedConcepts];
	for(uint32_t i = 0; i < numNamedConcepts; ++i) {
		char ch = fgetc(stream);
		while(ch > 0) {
			namedConcepts[i].name.push_back(ch);
			ch = fgetc(stream);
		}
	}
}

void parseNamedRoles(FILE* stream) {
	printf("Parsing %u Named Roles", fread(&numNamedRoles, 4, 1, stream));
	namedRoles = new Role[numNamedRoles];
	for(uint32_t i = 0; i < numNamedRoles; ++i) {
		char ch = fgetc(stream);
		while(ch > 0) {
			namedRoles[i].name.push_back(ch);
			ch = fgetc(stream);
		}
	}
}

void parseIntersectionConcepts(FILE* stream) {
	printf("Parsing %u Intersection Concepts", fread(&numIntersectionConcepts, 4, 1, stream));
	intersectionConcepts = new Concept[numIntersectionConcepts];
	for(uint32_t i = 0; i < numIntersectionConcepts; ++i) {
		intersectionConcepts[i].first = getConceptReference(stream);
		intersectionConcepts[i].second = getConceptReference(stream);
	}
}

void parseExistentialRestrictionConcepts(FILE* stream) {
	printf("Parsing %u Existential Restriction Concepts", fread(&numExistentialRestrictionConcepts, 4, 1, stream));
	existentialRestrictionConcepts = new Concept[numExistentialRestrictionConcepts];
	for(uint32_t i = 0; i < numExistentialRestrictionConcepts; ++i) {
		existentialRestrictionConcepts[i].role = getRoleReference(stream);
		existentialRestrictionConcepts[i].filler = getConceptReference(stream);
	}
}

void parseConceptInclusions(FILE* stream) {
	printf("Parsing %u Concept Inclusions", fread(&numConceptInclusions, 4, 1, stream));
	conceptInclusions = new ConceptInclusion[numConceptInclusions];
	for(uint32_t i = 0; i < numConceptInclusions; ++i) {
		conceptInclusions[i].sub = getConceptReference(stream);
		conceptInclusions[i].super = getConceptReference(stream);
	}
}
void parse(FILE* stream) {



	printf("Parsing %u Union Concepts", fread(&numUnionConcepts, 4, 1, stream));
	unionConcepts = new Concept[numUnionConcepts];
	for(uint32_t i = 0; i < numUnionConcepts; ++i) {
		unionConcepts[i].first = getConceptReference(stream);
		unionConcepts[i].second = getConceptReference(stream);
	}
	printf("Parsing %u Complement Concepts", fread(&numComplementConcepts, 4, 1, stream));
	complementConcepts = new Concept[numComplementConcepts];
	for(uint32_t i = 0; i < numComplementConcepts; ++i) {
		complementConcepts[i].complement = getConceptReference(stream);
	}
	printf("Parsing %u Nominal Concepts", fread(&numNominalConcepts, 4, 1, stream));
	nominalConcepts = new Concept[numNominalConcepts];
	for(uint32_t i = 0; i < numNominalConcepts; ++i) {
		nominalConcepts[i].member = getIndividualReference(stream);
	}
	printf("Parsing %u Universal Restriction Concepts", fread(&numUniversalRestrictionConcepts, 4, 1, stream));
	universalRestrictionConcepts = new Concept[numUniversalRestrictionConcepts];
	for(uint32_t i = 0; i < numUniversalRestrictionConcepts; ++i) {
		existentialRestrictionConcepts[i].role = getRoleReference(stream);
		existentialRestrictionConcepts[i].filler = getConceptReference(stream);
	}

	printf("Parsing %u Inverse Roles", fread(&numInverseRoles, 4, 1, stream));
	inverseRoles = new Role[numInverseRoles];
	for(uint32_t i = 0; i < numInverseRoles; ++i) {
		inverseRoles[i].inverse = getRoleReference(stream);
	}


	printf("Parsing %u Role Inclusions", fread(&numRoleInclusions, 4, 1, stream));
	roleInclusions = (RoleInclusion*) malloc(sizeof(RoleInclusion) * numRoleInclusions);
	for(uint32_t i = 0; i < numRoleInclusions; ++i) {
		roleInclusions[i] = getRoleInclusion(stream);
	}
}
//concept inclusion axioms


*/
