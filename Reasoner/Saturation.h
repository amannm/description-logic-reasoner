
struct Role;
struct Concept;

namespace Saturation {

	namespace EL {
		void initialize(Concept*);
	}
	namespace ELH {
		void initialize(Concept*);
		void initialize(Role*);
	}
	namespace ELR {
		void initialize(Concept*);
		void initialize(Role*);
	}

	void saturate(int);
}