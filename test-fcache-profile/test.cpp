#include <cstdio>
#include <fstream> 

#include "fcache_profile.hpp"

int main() {

	fcache_profile profile("fcache_profile");

	std::ofstream ostream("example_text.txt", std::ios::out);

	for (uint64_t i = 0; i < (1024 * 1024); ++i) {
		ostream.write(reinterpret_cast<char*>(&i), sizeof(i));
	}

	ostream.close();

	std::ifstream istream("example_text.txt", std::ios::in);

	while (istream.good()) {
		istream.get();
	}

	istream.close();

	std::remove("example_text.txt");

  return 0;
}
