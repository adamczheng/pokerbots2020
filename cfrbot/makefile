SOURCES = main.cpp player.cpp skeleton/runner.cpp skeleton/states.cpp skeleton/actions.cpp LUT.cpp
HEADERS = player.hpp skeleton/bot.hpp skeleton/runner.hpp skeleton/states.hpp skeleton/actions.hpp LUT.h

CXX = g++
CXXFLAGS = -Wall -std=c++11 -O3
LDLIBS = -lboost_system -lboost_program_options -lpthread

cppbot: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o cppbot $(SOURCES) $(LDLIBS)

clean:
	rm cppbot
