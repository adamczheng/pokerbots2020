
#include "LUT.h"
#include <iostream>
#include <time.h>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>


static int DISPLAY_SECONDS = 10;


void Poker_Abstraction::inittheevaluator()
{
	memset(hr, 0, sizeof(hr));
	FILE* fin = fopen("handranks.dat", "rb");
	// load the handranks.dat file data into the hr array
	size_t bytesread = fread(hr, sizeof(hr), 1, fin);
	fclose(fin);
}
Poker_Abstraction::Poker_Abstraction() {
	inittheevaluator();
}
int Poker_Abstraction::compare_two_hands(int c1_hero, int c2_hero, int c1_opp, int c2_opp, int c3, int c4, int c5, int c6, int c7) {

	int hand_hero = hr[53 + c1_hero];
	hand_hero = hr[hand_hero + c2_hero];
	hand_hero = hr[hand_hero + c3];
	hand_hero = hr[hand_hero + c4];
	hand_hero = hr[hand_hero + c5];
	hand_hero = hr[hand_hero + c6];
	hand_hero = hr[hand_hero + c7];

	int hand_opp = hr[53 + c1_opp];
	hand_opp = hr[hand_opp + c2_opp];
	hand_opp = hr[hand_opp + c3];
	hand_opp = hr[hand_opp + c4];
	hand_opp = hr[hand_opp + c5];
	hand_opp = hr[hand_opp + c6];
	hand_opp = hr[hand_opp + c7];

	if (hand_hero > hand_opp)
		return 1;
	else if (hand_hero == hand_opp)
		return 0;
	else
		return -1;

}
class HS_Data_Generator {
public:
	Poker_Abstraction* m_abstraction;
	HS_Data_Generator() {
		srand(time(NULL));
		m_abstraction = new Poker_Abstraction();
	}

	~HS_Data_Generator() {
		delete m_abstraction;
	}
	bool is_value_in_array(int value, int* intarray, int size) {
		for (int i = 0; i < size; i++) {
			if (value == intarray[i])
				return true;
		}
		return false;
	}


	void compute_single_hs_and_hs2(int h1, int h2, int f1, int f2, int f3, int t, int r, double& hs, double& hs2) {
		int v1, v2;
		int hands = 0;
		hs = 0.0;
		hs2 = 0.0;
		int temp;
		int hand[7] = { h1, h2, f1, f2, f3, t, r };
		int new_deck[45];
		generate_deck_without(hand, 7, new_deck);

		// generate first hole card
		for (int i = 0; i < 45; i++) {

			v1 = new_deck[i];
			// generate second hole card
			for (int j = i + 1; j < 45; j++) {

				v2 = new_deck[j];
				// using the 2+2 hand evaluator:
				temp = m_abstraction->compare_two_hands(h1 + 1, h2 + 1, v1 + 1, v2 + 1, f1 + 1, f2 + 1, f3 + 1, t + 1, r + 1);
				if (temp == 1)
					hs++;
				// if it is a draw the equity is split in half
				else if (temp == 0)
					hs += 0.5;
				hands++;
			}

		}

		hs = hs / (double)hands;
		hs2 = hs * hs;
	}

	void compute_hs_and_hs2_flop(int h1, int h2, int f1, int f2, int f3, double& hs, double& hs2) {
		int turn, river;
		hs = 0.0;
		hs2 = 0.0;
		double hstemp, hs2temp;
		int hands = 0;
		int handI[5] = { h1, h2, f1, f2, f3 };

		int new_deck[47];
		generate_deck_without(handI, 5, new_deck);

		// generate the new turn card
		for (int i = 0; i < 47; i++) {

			turn = new_deck[i];

			// generate the river card:
			for (int j = i + 1; j < 47; j++) {

				river = new_deck[j];
				compute_single_hs_and_hs2(h1, h2, f1, f2, f3, turn, river, hstemp, hs2temp);
				hs += hstemp;
				hs2 += hs2temp;
				hands++;
			}

		}
		hs = hs / (double)hands;
		hs2 = hs2 / (double)hands;

	}




	void generate_deck_without(int* cards, int size, int* new_deck) {
		int count = 0;;
		for (int i = 0; i < 52; i++) {
			if (!is_value_in_array(i, cards, size)) {
				new_deck[count] = i;
				count++;
			}
		}
	}


