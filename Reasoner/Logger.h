#pragma once

#include <ostream>
#include <sstream>

namespace Logger {
	void print(const std::string&);
	void unsafePrint(const std::string&);
	struct Section {
		Section(const std::string& name) {
			unsafePrint(" ------------------- " + name);
		}
		~Section() {
			unsafePrint(" ------------------- \n");
		}
	};
}

