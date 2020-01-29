#include <bits/stdc++.h>
using namespace std;

const int NUM_LINES_STRATEGY = 1162671;
map<pair<int, int >, vector<pair<pair<char,int>, double> > > strategy_map[7][200][7];

int main() {
    //ifstream fin("dump5.txt");
    ofstream fout("strategy.txt");
    string line;
    getline(cin, line);
    for (int i = 1; i < NUM_LINES_STRATEGY; i++) {
        getline(cin, line);
        istringstream ss(line);
        int street, infoset, player, pot;
        int button;
        int pot_odd_round;
        //array<int, 2> pips;
        ss >> street >> infoset /*>> player*/ >> button>> pot >> pot_odd_round;//pips[0] >> pips[1];
        char colon_dummy;
        ss >> colon_dummy;
        vector<pair<pair<char,int>,double> > strategy;
        char type;
        while (ss >> type) {
            int x;
            double p;
            ss >> x >> p;
            strategy.push_back({{type, x}, p});
        }
        strategy_map/*[player]*/[street][infoset][button][make_pair(pot, pot_odd_round)] = strategy;
    }
    //fin.close();
    fout << "strategy from cfr:" << endl;
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < 6; k++) {
            for (int j = 0; j < 200; j++) {
                for (auto it : strategy_map[i][j][k]) {
                    if (it.second.size()) {
                        int street = i;
                        int infoset = j;
                        int button = k;
                        int pot = it.first.first;
                        int pot_odd_round = it.first.second;
                        fout << street << ' ' << infoset << ' ' << button << ' ' << pot << ' ' << pot_odd_round <<": ";
                        for (pair<pair<char,int>, double> p : it.second) {
                            fout << p.first.first << ' ' << p.first.second << ' ' << p.second << ' ';
                        }
                        fout << endl;
                    }
                }
            }
        }
    }
}
