#include <bits/stdc++.h>
using namespace std;
vector<int> permute_values() {
    vector<int> orig_perm = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    vector<int> prop_perm;
    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    geometric_distribution<int> distribution(0.25);
    for (int i = 0; i < 13; i++) {
        int pop_i = orig_perm.size() - 1 - (distribution(rng) % orig_perm.size());
        prop_perm.push_back(orig_perm[pop_i]);
        orig_perm.erase(orig_perm.begin() + pop_i);
    }
    return prop_perm;
}
int main() {
    string ranks="23456789TJQKA";
    string suits="shdc";
    for (int i = 0; i < 2; i++) {
        vector<int> proposal_perm = permute_values();
        for (int j = 0; j < 13; j++) {
            cout << proposal_perm[j] << ' ';
        }
        cout << endl;
        for (int j = 0; j < 13; j++) {
            //for (char s : suits) {
                char r = ranks[j];
                string card = "";
                card += r;
                //card += s;
                cout << "Card: " << card << endl;
                int permuted_j = proposal_perm[j];
                char perm_r = ranks[permuted_j];
                string perm_card = "";
                perm_card += perm_r;
                //perm_card += s;
                cout << "Perm card: " << perm_card << endl;
            //}
        }
    }
    return 0;
}