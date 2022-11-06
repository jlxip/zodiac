#include <common.hpp>
#include <fcntl.h>
#include <unistd.h>

// Some auxiliary functions

std::vector<std::string> commaSplit(const char* str) {
	std::vector<std::string> ret;

	std::string aux;
	for(size_t i=0; str[i]; ++i) {
		char c = str[i];
		if(c == ',') {
			ret.push_back(aux);
			aux.clear();
		} else if(c != ' ') {
			// Ignore all spaces
			aux += c;
		}
	}

	if(aux.size())
		ret.push_back(aux);
	return ret;
}
