
#include "Saturation.h"
#include "Concept.h"
#include "Role.h"
#include "Job.h"

#include <mutex>
#include <queue>
#include <tuple>
#include <condition_variable>
#include <functional>

namespace {
	std::function<void (int)> saturateFunction;

	Job conceptSaturationJob("Concept Saturation");
	Job roleSaturationJob("Role Saturation");

	namespace EL {
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
	}

	namespace ELH {
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
		struct SuperConceptTask : ConceptTask {
			Concept* superConcept;
			SuperConceptTask(Concept* c, Concept* s) :
				ConceptTask(c), superConcept(s) {}
			void operator()();
		};
		struct PredecessorTask : ConceptTask {
			Role* role;
			Concept* predecessor;
			PredecessorTask(Concept* c, Role* r, Concept* p) :
				ConceptTask(c), role(r), predecessor(p) {}
			void operator()();
		};

		struct RoleTask {
			Role* context;
			RoleTask(Role* c) :
				context(c) {}
			void processQueue();
		};
		struct RoleInitializationTask : RoleTask {
			RoleInitializationTask(Role* r) :
				RoleTask(r) {}
			void operator()();
		};
		struct SuperRoleTask: RoleTask {
			Role* superRole;
			SuperRoleTask(Role* r, Role* s) :
				RoleTask(r), superRole(s) {}
			void operator()();
		};
	}

	namespace ELR {
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
		struct SuperConceptTask : ConceptTask {
			Concept* superConcept;
			SuperConceptTask(Concept* c, Concept* s) :
				ConceptTask(c), superConcept(s) {}
			void operator()();
		};
		struct PredecessorTask : ConceptTask {
			Role* role;
			Concept* predecessor;
			PredecessorTask(Concept* c, Role* r, Concept* p) :
				ConceptTask(c), role(r), predecessor(p) {}
			void operator()();
		};
		struct SuccessorTask : ConceptTask {
			Role* role;
			Concept* successor;
			SuccessorTask(Concept* c, Role* r, Concept* s) :
				ConceptTask(c), role(r), successor(s) {}
			void operator()();
		};
	}
}
//
//std::pair<std::function<void(Concept*)>, std::function<void(Role*)>> Saturation::getInitializers(DL dl) {
//	switch(dl) {
//	case DL::EL : 
//		Logger::unsafePrint("\nEL Selected\n");
//		return std::make_pair(
//			//EL saturation requires the use of 3 unique tasks: SuperConceptTask, PredecessorTask, and ConceptInitializationTask
//			[](Concept* c){},
//			//EL requires no role saturation stage
//			[](Role* c){}
//		);
//	//case DL::ELO : 
//	case DL::ELH :
//		Logger::unsafePrint("\nELH Selected\n");
//		return std::make_pair(
//			//EL->ELH requires the addition of an outer loop in PredecessorTask to address all role hierarchy relations
//			[](Concept* c){conceptSaturationJob.addTask(ELH::ConceptInitializationTask(c));},
//			//EL->ELH requires a role saturation stage to compute the transitive-reflexive closure of all atomic roles
//			[](Role* c){roleSaturationJob.addTask(ELH::RoleInitializationTask(c));}
//		);
//	case DL::ELR : 
//		Logger::unsafePrint("\nELR Selected\n");
//		return std::make_pair(
//			//ELH->ELR requires an additional SuccessorTask to support Role Compositions
//			//ELH->ELR requires a SuccessorTask entry point appended to PredecessorTask
//			[](Concept* c){conceptSaturationJob.addTask(ELR::ConceptInitializationTask(c));},
//			//ELR->ELH requires no changes in the role saturation stage
//			[](Role* c){roleSaturationJob.addTask(ELH::RoleInitializationTask(c));}
//		);
//	//case DL::ELRO :
//	default :
//		Logger::unsafePrint("\nUnsupported DL Selected (Saturation cannot be performed)\n");
//		return std::make_pair(
//			[](Concept* c){},
//			[](Role* c){}
//		);
//	}
//
//}


