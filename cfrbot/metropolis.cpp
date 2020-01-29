/*
g++ metropolis.cpp -O3 -o metropolis && ./metropolis
*/

#include <bits/stdc++.h>
using namespace std;

int HR[32487834];
int GetHandValue(array<int, 7> cards) {
    int p = HR[53 + cards[0] + 1];
    p = HR[p + cards[1] + 1];
    p = HR[p + cards[2] + 1];
    p = HR[p + cards[3] + 1];
    p = HR[p + cards[4] + 1];
    p = HR[p + cards[5] + 1];
    return HR[p + cards[6] + 1];
}

long long perm_to_mask(const vector<int> &perm) {
    long long res = 0;
    for (int i = 0; i < 13; i++) {
        res *= 16;
        res += perm[i];
    }
    return res;
}
vector<int> mask_to_perm(long long mask) {
    vector<int> res(13);
    for (int i = 0; i < 13; i++) {
        res[12-i] = mask % 16;
        mask /= 16;
    }
    return res;
}
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

double prior_prob[13][13];
void compute_prior_probs() {
    for (int i = 0; i < 13; i++) {
        double sum = 0;
        double prob = 0.25;
        for (int j = 0; j < 13 - i; j++) {
            if (j > 0) prob *= 0.75;
            prior_prob[i][j] = prob;
            sum += prior_prob[i][j];
        }
        for (int j = 0; j < 13 - i; j++) {
            prior_prob[i][j] /= sum;
        }
    }
}
double perm_prob(const vector<int> &guess) {
    double res = 1;
    vector<int> editing(13);
    for (int i = 0; i < 13; i++) {
        editing[i] = i;
    }
    for (int i = 0; i < 13; i++) {
        int id = -1;
        for (int j = 0; j < (int)editing.size(); j++) {
            if (editing[j] == guess[i]) {
                id = j;
                break;
            }
        }
        assert(id != -1);
        res *= prior_prob[i][id];
        editing.erase(editing.begin() + id);
    }
    return res;
}

int convert(const vector<int> &perm, int card) {
    for (int i = 0; i < 13; i++) {
        if (perm[i] == card / 4) {
            return 4 * i + (card % 4);
        }
    }
    assert(0);
}

mt19937 rng;

vector<int> gen_candidate(const vector<int> &guess) {
    vector<int> res(13);
    for (int i = 0; i < 13; i++) {
        res[i] = guess[i];
    }
    uniform_int_distribution<int> distribution(0, 12);
    int x = distribution(rng), y = distribution(rng);
    swap(res[x], res[y]);
    return res;
}

vector<int> actual_perm;
vector<pair<pair<array<int, 7>, array<int, 7> >, int> > showdown_rules;
void generate_showdown() {
    uniform_int_distribution<int> distribution(0, 51);
    bitset<52> bs;
    array<array<int, 2>, 2> hands;
    array<int, 5> board;
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            int x;
            do {
                x = distribution(rng);
            } while (bs[x]);
            bs[x] = 1;
            hands[a][b] = x;
        }
    }
    for (int a = 0; a < 5; a++) {
        int x;
        do {
            x = distribution(rng);
        } while (bs[x]);
        bs[x] = 1;
        board[a] = x;
    }
    array<int, 7> cards = { board[0], board[1], board[2], board[3], board[4], hands[0][0], hands[0][1] };
    array<int, 7> cards_converted;
    for (int i = 0; i < 7; i++) {
        cards_converted[i] = convert(actual_perm, cards[i]);
    }
    pair<pair<array<int, 7>, array<int, 7> >, int> pp;
    pp.first.first = cards;
    int player0strength = GetHandValue(cards_converted);
    cards[5] = hands[1][0];
    cards[6] = hands[1][1];
    for (int i = 5; i < 7; i++) {
        cards_converted[i] = convert(actual_perm, cards[i]);
    }
    pp.first.second = cards;
    int player1strength = GetHandValue(cards_converted);
    if (player0strength > player1strength) pp.second = 1;
    else if (player0strength < player1strength) pp.second = 2;
    else pp.second = 0;
    showdown_rules.push_back(pp);
}
int count_fails(const vector<int> &guess) {
    int fails = 0;
    for (pair<pair<array<int, 7>, array<int, 7> >, int> pp : showdown_rules) {
        array<int, 7> h1 = pp.first.first;
        array<int, 7> h2 = pp.first.second;
        int result = pp.second;
        for (int i = 0; i < 7; i++) {
            h1[i] = convert(guess, h1[i]);
            h2[i] = convert(guess, h2[i]);
        }
        int s1 = GetHandValue(h1);
        int s2 = GetHandValue(h2);
        int new_result = 0;
        if (s1 > s2) new_result = 1;
        if (s1 < s2) new_result = 2;
        if (result != new_result) {
            fails++;
        }
    }
    return fails;
}


