
#include "EL.h"
#include "ELSaturation.h"

#include "Logger.h"

#include <iterator>

namespace EL {

	TopConcept* top;

	Concept* Concept::getTop() {
		if(top == nullptr) {
			top = new TopConcept();
		}
		return top;
	}

	Concept::Concept() {
		attributes.negativeOccurences = 0;
	}

	void Concept::process(Role* predicate, Concept* object) {
		std::lock_guard<std::mutex> lock(access);
		if(predicate != nullptr) {
			log("Processing predecessor (" + object->id + " " + predicate->id + ")");
			if(inferences.predecessors[predicate].insert(object).second) {
				log("New predecessor insertion: ("+ object->id + " " + predicate->id + ")");
				for(auto pair : predicate->attributes.subsumedExistentialUses)  {
					if(inferences.superConcepts.find(pair.first) != inferences.superConcepts.end()) {
						Saturation::addMessage(object, pair.second);
					}
				}
			}
			if(top != nullptr) {
				inferences.superConceptQueue.push_back(top);
			}
			inferences.superConceptQueue.push_back(this);
		}
		else {
			inferences.superConceptQueue.push_back(object);
		}
		while(inferences.superConceptQueue.size() != 0) {
			Concept* superConcept = inferences.superConceptQueue.back();
			inferences.superConceptQueue.pop_back();
			if(inferences.superConcepts.insert(superConcept).second) {
				log("New SuperConcept Insertion: " + superConcept->id);

				superConcept->addSubClass(this);

				for(auto superSuper : superConcept->attributes.subConceptUses) {
					log("Conclusion (Local): New SuperConcept " + superSuper->id);
					inferences.superConceptQueue.push_back(superSuper);
				}

				for(auto pair : superConcept->attributes.subsumedIntersectionUses) {
					if(inferences.superConcepts.find(pair.first) != inferences.superConcepts.end()) {	
						log("Conclusion (Local): New Intersection SuperConcept " + pair.second->id);
						inferences.superConceptQueue.push_back(pair.second);
					}
				}

				for(auto pair : superConcept->attributes.subsumedExistentialUses) {
					auto it = inferences.predecessors.find(pair.first);
					if(it != inferences.predecessors.end()) {
						for(auto p : it->second) {
							log("Conclusion (Global): New Relation " + p->id + "-" + pair.second->role->id + "->" + pair.second->roleFiller->id);
							Saturation::addMessage(pair.second->roleFiller, pair.second->role, p);
						}
					}
				}
			}
		}
	}

	ExistentialConcept::ExistentialConcept(Role* r, Concept* c) :
		role(r),
		roleFiller(c) {
			attributes.ancestralConcepts.push_back(c);
			attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), c->attributes.ancestralConcepts.begin(), c->attributes.ancestralConcepts.end());
	}
	void ExistentialConcept::addSubClass(Concept* sub) {
		log("Conclusion (Processing Context = " + sub->id + ")(Global): New Relation " + sub->id + "-" + role->id + "->" + roleFiller->id);
		Saturation::addMessage(roleFiller, role, sub);
	}
	void ExistentialConcept::index() {
		log("Indexing Existential Concept");
		if(attributes.negativeOccurences > 0) {
			role->attributes.subsumedExistentialUses.emplace_back(roleFiller, this);
			roleFiller->attributes.subsumedExistentialUses.emplace_back(role, this);
		}
	}

	IntersectionConcept::IntersectionConcept(Concept* a, Concept* b) : 
		first(a),
		second(b) {
			attributes.ancestralConcepts.push_back(a);
			attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), a->attributes.ancestralConcepts.begin(), a->attributes.ancestralConcepts.end());
			attributes.ancestralConcepts.push_back(b);
			attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), b->attributes.ancestralConcepts.begin(), b->attributes.ancestralConcepts.end());
	}
	void IntersectionConcept::addSubClass(Concept* sub) {
		log("Conclusion (Processing Context = " + sub->id + ")(Local): New SuperConcept " + first->id);
		log("Conclusion (Processing Context = " + sub->id + ")(Local): New SuperConcept " + second->id);
		sub->inferences.superConceptQueue.push_back(first);
		sub->inferences.superConceptQueue.push_back(second);
	}
	void IntersectionConcept::index() {
		log("Indexing Intersection Concept");
		if(attributes.negativeOccurences > 0) {
			first->attributes.subsumedIntersectionUses.emplace_back(second, this);
			second->attributes.subsumedIntersectionUses.emplace_back(first, this);
		}
	}

	void AtomicConcept::index() {
		log("Indexing Atomic Concept");			
		if(top != nullptr) {
			inferences.superConceptQueue.push_back(top);
		}
		Saturation::addMessage(this);
	}
}