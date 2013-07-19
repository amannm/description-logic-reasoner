
struct Concept;
struct Role;

enum class DL {
	EL,
	ELH,
	ELR,
	ELO,
	ELRO,
};

namespace Indexer {
	void setDL(DL);
	void addConceptInclusion(Concept*, Concept*);
	void addRoleInclusion(Role*, Role*);
	void start(int);
};