	void compute_hs_and_hs2_turn(int h1, int h2, int f1, int f2, int f3, int t, double& hs, double& hs2) {
		int river;
		hs = 0.0;
		hs2 = 0.0;
		double hstemp, hs2temp;
		int hands = 0;
		int handI[6] = { h1, h2, f1, f2, f3, t };

		int new_deck[46];
		// store an int array containing all cards from 0 to 51 except the ones in array handI
		generate_deck_without(handI, 6, new_deck);


		for (int i = 0; i < 46; i++) {

			river = new_deck[i];
			compute_single_hs_and_hs2(h1, h2, f1, f2, f3, t, river, hstemp, hs2temp);
			hs += hstemp;
			hs2 += hs2temp;
			hands++;


		}
		hs = hs / (double)hands;
		hs2 = hs2 / (double)hands;
	}

	void compute_hs_and_hs2_preflop(int h1, int h2, double& hs, double& hs2) {
		int f1, f2, f3, t, r;
		hs = 0.0;
		hs2 = 0.0;
		int i, j, k, l, m;
		double hstemp, hs2temp;
		int hands = 0;
		int handI[2] = { h1,h2 };
		int new_deck[50];
		generate_deck_without(handI, 2, new_deck);
		for (i = 0; i < 50; i++) {
			f1 = new_deck[i];
			for (j = i + 1; j < 50; j++) {
				f2 = new_deck[j];
				for (k = j + 1; k < 50; k++) {
					f3 = new_deck[k];
					for (l = k + 1; l < 50; l++) {
						t = new_deck[l];
						for (m = l + 1; m < 50; m++) {
							r = new_deck[m];
							compute_single_hs_and_hs2(h1, h2, f1, f2, f3, t, r, hstemp, hs2temp);
							hs += hstemp;
							hs2 += hs2temp;
							hands++;
						}
					}
				}
			}
		}
		hs = hs / (double)hands;
		hs2 = hs2 / (double)hands;

	}
};

// Misc Funcs	
void LUT_save(char f_name[], int size, float data[])
{
	fstream f_bin(f_name, ios::out | ios::binary);

	f_bin.seekg(ios::beg);
	f_bin.write((char*)data, size * sizeof(float));
}

void LUT_load(char f_name[], int size, float data[])
{
	fstream f_bin(f_name, ios::in | ios::binary);

	f_bin.seekg(ios::beg);
	f_bin.read((char*)data, size * sizeof(float));
}


//CLut Preflop
/*
CLutPreFlop::CLutPreFlop() {
	data = new float[TABLE_SIZE];
	for (int i = 0; i < TABLE_SIZE; i++)
		data[i] = -1;
}

CLutPreFlop::~CLutPreFlop() {
	delete[] data;
}

void CLutPreFlop::generate(char f_name[])
{

	int index;

	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);

	int size = 0, s = 0, i, j;

	for (int I = 0;I < 52;I++) {
		for (int J = I + 1;J < 52;J++) {
			size++;
		}
	}

	for (i = 0; i < 52; i++) {
		for (j = i + 1; j < 52; j++) {

			time_now = time(NULL);
			s++;
			if (iteration_time + DISPLAY_SECONDS < time_now) {
				cout << "Iteration " << s << " of " << size << "   " << 100.0 * s / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / s * (size - s) << " min" << endl;
				cout << "Savings per second: " << (double)s / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}

			index = g_index(i, j);
			if (data[index] == -1) {
				// go ahead here you need to populate the lut
				//data[index] = ???

			}
		}
	}


	LUT_save(f_name, TABLE_SIZE, data);

}

int CLutPreFlop::g_index(int c1, int c2) {

	if (c1 > c2) {
		std::swap(c1, c2);
	}

	int suit1 = c1 % 4;
	int suit2 = c2 % 4;
	int rank1 = c1 / 4;
	int rank2 = c2 / 4;
	if (suit2 == suit1)
		return preflop_table.find(pair<int, int>(rank1, rank2))->second;
	else
		return preflop_table.find(pair<int, int>(rank1 + 100, rank2 + 100))->second;
}

void CLutPreFlop::prepare() {
	int count = 0;

	for (int i = 0; i < 13; i++) {
		for (int j = i + 1; j < 13; j++) {
			preflop_table.insert(pair<pair<int, int>, int>(pair<int, int>(i, j), count));
			count++;
		}
	}

	for (int i = 100; i < 113; i++) {
		for (int j = i; j < 113; j++) {
			preflop_table.insert(pair<pair<int, int>, int>(pair<int, int>(i, j), count));
			count++;
		}
	}

}

void CLutPreFlop::load(char f_name[])
{
	cout << "Loading " << f_name << "...";
	LUT_load(f_name, TABLE_SIZE, data);
	cout << "complete" << endl;
}
*/

