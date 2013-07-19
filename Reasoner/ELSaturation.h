
namespace EL {

	struct Concept;
	struct Role;

	namespace Saturation {
		void addMessage(Concept*);
		void addMessage(Concept*, Concept*);
		void addMessage(Concept*, Role*, Concept*);
		void start(int);
	}
}