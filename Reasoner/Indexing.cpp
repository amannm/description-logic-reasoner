
#include "Indexing.h"
#include "Saturation.h"
#include "Concept.h"
#include "Role.h"
#include "Job.h"
#include "Logger.h"

namespace {

	std::function<void (Concept*, Concept*)> addConceptInclusionFunction;
	std::function<void (Role*, Role*)> addRoleInclusionFunction;
	std::function<void ()> clearFunction;

	Job indexingJob("Indexing");

	namespace EL {
		std::unordered_map<Concept*, int> conceptPositiveOccurenceMap;
		std::unordered_map<Concept*, int> conceptNegativeOccurenceMap;
		struct IndexLeftConcept {
			Concept* context;
			IndexLeftConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptNegativeOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (-) Atomic Concept");
						Saturation::EL::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 
						context->log("Indexing (-) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						context->filler->attributes.subsumedExistentialUses.emplace_back(context->role, context);
						context->role->attributes.subsumedExistentialUses.emplace_back(context->filler, context);
						indexingJob.addTask(IndexLeftConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (-) Intersection of " + context->first->name + " and " + context->second->name);
						context->first->attributes.subsumedIntersectionUses.emplace_back(context->second, context);
						context->second->attributes.subsumedIntersectionUses.emplace_back(context->first, context);
						indexingJob.addTask(IndexLeftConcept(context->first));
						indexingJob.addTask(IndexLeftConcept(context->second));
						break;
					}
				}
			}
		};
		struct IndexRightConcept {
			Concept* context;
			IndexRightConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptPositiveOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (+) Atomic Concept");
						Saturation::EL::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 					
						context->log("Indexing (+) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						indexingJob.addTask(IndexRightConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (+) Intersection of " + context->first->name + " and " + context->second->name);
						indexingJob.addTask(IndexRightConcept(context->first));
						indexingJob.addTask(IndexRightConcept(context->second));
						break;
					}
				}
			}
		};
	}
	namespace ELH {
		std::unordered_map<Role*, int> roleOccurenceMap;
		std::unordered_map<Concept*, int> conceptPositiveOccurenceMap;
		std::unordered_map<Concept*, int> conceptNegativeOccurenceMap;
		struct IndexRole {
			Role* context;
			IndexRole(Role* c) :
				context(c) {
			}
			void operator()() {
				if(roleOccurenceMap[context]++ == 0) {
					context->log("Indexing Role (+/-)");
					Saturation::ELH::initialize(context);
				}
			}
		};
		struct IndexLeftConcept {
			Concept* context;
			IndexLeftConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptNegativeOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (-) Atomic Concept");
						Saturation::ELH::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 
						context->log("Indexing (-) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						context->filler->attributes.subsumedExistentialUses.emplace_back(context->role, context);
						context->role->attributes.subsumedExistentialUses.emplace_back(context->filler, context);
						//saturationRoleInitializer(context->role);
						indexingJob.addTask(IndexRole(context->role));
						indexingJob.addTask(IndexLeftConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (-) Intersection of " + context->first->name + " and " + context->second->name);

						context->first->attributes.subsumedIntersectionUses.emplace_back(context->second, context);
						context->second->attributes.subsumedIntersectionUses.emplace_back(context->first, context);

						indexingJob.addTask(IndexLeftConcept(context->first));
						indexingJob.addTask(IndexLeftConcept(context->second));
						break;
					}
				}
			}
		};
		struct IndexRightConcept {
			Concept* context;
			IndexRightConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptPositiveOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (+) Atomic Concept");
						Saturation::ELH::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 					
						context->log("Indexing (+) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						indexingJob.addTask(IndexRole(context->role));
						indexingJob.addTask(IndexRightConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (+) Intersection of " + context->first->name + " and " + context->second->name);
						indexingJob.addTask(IndexRightConcept(context->first));
						indexingJob.addTask(IndexRightConcept(context->second));
						break;
					}
				}
			}
		};

	}
	namespace ELR {
		std::unordered_map<Role*, int> rolePositiveOccurenceMap;
		std::unordered_map<Role*, int> roleNegativeOccurenceMap;
		std::unordered_map<Concept*, int> conceptPositiveOccurenceMap;
		std::unordered_map<Concept*, int> conceptNegativeOccurenceMap;
		void clear() {
			rolePositiveOccurenceMap.clear();
			roleNegativeOccurenceMap.clear();
			conceptPositiveOccurenceMap.clear();
			conceptNegativeOccurenceMap.clear();
		}
		struct IndexLeftRole {
			Role* context;
			IndexLeftRole(Role* c) :
				context(c) {
			}
			void operator()() {
				if(roleNegativeOccurenceMap[context]++ == 0) {
					context->log("Indexing (-) Role");
					Saturation::ELR::initialize(context);
					if(context->form == Role::Form::Composition) {
						//saturationRoleInitializer(context);
						context->right->attributes.rightRoleCompositionUses.emplace_back(context->left, context);
						context->left->attributes.leftRoleCompositionUses.emplace_back(context->right, context);
						indexingJob.addTask(IndexLeftRole(context->left));
						indexingJob.addTask(IndexLeftRole(context->right));
					}
				}
			}
		};
		struct IndexRightRole {
			Role* context;
			IndexRightRole(Role* c) :
				context(c) {
			}
			void operator()() {
				if(rolePositiveOccurenceMap[context]++ == 0) {
					context->log("Indexing (+) Role");
					Saturation::ELR::initialize(context);
				}
			}
		};
		struct IndexLeftConcept {
			Concept* context;
			IndexLeftConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptNegativeOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (-) Atomic Concept");
						Saturation::ELR::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 
						context->log("Indexing (-) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						context->filler->attributes.subsumedExistentialUses.emplace_back(context->role, context);
						context->role->attributes.subsumedExistentialUses.emplace_back(context->filler, context);
						//saturationRoleInitializer(context->role);
						indexingJob.addTask(IndexLeftRole(context->role));
						indexingJob.addTask(IndexLeftConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (-) Intersection of " + context->first->name + " and " + context->second->name);

						context->first->attributes.subsumedIntersectionUses.emplace_back(context->second, context);
						context->second->attributes.subsumedIntersectionUses.emplace_back(context->first, context);

						indexingJob.addTask(IndexLeftConcept(context->first));
						indexingJob.addTask(IndexLeftConcept(context->second));
						break;
					}
				}
			}
		};
		struct IndexRightConcept {
			Concept* context;
			IndexRightConcept(Concept* c) :
				context(c) {
			}
			void operator()() {
				if(conceptPositiveOccurenceMap[context]++ == 0) {
					switch(context->form) {
					case Concept::Form::Named: 
						context->log("Indexing (+) Atomic Concept");
						Saturation::ELR::initialize(context);
						break;
					case Concept::Form::ExistentialRestriction: 					
						context->log("Indexing (+) Existential Restriction of role " + context->role->name + " and filler " + context->filler->name);
						indexingJob.addTask(IndexRightRole(context->role));
						//saturationRoleInitializer(context->role);
						indexingJob.addTask(IndexRightConcept(context->filler));
						break;
					case Concept::Form::Intersection: 
						context->log("Indexing (+) Intersection of " + context->first->name + " and " + context->second->name);
						indexingJob.addTask(IndexRightConcept(context->first));
						indexingJob.addTask(IndexRightConcept(context->second));
						break;
					}
				}
			}
		};
	}
}