//Clut Flop
string CLutFlop::replace_suits_by_pattern(int board[]) {
	char patterns[4];
	for (int i = 0; i < 4; i++)
		patterns[i] = 0;
	int size = 5;
	int count = 0;
	int suit;
	char temp;
	char* pattern_board = new char[size];
	for (int i = 2; i < size; i++) {
		suit = board[i] % 4;
		if (patterns[suit] == 0) {
			switch (count) {
			case 0: temp = 'A'; break;
			case 1: temp = 'B'; break;
			case 2: temp = 'C'; break;
			}
			count++;
			patterns[suit] = temp;
			pattern_board[i] = temp;
		}
		else {
			pattern_board[i] = patterns[suit];
		}
	}

	//	// debugging
	//if (board[0] %4 == board[1]%4 && board[1]%4 == board[2] %4 && pattern_board[4] != 'C')
	//	int a = 0;

	// replace the hole cards
	int suit1 = board[0] % 4;
	int suit2 = board[1] % 4;
	if (patterns[suit1] == 0)
	{
		pattern_board[0] = 'O';
	}
	else {
		pattern_board[0] = patterns[suit1];
	}
	if (patterns[suit2] == 0)
	{
		pattern_board[0] = 'O';
	}
	else {
		pattern_board[0] = patterns[suit2];
	}

	string result(pattern_board, size);
	delete[] pattern_board;
	return result;


}


CLutFlop::CLutFlop() {
	data = new float[TABLE_SIZE];
	for (int I = 0;I < TABLE_SIZE;I++) data[I] = -1;
	flopindex = new int[2];

}

CLutFlop::~CLutFlop()
{
	delete[] data;
	delete[] flopindex;
}

void CLutFlop::load(char f_name[])
{
	cout << "Loading " << f_name << "...";
	LUT_load(f_name, TABLE_SIZE, data);
	cout << "complete" << endl;
}


void CLutFlop::generate(char f_name[])
{
	HS_Data_Generator* hsdg = new HS_Data_Generator();
	int I, J, K, L, M, index, board[3];


	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;



	for (I = 0;I < 52;I++)
	{

		cout << I / (double)52 << "% done" << endl;
		for (J = I + 1;J < 52;J++)
		{
			time_now = time(NULL);
			i++;
			if (iteration_time + DISPLAY_SECONDS < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 52;K++)
			{
				board[0] = I;
				board[1] = J;
				board[2] = K;

				for (L = 0;L < 52;L++)
				{
					for (M = L + 1;M < 52;M++)
					{


						if (L == I || L == J || L == K
							|| M == I || M == J || M == K)
							continue;

						//index = g_index(L, M, board);
						index = g_index(L, M, board);
						if (data[index] == -1)
						{
							// go ahead here you need to populate the lut
							//data[index] = ???
							double hs, hs2;
							hsdg->compute_hs_and_hs2_flop(L, M, board[0], board[1], board[2], hs, hs2);
							//cout << hs << ' ' << hs2 << endl;
							data[index] = (hs);
						}
					}
				}
			}
		}
	}



	LUT_save(f_name, TABLE_SIZE, data);

}

int CLutFlop::g_index(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 3);

	int cards[5] = { c1, c2, board[0], board[1], board[2] };

	string stemp;

	stemp = replace_suits_by_pattern(cards);

	int maximum = 13;
	int preflop_maximum = 91;
	int preflop_index;

	// compute the ranks
	int rc1 = c1 / 4;
	int rc2 = c2 / 4;
	int rf1 = board[0] / 4;
	int rf2 = board[1] / 4;
	int rf3 = board[2] / 4;

	// check if the hole cards are suited, if so they cannot be any pair
	if (stemp[0] == stemp[1] && stemp[0] != 'O') {
		preflop_index = preflop_table[rc1][rc2] - 13;
		preflop_maximum -= 13;
	}
	else {
		preflop_index = preflop_table[rc1][rc2];
	}

	// compute the index of the flop ranks
	int index = maximum * maximum * rf1 + maximum * rf2 + rf3;
	index = flopindex[index];
	// add the index of the hole cards
	index = index * preflop_maximum + preflop_index;
	// add the offset for the suit pattern
	index += suitoffset.find(stemp)->second;
	return index;

}

string CLutFlop::g_pattern(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 3);

	int cards[5] = { c1, c2, board[0], board[1], board[2] };

	string sboard;
	sboard = replace_suits_by_pattern(cards);

	return sboard;
}

