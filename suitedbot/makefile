SOURCES = main.cpp player.cpp skeleton/runner.cpp skeleton/states.cpp skeleton/actions.cpp
HEADERS = player.hpp skeleton/bot.hpp skeleton/runner.hpp skeleton/states.hpp skeleton/actions.hpp

CXX = g++
CXXFLAGS = -Wall -std=c++11 -O3
LDLIBS = -lboost_system -lpthread OMPEval/lib/ompeval.a

cppbot: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o cppbot $(SOURCES) $(LDLIBS)

clean:
	rm cppbot
