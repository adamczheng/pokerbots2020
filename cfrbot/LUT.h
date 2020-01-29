
#ifndef LUT_H
#define LUT_H

#pragma once

#include <fstream>
#include <map>
#include <string>
#include <unordered_map>


using namespace std;

class Poker_Abstraction {
public:
	int hr[32487834];
	void inittheevaluator();
	Poker_Abstraction();
	int compare_two_hands(int c1_hero, int c2_hero, int c1_opp, int c2_opp, int c3, int c4, int c5, int c6, int c7);
};



/*class CLutPreFlop
{
public:

	enum
	{
		TABLE_SIZE = 169,
	};

	float* data;

	map<pair<int,int>,int> preflop_table;


	CLutPreFlop();
	~CLutPreFlop();

	void load(char[]);
	void generate(char[]);
	int g_index(int, int);
	void prepare();
};*/

class CLutFlop
{
public:

	enum
	{
		TABLE_SIZE = 1886886,
		INDEX_SIZE = 2197,
	};

	float* data;

	int* flopindex;

	map<string, int> suitoffset;
	int preflop_table[55][55];
	CLutFlop();
	~CLutFlop();

	void load(char[]);
	void generate(char[]);
	int g_index(int, int, int[]);
	void write_suitfile();
	void read_suitfile(string filename);
	string g_pattern(int c1, int c2, int board[]);
	string replace_suits_by_pattern(int board[]);
};

class CLutTurn
{
public:

	enum
	{
		TABLE_SIZE = 11522421,
		INDEX_SIZE = 28561,
	};

	int* turnindex;

	map<string, int> suitoffset;
	int preflop_table[55][55];

	float* data;

	CLutTurn();
	~CLutTurn();

	void load(char[]);
	void generate(char[]);
	int g_index(int, int, int[]);

	void write_suitfile();
	void read_suitfile(string filename);
	string g_pattern(int c1, int c2, int board[]);
	string replace_suits_by_pattern(int board[]);



};

class CLutRiver
{
public:

	enum
	{
		TABLE_SIZE = 37486905,
		INDEX_SIZE = 371293,
	};

	int* riverindex;

	map<string, int> suitoffset;
	int preflop_table[55][55];

	float* data;

	CLutRiver();
	~CLutRiver();

	void load(char[]);
	void generate(char[]);
	int g_index(int, int, int[]);

	void write_suitfile();
	void read_suitfile(string filename);
	string g_pattern(int c1, int c2, int board[]);
	string replace_suits_by_pattern(int board[]);
};


#endif