void CLutFlop::write_suitfile() {

	int I, J, K, L, M, board[3];

	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;

	map<string, int> suitcount;
	map<string, int>::iterator it;
	string index;


	for (I = 0;I < 22;I++)
	{
		//cout << I/ (double) 52 << "% done" << endl;
		for (J = I + 1;J < 22;J++)
		{

			time_now = time(NULL);
			i++;
			if (iteration_time + 15 < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 22;K++)
			{
				board[0] = I;
				board[1] = J;
				board[2] = K;

				for (L = 0;L < 22;L++)
				{
					for (M = L + 1;M < 22;M++)
					{


						if (L == I || L == J || L == K
							|| M == I || M == J || M == K)
							continue;


						index = g_pattern(L, M, board);
						if ((it = (suitcount.find(index))) == suitcount.end())
							suitcount.insert(pair<string, int>(index, 1));

					}
				}
			}
		}
	}
	fstream filestream("flop_suit.dat", fstream::out);

	size = 0;
	for (int f1 = 0; f1 < 13; f1++) {
		for (int f2 = f1; f2 < 13; f2++) {
			for (int f3 = f2; f3 < 13; f3++) {
				size++;
			}
		}
	}

	it = suitcount.begin();
	int offset = 0;
	string stemp;
	while (it != suitcount.end()) {
		stemp = it->first;
		filestream << stemp << " " << offset << "\n";
		it++;
		if (stemp[0] == stemp[1] && stemp[0] != 'O')
		{
			offset += size * 78;
		}
		else {
			offset += size * 91;
		}
	}

	filestream << "Sum: " << offset;

	filestream.flush();
	filestream.close();

}

void CLutFlop::read_suitfile(string filename) {

	delete[] flopindex;
	flopindex = new int[INDEX_SIZE];
	//insert the pairs
	for (int i = 0; i < 13; i++)
		preflop_table[i][i] = i;

	int count = 12;
	for (int i = 0; i < 13; i++)
		for (int j = i + 1; j < 13; j++) {
			count++;
			preflop_table[i][j] = count;
		}

	// create the index table
	int rank_index = 0;
	int table_index;
	for (int i = 0; i < 13; i++)
		for (int j = 0; j < 13; j++)
			for (int k = 0; k < 13; k++) {
				table_index = 13 * 13 * i + 13 * j + k;
				if (i <= j && j <= k) {
					flopindex[table_index] = rank_index;
					rank_index++;
				}
				else {
					flopindex[table_index] = -1;
				}


			}

	fstream filestream(filename.c_str(), fstream::in);
	string temp_entry;
	char temp[256];
	char c[6];
	int size;

	while (filestream.good()) {
		filestream.getline(temp, 256, '\n');
		stringstream tempstr;
		tempstr << temp;
		tempstr.get(c, 6);
		tempstr >> size;
		suitoffset.insert(pair<string, int>(c, size));

	}

	filestream.flush();
	filestream.close();
}




// Turn LUT
CLutTurn::CLutTurn()
{
	data = new float[TABLE_SIZE];
	for (int I = 0;I < TABLE_SIZE;I++) data[I] = -1;

	turnindex = new int[2];

}

CLutTurn::~CLutTurn()
{
	delete[] data;
	delete[] turnindex;
}

void CLutTurn::load(char f_name[])
{
	cout << "Loading " << f_name << "...";
	LUT_load(f_name, TABLE_SIZE, data);
	cout << "complete" << endl;
}

void CLutTurn::generate(char f_name[])
{

	HS_Data_Generator* hsdg = new HS_Data_Generator();

	int I, J, K, L, M, N, index, board[4];


	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;



	for (I = 0;I < 52;I++)
	{
		for (J = I + 1;J < 52;J++)
		{
			time_now = time(NULL);
			i++;
			if (iteration_time + DISPLAY_SECONDS < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 52;K++)
			{
				for (L = K + 1;L < 52;L++)
				{
					board[0] = I;
					board[1] = J;
					board[2] = K;
					board[3] = L;
					//cout << I << ' ' << J << ' ' << K << ' ' << L << endl;

					for (M = 0;M < 52;M++)
					{
						for (N = M + 1;N < 52;N++)
						{
							if (N == I || N == J || N == K || N == L
								|| M == I || M == J || M == K || M == L)
								continue;

							index = g_index(M, N, board);

							if (data[index] == -1)
							{
								// go ahead here you need to populate the lut
								//data[index] = ???
								double hs, hs2;
								hsdg->compute_hs_and_hs2_turn(M, N, board[0], board[1], board[2], board[3], hs, hs2);
								data[index] = hs;
							}
						}
					}
				}
			}
		}
	}


	LUT_save(f_name, TABLE_SIZE, data);

}