void Indexer::setDL(DL dl) {
	switch(dl) {
	case DL::EL :
		Logger::unsafePrint("\nEL selected\n");
		addConceptInclusionFunction = [](Concept* left, Concept* right){
			left->attributes.subConceptUses.push_back(right);
			indexingJob.addTask(EL::IndexLeftConcept(left));
			indexingJob.addTask(EL::IndexRightConcept(right));
		};
		addRoleInclusionFunction = [](Role* left, Role* right){};
		clearFunction = [](){
			EL::conceptPositiveOccurenceMap.clear();
			EL::conceptNegativeOccurenceMap.clear();
		};
		break;
	case DL::ELH :
		Logger::unsafePrint("\nELH selected\n");
		addConceptInclusionFunction = [](Concept* left, Concept* right){
			left->attributes.subConceptUses.push_back(right);
			indexingJob.addTask(ELH::IndexLeftConcept(left));
			indexingJob.addTask(ELH::IndexRightConcept(right));
		};
		addRoleInclusionFunction = [](Role* left, Role* right){
			left->attributes.subRoleUses.push_back(right);
			indexingJob.addTask(ELH::IndexRole(left));
			indexingJob.addTask(ELH::IndexRole(right));
		};
		clearFunction = [](){
			ELH::roleOccurenceMap.clear();
			ELH::conceptPositiveOccurenceMap.clear();
			ELH::conceptNegativeOccurenceMap.clear();
		};
		break;
	case DL::ELO :
		break;
	case DL::ELR :
		Logger::unsafePrint("\nELR selected\n");
		addConceptInclusionFunction = [](Concept* left, Concept* right){
			left->attributes.subConceptUses.push_back(right);
			indexingJob.addTask(ELR::IndexLeftConcept(left));
			indexingJob.addTask(ELR::IndexRightConcept(right));
		};
		addRoleInclusionFunction = [](Role* left, Role* right){
			left->attributes.subRoleUses.push_back(right);
			indexingJob.addTask(ELR::IndexLeftRole(left));
			indexingJob.addTask(ELR::IndexRightRole(right));
		};
		clearFunction = [](){
			ELR::rolePositiveOccurenceMap.clear();
			ELR::roleNegativeOccurenceMap.clear();
			ELR::conceptPositiveOccurenceMap.clear();
			ELR::conceptNegativeOccurenceMap.clear();
		};
		break;
	case DL::ELRO :
		break;
	}
}
void Indexer::addConceptInclusion(Concept* left, Concept* right) {
	Logger::unsafePrint("Applying Concept Inclusion: " + left->name + " -> " + right->name);
	addConceptInclusionFunction(left, right);
}
void Indexer::addRoleInclusion(Role* left, Role* right) {
	Logger::unsafePrint("Applying Role Inclusion: " + left->name + " -> " + right->name);
	addRoleInclusionFunction(left, right);

}
void Indexer::start(int numThreads) {
	indexingJob.start(1);
	clearFunction();
}