void EL::ConceptTask::processQueue() {	
	while(context->inferences.superConceptQueue.size() != 0) {
		Concept* superConcept = context->inferences.superConceptQueue.back();
		context->inferences.superConceptQueue.pop_back();
		if(context->inferences.superConcepts.insert(superConcept).second) {
			context->log("New SuperConcept Insertion: " + superConcept->name);

			switch(superConcept->form) {
			case Concept::Form::ExistentialRestriction: 
				context->log("Conclusion (Global): New Relation " + context->name + "-" +  superConcept->role->name + "->" + superConcept->filler->name);
				conceptSaturationJob.addTask(PredecessorTask(superConcept->filler, superConcept->role, context));
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
						conceptSaturationJob.addTask(PredecessorTask(pair.second->filler, pair.second->role, p));
					}
				}
			}
		}
	}
}
void EL::ConceptInitializationTask::operator()() {
	context->log("Processing initialization");
	if(context->access.try_lock()) {
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void EL::PredecessorTask::operator()() {
	if(context->access.try_lock()) {
		context->log("Processing predecessor (" + role->name + " " + predecessor->name + ")");
		if(context->inferences.predecessors[role].insert(predecessor).second) {
			context->log("New predecessor insertion: ("+ predecessor->name + " " + role->name + ")");
			for(auto pair : role->attributes.subsumedExistentialUses)  {
				if(context->inferences.superConcepts.find(pair.first) != context->inferences.superConcepts.end()) {
					conceptSaturationJob.addTask(SuperConceptTask(predecessor, pair.second));
				}
			}
		}
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void EL::SuperConceptTask::operator()() {
	context->log("Processing SuperConcept (" + superConcept->name + ")");
	if(context->access.try_lock()) {
		//std::lock_guard<std::mutex> lock(context->access);
		context->inferences.superConceptQueue.push_back(superConcept);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}

void ELH::ConceptTask::processQueue() {
	while(context->inferences.superConceptQueue.size() != 0) {
		Concept* superConcept = context->inferences.superConceptQueue.back();
		context->inferences.superConceptQueue.pop_back();
		if(context->inferences.superConcepts.insert(superConcept).second) {
			context->log("New SuperConcept Insertion: " + superConcept->name);

			switch(superConcept->form) {
			case Concept::Form::ExistentialRestriction:
				context->log("Conclusion (Global): New Relation " + context->name + "-" +  superConcept->role->name + "->" + superConcept->filler->name);
				conceptSaturationJob.addTask(PredecessorTask(superConcept->filler, superConcept->role, context));
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
						conceptSaturationJob.addTask(PredecessorTask(pair.second->filler, pair.second->role, p));
					}
				}
			}
		}
	}
}
void ELH::ConceptInitializationTask::operator()() {
	context->log("Processing initialization");
	if(context->access.try_lock()) {
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void ELH::SuperConceptTask::operator()() {
	context->log("Processing SuperConcept (" + superConcept->name + ")");
	if(context->access.try_lock()) {
		context->inferences.superConceptQueue.push_back(superConcept);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void ELH::PredecessorTask::operator()() {
	if(context->access.try_lock()) {
		context->log("Processing predecessor (" + predecessor->name + " " + role->name + ")");
		for(auto predecessorRole : role->inferences.superRoles) {
			if(context->inferences.predecessors[predecessorRole].insert(predecessor).second) {
				context->log("New predecessor insertion: ("+ predecessor->name + " " + predecessorRole->name + ")");
				for(auto pair : predecessorRole->attributes.subsumedExistentialUses)  {
					if(context->inferences.superConcepts.find(pair.first) != context->inferences.superConcepts.end()) {
						conceptSaturationJob.addTask(SuperConceptTask(predecessor, pair.second));
					}
				}
			}
		}
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}

void ELH::RoleTask::processQueue() {
	while(context->inferences.superRoleQueue.size() != 0) {
		Role* superRole = context->inferences.superRoleQueue.back();
		context->inferences.superRoleQueue.pop_back();
		if(context->inferences.superRoles.insert(superRole).second) {
			context->log("New SuperRole Insertion: " + superRole->name);
			for(auto superSuper : superRole->attributes.subRoleUses) {
				context->log("Conclusion (Local): New SuperRole " + superSuper->name);
				context->inferences.superRoleQueue.push_back(superSuper);
			}
		}
	}
}
void ELH::RoleInitializationTask::operator()() {
	context->log("Processing Initialization");
	if(context->access.try_lock()) {
		context->inferences.superRoleQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		roleSaturationJob.addTask(*this);
	}
}
void ELH::SuperRoleTask::operator()() {
	context->log("Processing SuperRole (" + superRole->name + ")");
	if(context->access.try_lock()) {
		context->inferences.superRoleQueue.push_back(superRole);
		processQueue();
		context->access.unlock();
	}
	else {
		roleSaturationJob.addTask(*this);
	}
}

void ELR::ConceptTask::processQueue() {
	while(context->inferences.superConceptQueue.size() != 0) {
		Concept* superConcept = context->inferences.superConceptQueue.back();
		context->inferences.superConceptQueue.pop_back();
		if(context->inferences.superConcepts.insert(superConcept).second) {
			context->log("New SuperConcept Insertion: " + superConcept->name);

			switch(superConcept->form) {
			case Concept::Form::ExistentialRestriction:
				context->log("Conclusion (Global): New Relation " + context->name + "-" +  superConcept->role->name + "->" + superConcept->filler->name);
				conceptSaturationJob.addTask(PredecessorTask(superConcept->filler, superConcept->role, context));
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
						conceptSaturationJob.addTask(PredecessorTask(pair.second->filler, pair.second->role, p));
					}
				}
			}
		}
	}
}
void ELR::ConceptInitializationTask::operator()() {
	context->log("Processing initialization");
	if(context->access.try_lock()) {
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void ELR::SuperConceptTask::operator()() {
	context->log("Processing SuperConcept (" + superConcept->name + ")");
	if(context->access.try_lock()) {
		context->inferences.superConceptQueue.push_back(superConcept);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void ELR::PredecessorTask::operator()() {
	context->log("Processing predecessor (" + predecessor->name + " " + role->name + ")");
	if(context->access.try_lock()) {
		for(auto predecessorRole : role->inferences.superRoles) {
			if(context->inferences.predecessors[predecessorRole].insert(predecessor).second) {
				context->log("New predecessor insertion: ("+ predecessor->name + " " + predecessorRole->name + ")");
				for(auto pair : predecessorRole->attributes.subsumedExistentialUses)  {
					if(context->inferences.superConcepts.find(pair.first) != context->inferences.superConcepts.end()) {
						conceptSaturationJob.addTask(SuperConceptTask(predecessor, pair.second));
					}
				}
			}
		}
		conceptSaturationJob.addTask(SuccessorTask(predecessor, role, context));
		context->inferences.superConceptQueue.push_back(context);
		processQueue();
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}
void ELR::SuccessorTask::operator()() {
	context->log("Processing successor (" + successor->name + " " + role->name + ")");	
	if(context->access.try_lock()) {
		for(auto successorRole : role->inferences.superRoles) {
			if(context->inferences.successors[successorRole].insert(successor).second) {
				context->log("New successor insertion: ("+ successor->name + " " + successorRole->name + ")");
				for(auto pair : successorRole->attributes.leftRoleCompositionUses)  {
					for(auto predecessor : context->inferences.predecessors[pair.first]) {
						context->log("Conclusion (Global): New Relation " + predecessor->name + "-" + pair.second->name + "->" + successor->name);
						conceptSaturationJob.addTask(PredecessorTask(successor, pair.second, predecessor));
					}
				}
			}
		}
		context->access.unlock();
	}
	else {
		conceptSaturationJob.addTask(*this);
	}
}

void Saturation::saturate(int numThreads) {
	roleSaturationJob.start(numThreads);
	conceptSaturationJob.start(numThreads);
}

void Saturation::EL::initialize(Concept* c) {
	conceptSaturationJob.addTask(::EL::ConceptInitializationTask(c));
}

void Saturation::ELH::initialize(Concept* c) {
	conceptSaturationJob.addTask(::ELH::ConceptInitializationTask(c));
}
void Saturation::ELH::initialize(Role* r) {
	conceptSaturationJob.addTask(::ELH::RoleInitializationTask(r));
}

void Saturation::ELR::initialize(Concept* c) {
	conceptSaturationJob.addTask(::ELR::ConceptInitializationTask(c));
}
void Saturation::ELR::initialize(Role* r) {
	conceptSaturationJob.addTask(::ELH::RoleInitializationTask(r));
}