string CLutTurn::g_pattern(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 4);

	int cards[6] = { c1, c2, board[0], board[1], board[2], board[3] };

	string sboard;
	sboard = replace_suits_by_pattern(cards);

	return sboard;
}


int CLutTurn::g_index(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 4);

	int cards[6] = { c1, c2, board[0], board[1], board[2], board[3] };

	string stemp;

	stemp = replace_suits_by_pattern(cards);

	int maximum = 13;
	int preflop_maximum = 91;
	int preflop_index;

	// compute the ranks
	int rc1 = c1 / 4;
	int rc2 = c2 / 4;
	int rf1 = board[0] / 4;
	int rf2 = board[1] / 4;
	int rf3 = board[2] / 4;
	int rt = board[3] / 4;

	// check if the hole cards are suited, if so they cannot be any pair
	if (stemp[0] == stemp[1] && stemp[0] != 'O') {
		preflop_index = preflop_table[rc1][rc2] - 13;
		preflop_maximum -= 13;
	}
	else {
		preflop_index = preflop_table[rc1][rc2];
	}

	// compute the index of the flop ranks
	int index = maximum * maximum * maximum * rf1 + maximum * maximum * rf2 + maximum * rf3 + rt;
	index = turnindex[index];
	// add the index of the hole cards
	index = index * preflop_maximum + preflop_index;
	// add the offset for the suit pattern
	index += suitoffset.find(stemp)->second;
	return index;
}

string CLutTurn::replace_suits_by_pattern(int board[]) {
	char patterns[4];
	for (int i = 0; i < 4; i++)
		patterns[i] = 0;
	int size = 6;
	int count = 0, A = 0, B = 0, C = 0;
	int suit;
	char temp;
	char* pattern_board = new char[size];
	for (int i = 2; i < size; i++) {
		suit = board[i] % 4;
		if (patterns[suit] == 0) {
			switch (count) {
			case 0: temp = 'A';A++; break;
			case 1: temp = 'B';B++; break;
			case 2: temp = 'C';C++; break;
				// the last found suit can never be part of a FD
			case 3: temp = 'O'; break;
			}
			count++;
			// if there will be an 'O' we do not need to add it to the map
			if (count != 4)
				patterns[suit] = temp;
			pattern_board[i] = temp;
		}
		else {
			pattern_board[i] = patterns[suit];
			if (pattern_board[i] == 'A')
				A++;
			else if (pattern_board[i] == 'B')
				B++;
			else if (pattern_board[i] == 'C')
				C++;
		}
	}

	// replace the hole cards
	int suit1 = board[0] % 4;
	int suit2 = board[1] % 4;
	if (patterns[suit1] == 0)
	{
		pattern_board[0] = 'O';
	}
	else {
		// if the suit appears only once on the turn board the suit does not matter, because it cannot participate in a FD:
		pattern_board[0] = patterns[suit1];
		if (pattern_board[0] == 'A' && A == 1)
			pattern_board[0] = 'O';
		else if (pattern_board[0] == 'B' && B == 1)
			pattern_board[0] = 'O';
		else if (pattern_board[0] == 'C' && C == 1)
			pattern_board[0] = 'O';
	}
	if (patterns[suit2] == 0) {
		pattern_board[1] = 'O';
	}
	else {
		pattern_board[1] = patterns[suit2];
		if (pattern_board[1] == 'A' && A == 1)
			pattern_board[1] = 'O';
		else if (pattern_board[1] == 'B' && B == 1)
			pattern_board[1] = 'O';
		else if (pattern_board[1] == 'C' && C == 1)
			pattern_board[1] = 'O';
	}

	//if the number of patterns is 1 they do not play any role in drawing a flush, hence they can be ignored
	// we do not need to check the last entry, because if it is a single appearence it is already an 'O'
	int reduced_size = size - 1;
	for (int i = 2; i < reduced_size; i++) {
		if (A == 1 && pattern_board[i] == 'A')
			pattern_board[i] = 'O';
		else if (B == 1 && pattern_board[i] == 'B')
			pattern_board[i] = 'O';
		else if (C == 1 && pattern_board[i] == 'C')
			pattern_board[i] = 'O';
	}


	string result(pattern_board, size);
	delete[] pattern_board;
	return result;
}

