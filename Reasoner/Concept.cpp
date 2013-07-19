
#include "Concept.h"
#include "Role.h"
#include "Saturation.h"

#include "Logger.h"

#include <iterator>


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



ExistentialConcept::ExistentialConcept(Role* r, Concept* c) :
	role(r),
	roleFiller(c) {
		attributes.ancestralConcepts.push_back(c);
		attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), c->attributes.ancestralConcepts.begin(), c->attributes.ancestralConcepts.end());
}



IntersectionConcept::IntersectionConcept(Concept* a, Concept* b) : 
	first(a),
	second(b) {
		attributes.ancestralConcepts.push_back(a);
		attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), a->attributes.ancestralConcepts.begin(), a->attributes.ancestralConcepts.end());
		attributes.ancestralConcepts.push_back(b);
		attributes.ancestralConcepts.insert(attributes.ancestralConcepts.end(), b->attributes.ancestralConcepts.begin(), b->attributes.ancestralConcepts.end());

}
