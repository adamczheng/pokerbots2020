#include "omp/EquityCalculator.h"
#include "omp/CardRange.h"
#include <iostream>
using namespace omp;
int main()
{
    omp::EquityCalculator eq;
	uint64_t board = CardRange::getCardMask("3s4c5h6d7s");
    eq.start({"89", "78"}, board);
    eq.wait();
    auto r = eq.getResults();
    std::cout << r.equity[0] << " " << r.equity[1] << std::endl;
}