void CLutTurn::read_suitfile(string filename) {

	delete[] turnindex;
	turnindex = new int[INDEX_SIZE];
	//insert the pairs
	for (int i = 0; i < 13; i++)
		preflop_table[i][i] = i;

	int count = 12;
	for (int i = 0; i < 13; i++)
		for (int j = i + 1; j < 13; j++) {
			count++;
			preflop_table[i][j] = count;
		}

	// create the index table
	int rank_index = 0;
	int table_index;
	for (int i = 0; i < 13; i++)
		for (int j = 0; j < 13; j++)
			for (int k = 0; k < 13; k++)
				for (int l = 0; l < 13; l++) {
					table_index = 13 * 13 * 13 * i + 13 * 13 * j + 13 * k + l;
					if (i <= j && j <= k && k <= l) {
						turnindex[table_index] = rank_index;
						rank_index++;
					}
					else {
						turnindex[table_index] = -1;
					}
				}

	fstream filestream(filename.c_str(), fstream::in);
	string temp_entry;
	char temp[256];
	char c[7];
	int size;

	while (filestream.good()) {
		filestream.getline(temp, 256, '\n');
		stringstream tempstr;
		tempstr << temp;
		tempstr.get(c, 7);
		tempstr >> size;
		suitoffset.insert(pair<string, int>(c, size));
	}

	filestream.flush();
	filestream.close();
}

void CLutTurn::write_suitfile() {

	int I, J, K, L, M, N, board[4];

	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;

	map<string, int> suitcount;
	map<string, int>::iterator it;
	string index;


	for (I = 0;I < 36;I++)
	{
		//cout << I/ (double) 52 << "% done" << endl;
		for (J = I + 1;J < 36;J++)
		{

			time_now = time(NULL);
			i++;
			if (iteration_time + 15 < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 36;K++)
			{
				for (L = K + 1;L < 36;L++)
				{
					board[0] = I;
					board[1] = J;
					board[2] = K;
					board[3] = L;

					for (M = 0;M < 36;M++)
					{
						for (N = M + 1;N < 36;N++)
						{
							if (N == I || N == J || N == K || N == L
								|| M == I || M == J || M == K || M == L)
								continue;


							index = g_pattern(M, N, board);
							if ((it = (suitcount.find(index))) == suitcount.end())
								suitcount.insert(pair<string, int>(index, 1));
						}
					}
				}
			}

		}
	}
	fstream filestream("turn_suit.dat", fstream::out);

	size = 0;
	for (int f1 = 0; f1 < 13; f1++) {
		for (int f2 = f1; f2 < 13; f2++) {
			for (int f3 = f2; f3 < 13; f3++) {
				for (int t = f3; t < 13; t++) {
					size++;
				}
			}
		}
	}

	it = suitcount.begin();
	int offset = 0;
	string stemp;
	while (it != suitcount.end()) {
		stemp = it->first;
		filestream << stemp << " " << offset << "\n";
		it++;
		if (stemp[0] == stemp[1] && stemp[0] != 'O')
		{
			offset += size * 78;
		}
		else {
			offset += size * 91;
		}
	}

	filestream << "Sum: " << offset;

	filestream.flush();
	filestream.close();

}





// River LUT
CLutRiver::CLutRiver()
{
	data = new float[TABLE_SIZE];
	for (int I = 0;I < TABLE_SIZE;I++) data[I] = -1;

	riverindex = new int[2];


}
CLutRiver::~CLutRiver()
{
	delete[] data;
	delete[] riverindex;
}

void CLutRiver::load(char f_name[])
{
	cout << "Loading " << f_name << "...";
	LUT_load(f_name, TABLE_SIZE, data);
	/*for (int i = 0; i < TABLE_SIZE; i++) {
		if(data[i] > -1) cout << i << ' ' << data[i] << endl;
	}*/
	cout << "complete" << endl;
}
void CLutRiver::generate(char f_name[])
{
	HS_Data_Generator* hsdg = new HS_Data_Generator();
	int I, J, K, L, M, N, O, index, board[5];


	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;

	for (I = 0;I < 52;I++)
	{
		for (J = I + 1;J < 52;J++)
		{
			time_now = time(NULL);
			i++;
			if (iteration_time + DISPLAY_SECONDS < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 52;K++)
			{
				for (L = K + 1;L < 52;L++)
				{
					for (M = L + 1;M < 52;M++)
					{
						board[0] = I;
						board[1] = J;
						board[2] = K;
						board[3] = L;
						board[4] = M;

						for (N = 0;N < 52;N++)
						{
							for (O = N + 1;O < 52;O++)
							{
								if (N == I || N == J || N == K || N == L || N == M
									|| O == I || O == J || O == K || O == L || O == M)
									continue;

								index = g_index(N, O, board);

								if (data[index] == -1)
								{
									// go ahead here you need to populate the lut
									//data[index] = ???
									double hs, hs2;
									hsdg->compute_single_hs_and_hs2(N, O, board[0], board[1], board[2], board[3], board[4], hs, hs2);
									data[index] = hs;
								}
							}
						}
					}
				}
			}
		}
	}

	LUT_save(f_name, TABLE_SIZE, data);
}


