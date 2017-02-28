/*******************************************************************************
 * test-fcache-profile.cpp
 *
 * Copyright (c) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <cstdio>
#include <fstream> 

#include "fcache_profile.hpp"

int main() {

	fcache_profile profile("fcache_profile");

	std::ofstream ostream("example_text.txt", std::ios::out);
	std::ofstream ostream2("other_example_text.txt", std::ios::out);

	for (uint64_t i = 0; i < (1024 * 1024); ++i) {
		ostream.write(reinterpret_cast<char*>(&i), sizeof(i));
		ostream2.write(reinterpret_cast<char*>(&i), sizeof(i));
	}

	ostream.close();
	ostream2.close();

	std::ifstream istream("example_text.txt", std::ios::in);

	while (istream.good()) {
		istream.get();
	}

	profile.log_message("Opening other ifstream");

	std::ifstream istream2("example_text.txt", std::ios::in);

	while (istream2.good()) {
		istream2.get();
	}

	istream.close();
	istream2.close();

	std::remove("example_text.txt");
	std::remove("other_example_text.txt");

  return 0;
}

/******************************************************************************/
