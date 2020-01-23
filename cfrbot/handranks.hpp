#ifndef HAND_RANKS
#define HAND_RANKS

#include <cstdio>
#include <stdexcept>
#include <array>
#include <cstring>
using std::array;

class HandRanks {
	int HR[32487834];
public:
	HandRanks(const char* filename) {
		if (!load_lookup(filename))
			throw std::runtime_error("HandRanks file could not be loaded.");
	}

	int GetHandValue(array<int, 7> cards) {
		int p = HR[53 + cards[0] + 1];
		p = HR[p + cards[1] + 1];
		p = HR[p + cards[2] + 1];
		p = HR[p + cards[3] + 1];
		p = HR[p + cards[4] + 1];
		p = HR[p + cards[5] + 1];
		return HR[p + cards[6] + 1];
	}

	bool load_lookup(const char* filename) {
		memset(HR, 0, sizeof(HR));
		FILE* fin = fopen("handranks.dat", "rb");
		size_t bytesread = fread(HR, sizeof(HR), 1, fin);
		fclose(fin);
		return true;
	}

	~HandRanks() {}
};

#endif