int CLutRiver::g_index(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 5);

	int cards[7] = { c1, c2, board[0], board[1], board[2], board[3], board[4] };

	string stemp;

	stemp = replace_suits_by_pattern(cards);

	int maximum = 13;
	int preflop_maximum = 91;
	int preflop_index;

	// compute the ranks
	int rc1 = c1 / 4;
	int rc2 = c2 / 4;
	int rf1 = board[0] / 4;
	int rf2 = board[1] / 4;
	int rf3 = board[2] / 4;
	int rt = board[3] / 4;
	int rr = board[4] / 4;

	// check if the hole cards are suited, if so they cannot be any pair
	if (stemp[0] == stemp[1] && stemp[0] != 'O') {
		preflop_index = preflop_table[rc1][rc2] - 13;
		preflop_maximum -= 13;
	}
	else {
		preflop_index = preflop_table[rc1][rc2];
	}

	// compute the index of the flop ranks
	int index = maximum * maximum * maximum * maximum * rf1 + maximum * maximum * maximum * rf2 + maximum * maximum * rf3 + maximum * rt + rr;
	index = riverindex[index];
	// add the index of the hole cards
	index = index * preflop_maximum + preflop_index;
	// add the offset for the suit pattern
	index += suitoffset.find(stemp)->second;
	return index;
}


string CLutRiver::replace_suits_by_pattern(int board[]) {
	char patterns[4];
	for (int i = 0; i < 4; i++)
		patterns[i] = 0;
	int size = 7;
	int count = 0, A = 0, B = 0, C = 0;
	int suit;
	char temp;
	char* pattern_board = new char[size];
	for (int i = 2; i < size; i++) {
		suit = board[i] % 4;
		if (patterns[suit] == 0) {
			switch (count) {
			case 0: temp = 'A';A++; break;
			case 1: temp = 'B';B++; break;
			case 2: temp = 'C';C++; break;
				// the last found suit can never be part of a Flush
			case 3: temp = 'O'; break;
			case 4: temp = 'O'; break;
			}
			count++;
			// if there will be an 'O' we do not need to add it to the map
			if (temp != 'O')
				patterns[suit] = temp;
			pattern_board[i] = temp;
		}
		else {
			pattern_board[i] = patterns[suit];
			if (pattern_board[i] == 'A')
				A++;
			else if (pattern_board[i] == 'B')
				B++;
			else if (pattern_board[i] == 'C')
				C++;
		}
	}

	// replace the hole cards
	int suit1 = board[0] % 4;
	int suit2 = board[1] % 4;

	if (patterns[suit1] == 0)
	{
		pattern_board[0] = 'O';
	}
	else {
		// if the suit is only once on the turn board the suit does not matter, because it cannot participate in a FD:
		pattern_board[0] = patterns[suit1];
		if (pattern_board[0] == 'A' && A < 3)
			pattern_board[0] = 'O';
		else if (pattern_board[0] == 'B' && B < 3)
			pattern_board[0] = 'O';
		else if (pattern_board[0] == 'C' && C < 3)
			pattern_board[0] = 'O';
	}

	if (patterns[suit2] == 0) {
		pattern_board[1] = 'O';
	}
	else {
		pattern_board[1] = patterns[suit2];
		if (pattern_board[1] == 'A' && A < 3)
			pattern_board[1] = 'O';
		else if (pattern_board[1] == 'B' && B < 3)
			pattern_board[1] = 'O';
		else if (pattern_board[1] == 'C' && C < 3)
			pattern_board[1] = 'O';
	}
	//if the number of patterns is 1 or 2 they do not play any role for a flush, hence they can be ignored
	for (int i = 2; i < size; i++) {
		if (A < 3 && pattern_board[i] == 'A')
			pattern_board[i] = 'O';
		else if (B < 3 && pattern_board[i] == 'B')
			pattern_board[i] = 'O';
		else if (C < 3 && pattern_board[i] == 'C')
			pattern_board[i] = 'O';
	}


	string result(pattern_board, size);
	delete[] pattern_board;
	return result;
}

