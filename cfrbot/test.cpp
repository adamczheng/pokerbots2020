#include <bits/stdc++.h>
#include "handranks.hpp"
using namespace std;

int hand_to_int(string s) {
    string ranks = "23456789TJQKA";
    string suits = "cdhs";
    map<char, int> rmap, smap;
    int i = 0;
    for (char c : ranks) rmap[c] = i++;
    i = 0;
    for (char c : suits) smap[c] = i++;
    return 4 * rmap[s[0]] + smap[s[1]];
}

int main() {
    HandRanks* hr = new HandRanks("handranks.dat");
    array<int, 7> cards;
    cards[0] = hand_to_int("As");
    cards[1] = hand_to_int("2s");
    cards[2] = hand_to_int("3c");
    cards[3] = hand_to_int("4c");
    cards[4] = hand_to_int("6s");
    cards[5] = hand_to_int("Ks");
    cards[6] = hand_to_int("Ah");
    int handinfo = hr->GetHandValue(cards);
    int category = handinfo >> 12;
    int rankwithincategory = handinfo & 0x00000FFF;
    cout << handinfo << ' ' << category << ' ' << rankwithincategory;
    return 0;
}