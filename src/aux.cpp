#include <common.hpp>

// Some auxiliary functions

// How many threads in the CPU?
size_t nproc() {
	size_t a=11, b=0, c=1, d=0;
	asm volatile("cpuid"
				 : "=a" (a),
				   "=b" (b),
				   "=c" (c),
				   "=d" (d)
				 : "0" (a), "2" (c));
	return b;
}

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