int guess_fails = 0;
void metropolis_hastings(vector<int> &guess) {
    guess_fails = count_fails(guess);
    int TOT_ITERS = 1000;
    double REJECTION_PROB = 0.2;
    map<long long, int> visits;
    for (int iter = 0; iter < TOT_ITERS; iter++) {
        //cout << iter << endl;
        vector<int> candidate = gen_candidate(guess);
       // cout << iter << endl;
        int candidate_fails = count_fails(candidate);
        double prior_ratio = 1;//perm_prob(candidate) / perm_prob(guess); /* CHANGE */
        double showdown_ratio = pow(REJECTION_PROB, candidate_fails) / pow(REJECTION_PROB, guess_fails);
        double alpha = min(1.0, prior_ratio * showdown_ratio);
        uniform_real_distribution<double> distrib(0, 1);
        if (distrib(rng) < alpha) {
            // accept
            guess = candidate;
            guess_fails = candidate_fails;
        }
        if (iter >= TOT_ITERS / 4) {
            visits[perm_to_mask(guess)]++;
        }
    }
    long long best = -1;
    for (auto it : visits) {
        if (best == -1) best = it.first;
        else if (it.second > visits[best]) best = it.first;
    }
    guess = mask_to_perm(best);
}

int main() {

    memset(HR, 0, sizeof(HR));
    FILE* hrfin = fopen("handranks.dat", "rb");
    fread(HR, sizeof(HR), 1, hrfin);
    fclose(hrfin);
    rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    int running_tot = 0;
    int running_max = 0;
    compute_prior_probs();
    for (int asdf = 0; asdf < 1000; asdf++) {
    actual_perm = permute_values();
    for (int i = 0; i < 13; i++) {
        //cout << actual_perm[i] << ' ';
    }
    //cout << endl;
    vector<int> guess = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    //cout << fixed << setprecision(10) << perm_prob(guess) << endl;
    showdown_rules.clear();
    for (int iter = 0; iter < 1000; iter++) {
        metropolis_hastings(guess);
        //for (int i = 0; i < 13; i++) {
          //  cout << guess[i] << ' ';
           
        //}
        // cout << ": " << guess_fails << endl;
        bool equal = true;
        for (int i = 0; i < 13; i++) {
            if (guess[i] != actual_perm[i]) equal = false;
        }
        if (equal) {
            running_tot +=  iter + 1;
            running_max = max(running_max, iter + 1);
            break;
        }
        generate_showdown();
    }
    cout << running_max << endl;
    cout << 1.0*running_tot/(asdf+1) << endl;
    }
     /*for (int i = 0; i < 13; i++) {
        cout << actual_perm[i] << ' ';
    }
    cout << endl;
    for (int i = 0; i < 13; i++) {
        cout << guess[i] << ' ';
        
    }
    cout << endl;
    guess_fails = count_fails(guess);
    cout << guess_fails << endl;*/
    return 0;
}