void CLutRiver::write_suitfile() {

	int I, J, K, L, M, N, O, board[5];

	// stp the time for performance measurments:
	time_t iteration_time, start_time, time_now;
	start_time = iteration_time = time(NULL);
	int size = 0;
	for (I = 0;I < 52;I++)
		for (J = I + 1;J < 52;J++)
			size++;

	int i = 0;

	map<string, int> suitcount;
	map<string, int>::iterator it;
	string index;


	for (I = 0;I < 36;I++)
	{
		//cout << I/ (double) 52 << "% done" << endl;
		for (J = I + 1;J < 36;J++)
		{

			time_now = time(NULL);
			i++;
			if (iteration_time + 15 < time_now) {
				cout << "Iteration " << i << " of " << size << "   " << 100.0 * i / (double)size << "%" << endl;
				cout << "Time elapsed: " << (time_now - start_time) / 60.0 << " min" << endl;
				cout << "Time remaining: " << 1.0 / 60.0 * (time_now - start_time) / i * (size - i) << " min" << endl;
				cout << "Savings per second: " << (double)i / (time_now - start_time) << endl << endl;;
				iteration_time = time_now;
			}
			for (K = J + 1;K < 36;K++)
			{
				for (L = K + 1;L < 36;L++)
				{
					for (M = L + 1;M < 36;M++)
					{
						board[0] = I;
						board[1] = J;
						board[2] = K;
						board[3] = L;
						board[4] = M;

						for (N = 0;N < 36;N++)
						{
							for (O = N + 1;O < 36;O++)
							{
								if (N == I || N == J || N == K || N == L || N == M
									|| O == I || O == J || O == K || O == L || O == M)
									continue;


								index = g_pattern(N, O, board);
								if ((it = (suitcount.find(index))) == suitcount.end())
									suitcount.insert(pair<string, int>(index, 1));
							}
						}
					}
				}
			}
		}
	}
	fstream filestream("river_suit.dat", fstream::out);

	size = 0;
	for (int f1 = 0; f1 < 13; f1++) {
		for (int f2 = f1; f2 < 13; f2++) {
			for (int f3 = f2; f3 < 13; f3++) {
				for (int t = f3; t < 13; t++) {
					for (int r = t; r < 13; r++) {
						size++;
					}
				}
			}
		}
	}

	it = suitcount.begin();
	int offset = 0;
	string stemp;
	while (it != suitcount.end()) {
		stemp = it->first;
		filestream << stemp << " " << offset << "\n";
		it++;
		if (stemp[0] == stemp[1] && stemp[0] != 'O')
		{
			offset += size * 78;
		}
		else {
			offset += size * 91;
		}
	}

	filestream << "Sum: " << offset;

	filestream.flush();
	filestream.close();

}

string CLutRiver::g_pattern(int c1, int c2, int board[])
{
	// sort cards
	if (c1 > c2)
		std::swap(c1, c2);

	std::sort(board, board + 5);

	int cards[7] = { c1, c2, board[0], board[1], board[2], board[3], board[4] };

	string sboard;
	sboard = replace_suits_by_pattern(cards);

	return sboard;
}

void CLutRiver::read_suitfile(string filename) {

	delete[] riverindex;
	riverindex = new int[INDEX_SIZE];

	//insert the pairs
	for (int i = 0; i < 13; i++)
		preflop_table[i][i] = i;;

	int count = 12;
	for (int i = 0; i < 13; i++)
		for (int j = i + 1; j < 13; j++) {
			count++;
			preflop_table[i][j] = count;
		}

	// create the index table
	int rank_index = 0;
	int table_index;
	for (int i = 0; i < 13; i++)
		for (int j = 0; j < 13; j++)
			for (int k = 0; k < 13; k++)
				for (int l = 0; l < 13; l++)
					for (int m = 0; m < 13; m++) {
						table_index = 13 * 13 * 13 * 13 * i + 13 * 13 * 13 * j + 13 * 13 * k + 13 * l + m;
						if (i <= j && j <= k && k <= l && l <= m) {
							riverindex[table_index] = rank_index;
							rank_index++;
						}
						else {
							riverindex[table_index] = -1;
						}
					}


	fstream filestream(filename.c_str(), fstream::in);
	string temp_entry;
	char temp[256];
	char c[8];
	int size;

	while (filestream.good()) {
		filestream.getline(temp, 256, '\n');
		stringstream tempstr;
		tempstr << temp;
		tempstr.get(c, 8);
		tempstr >> size;
		suitoffset.insert(pair<string, int>(c, size));
	}

	filestream.flush();
	filestream.close();
}

