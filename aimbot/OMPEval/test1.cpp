#include "omp/EquityCalculator.h"
#include "omp/CardRange.h"
#include <iostream>
#include <bits/stdc++.h>
using namespace omp;
using namespace std;
int main()
{
    omp::EquityCalculator eq;
	uint64_t board = CardRange::getCardMask("3s4c5h6d7s");
	cout << bitset<64>(board) << endl;
	cout << "test" << endl;
    //eq.start({"89", "78"}, board);
    eq.start({"78","89"},board);
    eq.wait();
    auto r = eq.getResults();
    std::cout << r.equity[0] << " " << r.equity[1] << std::endl;
}
